// SPDX-License-Identifier: Apache-2.0
#include "services.hpp"

#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/ObjectMapper/client.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/client.hpp>
#include <xyz/openbmc_project/State/BMC/client.hpp>

#include <cstdint>
#include <format>
#include <fstream>
#include <string>

namespace state_ns = sdbusplus::common::xyz::openbmc_project::state;
namespace bmc_ns = sdbusplus::common::xyz::openbmc_project::state::bmc;
namespace rules = sdbusplus::bus::match::rules;

namespace util
{

sdbusplus::async::task<std::string>
    getService(sdbusplus::async::context& ctx, const std::string& path,
               const std::string& interface)
{
    using ObjectMapper =
        sdbusplus::client::xyz::openbmc_project::ObjectMapper<>;

    auto mapper = ObjectMapper(ctx)
                      .service(ObjectMapper::default_service)
                      .path(ObjectMapper::instance_path);

    std::vector<std::string> interfaces{interface};

    auto object = co_await mapper.get_object(path, interfaces);
    co_return object.begin()->first;
}

} // namespace util

Services::Services(sdbusplus::async::context& ctx,
                   BMCStateCallback&& stateCallback,
                   RoleCallback&& roleCallback,
                   RedEnabledCallback&& redEnabledCallback,
                   FailoversPausedCallback&& failoversPausedCallback) :
    ctx(ctx), bmcStateCallback(std::move(stateCallback)),
    roleCallback(roleCallback), redEnabledCallback(redEnabledCallback),
    failoversPausedCallback(failoversPausedCallback)
{
    startup();
}

void Services::startup()
{
    ctx.spawn(watchBMCStateProp());
    ctx.spawn(watchRedundancyProps());
    ctx.spawn(watchBMCInterfaceAdded());
}

std::string Services::getFWVersionID(const std::filesystem::path& file)
{
    std::ifstream versionFile{file};
    std::string line;
    std::string keyPattern{"VERSION_ID="};
    std::string version;

    while (std::getline(versionFile, line))
    {
        // Handle either quotes or no quotes around the value
        if (line.substr(0, keyPattern.size()).find(keyPattern) !=
            std::string::npos)
        {
            // If the value isn't surrounded by quotes, then pos will be
            // npos + 1 = 0, and the 2nd arg to substr() will be npos
            // which means get the rest of the string.
            auto value = line.substr(keyPattern.size());
            std::size_t pos = value.find_first_of('"') + 1;
            version = value.substr(pos, value.find_last_of('"') - pos);
            break;
        }
    }

    if (version.empty())
    {
        throw std::runtime_error(
            "Unable to parse VERSION_ID out of " + file.string());
    }
    return version;
}

uint32_t Services::getBMCPosition()
{
    uint32_t bmcPosition = 0;

    // NOTE:  This a temporary solution for simulation until the
    // daemon that should be providing this information is in place.

    // Read it out of the bmc_position uboot environment variable
    std::string cmd{"/sbin/fw_printenv -n bmc_position"};

    // NOLINTBEGIN(cert-env33-c)
    FILE* pipe = popen(cmd.c_str(), "r");
    // NOLINTEND(cert-env33-c)
    if (!pipe)
    {
        throw std::runtime_error("Error calling popen to get bmc_position");
    }

    std::string output;
    std::array<char, 128> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        output.append(buffer.data());
    }

    int rc = pclose(pipe);
    if (WEXITSTATUS(rc) != 0)
    {
        throw std::runtime_error{std::format(
            "Error running cmd: {}, output = {}, rc = {}", cmd, output, rc)};
    }

    auto [_,
          ec] = std::from_chars(&*output.begin(), &*output.end(), bmcPosition);
    if (ec != std::errc())
    {
        throw std::runtime_error{
            std::format("Could not extract position from {}: rc {}", output,
                        std::to_underlying(ec))};
    }

    return bmcPosition;
}

sdbusplus::async::task<Services::BMCState> Services::getBMCState()
{
    using np = state_ns::BMC::namespace_path;
    std::string objectPath =
        sdbusplus::message::object_path(np::value) / np::bmc;

    auto service =
        co_await util::getService(ctx, objectPath, state_ns::BMC::interface);

    using StateMgr = sdbusplus::client::xyz::openbmc_project::state::BMC<>;
    auto stateMgr = StateMgr(ctx).service(service).path(objectPath);
    co_return co_await stateMgr.current_bmc_state();
}

sdbusplus::async::task<std::tuple<Services::Role, bool, bool>>
    Services::getRedundancyProps()
{
    auto service = co_await util::getService(
        ctx, bmc_ns::Redundancy::instance_path, bmc_ns::Redundancy::interface);

    auto rbmcMgr = sdbusplus::async::proxy()
                       .service(service)
                       .path(bmc_ns::Redundancy::instance_path)
                       .interface(bmc_ns::Redundancy::interface);

    auto props =
        co_await rbmcMgr
            .get_all_properties<bmc_ns::Redundancy::PropertiesVariant>(ctx);
    auto role = std::get<Role>(props.at("Role"));
    auto enabled = std::get<bool>(props.at("RedundancyEnabled"));
    auto paused = std::get<bool>(props.at("FailoversPaused"));
    co_return std::make_tuple(role, enabled, paused);
}

sdbusplus::async::task<> Services::watchBMCStateProp()
{
    using np = state_ns::BMC::namespace_path;
    std::string objectPath =
        sdbusplus::message::object_path(np::value) / np::bmc;

    sdbusplus::async::match match(
        ctx, rules::propertiesChanged(objectPath, state_ns::BMC::interface));

    using PropertyMap = std::map<std::string, state_ns::BMC::PropertiesVariant>;

    while (!ctx.stop_requested())
    {
        auto [iface,
              propertyMap] = co_await match.next<std::string, PropertyMap>();

        auto it = propertyMap.find("CurrentBMCState");
        if (it != propertyMap.end())
        {
            bmcStateCallback(std::get<BMCState>(it->second));
        }
    }
    co_return;
}

sdbusplus::async::task<> Services::watchRedundancyProps()
{
    sdbusplus::async::match match(
        ctx, rules::propertiesChanged(bmc_ns::Redundancy::instance_path,
                                      bmc_ns::Redundancy::interface));

    using PropertyMap =
        std::map<std::string, bmc_ns::Redundancy::PropertiesVariant>;

    while (!ctx.stop_requested())
    {
        auto [iface,
              propertyMap] = co_await match.next<std::string, PropertyMap>();

        auto it = propertyMap.find("Role");
        if (it != propertyMap.end())
        {
            roleCallback(std::get<Role>(it->second));
        }

        it = propertyMap.find("RedundancyEnabled");
        if (it != propertyMap.end())
        {
            redEnabledCallback(std::get<bool>(it->second));
        }

        it = propertyMap.find("FailoversPaused");
        if (it != propertyMap.end())
        {
            failoversPausedCallback(std::get<bool>(it->second));
        }
    }
    co_return;
}

sdbusplus::async::task<> Services::watchBMCInterfaceAdded()
{
    namespace rules = sdbusplus::bus::match::rules;
    using np = state_ns::BMC::namespace_path;
    std::string objectPath =
        sdbusplus::message::object_path(np::value) / np::bmc;

    // Note: BMC State and Redundancy are on the same object path
    sdbusplus::async::match match(ctx,
                                  rules::interfacesAddedAtPath(objectPath));

    using PropertiesVariant = std::variant<std::string, state_ns::BMC::BMCState,
                                           bmc_ns::Redundancy::Role, bool>;
    using PropertyMap = std::map<std::string, PropertiesVariant>;
    using InterfaceMap = std::map<std::string, PropertyMap>;

    while (!ctx.stop_requested())
    {
        auto [_, interfaces] =
            co_await match
                .next<sdbusplus::message::object_path, InterfaceMap>();

        auto it = interfaces.find(state_ns::BMC::interface);
        if (it != interfaces.end())
        {
            const auto& props = it->second;
            auto propIt = props.find("CurrentBMCState");
            if (propIt != props.end())
            {
                bmcStateCallback(std::get<BMCState>(propIt->second));
            }
        }

        it = interfaces.find(bmc_ns::Redundancy::interface);
        if (it != interfaces.end())
        {
            const auto& props = it->second;

            auto propIt = props.find("Role");
            if (propIt != props.end())
            {
                roleCallback(std::get<Role>(propIt->second));
            }

            propIt = props.find("RedundancyEnabled");
            if (propIt != props.end())
            {
                redEnabledCallback(std::get<bool>(propIt->second));
            }

            propIt = props.find("FailoversPaused");
            if (propIt != props.end())
            {
                failoversPausedCallback(std::get<bool>(propIt->second));
            }
        }
    }

    co_return;
}
