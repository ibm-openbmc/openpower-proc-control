// SPDX-License-Identifier: Apache-2.0
#include "local_cfam.hpp"

#include <CLI/CLI.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/common.hpp>

#include <format>
#include <iostream>
#include <string_view>

namespace redundancy_ns = sdbusplus::common::xyz::openbmc_project::state::bmc;
namespace state_ns = sdbusplus::common::xyz::openbmc_project::state;

std::string formatValue(BMCCFAM::Field field, uint32_t value)
{
    using enum BMCCFAM::Field;
    std::string result;

    switch (field)
    {
        case apiVersion:
        case bmcPosition:
        case heartbeat:
        case fwVersion:
            result = std::format("{:#x}", value);
            break;
        case redundancyEnabled:
        case failoversPaused:
        case provisioned:
        case siblingCommsOK:
            result = std::format("{}", value ? "true" : "false");
            break;
        case role:
            try
            {
                result = redundancy_ns::Redundancy::convertRoleToString(
                    static_cast<redundancy_ns::Redundancy::Role>(value));
            }
            catch (const std::invalid_argument&)
            {
                result = std::format("{:#x} (Unknown enum value)", value);
            }

            break;
        case bmcState:
            try
            {
                result = state_ns::BMC::convertBMCStateToString(
                    static_cast<state_ns::BMC::BMCState>(value));
            }
            catch (const std::invalid_argument&)
            {
                result = std::format("{:#x} (Unknown enum value)", value);
            }
            break;
        default:
            result = std::format("{:#x} (Unknown field {})", value,
                                 std::to_underlying(field));
            break;
    }
    return result;
}

void displayCFAMFields(std::string_view name, size_t link)
{
    SysFSImpl sysfs;
    LocalCFAM cfam{link, sysfs};

    std::cout << std::format("{} CFAM-S scratchpad fields\n", name);

    for (const auto& [field, meta] : LocalCFAM::cfamFields)
    {
        auto data = cfam.readField(field);
        if (!data.has_value())
        {
            std::cerr << std::format("Unable to read {} CFAM-S ({})\n", name,
                                     data.error());
            return;
        }

        std::cout << std::format("{:26} {}\n", meta.desc,
                                 formatValue(field, data.value()));
    }
}

void displayCFAMs()
{
    displayCFAMFields("Local BMC", 0);
    std::cout << '\n';
    displayCFAMFields("Sibling BMC", 1);
}

int main(int argc, char** argv)
{
    CLI::App app{"RBMC CFAM-S Tool"};
    bool display{};

    app.add_flag("-d", display, "Display CFAM-S Scratch Pad Register Contents");

    CLI11_PARSE(app, argc, argv);

    if (display)
    {
        displayCFAMs();
    }
    else
    {
        std::cout << app.help();
    }

    return 0;
}
