#pragma once

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <ranges>
#include <string>
namespace istep_table
{
// ---------------------------------------------------------------------
// Strongly-typed destination enum
// ---------------------------------------------------------------------
enum class IStepDestination : std::uint8_t
{
    EDBG_ISTEP_HOST = 0x0, // IStep is executed by host
    EDBG_ISTEP_SBE,        // IStep is executed by Self Boot Engine
    EDBG_ISTEP_BMC,        // IStep is executed by BMC
    EDBG_ISTEP_NOOP,       // This istep is NOOP
    EDBG_ISTEP_INVALID_DESTINATION
};

// ---------------------------------------------------------------------
// IPL Step structure
// ---------------------------------------------------------------------
struct IStep
{
    std::uint16_t major{};
    std::uint16_t minor{};
    std::string_view name{};
    IStepDestination dest{};
};

/****************************************************************************/
/* !!! --- THIS LIST MUST BE IN ORDER THAT THEY'RE CALLED IN AN IPL --- !!! */
/****************************************************************************/

/****************************************************************************/
/* NOTE: The istep is executed by the self boot engine/the host code/       */
/*       the attached bmc depending upon the destination type.              */
/*       Options:                                                           */
/*       EDBG_ISTEP_HOST - The istep is performed by the host               */
/*       EDBG_ISTEP_SBE  - The istep is performed by the self boot engine   */
/*       EDBG_ISTEP_BMC  - The istep is performed by the BMC                */
/*       EDBG_ISTEP_NOOP - The istep is No OP                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* Warning : The following constants are defined based on the values of this*/
/*           table. Any changes to this table requires examination of the   */
/*           values assigned to these constants.                            */
/*                                                                          */
/*       EDBG_FIRST_ISTEP_NUM   = 0                                         */
/*       EDBG_LAST_ISTEP_NUM    = 21                                        */
/*       EDBG_INVALID_ISTEP_NUM = 0xFFFF                                    */
/*       EDBG_INVALID_POSITION  = 0xFFFF                                    */
/****************************************************************************/
// major | minor |                          istep name   | destination   |
// number| number|                                       | |
inline constexpr auto ISteps = std::to_array<IStep>({
    {0, 1, "poweron", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 2, "startipl", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 3, "disableattns", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 4, "updatehwmodel", IStepDestination::EDBG_ISTEP_BMC},
    {0, 5, "alignment_check", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 6, "set_ref_clock", IStepDestination::EDBG_ISTEP_BMC},
    {0, 7, "proc_clock_test", IStepDestination::EDBG_ISTEP_BMC},
    {0, 8, "proc_prep_ipl", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 9, "edramrepair", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 10, "asset_protection", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 11, "proc_select_boot_prom", IStepDestination::EDBG_ISTEP_BMC},
    {0, 12, "hb_config_update", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 13, "sbe_config_update", IStepDestination::EDBG_ISTEP_BMC},
    {0, 14, "sbe_start", IStepDestination::EDBG_ISTEP_BMC},
    {0, 15, "startprd", IStepDestination::EDBG_ISTEP_NOOP},
    {0, 16, "proc_attn_listen", IStepDestination::EDBG_ISTEP_BMC},
    {1, 1, "proc_sbe_enable_seeprom", IStepDestination::EDBG_ISTEP_NOOP},
    {1, 2, "proc_sbe_pib_init", IStepDestination::EDBG_ISTEP_NOOP},
    {1, 3, "proc_sbe_measure", IStepDestination::EDBG_ISTEP_NOOP},
    {2, 1, "proc_sbe_ld_image", IStepDestination::EDBG_ISTEP_NOOP},
    {2, 2, "proc_sbe_attr_setup", IStepDestination::EDBG_ISTEP_SBE},
    {2, 3, "proc_sbe_tp_dpll_bypass", IStepDestination::EDBG_ISTEP_SBE},
    {2, 4, "proc_sbe_tp_chiplet_reset", IStepDestination::EDBG_ISTEP_SBE},
    {2, 5, "proc_sbe_tp_gptr_time_initf", IStepDestination::EDBG_ISTEP_SBE},
    {2, 6, "proc_sbe_dft_probe_setup_1", IStepDestination::EDBG_ISTEP_SBE},
    {2, 7, "proc_sbe_npll_initf", IStepDestination::EDBG_ISTEP_SBE},
    {2, 8, "proc_sbe_rcs_setup", IStepDestination::EDBG_ISTEP_SBE},
    {2, 9, "proc_sbe_tp_switch_gears", IStepDestination::EDBG_ISTEP_SBE},
    {2, 10, "proc_sbe_npll_setup", IStepDestination::EDBG_ISTEP_SBE},
    {2, 11, "proc_sbe_tp_repr_intf", IStepDestination::EDBG_ISTEP_SBE},
    {2, 12, "proc_sbe_setup_tp_abist", IStepDestination::EDBG_ISTEP_SBE},
    {2, 13, "proc_sbe_tp_arrayinit", IStepDestination::EDBG_ISTEP_SBE},
    {2, 14, "proc_sbe_tp_intf", IStepDestination::EDBG_ISTEP_SBE},
    {2, 15, "proc_sbe_dft_probesetup_2", IStepDestination::EDBG_ISTEP_SBE},
    {2, 16, "proc_sbe_tp_chiplet_init", IStepDestination::EDBG_ISTEP_SBE},
    {3, 1, "proc_sbe_chiplet_setup", IStepDestination::EDBG_ISTEP_SBE},
    {3, 2, "proc_sbe_chiplet_clk_config", IStepDestination::EDBG_ISTEP_SBE},
    {3, 3, "proc_sbe_chiplet_reset", IStepDestination::EDBG_ISTEP_SBE},
    {3, 4, "proc_sbe_gptr_time_initf", IStepDestination::EDBG_ISTEP_SBE},
    {3, 5, "proc_sbe_chiplet_pll_initf", IStepDestination::EDBG_ISTEP_SBE},
    {3, 6, "proc_sbe_chiplet_pll_setup", IStepDestination::EDBG_ISTEP_SBE},
    {3, 7, "proc_sbe_repr_intf", IStepDestination::EDBG_ISTEP_SBE},
    {3, 8, "proc_sbe_abist_setup", IStepDestination::EDBG_ISTEP_SBE},
    {3, 9, "proc_sbe_arrayinit", IStepDestination::EDBG_ISTEP_SBE},
    {3, 10, "proc_sbe_lbist", IStepDestination::EDBG_ISTEP_SBE},
    {3, 11, "proc_sbe_initf", IStepDestination::EDBG_ISTEP_SBE},
    {3, 12, "proc_sbe_startclocks", IStepDestination::EDBG_ISTEP_SBE},
    {3, 13, "proc_sbe_chiplet_init", IStepDestination::EDBG_ISTEP_SBE},
    {3, 14, "proc_sbe_chiplet_fir_init", IStepDestination::EDBG_ISTEP_SBE},
    {3, 15, "proc_sbe_dts_init", IStepDestination::EDBG_ISTEP_SBE},
    {3, 16, "proc_sbe_skew_adjust_setup", IStepDestination::EDBG_ISTEP_SBE},
    {3, 17, "proc_sbe_nest_enable_ridi", IStepDestination::EDBG_ISTEP_SBE},
    {3, 18, "proc_sbe_scominit", IStepDestination::EDBG_ISTEP_SBE},
    {3, 19, "proc_sbe_lpc", IStepDestination::EDBG_ISTEP_SBE},
    {3, 20, "proc_sbe_fabricinit", IStepDestination::EDBG_ISTEP_SBE},
    {3, 21, "proc_sbe_check_boot_proc", IStepDestination::EDBG_ISTEP_SBE},
    {3, 22, "proc_sbe_mcs_setup", IStepDestination::EDBG_ISTEP_SBE},
    {3, 23, "proc_sbe_select_ex", IStepDestination::EDBG_ISTEP_SBE},
    {4, 1, "proc_hcd_cache_poweron", IStepDestination::EDBG_ISTEP_SBE},
    {4, 2, "proc_hcd_cache_reset", IStepDestination::EDBG_ISTEP_SBE},
    {4, 3, "proc_hcd_cache_gptr_time_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 4, "proc_hcd_cache_repair_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 5, "proc_hcd_cache_arrayinit", IStepDestination::EDBG_ISTEP_SBE},
    {4, 6, "proc_hcd_cache_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 7, "proc_hcd_cache_startclocks", IStepDestination::EDBG_ISTEP_SBE},
    {4, 8, "proc_hcd_cache_scominit", IStepDestination::EDBG_ISTEP_SBE},
    {4, 9, "proc_hcd_cache_scom_customize", IStepDestination::EDBG_ISTEP_SBE},
    {4, 10, "proc_hcd_cache_ras_runtime_scom",
     IStepDestination::EDBG_ISTEP_SBE},
    {4, 11, "proc_hcd_core_poweron", IStepDestination::EDBG_ISTEP_SBE},
    {4, 12, "proc_hcd_core_reset", IStepDestination::EDBG_ISTEP_SBE},
    {4, 13, "proc_hcd_core_gptr_time_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 14, "proc_hcd_core_repair_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 15, "proc_hcd_core_arrayinit", IStepDestination::EDBG_ISTEP_SBE},
    {4, 16, "proc_hcd_core_initf", IStepDestination::EDBG_ISTEP_SBE},
    {4, 17, "proc_hcd_core_startclocks", IStepDestination::EDBG_ISTEP_SBE},
    {4, 18, "proc_hcd_core_scominit", IStepDestination::EDBG_ISTEP_SBE},
    {4, 19, "proc_hcd_core_scom_customize", IStepDestination::EDBG_ISTEP_SBE},
    {4, 20, "proc_hcd_core_ras_runtime_scom", IStepDestination::EDBG_ISTEP_SBE},
    {5, 1, "proc_sbe_load_bootloader", IStepDestination::EDBG_ISTEP_SBE},
    {5, 2, "proc_sbe_core_spr_setup", IStepDestination::EDBG_ISTEP_SBE},
    {5, 3, "proc_sbe_instruct_start", IStepDestination::EDBG_ISTEP_SBE},
    {6, 1, "host_bootloader", IStepDestination::EDBG_ISTEP_NOOP},
    {6, 2, "host_setup", IStepDestination::EDBG_ISTEP_NOOP},
    {6, 3, "host_istep_enable", IStepDestination::EDBG_ISTEP_NOOP},
    {6, 4, "host_init_fsi", IStepDestination::EDBG_ISTEP_HOST},
    {6, 5, "host_set_ipl_parms", IStepDestination::EDBG_ISTEP_HOST},
    {6, 6, "host_discover_targets", IStepDestination::EDBG_ISTEP_HOST},
    {6, 7, "host_update_primary_tpm", IStepDestination::EDBG_ISTEP_HOST},
    {6, 8, "host_gard", IStepDestination::EDBG_ISTEP_HOST},
    {6, 9, "host_voltage_config", IStepDestination::EDBG_ISTEP_HOST},
    {7, 1, "host_mss_attr_cleanup", IStepDestination::EDBG_ISTEP_HOST},
    {7, 2, "mss_volt", IStepDestination::EDBG_ISTEP_HOST},
    {7, 3, "mss_freq", IStepDestination::EDBG_ISTEP_HOST},
    {7, 4, "mss_eff_config", IStepDestination::EDBG_ISTEP_HOST},
    {7, 5, "mss_attr_update", IStepDestination::EDBG_ISTEP_HOST},
    {8, 1, "host_setup_sbe", IStepDestination::EDBG_ISTEP_HOST},
    {8, 2, "host_secondary_sbe_config", IStepDestination::EDBG_ISTEP_HOST},
    {8, 3, "host_cbs_start", IStepDestination::EDBG_ISTEP_HOST},
    {8, 4, "proc_check_secondary_sbe_seeprom_complete",
     IStepDestination::EDBG_ISTEP_HOST},
    {8, 5, "host_attnlisten_proc", IStepDestination::EDBG_ISTEP_HOST},
    {8, 6, "host_fbc_eff_config", IStepDestination::EDBG_ISTEP_HOST},
    {8, 7, "hosteff_config_links", IStepDestination::EDBG_ISTEP_HOST},
    {8, 8, "proc_attr_update", IStepDestination::EDBG_ISTEP_HOST},
    {8, 9, "proc_chiplet_fabric_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {8, 10, "host_set_voltages", IStepDestination::EDBG_ISTEP_HOST},
    {8, 11, "proc_io_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {8, 12, "proc_load_ioppe", IStepDestination::EDBG_ISTEP_HOST},
    {8, 13, "proc_init_ioppe", IStepDestination::EDBG_ISTEP_HOST},
    {8, 14, "proc_iohs_enable_ridi", IStepDestination::EDBG_ISTEP_HOST},
    {9, 1, "proc_io_dccal_done", IStepDestination::EDBG_ISTEP_HOST},
    {9, 2, "fabric_dl_pre_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {9, 3, "fabric_dl_setup_training", IStepDestination::EDBG_ISTEP_HOST},
    {9, 4, "proc_fabric_link_layer", IStepDestination::EDBG_ISTEP_HOST},
    {9, 5, "fabric_dl_post_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {9, 6, "proc_fabric_iovalid", IStepDestination::EDBG_ISTEP_HOST},
    {9, 7, "proc_fbc_eff_config_aggregate", IStepDestination::EDBG_ISTEP_HOST},
    {10, 1, "proc_build_smp", IStepDestination::EDBG_ISTEP_HOST},
    {10, 2, "host_sbe_update", IStepDestination::EDBG_ISTEP_HOST},
    {10, 3, "host_secureboot_lockdown", IStepDestination::EDBG_ISTEP_HOST},
    {10, 4, "proc_chiplet_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {10, 5, "proc_pau_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {10, 6, "proc_pcie_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {10, 7, "proc_scomoverride_chiplets", IStepDestination::EDBG_ISTEP_HOST},
    {10, 8, "proc_chiplet_enable_ridi", IStepDestination::EDBG_ISTEP_HOST},
    {10, 9, "host_rng_bist", IStepDestination::EDBG_ISTEP_HOST},
    {11, 1, "host_prd_hwreconfig", IStepDestination::EDBG_ISTEP_HOST},
    {11, 2, "host_set_mem_volt", IStepDestination::EDBG_ISTEP_HOST},
    {11, 3, "proc_ocmb_enable", IStepDestination::EDBG_ISTEP_HOST},
    {11, 4, "ocmb_check_for_ready", IStepDestination::EDBG_ISTEP_HOST},
    {12, 1, "mss_getecid", IStepDestination::EDBG_ISTEP_HOST},
    {12, 2, "omi_attr_update", IStepDestination::EDBG_ISTEP_HOST},
    {12, 3, "proc_omi_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {12, 4, "ocmb_omi_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {12, 5, "omi_pre_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {12, 6, "omi_setup", IStepDestination::EDBG_ISTEP_HOST},
    {12, 7, "omi_io_run_training", IStepDestination::EDBG_ISTEP_HOST},
    {12, 8, "omi_train_check", IStepDestination::EDBG_ISTEP_HOST},
    {12, 9, "omi_post_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {12, 10, "host_attnlisten_memb", IStepDestination::EDBG_ISTEP_HOST},
    {12, 11, "host_omi_init", IStepDestination::EDBG_ISTEP_HOST},
    {12, 12, "update_omi_firmaware", IStepDestination::EDBG_ISTEP_HOST},
    {13, 1, "mss_scominit", IStepDestination::EDBG_ISTEP_HOST},
    {13, 2, "mss_draminit", IStepDestination::EDBG_ISTEP_HOST},
    {13, 3, "mss_draminit_mc", IStepDestination::EDBG_ISTEP_HOST},
    {14, 1, "mss_memdiag", IStepDestination::EDBG_ISTEP_HOST},
    {14, 2, "mss_thermal_init", IStepDestination::EDBG_ISTEP_HOST},
    {14, 3, "proc_load_iop_xram", IStepDestination::EDBG_ISTEP_HOST},
    {14, 4, "proc_pcie_config", IStepDestination::EDBG_ISTEP_HOST},
    {14, 5, "proc_setup_mmio_bars", IStepDestination::EDBG_ISTEP_HOST},
    {14, 6, "host_secure_rng", IStepDestination::EDBG_ISTEP_HOST},
    {14, 7, "host_enable_memory_encryption", IStepDestination::EDBG_ISTEP_HOST},
    {14, 8, "proc_exit_cache_contained", IStepDestination::EDBG_ISTEP_HOST},
    {14, 9, "proc_htm_setup", IStepDestination::EDBG_ISTEP_HOST},
    {14, 10, "host_mpipl_service", IStepDestination::EDBG_ISTEP_HOST},
    {14, 11, "proc_psiinit", IStepDestination::EDBG_ISTEP_NOOP}, // cronus only
    {14, 12, "proc_bmc_pciinit",
     IStepDestination::EDBG_ISTEP_NOOP},                         // cronus only
    {15, 1, "host_build_stop_image", IStepDestination::EDBG_ISTEP_HOST},
    {15, 2, "proc_set_homer_bar", IStepDestination::EDBG_ISTEP_HOST},
    {15, 3, "host_establish_ec_chiplet", IStepDestination::EDBG_ISTEP_HOST},
    {15, 4, "host_start_stop_engine", IStepDestination::EDBG_ISTEP_HOST},
    {16, 1, "host_activate_boot_core", IStepDestination::EDBG_ISTEP_HOST},
    {16, 2, "host_activate_secondary_cores", IStepDestination::EDBG_ISTEP_HOST},
    {16, 3, "host_secure_rng_noop", IStepDestination::EDBG_ISTEP_HOST},
    {16, 4, "mss_scrub", IStepDestination::EDBG_ISTEP_HOST},
    {16, 5, "host_ipl_complete", IStepDestination::EDBG_ISTEP_HOST},
    {17, 1, "collect_drawers", IStepDestination::EDBG_ISTEP_NOOP},
    {17, 2, "proc_psiinit", IStepDestination::EDBG_ISTEP_NOOP},
    {17, 3, "psi_diag", IStepDestination::EDBG_ISTEP_NOOP},
    {18, 1, "sys_proc_eff_config_links", IStepDestination::EDBG_ISTEP_HOST},
    {18, 2, "sys_proc_chiplet_fabric_scominit",
     IStepDestination::EDBG_ISTEP_HOST},
    {18, 3, "sys_fabric_dl_pre_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {18, 4, "sys_fabric_dl_setup_training", IStepDestination::EDBG_ISTEP_HOST},
    {18, 5, "sys_proc_fabric_link_layer", IStepDestination::EDBG_ISTEP_HOST},
    {18, 6, "sys_fabric_dl_post_trainadv", IStepDestination::EDBG_ISTEP_HOST},
    {18, 7, "sys_proc_fabric_iovalid", IStepDestination::EDBG_ISTEP_HOST},
    {18, 8, "sys_proc_fbc_eff_config_aggregate",
     IStepDestination::EDBG_ISTEP_HOST},
    {18, 9, "proc_tod_setup", IStepDestination::EDBG_ISTEP_HOST},
    {18, 10, "proc_tod_init", IStepDestination::EDBG_ISTEP_HOST},
    {18, 11, "cec_ipl_complete", IStepDestination::EDBG_ISTEP_NOOP},
    {18, 12, "startprd_system", IStepDestination::EDBG_ISTEP_NOOP},
    {18, 13, "attn_listenall", IStepDestination::EDBG_ISTEP_NOOP},
    {19, 1, "prep_host", IStepDestination::EDBG_ISTEP_NOOP},
    {20, 1, "host_load_payload", IStepDestination::EDBG_ISTEP_HOST},
    {20, 2, "host_load_complete", IStepDestination::EDBG_ISTEP_HOST},
    {21, 1, "host_micro_update", IStepDestination::EDBG_ISTEP_HOST},
    {21, 2, "host_runtime_setup", IStepDestination::EDBG_ISTEP_HOST},
    {21, 3, "host_verify_hdat", IStepDestination::EDBG_ISTEP_HOST},
    {21, 4, "host_start_payload", IStepDestination::EDBG_ISTEP_HOST},
    {21, 5, "host_post_start_payload", IStepDestination::EDBG_ISTEP_NOOP},
    {21, 6, "switchbcu", IStepDestination::EDBG_ISTEP_NOOP},
    {21, 7, "completeipl", IStepDestination::EDBG_ISTEP_NOOP},
}); // end - array initialization

constexpr std::uint16_t EDBG_FIRST_ISTEP_NUM = 0;
constexpr std::uint16_t EDBG_LAST_ISTEP_NUM = 21;
constexpr std::uint16_t EDBG_INVALID_POSITION = 0xFFFF;
constexpr std::uint16_t EDBG_NUMBER_OF_ISTEPS = ISteps.size();
constexpr std::uint16_t EDBG_INVALID_ISTEP_NUM = 0xFFFF;

constexpr std::optional<std::string_view> getStepName(uint16_t major,
                                                      uint16_t minor)
{
    auto it = std::ranges::find_if(ISteps, [=](auto& s) {
        return s.major == major && s.minor == minor;
    });
    if (it != ISteps.end())
    {
        return it->name;
    }
    return std::nullopt;
}

constexpr bool isValid(uint8_t majorNum)
{
    return std::ranges::any_of(ISteps, [=](auto& s) {
        return s.major == majorNum;
    });
}

constexpr bool isValid(std::string_view name)
{
    return std::ranges::any_of(ISteps, [=](auto& s) { return s.name == name; });
}

constexpr IStepDestination getDestination(uint16_t majorNum, uint16_t minorNum)
{
    if (auto it = std::ranges::find_if(ISteps,
                                       [=](const IStep& step) {
                                           return step.major == majorNum &&
                                                  step.minor == minorNum;
                                       });
        it != ISteps.end())
    {
        return it->dest;
    }

    return IStepDestination::EDBG_ISTEP_INVALID_DESTINATION;
}
// ---------------------------------------------------------------------
// Get the position (index) of the last minor number for a given major
// ---------------------------------------------------------------------
constexpr std::uint16_t getPosLastMinorNumber(std::uint16_t majorNum)
{
    if (majorNum > EDBG_LAST_ISTEP_NUM)
    {
        return EDBG_INVALID_POSITION;
    }

    std::uint16_t position = EDBG_INVALID_POSITION;

    for (std::uint16_t row = 0; row < EDBG_NUMBER_OF_ISTEPS; ++row)
    {
        if (ISteps[row].major == majorNum)
        {
            // If this is the last row or next row has a greater major number,
            // current row marks the last minor number for this major.
            if ((row + 1 == EDBG_NUMBER_OF_ISTEPS) ||
                (ISteps[row + 1].major > majorNum))
            {
                position = row;
                break;
            }
        }
    }

    return position;
}

// ---------------------------------------------------------------------
// Get the position (index) of the first minor number for a given major
// ---------------------------------------------------------------------
constexpr std::uint16_t getPosFirstMinorNumber(std::uint16_t majorNum)
{
    if (majorNum > EDBG_LAST_ISTEP_NUM)
    {
        return EDBG_INVALID_POSITION;
    }

    for (std::uint16_t row = 0; row < EDBG_NUMBER_OF_ISTEPS; ++row)
    {
        if (ISteps[row].major == majorNum)
        {
            return row;
        }
    }

    return EDBG_INVALID_POSITION;
}

// ---------------------------------------------------------------------
// Get the major and minor number for the given istep name
// ---------------------------------------------------------------------
constexpr bool getIStepNumber(const std::string& istepName, uint16_t& major,
                              uint16_t& minor)
{
    using namespace istep_table;

    major = EDBG_INVALID_ISTEP_NUM;
    minor = EDBG_INVALID_ISTEP_NUM;

    for (const auto& step : ISteps)
    {
        if (istepName == step.name)
        {
            major = step.major;
            minor = step.minor;
            return true;
        }
    }
    return false;
}
// ---------------------------------------------------------------------
// Get the position (index) of the istep name
// ---------------------------------------------------------------------
constexpr std::uint16_t getPosition(std::string_view istepName)
{
    for (std::uint16_t row = 0; row < EDBG_NUMBER_OF_ISTEPS; ++row)
    {
        if (ISteps[row].name == istepName)
        {
            return row;
        }
    }
    return EDBG_INVALID_POSITION;
}

// ---------------------------------------------------------------------
// Get the minor number for a given istep table position
// ---------------------------------------------------------------------
constexpr std::uint16_t getIStepMinorNumber(std::uint16_t position)
{
    if (position < EDBG_NUMBER_OF_ISTEPS)
    {
        return ISteps[position].minor;
    }

    return EDBG_INVALID_ISTEP_NUM;
}
} // namespace istep_table
