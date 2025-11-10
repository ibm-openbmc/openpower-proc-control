#include <ecmdReturnCodes.H>
#include <targeting/predicates/predicateattrval.H>
#include <targeting/predicates/predicatepostfixexpr.H>
#include <targeting/target_service.H>
#include <targeting/xmltohb/attributeenums.H>
#include <targeting/xmltohb/attributetraits.H>

#include <ecmd_util.hpp>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace fs = std::filesystem;

namespace ecmd_util
{
constexpr auto GENESIS_BOOT_FILE = "/var/lib/phal/genesisboot";

TARGETING::TargetPtr getProcTargetByPos(uint32_t posValue)
{
    using namespace TARGETING;
    auto& ts = TargetService::instance();
    auto top = ts.getTopLevelTarget();

    // Build predicate to match processor type and position
    auto typeProc = std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC);
    auto pos = std::make_shared<PredicateAttrVal<ATTR_FAPI_POS>>(posValue);

    PredicatePostfixExpr procPred;
    procPred.push(typeProc).push(pos).And();

    // Find all matching processor targets
    auto&& targets = ts.getAssociated(top, AssociationType::childByPhysical,
                                      RecursionLevel::all, &procPred);

    // Return the matching target (or nullptr)
    if (targets.size() == 1)
    {
        return targets.front();
    }
    else
    {
        std::cerr << "processor target not found for FAPI_POS: " << posValue
                  << "\n";
        return nullptr;
    }
}

// Check if chassis is on/off
bool isChassisOn()
{
    constexpr std::string_view cmd = "obmcutil chassisstate 2>&1";
    std::array<char, 128> buffer{};
    std::string output;

    // Define unique_ptr with custom deleter for FILE*
    using FileCloser = int (*)(FILE*);

    auto pipePtr =
        std::unique_ptr<FILE, FileCloser>(popen(cmd.data(), "r"), pclose);
    if (!pipePtr)
    {
        std::cerr << "Failed to execute command: " << cmd << std::endl;
        return false;
    }

    // Read command output
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipePtr.get()))
    {
        output += buffer.data();
    }

    // Return true if the chassis is ON, false otherwise
    return output.contains("State.Chassis.PowerState.On");
}

// Start attention handler sevice
int startAttentionHandlerService()
{
    constexpr const char* cmd = "systemctl start attn_handler.service";
    int rc = std::system(cmd);
    if (rc != 0)
    {
        std::cerr << "system() call failed" << rc << std::endl;
        rc = -errno;
        return rc;
    }
    return rc;
}

int startAttnHandler()
{
    int rc = ECMD_SUCCESS;
    std::string start_attn_handler_service_cmd =
        "systemctl start attn_handler.service";

    // User the system handler service
    rc = system(start_attn_handler_service_cmd.c_str());
    if (rc != 0)
    {
        rc = -errno;
        return rc;
    }

    return rc;
}

int istepPowerOn()
{
    int rc = ECMD_SUCCESS;
    const std::string host_reboot_off_cmd = "obmcutil hostrebootoff";
    const std::string chassis_on_cmd = "obmcutil --wait chassison";
    const std::string mbox_reset_cmd = "/usr/sbin/mboxctl --reset";

    bool chassisOn = isChassisOn();
    uint16_t count = 0;

    if (!chassisOn)
    {
        // Disable host recovery (istep mode)
        rc = std::system(host_reboot_off_cmd.c_str());
        if (rc != 0)
        {
            rc = -errno;
            return rc;
        }

        // Trigger chassis on
        rc = std::system(chassis_on_cmd.c_str());
        if (rc != 0)
        {
            rc = -errno;
            return rc;
        }

        // Wait up to 3 minutes for chassis to turn ON
        do
        {
            chassisOn = isChassisOn();
            if (chassisOn)
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        } while (++count < 180);
    }

    // Check final state
    if (!chassisOn)
    {
        return -1; // chassis did not power on
    }

    // Trigger mbox reset
    rc = std::system(mbox_reset_cmd.c_str());
    if (rc != 0)
    {
        rc = -errno;
        return rc;
    }

    // Remove genesis boot file if it exists
    fs::path genesis_boot_file = GENESIS_BOOT_FILE;
    if (fs::exists(genesis_boot_file))
    {
        fs::remove(genesis_boot_file);
    }

    // Start attention handler service
    rc = startAttnHandler();
    if (rc != 0)
    {
        return rc;
    }

    return ECMD_SUCCESS;
}

// Set host state to running
int setHostStateToRunning()
{
    // TODO:use D-Bus method
    const std::string set_host_state_cmd =
        "busctl set-property "
        "xyz.openbmc_project.State.Host "
        "/xyz/openbmc_project/state/host0 "
        "xyz.openbmc_project.State.Host "
        "CurrentHostState s "
        "xyz.openbmc_project.State.Host.HostState.Running";

    // Try to set host state to Running
    int rc = std::system(set_host_state_cmd.c_str());
    if (rc != 0)
    {
        std::cout << "Failed to set host state to Running " << rc << std::endl;
        return rc;
    }

    // Best-effort: stop systemd targets; ignore errors
    rc = std::system("obmcutil stopofftargets");
    if (rc != 0)
    {
        std::cerr << "Failed to execute obmcutil stopofftargets, rc=" << rc
                  << std::endl;
        return rc;
    }
    return ECMD_SUCCESS;
}
} // namespace ecmd_util
