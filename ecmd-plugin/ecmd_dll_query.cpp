#include <ecmd_dll_capi.H>

#include <ecmdDataBuffer.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>

#include <ecmd_util.hpp>
#include <phosphor-logging/lg2.hpp>
extern "C"
{
#include <libpdbg.h>
}
#include <cstdint>
extern "C"
{

uint32_t queryConfigExist(const ecmdChipTarget& ecmdTarget,
                          ecmdQueryData& queryData, ecmdQueryDetail_t detail,
                          bool allowDisabled);

uint32_t queryConfigExistCages(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdCageData>& cageData,
                               ecmdQueryDetail_t detail, bool allowDisabled);

uint32_t queryConfigExistNodes(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdNodeData>& nodeData,
                               ecmdQueryDetail_t detail, bool allowDisabled);

uint32_t queryConfigExistSlots(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdSlotData>& slotData,
                               ecmdQueryDetail_t detail, bool allowDisabled);

uint32_t queryConfigExistChips(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdChipData>& chipData,
                               ecmdQueryDetail_t detail, bool allowDisabled);

uint32_t queryConfigExistChipUnits(
    const ecmdChipTarget& ecmdTarget, struct pdbg_target* target,
    std::string class_type, std::list<ecmdChipUnitData>& chipUnitData,
    ecmdQueryDetail_t detail, bool allowDisabled);

uint32_t dllQueryConfig(const ecmdChipTarget& target, ecmdQueryData& queryData,
                        ecmdQueryDetail_t detail)
{
    lg2::info("enter dllQueryConfig");
    return queryConfigExist(target, queryData, detail, false);
}

uint32_t dllQueryExist(const ecmdChipTarget& target, ecmdQueryData& queryData,
                       ecmdQueryDetail_t detail)
{
    return queryConfigExist(target, queryData, detail, true);
}

uint32_t queryConfigExist(const ecmdChipTarget& ecmdTarget,
                          ecmdQueryData& queryData, ecmdQueryDetail_t detail,
                          bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;

    // Need to clear out the queryConfig data before pushing stuff in
    // This is in case there is stale data in there
    queryData.cageData.clear();

    // From here, we will recursively work our way through all levels of
    // hierarchy in the target
    if (ecmdTarget.cageState == ECMD_TARGET_FIELD_VALID ||
        ecmdTarget.cageState == ECMD_TARGET_FIELD_WILDCARD)
    {
        rc = queryConfigExistCages(ecmdTarget, queryData.cageData, detail,
                                   allowDisabled);
        if (rc)
        {
            return rc;
        }
    }

    return rc;
}

uint32_t queryConfigExistCages(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdCageData>& o_cageData,
                               ecmdQueryDetail_t detail, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdCageData cageData;

    // We only have 1 cage for edbg, create that data
    // Then walk down through our nodes
    cageData.cageId = 0;

    // If the node states are set, see what nodes are in this cage
    if (ecmdTarget.nodeState == ECMD_TARGET_FIELD_VALID ||
        ecmdTarget.nodeState == ECMD_TARGET_FIELD_WILDCARD)
    {
        rc = queryConfigExistNodes(ecmdTarget, cageData.nodeData, detail,
                                   allowDisabled);
        if (rc)
        {
            return rc;
        }
    }

    // Save what we got from recursing down, or just being happy at this level
    o_cageData.push_back(cageData);

    return rc;
}

uint32_t queryConfigExistNodes(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdNodeData>& o_nodeData,
                               ecmdQueryDetail_t detail, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdNodeData nodeData;

    // We only have 1 node for edbg, create that data
    // Then walk down through our slots
    nodeData.nodeId = 0;

    // If the slot states are set, see what slots are in this node
    if (ecmdTarget.slotState == ECMD_TARGET_FIELD_VALID ||
        ecmdTarget.slotState == ECMD_TARGET_FIELD_WILDCARD)
    {
        rc = queryConfigExistSlots(ecmdTarget, nodeData.slotData, detail,
                                   allowDisabled);
        if (rc)
        {
            return rc;
        }
    }

    // Save what we got from recursing down, or just being happy at this level
    o_nodeData.push_back(nodeData);

    return rc;
}

uint32_t queryConfigExistSlots(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdSlotData>& liSlotData,
                               ecmdQueryDetail_t detail, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdSlotData slotData;

    // We only have 1 slot for edbg, create that data
    // Then walk down through our chips
    slotData.slotId = 0;

    // If the chipType states are set, see what chipTypes are in this slot
    if (ecmdTarget.chipTypeState == ECMD_TARGET_FIELD_VALID ||
        ecmdTarget.chipTypeState == ECMD_TARGET_FIELD_WILDCARD)
    {
        rc = queryConfigExistChips(ecmdTarget, slotData.chipData, detail,
                                   allowDisabled);
        if (rc)
        {
            return rc;
        }
    }

    // Save what we got from recursing down, or just being happy at this level
    liSlotData.push_back(slotData);

    return rc;
}

uint32_t queryConfigExistChips(const ecmdChipTarget& ecmdTarget,
                               std::list<ecmdChipData>& liChipData,
                               ecmdQueryDetail_t detail, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    // The display order is proc chip followed by memory chip
    // Within the proc/memory chip, sort them by position.
    // To keep in sync with lab and cronus users, order and display
    // the memory chips by FAPI_POS.
    const char* processor_class_name;
    std::string expectedChipType;
    /*
    if (pdbg_get_proc() == PDBG_PROC_PST)
    {
        expectedChipType = ECMD_CHIPT_PROC_HUB;
        processor_class_name = "hubchip";
    }
    else
    */
    {
        expectedChipType = ECMD_CHIPT_PROCESSOR;
        processor_class_name = "proc";
    }

    struct pdbg_target* ptarget;
    ecmdChipUnitData chipUnitData;
    pdbg_for_each_class_target(processor_class_name, ptarget)
    {
        // If posState is set to VALID, check that our values match
        // If posState is set to WILDCARD, we don't care
        if ((pdbg_target_index(ptarget) <= 0) ||
            ((ecmdTarget.posState == ECMD_TARGET_FIELD_VALID) &&
             (pdbg_target_index(ptarget) != ecmdTarget.pos)))
        {
            continue;
        }
        // if chip type is not pu or ph or wildcard then, skip adding.
        if (ecmdTarget.chipType != expectedChipType &&
            (ecmdTarget.chipTypeState != ECMD_TARGET_FIELD_WILDCARD))
        {
            continue;
        }

        // We passed our checks, load up our data
        ecmdChipData chipData;
        /*
        if (pdbg_get_proc() == PDBG_PROC_PST)
        {
            chipData.chipType = ECMD_CHIPT_PROC_HUB;
        }
        else
        */
        {
            chipData.chipType = ECMD_CHIPT_PROCESSOR;
        }
        chipData.chipShortType = ecmdTarget.chipUnitType;
        chipData.pos = pdbg_target_index(ptarget);

        // If the chipUnitType states are set, see what chipUnitTypes are in
        // this chipType
        if (ecmdTarget.chipUnitTypeState == ECMD_TARGET_FIELD_VALID ||
            ecmdTarget.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD)
        {
            // Look for chipunits
            rc = queryConfigExistChipUnits(
                ecmdTarget, ptarget, chipData.chipType, chipData.chipUnitData,
                detail, allowDisabled);
            if (rc)
            {
                return rc;
            }
        }
        // Save what we got from recursing down, or just being happy at this
        // level
        liChipData.push_back(chipData);
    }
    return rc;
}

uint32_t addChipUnits(const ecmdChipTarget& ecmdTarget,
                      struct pdbg_target* ptarget, std::string class_name,
                      std::list<ecmdChipUnitData>& liChipUnitData,
                      ecmdQueryDetail_t /*detail*/, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdChipUnitData chipUnitData;
    std::string cuString;
    ecmdChipTarget o_target;

    rc = ecmd_util::p10x_convertPDBGClassString_to_CUString(
        class_name, cuString);
    struct pdbg_target* target;
    pdbg_for_each_target(class_name.c_str(), ptarget, target)
    {
        // If posState is set to VALID, check that our values match
        // If posState is set to WILDCARD, we don't care
        if ((ecmdTarget.chipUnitNumState == ECMD_TARGET_FIELD_VALID) &&
            (pdbg_target_index(target) != ecmdTarget.chipUnitNum))
        {
            continue;
        }
        if ((ecmdTarget.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
            (cuString != ecmdTarget.chipUnitType))
        {
            continue;
        }
        // Check for the next target, if the current one is
        // not functional and we do not allow disabled
        if (!allowDisabled && !ecmd_util::isFunctionalTarget(target))
        {
            continue;
        }

        uint32_t chipUnitNum = ecmd_util::getChipUnitPos(target);
        chipUnitData.threadData.clear();
        chipUnitData.chipUnitType = cuString;
        chipUnitData.chipUnitNum = chipUnitNum;
        chipUnitData.numThreads = 4;
        liChipUnitData.push_back(chipUnitData);
    }
    return rc;
}

uint32_t queryConfigExistChipUnits(
    const ecmdChipTarget& ecmdTarget, struct pdbg_target* ptarget,
    std::string class_type, std::list<ecmdChipUnitData>& liChipUnitData,
    ecmdQueryDetail_t detail, bool allowDisabled)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdChipUnitData chipUnitData;
    uint32_t l_index;
    // TODO:PST - In the future this needs to have chipunit table for p12
    // and traverse through that list
    if (pdbg_get_proc() ==
        PDBG_PROC_P10 /*|| pdbg_get_proc() == PDBG_PROC_PST*/)
    {
        if (class_type ==
            ECMD_CHIPT_PROCESSOR /*||class_type == ECMD_CHIPT_PROC_HUB*/)
        {
            for (l_index = 0; l_index < (sizeof(ecmd_util::ChipUnitTable) /
                                         sizeof(ecmd_util::p10_chipUnit_t));
                 l_index++)
            {
                // If pdbg class type is pib , don't add the chip unit to the
                // queryConfigExistChipUnits
                if (ecmd_util::ChipUnitTable[l_index].pdbgClassType != "pib")
                {
                    rc = addChipUnits(
                        ecmdTarget, ptarget,
                        ecmd_util::ChipUnitTable[l_index].pdbgClassType,
                        liChipUnitData, detail, allowDisabled);
                    if (rc)
                    {
                        lg2::error(
                            "Failed to add chip unit:{UNIT}", "UNIT",
                            ecmd_util::ChipUnitTable[l_index].pdbgClassType);
                    }
                }
            }
        }
    }
    return rc;
}

uint32_t dllRelatedTargets(const ecmdChipTarget&, const std::string,
                           std::list<ecmdChipTarget>&, const ecmdLoopMode_t)
{
    lg2::error("dllRelatedTargets is not implemented");
    return ECMD_FUNCTION_NOT_SUPPORTED;
}
uint32_t dllQueryFileLocation(const ecmdChipTarget&, ecmdFileType_t,
                              std::list<ecmdFileLocation>&, std::string&)
{
    // TODO: check and implement the method from edbgEcmdDll.C
    return ECMD_SUCCESS;
}

uint32_t dllQueryConnectedTargets(const ecmdChipTarget&, const char*,
                                  std::list<ecmdConnectionData>&)
{
    lg2::error("dllQueryConnectedTargets is not implemented");
    return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetChipData(const ecmdChipTarget& ecmdTarget, ecmdChipData& data)
{
    uint32_t rc = ECMD_SUCCESS;

    if (pdbg_get_proc() ==
        PDBG_PROC_P10 /*|| pdbg_get_proc() == PDBG_PROC_PST*/)
    {
        // chipEC is 0 if we fail to read via attribute
        uint8_t chipEC = 0;
        const char* processor_class_name;
        std::string expectedChipType;
        /*
        if (pdbg_get_proc() == PDBG_PROC_PST)
        {
            expectedChipType = ECMD_CHIPT_PROC_HUB;
            processor_class_name = "hubchip";
        }
        else
        */
        {
            expectedChipType = ECMD_CHIPT_PROCESSOR;
            processor_class_name = "proc";
        }

        struct pdbg_target* ptarget;
        pdbg_for_each_class_target(processor_class_name, ptarget)
        {
            uint32_t index = pdbg_target_index(ptarget);

            // If posState is set to VALID, check that our values match
            // If posState is set to WILDCARD, we don't care
            if ((index <= 0) ||
                ((ecmdTarget.posState == ECMD_TARGET_FIELD_VALID) &&
                 (index != ecmdTarget.pos)))
            {
                continue;
            }
            // We passed our checks, load up our data
            ecmdChipData chipData;
            chipData.chipUnitData.clear();
            chipData.chipType = ecmd_util::getChipType();
            chipData.chipShortType = ecmd_util::getChipType();
            chipData.chipCommonType = expectedChipType;
            chipData.pos = index;

            // TODO:PST - do we need to get ATTR_EC for PST system?
            // Currently it is not part of the device tree. Need to add
            // TARGETING::ATTR_EC
            // if (pdbg_get_proc() != PDBG_PROC_PST)
            {
                if (!pdbg_target_get_attribute(ptarget, "ATTR_EC", 1, 1,
                                               &chipEC))
                {
                    lg2::error("ATTR_EC Attribute get failed");
                    return ECMD_FAILURE;
                }
            }

            chipData.chipEc = chipEC;

            // Will use chip EC we got from device tree
            chipData.simModelEc = chipEC;

            // For FSI, the interface type is CFAM.
            chipData.interfaceType = ECMD_INTERFACE_CFAM;

            // FSI is hardcoded.
            chipData.chipFlags = ECMD_CHIPFLAG_FSI;

            // copy data
            data = chipData;
        }
    }
    else
    {
        lg2::error("Function not supported");
        return ECMD_FUNCTION_NOT_SUPPORTED;
    }
    return rc;
}

uint32_t dllQueryMode(const ecmdChipTarget&, std::string&, std::string&)
{
    lg2::error("dllQueryMode is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllQueryScom(const ecmdChipTarget&, std::list<ecmdScomData>&, uint64_t,
                      ecmdQueryDetail_t)
{
    lg2::error("dllQueryScom is not implemented");
    return ECMD_SUCCESS;
}
} //extern C
