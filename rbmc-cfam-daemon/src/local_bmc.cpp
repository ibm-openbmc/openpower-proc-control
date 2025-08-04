// SPDX-License-Identifier: Apache-2.0
#include "local_bmc.hpp"

#include <openssl/evp.h>

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/async.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/client.hpp>

constexpr uint8_t apiVersion{0x01};

sdbusplus::async::task<> LocalBMC::start()
{
    writeApiVersion();
    writeFWVersion();
    writeBMCPosition();
    writeProvisioned();
    writeSiblingCommsNotOK();

    services = std::make_unique<Services>(
        ctx, [this](auto state) { bmcStateChanged(state); },
        [this](auto role) { roleChanged(role); },
        [this](auto enabled) { redEnabledChanged(enabled); },
        [this](auto allowed) { failoversAllowedChanged(allowed); },
        [this](auto imminent) { failoverImminentChanged(imminent); },
        [this](auto inProgress) { failoverInProgressChanged(inProgress); });

    co_await writeRedundancyProps();
    co_await writeBMCState();

    ctx.spawn(watchHeartBeat());

    co_return;
}

void LocalBMC::waitForCFAM()
{
    constexpr int timeout = 25;
    int seconds = 0;

    while (!cfam.isReady() && (seconds < timeout))
    {
        seconds++;
        if (seconds == 1)
        {
            lg2::info("Waiting for local CFAM to be ready");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!cfam.isReady())
    {
        // TODO: Create an error log calling out this card.
        lg2::error("Local CFAM is not accessible");
        throw std::runtime_error("Local CFAM is not accessible");
    }
    else if (seconds != 0)
    {
        lg2::info("Done waiting for local CFAM to become ready");
    }
}

sdbusplus::async::task<> LocalBMC::watchHeartBeat()
{
    using namespace sdbusplus::bus::match;
    using Redundancy =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy;

    sdbusplus::async::match match(ctx, rules::interface(Redundancy::interface) +
                                           rules::member("Heartbeat"));

    while (!ctx.stop_requested())
    {
        co_await match.next<>();

        cfam.incHeartbeat();
    }

    co_return;
}

void LocalBMC::writeApiVersion()
{
    cfam.writeApiVersion(apiVersion);
}

void LocalBMC::writeFWVersion()
{
    const std::filesystem::path osRelease{"/etc/os-release"};

    auto versionID = services->getFWVersionID(osRelease);

    // Write the SHA512 hash to the CFAM
    using EVP_MD_CTX_Ptr =
        std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>;

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    EVP_MD_CTX_Ptr context(EVP_MD_CTX_new(), &::EVP_MD_CTX_free);

    EVP_DigestInit(context.get(), EVP_sha512());
    EVP_DigestUpdate(context.get(), versionID.c_str(),
                     strlen(versionID.c_str()));
    EVP_DigestFinal(context.get(), digest.data(), nullptr);

    auto versionStr = std::format("{:02X}{:02X}{:02X}{:02X}", digest[0],
                                  digest[1], digest[2], digest[3]);

    cfam.writeFWVersion(std::stoul(versionStr, nullptr, 16));
}

void LocalBMC::writeBMCPosition()
{
    cfam.writeBMCPosition(services->getBMCPosition());
}

void LocalBMC::writeSiblingCommsNotOK()
{
    cfam.writeSiblingCommsOK(false);
    siblingOK = false;
}

void LocalBMC::writeProvisioned()
{
    // TODO: Get this value from somewhere.
    cfam.writeProvisioned(true);
}

void LocalBMC::bmcStateChanged(BMCState state)
{
    lg2::info("Local BMC state changed to {STATE}", "STATE", state);
    cfam.writeBMCState(state);
}

void LocalBMC::roleChanged(Role role)
{
    lg2::info("Local Role changed to {ROLE}", "ROLE", role);
    cfam.writeRole(role);
}
void LocalBMC::redEnabledChanged(bool enabled)
{
    lg2::info("Local Redundancy enabled changed to {ENABLED}", "ENABLED",
              enabled);
    cfam.writeRedundancyEnabled(enabled);
}

void LocalBMC::failoversAllowedChanged(bool allowed)
{
    lg2::info("Local Failovers Allowed changed to {ALLOWED}", "ALLOWED",
              allowed);
    cfam.writeFailoversAllowed(allowed);
}

void LocalBMC::failoverImminentChanged(bool imminent)
{
    lg2::info("Local Failover Imminent changed to {IMMINENT}", "IMMINENT",
              imminent);
    cfam.writeFailoverImminent(imminent);
}

void LocalBMC::failoverInProgressChanged(bool inProgress)
{
    lg2::info("Local Failover In Progress changed to {INPROGRESS}",
              "INPROGRESS", inProgress);
    cfam.writeFailoverInProgress(inProgress);
}

sdbusplus::async::task<> LocalBMC::writeRedundancyProps()
{
    try
    {
        auto props = co_await services->getRedundancyProps();
        lg2::debug(
            "Initial values of local role, redEnabled, fo allowed, fo imminent, fo in "
            "progress: {ROLE} {ENABLED} {ALLOWED} {IMMINENT} {IN_PROGRESS}",
            "ROLE", props.role, "ENABLED", props.redundancy_enabled, "ALLOWED",
            props.failovers_allowed, "IMMINENT", props.failover_imminent,
            "IN_PROGRESS", props.failover_in_progress);

        cfam.writeRole(props.role);
        cfam.writeRedundancyEnabled(props.redundancy_enabled);
        cfam.writeFailoversAllowed(props.failovers_allowed);
        cfam.writeFailoverImminent(props.failover_imminent);
        cfam.writeFailoverInProgress(props.failover_in_progress);
    }
    catch (const sdbusplus::exception_t& e)
    {
        lg2::info("Redundancy interface not on D-Bus: {ERROR}", "ERROR", e);
    }

    co_return;
}

sdbusplus::async::task<> LocalBMC::writeBMCState()
{
    try
    {
        auto state = co_await services->getBMCState();
        lg2::info("Initial value of local BMC state is {STATE}", "STATE",
                  state);
        cfam.writeBMCState(state);
    }
    catch (const sdbusplus::exception_t& e)
    {
        lg2::info("Local BMC state not on D-Bus: {ERROR}", "ERROR", e);
    }

    co_return;
}
void LocalBMC::setSiblingCommsOK(bool ok)
{
    if (ok != siblingOK)
    {
        cfam.writeSiblingCommsOK(ok);
        siblingOK = ok;
    }
}
