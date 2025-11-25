#include <ecmdReturnCodes.H>

#include <ecmd_util.hpp>

#include <thread>
extern "C"
{
#include <libpdbg.h>
}
#include <phosphor-logging/lg2.hpp>

namespace fs = std::filesystem;

namespace ecmd_util
{
constexpr auto GENESIS_BOOT_FILE = "/var/lib/phal/genesisboot";

bool isFunctionalTarget(struct pdbg_target* target)
{
    uint8_t buf[5];
    bool isFunc = false;

    if (!pdbg_target_get_attribute_packed(target, "ATTR_HWAS_STATE", "41", 1,
                                          buf))
    {
        lg2::error("ATTR_HWAS_STATE Attribute get failed");
        isFunc = false;
    }

    // isFuntional bit is stored in 4th byte and bit 3 position in HWAS_STATE
    if (buf[4] & 0x20)
    {
        isFunc = true;
    }

    return isFunc;
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
        lg2::error("Failed to execute command {CMD}", "CMD", cmd);
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

int startAttnHandler()
{
    int rc = ECMD_SUCCESS;
    std::string cmd = "systemctl start attn_handler.service";

    // User the system handler service
    rc = system(cmd.c_str());
    if (rc != 0)
    {
        lg2::error("Failed to execute system command {CMD}", "CMD", cmd);
        rc = -errno;
        return rc;
    }

    return rc;
}

int istepPowerOn()
{
    lg2::info("enter istepPowerOn");
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
    const std::string cmd =
        "busctl set-property "
        "xyz.openbmc_project.State.Host "
        "/xyz/openbmc_project/state/host0 "
        "xyz.openbmc_project.State.Host "
        "CurrentHostState s "
        "xyz.openbmc_project.State.Host.HostState.Running";

    // Try to set host state to Running
    int rc = std::system(cmd.c_str());
    if (rc != 0)
    {
        lg2::error("Failed to set host state running {CMD}", "CMD", cmd);
        return rc;
    }

    // Best-effort: stop systemd targets; ignore errors
    const std::string stopCmd = "obmcutil stopofftargets";
    rc = std::system(stopCmd.c_str());
    if (rc != 0)
    {
        lg2::error("Failed to execute command {CMD}", "CMD", stopCmd);
        return rc;
    }
    return ECMD_SUCCESS;
}
} // namespace ecmd_util
