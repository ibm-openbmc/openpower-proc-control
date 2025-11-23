#include <ecmdReturnCodes.H>

#include <ecmd_util.hpp>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
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

struct pdbg_target* get_fsi_target(uint32_t pos)
{
    struct pdbg_target* fsi = nullptr;

    pdbg_for_each_class_target("fsi", fsi)
    {
        if (pdbg_target_index(fsi) == pos)
        {
            return fsi;
        }
    }
    lg2::error("failed to find the fsi target");
    return nullptr;
}

// Check if chassis is on/off
bool isChassisOn()
{
    lg2::info("enter isChassisOn");
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
    lg2::info("enter startAttnHandler");
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

    lg2::info("invoke mbox reset");
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
    lg2::info("invoke startAttnHandler");
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

// convert chipunit string to pdbg class type, as pdbg does not accept ecmd
// strings
uint32_t p10x_convertCUString_to_pdbgClassString(std::string cuString,
                                                 std::string& o_pdbgClassType)
{
    uint32_t rc = ECMD_SUCCESS;
    uint32_t l_index;

    for (l_index = 0;
         l_index < (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t)); l_index++)
    {
        // Looking for input chip unit type in table
        if (cuString == ChipUnitTable[l_index].chipUnitType)
            break;
    }
    // Can't find cuString in table
    if (l_index >= (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t)))
    {
        lg2::error("Unknown chip unit {CHIP}", "CHIP", cuString);
        return ECMD_FAILURE;
    }

    o_pdbgClassType = ChipUnitTable[l_index].pdbgClassType;
    return rc;
}

uint8_t getChipUnitPos(pdbg_target* target)
{
    uint8_t chipUnitPos = -1; // chip unit position

    // size: uint8 => 1, uint16 => 2. uint32 => 4 uint64=> 8
    // typedef uint8_t ATTR_CHIP_UNIT_POS_Type;
    if (!pdbg_target_get_attribute(target, "ATTR_CHIP_UNIT_POS", 1, 1,
                                   &chipUnitPos))
    {
        lg2::error("ATTR_CHIP_UNIT_POS Attribute get failed");
    }

    return chipUnitPos;
}

std::string getChipType()
{
    std::string chipType;

    // determine the chip type
    switch (pdbg_get_proc())
    {
        case PDBG_PROC_P9:
            chipType = "p9";
            break;

        case PDBG_PROC_P10:
            chipType = "p10";
            break;

        /*
        case PDBG_PROC_PST:
          chipType = "pst";
          break;
        */
        default:
            chipType = "Unknown";
            break;
    }
    return chipType;
}

uint32_t p10x_convertPDBGClassString_to_CUString(std::string_view pdbgClassType,
                                                 std::string& chipUnitType)
{
    // Search for matching pdbgClassType
    const auto it =
        std::find_if(std::begin(ChipUnitTable), std::end(ChipUnitTable),
                     [&](const auto& entry) {
                         return pdbgClassType == entry.pdbgClassType;
                     });

    if (it == std::end(ChipUnitTable))
    {
        lg2::error("Unknown pdbg class unit {UNIT}", "UNIT",
                   std::string(pdbgClassType));
        return ECMD_FAILURE;
    }

    chipUnitType = it->chipUnitType;
    return ECMD_SUCCESS;
}

} // namespace ecmd_util
