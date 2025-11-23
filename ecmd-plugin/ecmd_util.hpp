#pragma once
extern "C"
{
#include <libpdbg.h>
}
#include <p10_cu.H>

#include <string>

namespace ecmd_util
{
struct pdbg_target* get_fsi_target(uint32_t pos);

bool isChassisOn();

int startAttnHandler();

// Trigger obmcutil hostrebootoff->chassison->wait for chassison()
int istepPowerOn();

// Set host state to running
int setHostStateToRunning();

struct p10_chipUnit_t
{
    p10ChipUnits_t ekbChipUnit;
    std::string chipUnitType;
    std::string pdbgClassType;
};

// mapping table for ekb, ecmd and pdbg
const p10_chipUnit_t ChipUnitTable[] = {
    {P10_NO_CU, "", "pib"}, // chip unit not passed. Then,use pib.
    {PU_C_CHIPUNIT, "c", "core"},
    {PU_EQ_CHIPUNIT, "eq", "eq"},
    {PU_PEC_CHIPUNIT, "pec", "pec"},
    {PU_PHB_CHIPUNIT, "phb", "phb"},
    {PU_MI_CHIPUNIT, "mi", "mi"},
    {PU_MCC_CHIPUNIT, "mcc", "mcc"},
    {PU_OMIC_CHIPUNIT, "omic", "omic"},
    {PU_OMI_CHIPUNIT, "omi", "omi"},
    {PU_PERV_CHIPUNIT, "perv", "chiplet"},
    {PU_MC_CHIPUNIT, "mc", "mc"},
    {PU_NMMU_CHIPUNIT, "nmmu", "nmmu"},
    {PU_IOHS_CHIPUNIT, "iohs", "iohs"},
    {PU_PAU_CHIPUNIT, "pau", "pau"},
    {PU_PAUC_CHIPUNIT, "pauc", "pauc"},
    // SMPGROUP is a target type that firmware has had for a while to represent
    // the half-links below a SMP link, e.g. each SMP cable is a SMPGROUP.
    // In P10, we have added a new IOLINK taget to fapi2 to represent same thing
    // in cronus.
    {P10_NO_CU, "smpgroup",
     "smpgroup"}, // Not a scommable target. leaving it NO_CU
};

uint32_t p10x_convertPDBGClassString_to_CUString(std::string_view pdbgClassType,
                                                 std::string& chipUnitType);

uint8_t getChipUnitPos(pdbg_target* target);

bool isFunctionalTarget(struct pdbg_target* target);

std::string getChipType();
} // namespace ecmd_util
