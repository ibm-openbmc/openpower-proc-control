//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <ecmd_dll_capi.H>
#include <ecmdSharedUtils.H>
#include <ecmdStructs.H>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <unistd.h>
extern "C"
{
#include <libpdbg.h>
}
#include <phosphor-logging/lg2.hpp>

extern "C"
{

struct ecmdUserInfo {
  std::string cage;
  std::string node;
  std::string slot;
  std::string pos;
  std::string chipUnitNum;
  std::string thread;
} ecmdUserArgs;

/* This is used by the ecmdPush/PopCommandArgs functions */
std::list<ecmdUserInfo> ecmdArgsStack;
//----------------------------------------------------------------------
//  Internal Function Prototypes
//----------------------------------------------------------------------
/* @brief Returns true if curPos is not in userArgs */
uint8_t removeCurrentElement(int curPos, std::string userArgs);

/* @brief Returns true if all chars of str are decimal numbers */
bool isValidTargetString(std::string& str);

/* @brief used by TargetConfigured/TargetExist functions */
bool queryTargetConfigExist(const ecmdChipTarget& target,
                            const ecmdQueryData* i_queryData,
                            bool i_existQuery);

/* @brief used by QuerySelected/QuerySelectedExist functions */
uint32_t queryConfigExistSelected(ecmdChipTarget& target,
                                  ecmdQueryData& queryData,
                                  ecmdLoopType_t looptype, bool existMode);

/* @brief used by dllCommonCommandArgs when ":" found, sets ecmdUserArgs */
uint32_t ecmdTargetExpansion(std::string arg_string, const char* input_target);

//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------
/* @brief This is a global var set by -quiet */
uint32_t ecmdGlobal_quiet = 0;

/* @brief This is a global error var set by -quieterror */
uint32_t ecmdGlobal_quietError = 1;

/* @brief This is a global var set by -coe */
uint32_t ecmdGlobal_continueOnError = 0;

/* @brief This is a global var to determine how the looper runs */
/* ECMD_CONFIG_LOOP, the default */
/* ECMD_EXIST_LOOP, turned on by -exist */
uint32_t ecmdGlobal_looperMode = ECMD_CONFIG_LOOP;

/* @brief This is a global var set by ecmdSetCurrentCmdline() */
std::string ecmdGlobal_currentCmdline = "";

/* @brief This is a global var set by ecmdMain.C to say we are in a cmdline
 * program */
uint32_t ecmdGlobal_cmdLineMode = 0;

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------
uint32_t dllLoadDll(const char* clientVersion, uint32_t)
{
    uint32_t rc;

    /* First off let's check our version */
    /* Let's found our '.' char because we only fail if the Major number changes
     */
    // lint -e613 clientVersion is ECMD_CAPI_VERSION so it can't be NULL @02a
    uint32_t majorlength =
        (uint32_t)(strchr(clientVersion, '.') - clientVersion);

    if (strncmp(clientVersion, ECMD_CAPI_VERSION, majorlength))
    {
        fprintf(
            stderr,
            "**** FATAL : eCMD DLL and your client Major version numbers don't match, they are not compatible\n");
        fprintf(stderr,
                "**** FATAL : Client Version : %s   : DLL Version : %s\n",
                clientVersion, ECMD_CAPI_VERSION);

        if (atoi(clientVersion) < atoi(ECMD_CAPI_VERSION))
        {
            fprintf(
                stderr,
                "**** FATAL : Your client is older than the eCMD Dll Plugin you are running\n");
            fprintf(
                stderr,
                "**** FATAL : You must grab the latest client libraries and rebuild your client to continue\n");
        }
        else
        {
            fprintf(
                stderr,
                "**** FATAL : It appears your client is newer than the eCMD Dll Plugin you are running\n");
            fprintf(
                stderr,
                "**** FATAL : Contact the eCMD team to have the Plugin rebuilt to match your client\n");
            fprintf(
                stderr,
                "**** FATAL : Or get ahold of down level client libraries and rebuild your client to match\n");
        }

        return ECMD_FATAL_FAILURE;
    }

    /* Now we are going to check the version of the shared lib we loaded */
    if (strncmp(ecmdGetSharedLibVersion().c_str(), ECMD_CAPI_VERSION,
                majorlength))
    {
        fprintf(
            stderr,
            "**** FATAL : eCMD Shared Library and your Plugin Major version numbers don't match, they are not compatible\n");
        fprintf(
            stderr,
            "**** FATAL : Shared Library Version : %s   : DLL Version : %s\n",
            ecmdGetSharedLibVersion().c_str(), ECMD_CAPI_VERSION);

        if (atoi(ecmdGetSharedLibVersion().c_str()) < atoi(ECMD_CAPI_VERSION))
        {
            fprintf(
                stderr,
                "**** FATAL : Your shared library is older than the eCMD Dll Plugin you are running\n");
            fprintf(
                stderr,
                "**** FATAL : You must grab the latest library to continue\n");
        }
        else
        {
            fprintf(
                stderr,
                "**** FATAL : It appears your shared library is newer than the eCMD Dll Plugin you are running\n");
            fprintf(
                stderr,
                "**** FATAL : Contact the eCMD team to have the Plugin rebuilt to match\n");
            fprintf(
                stderr,
                "**** FATAL : Or get ahold of a down level shared library and rerun\n");
        }

        return ECMD_FATAL_FAILURE;
    }

    rc = dllInitDll();
    return rc;
}

uint32_t dllUnloadDll()
{
    uint32_t rc = 0;
    rc = dllFreeDll();
    return rc;
}

uint32_t dllInitDll()
{
    lg2::info("dllInitDll ");
    const char* dtbPath = getenv("PDBG_DTB");
    if (!dtbPath)
    {
        lg2::error("Failed to get PDBG_DTB env variable");
        return -1;
    }
    const char* ecmdExe = getenv("ECMD_EXE");
    if (!ecmdExe)
    {
        lg2::error("Failed to get ECMD_EXE env variable");
        return -1;
    }

    pdbg_targets_init(NULL);
    return ECMD_SUCCESS;
}

uint32_t dllFreeDll()
{
    lg2::info("dllUnloadDll ");
    return ECMD_SUCCESS;
}

uint32_t dllQueryDllInfo(ecmdDllInfo&)
{
    lg2::info("dllQueryDllInfo not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllCheckDllVersion(const char* options)
{
    char ver[20];
    strcpy(ver, ECMD_CAPI_VERSION);
    char major[10];
    char minor[10];

    // lint -e613 ver is set by strcpy above so can't be NULL  @02a
    uint32_t majorlength = (uint32_t)(strchr(ver, '.') - ver);
    strncpy(major, ver, majorlength);
    major[majorlength] = '\0';
    strncpy(minor, &(ver[majorlength + 1]), strlen(ver) - majorlength - 1);
    minor[strlen(ver) - majorlength - 1] = '\0';

    /* Default is just the major number */
    if ((options == NULL) || (strlen(options) == 0))
    {
        printf("ver%s", major);
    }
    else if (!strcmp(options, "full"))
    {
        printf("ver-%s-%s", major, minor);
    }

    /* Force an exit here as the dll is not properly initialized we can't allow
     * things to continue */
    exit(0);

    return 0; // JTA 09/27/06 - added to shut down a compiler warning.  It's not
              // smart enough to see the exit above
}

bool dllQueryVersionGreater(const char* version)
{
    std::string plver = ECMD_CAPI_VERSION;
    std::string clver = version;
    int plmajor, plminor;
    int clmajor, clminor;

    plmajor = atoi(plver.substr(0, plver.find('.')).c_str());
    plminor =
        atoi(plver.substr(plver.find('.') + 1, std::string::npos).c_str());

    clmajor = atoi(clver.substr(0, clver.find('.')).c_str());
    clminor =
        atoi(clver.substr(clver.find('.') + 1, std::string::npos).c_str());

    if (plmajor < clmajor)
        return false;
    if (plmajor > clmajor)
        return true;
    if (plminor >= clminor)
        return true;

    return false;
}

void ecmdIncrementLooperIterators(uint8_t level, ecmdLooperData& state);

// dllConfigLooperInit and dllExistLooperInit just call dllLooperInit in the
// correct mode
uint32_t dllConfigLooperInit(ecmdChipTarget& target,
                             ecmdLoopType_t looptype,
                             ecmdLooperData& state)
{
    return dllLooperInit(target, looptype, state, ECMD_CONFIG_LOOP);
}

uint32_t dllExistLooperInit(ecmdChipTarget& target,
                            ecmdLoopType_t looptype, ecmdLooperData& state)
{
    return dllLooperInit(target, looptype, state, ECMD_EXIST_LOOP);
}

uint32_t dllLooperInit(ecmdChipTarget& target, ecmdLoopType_t looptype,
                       ecmdLooperData& state, ecmdLoopMode_t mode)
{
    uint32_t rc = ECMD_SUCCESS;
    ecmdChipTarget queryTarget;

    /* If we aren't told what mode to run in, figure it out */
    if (mode == ECMD_DYNAMIC_LOOP)
    {
        mode = (ecmdLoopMode_t)ecmdGlobal_looperMode;
    }
    /* If it's a dynamic reverse, we need to set things up properly based on the
     * default */
    if (mode == ECMD_DYNAMIC_REVERSE_LOOP)
    {
        if (ecmdGlobal_looperMode == ECMD_CONFIG_LOOP)
        {
            mode = ECMD_CONFIG_REVERSE_LOOP;
        }
        else
        {
            mode = ECMD_EXIST_REVERSE_LOOP;
        }
    }

    // Set to unknown so we can error check later
    state.initialized = false;

    state.ecmdUseUnitid = false;

    queryTarget = target;

    /* Initialize defaults into the incoming target */
    if (target.cageState == ECMD_TARGET_FIELD_WILDCARD)
        target.cage = 0;
    if (target.nodeState == ECMD_TARGET_FIELD_WILDCARD)
        target.node = 0;
    if (target.slotState == ECMD_TARGET_FIELD_WILDCARD)
        target.slot = 0;
    if (target.chipTypeState == ECMD_TARGET_FIELD_WILDCARD)
        target.chipType = "na";
    if (target.posState == ECMD_TARGET_FIELD_WILDCARD)
        target.pos = 0;
    if (target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD)
        target.chipUnitType = "na";
    if (target.chipUnitNumState == ECMD_TARGET_FIELD_WILDCARD)
        target.chipUnitNum = 0;
    if (target.threadState == ECMD_TARGET_FIELD_WILDCARD)
        target.thread = 0;

    /* Set all the states to valid, unless they are unused */
    if (target.cageState != ECMD_TARGET_FIELD_UNUSED)
        target.cageState = ECMD_TARGET_FIELD_VALID;
    if (target.nodeState != ECMD_TARGET_FIELD_UNUSED)
        target.nodeState = ECMD_TARGET_FIELD_VALID;
    if (target.slotState != ECMD_TARGET_FIELD_UNUSED)
        target.slotState = ECMD_TARGET_FIELD_VALID;
    if (target.chipTypeState != ECMD_TARGET_FIELD_UNUSED)
        target.chipTypeState = ECMD_TARGET_FIELD_VALID;
    if (target.posState != ECMD_TARGET_FIELD_UNUSED)
        target.posState = ECMD_TARGET_FIELD_VALID;
    if (target.chipUnitTypeState != ECMD_TARGET_FIELD_UNUSED)
        target.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
    if (target.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED)
        target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
    if (target.threadState != ECMD_TARGET_FIELD_UNUSED)
        target.threadState = ECMD_TARGET_FIELD_VALID;

    if (looptype == ECMD_ALL_TARGETS_LOOP)
    {
        if (mode == ECMD_EXIST_LOOP || mode == ECMD_EXIST_REVERSE_LOOP)
        {
            rc = dllQueryExist(queryTarget, state.ecmdSystemConfigData,
                               ECMD_QUERY_DETAIL_LOW);
        }
        else
        {
            rc = dllQueryConfig(queryTarget, state.ecmdSystemConfigData,
                                ECMD_QUERY_DETAIL_LOW);
        }
    }
    else
    {
        if (mode == ECMD_EXIST_LOOP || mode == ECMD_EXIST_REVERSE_LOOP)
        {
            rc = dllQueryExistSelected(
                queryTarget, state.ecmdSystemConfigData, looptype);
        }
        else
        {
            rc = dllQueryConfigSelected(
                queryTarget, state.ecmdSystemConfigData, looptype);
        }

        /* Selected queries can change our states, so let's update them */
        if (queryTarget.cageState == ECMD_TARGET_FIELD_UNUSED)
            target.cageState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.nodeState == ECMD_TARGET_FIELD_UNUSED)
            target.nodeState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.slotState == ECMD_TARGET_FIELD_UNUSED)
            target.slotState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.chipTypeState == ECMD_TARGET_FIELD_UNUSED)
            target.chipTypeState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.posState == ECMD_TARGET_FIELD_UNUSED)
            target.posState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.chipUnitTypeState == ECMD_TARGET_FIELD_UNUSED)
            target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED)
            target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;
        if (queryTarget.threadState == ECMD_TARGET_FIELD_UNUSED)
            target.threadState = ECMD_TARGET_FIELD_UNUSED;
    }
    if (rc)
        return rc;

    /* If the reverse option is set, go through and flip all the data
     * structures */
    if (mode == ECMD_EXIST_REVERSE_LOOP ||
        mode == ECMD_CONFIG_REVERSE_LOOP)
    {
        std::list<ecmdCageData>::iterator curCage;
        std::list<ecmdNodeData>::iterator curNode;
        std::list<ecmdSlotData>::iterator curSlot;
        std::list<ecmdChipData>::iterator curChip;
        std::list<ecmdChipUnitData>::iterator curChipUnit;
        std::list<ecmdThreadData>::iterator curThread;

        for (curCage = state.ecmdSystemConfigData.cageData.begin();
             curCage != state.ecmdSystemConfigData.cageData.end();
             curCage++)
        {
            for (curNode = curCage->nodeData.begin();
                 curNode != curCage->nodeData.end(); curNode++)
            {
                for (curSlot = curNode->slotData.begin();
                     curSlot != curNode->slotData.end(); curSlot++)
                {
                    for (curChip = curSlot->chipData.begin();
                         curChip != curSlot->chipData.end(); curChip++)
                    {
                        for (curChipUnit = curChip->chipUnitData.begin();
                             curChipUnit != curChip->chipUnitData.end();
                             curChipUnit++)
                        {
                            /* All done at this level, now reverse it */
                            curChipUnit->threadData.reverse();
                        }
                        /* All done at this level, now reverse it */
                        curChip->chipUnitData.reverse();
                    }
                    /* All done at this level, now reverse it */
                    curSlot->chipData.reverse();
                }
                /* All done at this level, now reverse it */
                curNode->slotData.reverse();
            }
            /* All done at this level, now reverse it */
            curCage->nodeData.reverse();
        }
        /* All done at this level, now reverse it */
        state.ecmdSystemConfigData.cageData.reverse();
    }

    state.ecmdCurCage = state.ecmdSystemConfigData.cageData.begin();
    state.ecmdLooperInitFlag = true;
    state.prevTarget = target;

    /* Success! */
    state.initialized = true;

    return rc;
}

// ConfigLooperNext and ExistLooperNext can share the same code with just a
// switch to call the right function
uint32_t dllConfigLooperNext(ecmdChipTarget& target,
                             ecmdLooperData& state)
{
    return dllLooperNext(target, state, ECMD_CONFIG_LOOP);
}

uint32_t dllExistLooperNext(ecmdChipTarget& target, ecmdLooperData& state)
{
    return dllLooperNext(target, state, ECMD_EXIST_LOOP);
}

uint32_t dllLooperNext(ecmdChipTarget& target, ecmdLooperData& state,
                       ecmdLoopMode_t mode)
{
    uint32_t rc = ECMD_SUCCESS;
    const uint8_t CAGE = 0;
    const uint8_t NODE = 1;
    const uint8_t SLOT = 2;
    const uint8_t CHIP = 3;
    const uint8_t CHIPUNIT = 4;
    const uint8_t THREAD = 5;
    bool done = false;
    uint8_t level = 0;
    ;

    if (!state.initialized)
    {
        dllOutputError(
            "ecmdConfigLooperNext - Invalid state passed, verify ecmdConfigLooperInit was run successfully\n");
        /* We return 0 which stops the loop, we can't return any failure from
         * here */
        return 0;
    }

    /* If we aren't told what mode to run in, figure it out */
    if (mode == ECMD_DYNAMIC_LOOP)
    {
        mode = (ecmdLoopMode_t)ecmdGlobal_looperMode;
    }
    /* If it's a dynamic reverse, we need to set things up properly based on the
     * default */
    if (mode == ECMD_DYNAMIC_REVERSE_LOOP)
    {
        if (ecmdGlobal_looperMode == ECMD_CONFIG_LOOP)
        {
            mode = ECMD_CONFIG_REVERSE_LOOP;
        }
        else
        {
            mode = ECMD_EXIST_REVERSE_LOOP;
        }
    }

    /* Are we using unitids ? */
    if (state.ecmdUseUnitid)
    {
        /* We at the end ? */
        while (!done)
        {
            if (state.curUnitIdTarget == state.unitIdTargets.end())
            {
                return 0;
            }

            target = *(state.curUnitIdTarget);
            state.curUnitIdTarget++;

            /* Is this target actually configured, if not try the next one */
            if (mode == ECMD_EXIST_LOOP)
            {
                if (dllQueryTargetExist(target,
                                        &(state.ecmdSystemConfigData)))
                {
                    done = true;
                }
            }
            else
            {
                if (dllQueryTargetConfigured(target,
                                             &(state.ecmdSystemConfigData)))
                {
                    done = true;
                }
            }
        } /* while !done */

        /* Not using unitid's use physical targets */
    }
    else
    {
        while (!done)
        {
            level = CAGE;
            uint8_t valid = 1;

            /* We are at the end of the loop, nothing left to loop on, get out
             * of here */
            if (state.ecmdCurCage ==
                state.ecmdSystemConfigData.cageData.end())
            {
                return rc;
            }

            /* ******** NOTE : The iterators in state always point to the
             * next instance to use */
            /*           (the one that should be returned from this function
             * ****************     */

            /* Enter if : */
            /* First time in config looper */
            /* last cage != current cage */
            if (state.ecmdLooperInitFlag ||
                target.cage != (*state.ecmdCurCage).cageId)
            {
                /* Data is valid, let's setup this part of the target */
                target.cage = (*state.ecmdCurCage).cageId;
                state.ecmdCurNode = (*state.ecmdCurCage).nodeData.begin();
                valid = 0;

                /* If next level is unused we default to 0 */
                if ((state.prevTarget.nodeState == ECMD_TARGET_FIELD_UNUSED))
                {
                    /* If the next level is required but empty, this position
                     * isn't valid we need to restart */
                }
                else if ((state.prevTarget.nodeState !=
                          ECMD_TARGET_FIELD_UNUSED) &&
                         (state.ecmdCurNode ==
                          (*state.ecmdCurCage).nodeData.end()))
                {
                    /* Increment the iterators to point to the next target (at
                     * the level above us) */
                    ecmdIncrementLooperIterators(level - 1, state);
                    continue;

                    /* Everything is grand, let's continue to the next level */
                }
                else
                {
                    level = NODE;
                }
            }
            else
            {
                level = NODE;
            }

            /* Enter if : */
            /* Level == Node (the user is looping with nodes  */
            /* !valid - current node iterator isn't valid */
            /* last node != current node */
            if (level == NODE &&
                (!valid || target.node != (*state.ecmdCurNode).nodeId))
            {
                /* Data is valid, let's setup this part of the target */
                target.node = (*state.ecmdCurNode).nodeId;
                state.ecmdCurSlot = (*state.ecmdCurNode).slotData.begin();
                valid = 0;

                /* If next level is unused we default to 0 */
                if ((state.prevTarget.slotState == ECMD_TARGET_FIELD_UNUSED))
                {
                    /* If the next level is required but empty, this position
                     * isn't valid we need to restart */
                }
                else if ((state.prevTarget.slotState !=
                          ECMD_TARGET_FIELD_UNUSED) &&
                         (state.ecmdCurSlot ==
                          (*state.ecmdCurNode).slotData.end()))
                {
                    /* Increment the iterators to point to the next target (at
                     * the level above us) */
                    ecmdIncrementLooperIterators(level - 1, state);
                    continue;

                    /* Everything is grand, let's continue to the next level */
                }
                else
                {
                    level = SLOT;
                }
            }
            else if (valid)
            {
                level = SLOT;
            }

            /* Enter if : */
            /* Level == Slot (the user is looping with Slots  */
            /* !valid - current Slot iterator isn't valid */
            /* last Slot != current Slot */
            if (level == SLOT &&
                (!valid || target.slot != (*state.ecmdCurSlot).slotId))
            {
                /* Data is valid, let's setup this part of the target */
                target.slot = (*state.ecmdCurSlot).slotId;
                state.ecmdCurChip = (*state.ecmdCurSlot).chipData.begin();
                valid = 0;

                /* If next level is unused we default to 0 */
                if ((state.prevTarget.chipTypeState ==
                         ECMD_TARGET_FIELD_UNUSED ||
                     state.prevTarget.posState == ECMD_TARGET_FIELD_UNUSED))
                {
                    /* If the next level is required but empty, this position
                     * isn't valid we need to restart */
                }
                else if ((state.prevTarget.chipTypeState ==
                              ECMD_TARGET_FIELD_UNUSED ||
                          state.prevTarget.posState ==
                              ECMD_TARGET_FIELD_UNUSED) &&
                         (state.ecmdCurChip ==
                          (*state.ecmdCurSlot).chipData.end()))
                {
                    /* Increment the iterators to point to the next target (at
                     * the level above us) */
                    ecmdIncrementLooperIterators(level - 1, state);
                    continue;

                    /* Everything is grand, let's continue to the next level */
                }
                else
                {
                    level = CHIP;
                }
            }
            else if (valid)
            {
                level = CHIP;
            }

            /* Enter if : */
            /* Level == Chip (the user is looping with Chips  */
            /* !valid - current Chip iterator isn't valid */
            /* last ChipType != current ChipType */
            /* last Chip pos != current Chip pos */
            if (level == CHIP &&
                (!valid ||
                 target.chipType != (*state.ecmdCurChip).chipType ||
                 target.pos != (*state.ecmdCurChip).pos))
            {
                /* Data is valid, let's setup this part of the target */
                target.chipType = (*state.ecmdCurChip).chipType;
                target.pos = (*state.ecmdCurChip).pos;
                state.ecmdCurChipUnit =
                    (*state.ecmdCurChip).chipUnitData.begin();
                valid = 0;

                /* If next level is unused we default to 0 */
                if ((state.prevTarget.chipUnitNumState ==
                     ECMD_TARGET_FIELD_UNUSED))
                {
                    /* If the next level is required but empty, this position
                     * isn't valid we need to restart */
                }
                else if ((state.prevTarget.chipUnitNumState !=
                          ECMD_TARGET_FIELD_UNUSED) &&
                         (state.ecmdCurChipUnit ==
                          (*state.ecmdCurChip).chipUnitData.end()))
                {
                    /* Increment the iterators to point to the next target (at
                     * the level above us) */
                    ecmdIncrementLooperIterators(level - 1, state);
                    continue;

                    /* Everything is grand, let's continue to the next level */
                }
                else
                {
                    level = CHIPUNIT;
                }
            }
            else if (valid)
            {
                level = CHIPUNIT;
            }

            /* Enter if : */
            /* Level == ChipUnit (the user is looping with ChipUnits  */
            /* !valid - current ChipUnit iterator isn't valid */
            /* last ChipUnitType != current ChipUnitType */
            /* last ChipUnitNum != current ChipUnitNum */
            if (level == CHIPUNIT &&
                (!valid ||
                 target.chipUnitType !=
                     (*state.ecmdCurChipUnit).chipUnitType ||
                 target.chipUnitNum !=
                     (*state.ecmdCurChipUnit).chipUnitNum))
            {
                /* Data is valid, let's setup this part of the target */
                target.chipUnitType =
                    (*state.ecmdCurChipUnit).chipUnitType;
                target.chipUnitNum = (*state.ecmdCurChipUnit).chipUnitNum;
                state.ecmdCurThread =
                    (*state.ecmdCurChipUnit).threadData.begin();
                valid = 0;

                /* If next level is unused we default to 0 */
                if (state.prevTarget.threadState == ECMD_TARGET_FIELD_UNUSED)
                {
                    /* If the next level is required but empty, this position
                     * isn't valid we need to restart */
                }
                else if ((state.prevTarget.threadState !=
                          ECMD_TARGET_FIELD_UNUSED) &&
                         (state.ecmdCurThread ==
                          (*state.ecmdCurChipUnit).threadData.end()))
                {
                    /* Increment the iterators to point to the next target (at
                     * the level above us) */
                    ecmdIncrementLooperIterators(level - 1, state);
                    continue;

                    /* Everything is grand, let's continue to the next level */
                }
                else
                {
                    level = THREAD;
                }
            }
            else if (valid)
            {
                level = THREAD;
            }

            /* Enter if : */
            /* Level == Thread (the user is looping with Threads  */
            /* !valid - current Thread iterator isn't valid */
            /* last Thread != current Thread */
            if (level == THREAD &&
                (!valid ||
                 target.thread != (*state.ecmdCurThread).threadId))
            {
                /* Data is valid, let's setup this part of the target */
                target.thread = (*state.ecmdCurThread).threadId;
            }

            /* We got here, must be done */
            done = true;

        } /* End while */

        /* Increment the iterators to point to the next target */
        ecmdIncrementLooperIterators(level, state);

    } /* end phys/unitid if */

    /* We are through the first init loop */
    if (state.ecmdLooperInitFlag)
    {
        state.ecmdLooperInitFlag = false;
    }

    /* We got here, we have more to do, let's tell the client */
    rc = 1;

    return rc;
}

void ecmdIncrementLooperIterators(uint8_t level, ecmdLooperData& state)
{
    /* Let's start incrementing our lowest pointer so it points to the next
     * object for the subsequent call to this function */
    const uint8_t CAGE = 0;
    const uint8_t NODE = 1;
    const uint8_t SLOT = 2;
    const uint8_t CHIP = 3;
    const uint8_t CHIPUNIT = 4;
    const uint8_t THREAD = 5;

    // The following switch statement makes deliberate use of falling through
    // from one case statement to the next.  So tell Beam to not flag those ass
    // errors with the /*fall through*/ comments. @02a
    switch (level)
    {
        case THREAD: // thread
            state.ecmdCurThread++;
            /* Did we find another thread, if not we will try chipUnit */
            if (state.ecmdCurThread !=
                (*state.ecmdCurChipUnit).threadData.end())
            {
                break;
            }
            /*fall through*/
        case CHIPUNIT: // chipUnit
            state.ecmdCurChipUnit++;
            /* Did we find another chipUnit, if not we will try chip */
            if (state.ecmdCurChipUnit !=
                (*state.ecmdCurChip).chipUnitData.end())
            {
                break;
            }
            /*fall through*/
        case CHIP: // chip
            state.ecmdCurChip++;
            /* Did we find another chip, if not we will try slot */
            if (state.ecmdCurChip != (*state.ecmdCurSlot).chipData.end())
            {
                break;
            }
            /*fall through*/
        case SLOT: // slot
            state.ecmdCurSlot++;
            /* Did we find another slot, if not we will try node */
            if (state.ecmdCurSlot != (*state.ecmdCurNode).slotData.end())
            {
                break;
            }
            /*fall through*/
        case NODE: // node
            state.ecmdCurNode++;
            /* Did we find another node, if not we will try cage */
            if (state.ecmdCurNode != (*state.ecmdCurCage).nodeData.end())
            {
                break;
            }
            /*fall through*/
        case CAGE: // cage
            state.ecmdCurCage++;
            break;

        default:
            // shouldn't get here
            break;
    }
}

uint32_t dllQuerySelected(ecmdChipTarget& target, ecmdQueryData& queryData,
                          ecmdLoopType_t looptype)
{
    return queryConfigExistSelected(target, queryData, looptype, false);
}

uint32_t dllQueryConfigSelected(ecmdChipTarget& target,
                                ecmdQueryData& queryData,
                                ecmdLoopType_t looptype)
{
    return queryConfigExistSelected(target, queryData, looptype, false);
}

uint32_t dllQueryExistSelected(ecmdChipTarget& target,
                               ecmdQueryData& queryData,
                               ecmdLoopType_t looptype)
{
    return queryConfigExistSelected(target, queryData, looptype, true);
}

uint32_t queryConfigExistSelected(ecmdChipTarget& target,
                                  ecmdQueryData& queryData,
                                  ecmdLoopType_t looptype, bool existMode)
{
    uint32_t rc = ECMD_SUCCESS;

    uint8_t SINGLE = 0;
    uint8_t ALL = 1;
    uint8_t MULTI = 2;
    uint8_t FT = 3;
    uint8_t LT = 4;
    uint8_t ET = 5;
    uint8_t OT = 6;

    //@01c Add init to 2
    uint8_t cageType = 2;
    uint8_t nodeType = 2;
    uint8_t slotType = 2;
    uint8_t posType = 2;
    uint8_t chipUnitNumType = 2;
    uint8_t threadType = 2;

    std::string patterns = ",.";

    /* Let's setup for the Variable depth, walk up until we find something
     * specified */
    if ((looptype == ECMD_SELECTED_TARGETS_LOOP_VD) ||
        (looptype == ECMD_SELECTED_TARGETS_LOOP_VD_DEFALL))
    {
        if ((target.threadState == ECMD_TARGET_FIELD_UNUSED) ||
            (target.threadState != ECMD_TARGET_FIELD_VALID &&
             ecmdUserArgs.thread == ""))
        {
            target.threadState = ECMD_TARGET_FIELD_UNUSED;

            if ((target.chipUnitNumState == ECMD_TARGET_FIELD_UNUSED) ||
                (target.chipUnitNumState != ECMD_TARGET_FIELD_VALID &&
                 ecmdUserArgs.chipUnitNum == ""))
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_UNUSED;

                if ((target.posState == ECMD_TARGET_FIELD_UNUSED) ||
                    (target.posState != ECMD_TARGET_FIELD_VALID &&
                     ecmdUserArgs.pos == ""))
                {
                    target.posState = ECMD_TARGET_FIELD_UNUSED;

                    if ((target.slotState == ECMD_TARGET_FIELD_UNUSED) ||
                        (target.slotState != ECMD_TARGET_FIELD_VALID &&
                         ecmdUserArgs.slot == ""))
                    {
                        target.slotState = ECMD_TARGET_FIELD_UNUSED;

                        if ((target.nodeState == ECMD_TARGET_FIELD_UNUSED) ||
                            (target.nodeState != ECMD_TARGET_FIELD_VALID &&
                             ecmdUserArgs.node == ""))
                        {
                            target.nodeState = ECMD_TARGET_FIELD_UNUSED;

                            if ((target.cageState ==
                                 ECMD_TARGET_FIELD_UNUSED) ||
                                (target.cageState !=
                                     ECMD_TARGET_FIELD_VALID &&
                                 ecmdUserArgs.cage == ""))
                            {
                                target.cageState = ECMD_TARGET_FIELD_UNUSED;

                            } /* cage */
                        } /* node */
                    } /* slot */
                } /* pos */
            } /* chipUnitNum */
        } /* thread */
        /* Go back to a standard loop now that states are set */
        if (looptype == ECMD_SELECTED_TARGETS_LOOP_VD)
        {
            looptype = ECMD_SELECTED_TARGETS_LOOP;
        }
        else
        {
            looptype = ECMD_SELECTED_TARGETS_LOOP_DEFALL;
        }
    }

    /* If the cage is set to ignore we can't return anything so let's just short
     * circuit */
    if (target.cageState == ECMD_TARGET_FIELD_UNUSED)
    {
        return ECMD_SUCCESS;
    }

    /* ----------------------------------------------------------- */
    /* update target with useful info from the ecmdUserArgs struct */
    /* ----------------------------------------------------------- */

    // cage
    /* If the state is already valid we just continue on */
    if (target.cageState == ECMD_TARGET_FIELD_VALID)
    {
        cageType = SINGLE;
    }
    else if (target.cageState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any cage args */
        if (ecmdUserArgs.cage.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.cage.find_first_of(patterns) <
                ecmdUserArgs.cage.length())
            {
                if (!isValidTargetString(ecmdUserArgs.cage))
                {
                    dllOutputError(
                        "dllQuerySelected - cage (-k#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = MULTI;
            }

            /* See if we have a single entry -k1 */
            else if (isValidTargetString(ecmdUserArgs.cage))
            {
                target.cageState = ECMD_TARGET_FIELD_VALID;
                target.cage = (uint32_t)atoi(ecmdUserArgs.cage.c_str());
                cageType = SINGLE;
            }

            /* See if the user specified -aall or -kall */
            else if (ecmdUserArgs.cage == "all")
            {
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = ALL;
            }

            /* See if the user specified -aft or -kft */
            else if (ecmdUserArgs.cage == "ft")
            {
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = FT;
            }

            /* See if the user specified -alt or -klt */
            else if (ecmdUserArgs.cage == "lt")
            {
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = LT;
            }

            /* See if the user specified -aet or -ket */
            else if (ecmdUserArgs.cage == "et")
            {
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = ET;
            }

            /* See if the user specified -aot or -kot */
            else if (ecmdUserArgs.cage == "ot")
            {
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = OT;
            }

            /* See if the user specified or -k- */
            else if (ecmdUserArgs.cage == "-")
            {
                dllOutputError(
                    "dllQuerySelected - argument -k- not supported\n");
                return ECMD_INVALID_ARGS;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -k options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.cageState = ECMD_TARGET_FIELD_WILDCARD;
                cageType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.cage = 0;
                cageType = SINGLE;
                target.cageState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    // node
    /* If the state is already valid we just continue on */
    if (target.nodeState == ECMD_TARGET_FIELD_VALID)
    {
        nodeType = SINGLE;
    }
    else if (target.nodeState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any node args */
        if (ecmdUserArgs.node.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.node.find_first_of(patterns) <
                ecmdUserArgs.node.length())
            {
                if (!isValidTargetString(ecmdUserArgs.node))
                {
                    dllOutputError(
                        "dllQuerySelected - node (-n#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = MULTI;
            }

            /* See if we have a single entry -n1 */
            else if (isValidTargetString(ecmdUserArgs.node))
            {
                target.nodeState = ECMD_TARGET_FIELD_VALID;
                target.node = (uint32_t)atoi(ecmdUserArgs.node.c_str());
                nodeType = SINGLE;
            }

            /* See if the user specified -aall or -nall */
            else if (ecmdUserArgs.node == "all")
            {
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = ALL;
            }

            /* See if the user specified -aft or -nft */
            else if (ecmdUserArgs.node == "ft")
            {
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = FT;
            }

            /* See if the user specified -alt or -nlt */
            else if (ecmdUserArgs.node == "lt")
            {
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = LT;
            }

            /* See if the user specified -aet or -net */
            else if (ecmdUserArgs.node == "et")
            {
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = ET;
            }

            /* See if the user specified -aot or -not */
            else if (ecmdUserArgs.node == "ot")
            {
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = OT;
            }

            /* See if the user specified or -n- */
            else if (ecmdUserArgs.node == "-")
            {
                target.nodeState = ECMD_TARGET_FIELD_VALID;
                target.node = ECMD_TARGETDEPTH_NA;
                nodeType = SINGLE;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -n options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.nodeState = ECMD_TARGET_FIELD_WILDCARD;
                nodeType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.node = 0;
                nodeType = SINGLE;
                target.nodeState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    // slot
    /* If the state is already valid we just continue on */
    if (target.slotState == ECMD_TARGET_FIELD_VALID)
    {
        slotType = SINGLE;
    }
    else if (target.slotState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any slot args */
        if (ecmdUserArgs.slot.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.slot.find_first_of(patterns) <
                ecmdUserArgs.slot.length())
            {
                if (!isValidTargetString(ecmdUserArgs.slot))
                {
                    dllOutputError(
                        "dllQuerySelected - slot (-s#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = MULTI;
            }

            /* See if we have a single entry -s1 */
            else if (isValidTargetString(ecmdUserArgs.slot))
            {
                target.slotState = ECMD_TARGET_FIELD_VALID;
                target.slot = (uint32_t)atoi(ecmdUserArgs.slot.c_str());
                slotType = SINGLE;
            }

            /* See if the user specified -aall or -sall */
            else if (ecmdUserArgs.slot == "all")
            {
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = ALL;
            }

            /* See if the user specified -aft or -sft */
            else if (ecmdUserArgs.slot == "ft")
            {
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = FT;
            }

            /* See if the user specified -alt or -slt */
            else if (ecmdUserArgs.slot == "lt")
            {
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = LT;
            }

            /* See if the user specified -aet or -set */
            else if (ecmdUserArgs.slot == "et")
            {
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = ET;
            }

            /* See if the user specified -aot or -sot */
            else if (ecmdUserArgs.slot == "ot")
            {
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = OT;
            }

            /* See if the user specified or -s- */
            else if (ecmdUserArgs.slot == "-")
            {
                target.slotState = ECMD_TARGET_FIELD_VALID;
                target.slot = ECMD_TARGETDEPTH_NA;
                slotType = SINGLE;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -s options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.slotState = ECMD_TARGET_FIELD_WILDCARD;
                slotType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.slot = 0;
                slotType = SINGLE;
                target.slotState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    // pos
    /* If the state is already valid we just continue on */
    if (target.posState == ECMD_TARGET_FIELD_VALID)
    {
        posType = SINGLE;
    }
    else if (target.posState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any pos args */
        if (ecmdUserArgs.pos.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.pos.find_first_of(patterns) <
                ecmdUserArgs.pos.length())
            {
                if (!isValidTargetString(ecmdUserArgs.pos))
                {
                    dllOutputError(
                        "dllQuerySelected - pos (-p#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = MULTI;
            }

            /* See if we have a single entry -p1 */
            else if (isValidTargetString(ecmdUserArgs.pos))
            {
                target.posState = ECMD_TARGET_FIELD_VALID;
                target.pos = (uint32_t)atoi(ecmdUserArgs.pos.c_str());
                posType = SINGLE;
            }

            /* See if the user specified -aall or -pall */
            else if (ecmdUserArgs.pos == "all")
            {
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = ALL;
            }

            /* See if the user specified -aft or -pft */
            else if (ecmdUserArgs.pos == "ft")
            {
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = FT;
            }

            /* See if the user specified -alt or -plt */
            else if (ecmdUserArgs.pos == "lt")
            {
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = LT;
            }

            /* See if the user specified -aet or -pet */
            else if (ecmdUserArgs.pos == "et")
            {
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = ET;
            }

            /* See if the user specified -aot or -pot */
            else if (ecmdUserArgs.pos == "ot")
            {
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = OT;
            }

            /* See if the user specified or -p- */
            else if (ecmdUserArgs.pos == "-")
            {
                dllOutputError(
                    "dllQuerySelected - argument -p- not supported\n");
                return ECMD_INVALID_ARGS;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -p options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.posState = ECMD_TARGET_FIELD_WILDCARD;
                posType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.pos = 0;
                posType = SINGLE;
                target.posState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    // chipUnitNum
    /* If the state is already valid we just continue on */
    if (target.chipUnitNumState == ECMD_TARGET_FIELD_VALID)
    {
        chipUnitNumType = SINGLE;
    }
    else if (target.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any chipUnitNum args */
        if (ecmdUserArgs.chipUnitNum.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.chipUnitNum.find_first_of(patterns) <
                ecmdUserArgs.chipUnitNum.length())
            {
                if (!isValidTargetString(ecmdUserArgs.chipUnitNum))
                {
                    dllOutputError(
                        "dllQuerySelected - chipUnitNum/core (-c#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = MULTI;
            }

            /* See if we have a single entry -c1 */
            else if (isValidTargetString(ecmdUserArgs.chipUnitNum))
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
                target.chipUnitNum =
                    (uint32_t)atoi(ecmdUserArgs.chipUnitNum.c_str());
                chipUnitNumType = SINGLE;
            }

            /* See if the user specified -aall or -call */
            else if (ecmdUserArgs.chipUnitNum == "all")
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = ALL;
            }

            /* See if the user specified -aft or -cft */
            else if (ecmdUserArgs.chipUnitNum == "ft")
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = FT;
            }

            /* See if the user specified -alt or -clt */
            else if (ecmdUserArgs.chipUnitNum == "lt")
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = LT;
            }

            /* See if the user specified -aet or -cet */
            else if (ecmdUserArgs.chipUnitNum == "et")
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = ET;
            }

            /* See if the user specified -aot or -cot */
            else if (ecmdUserArgs.chipUnitNum == "ot")
            {
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = OT;
            }

            /* See if the user specified or -c- */
            else if (ecmdUserArgs.chipUnitNum == "-")
            {
                dllOutputError(
                    "dllQuerySelected - argument -c- not supported\n");
                return ECMD_INVALID_ARGS;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -c options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.chipUnitNumState = ECMD_TARGET_FIELD_WILDCARD;
                chipUnitNumType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.chipUnitNum = 0;
                chipUnitNumType = SINGLE;
                target.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    // thread
    /* If the state is already valid we just continue on */
    if (target.threadState == ECMD_TARGET_FIELD_VALID)
    {
        threadType = SINGLE;
    }
    else if (target.threadState != ECMD_TARGET_FIELD_UNUSED)
    {
        /* Did the user specify any thread args */
        if (ecmdUserArgs.thread.length())
        {
            /* If the user used any sort of list 0,1,2,4 or range 2..5 then we
             * do multi */
            if (ecmdUserArgs.thread.find_first_of(patterns) <
                ecmdUserArgs.thread.length())
            {
                if (!isValidTargetString(ecmdUserArgs.thread))
                {
                    dllOutputError(
                        "dllQuerySelected - thread (-t#) argument contained invalid characters\n");
                    return ECMD_INVALID_ARGS;
                }
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = MULTI;
            }

            /* See if we have a single entry -t1 */
            else if (isValidTargetString(ecmdUserArgs.thread))
            {
                target.threadState = ECMD_TARGET_FIELD_VALID;
                target.thread = (uint32_t)atoi(ecmdUserArgs.thread.c_str());
                threadType = SINGLE;
            }

            /* See if the user specified -aall or -tall */
            else if (ecmdUserArgs.thread == "all" ||
                     ecmdUserArgs.thread == "alive")
            {
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = ALL;
            }

            /* See if the user specified -aft or -tft */
            else if (ecmdUserArgs.thread == "ft")
            {
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = FT;
            }

            /* See if the user specified -alt or -tlt */
            else if (ecmdUserArgs.thread == "lt")
            {
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = LT;
            }

            /* See if the user specified -aet or -tet */
            else if (ecmdUserArgs.thread == "et")
            {
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = ET;
            }

            /* See if the user specified -aot or -tot */
            else if (ecmdUserArgs.thread == "ot")
            {
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = OT;
            }

            /* See if the user specified or -t- */
            else if (ecmdUserArgs.thread == "-")
            {
                dllOutputError(
                    "dllQuerySelected - argument -t- not supported\n");
                return ECMD_INVALID_ARGS;
            }

            /* Finally, we can error out */
            else
            {
                dllOutputError(
                    "dllQuerySelected - unknown -t options found!\n");
                return ECMD_INVALID_ARGS;
            }
        }
        /* Nothing was specified, set some defaults */
        else
        {
            if (looptype == ECMD_SELECTED_TARGETS_LOOP_DEFALL)
            {
                /* Default to all */
                target.threadState = ECMD_TARGET_FIELD_WILDCARD;
                threadType = ALL;
            }
            else
            {
                /* User didn't specify anything and we default to 0 */
                target.thread = 0;
                threadType = SINGLE;
                target.threadState = ECMD_TARGET_FIELD_VALID;
            }
        }
    }

    /* Okay, target setup as best we can, let's go out to query cnfg with it */
    if (existMode)
    {
        rc = dllQueryExist(target, queryData, ECMD_QUERY_DETAIL_LOW);
    }
    else
    {
        rc = dllQueryConfig(target, queryData, ECMD_QUERY_DETAIL_LOW);
    }
    if (rc)
        return rc;

    // Within this loop the first for calls to removeCurrentElement()
    // generates lint msg 713 & 820: Loss of precision (arg. no. 1)
    // So disabling that message within this scope (re-enable at end of
    // loop).      @02a
    // lint -e713
    // lint -e820

    /* now I need to go in and clean out any excess stuff */
    std::list<ecmdCageData>::iterator curCage = queryData.cageData.begin();
    while (curCage != queryData.cageData.end())
    {
        /* If cageType >= MULTI, they specified one of the special queries where
         * items need to be removed */
        if (cageType >= MULTI)
        {
            /* Is the current element in the list of numbers the user provided,
             * if not remove it */
            if (cageType == MULTI)
            {
                if (removeCurrentElement(curCage->cageId, ecmdUserArgs.cage))
                {
                    curCage = queryData.cageData.erase(curCage);
                    continue;
                }
            }
            /* Is the current element in the list is first, keep it.  Otherwise,
             * remove */
            else if (cageType == FT)
            {
                if (curCage != queryData.cageData.begin())
                {
                    curCage = queryData.cageData.erase(curCage);
                    continue;
                }
            }
            /* Is the current element in the list is last, keep it.  Otherwise,
             * remove */
            else if (cageType == LT)
            {
                std::list<ecmdCageData>::iterator lastCage = curCage;
                lastCage++;
                if (lastCage != queryData.cageData.end())
                {
                    curCage = queryData.cageData.erase(curCage);
                    continue;
                }
            }
            /* If the current element in the list is even, keep it.  Otherwise,
             * remove */
            else if (cageType == ET)
            {
                if ((curCage->cageId % 2) == 1)
                {
                    curCage = queryData.cageData.erase(curCage);
                    continue;
                }
            }
            /* If the current element in the list is odd, keep it.  Otherwise,
             * remove */
            else if (cageType == OT)
            {
                if ((curCage->cageId % 2) == 0)
                {
                    curCage = queryData.cageData.erase(curCage);
                    continue;
                }
            }
        }

        /* Walk through the nodes */
        std::list<ecmdNodeData>::iterator curNode = curCage->nodeData.begin();
        while (curNode != curCage->nodeData.end())
        {
            /* If nodeType >= MULTI, they specified one of the special queries
             * where items need to be removed */
            if (nodeType >= MULTI)
            {
                /* Is the current element in the list of numbers the user
                 * provided, if not remove it */
                if (nodeType == MULTI)
                {
                    if (removeCurrentElement(curNode->nodeId,
                                             ecmdUserArgs.node))
                    {
                        curNode = curCage->nodeData.erase(curNode);
                        continue;
                    }
                }
                /* Is the current element in the list is first, keep it.
                 * Otherwise, remove */
                else if (nodeType == FT)
                {
                    if (curNode != curCage->nodeData.begin())
                    {
                        curNode = curCage->nodeData.erase(curNode);
                        continue;
                    }
                }
                /* Is the current element in the list is last, keep it.
                 * Otherwise, remove */
                else if (nodeType == LT)
                {
                    std::list<ecmdNodeData>::iterator lastNode = curNode;
                    lastNode++;
                    if (lastNode != curCage->nodeData.end())
                    {
                        curNode = curCage->nodeData.erase(curNode);
                        continue;
                    }
                }
                /* If the current element in the list is even, keep it.
                 * Otherwise, remove */
                else if (nodeType == ET)
                {
                    if ((curNode->nodeId % 2) == 1)
                    {
                        curNode = curCage->nodeData.erase(curNode);
                        continue;
                    }
                }
                /* If the current element in the list is odd, keep it.
                 * Otherwise, remove */
                else if (nodeType == OT)
                {
                    if ((curNode->nodeId % 2) == 0)
                    {
                        curNode = curCage->nodeData.erase(curNode);
                        continue;
                    }
                }
            }

            /* Walk through the slots */
            std::list<ecmdSlotData>::iterator curSlot =
                curNode->slotData.begin();
            while (curSlot != curNode->slotData.end())
            {
                /* If slotType >= MULTI, they specified one of the special
                 * queries where items need to be removed */
                if (slotType >= MULTI)
                {
                    /* Is the current element in the list of numbers the user
                     * provided, if not remove it */
                    if (slotType == MULTI)
                    {
                        if (removeCurrentElement(curSlot->slotId,
                                                 ecmdUserArgs.slot))
                        {
                            curSlot = curNode->slotData.erase(curSlot);
                            continue;
                        }
                    }
                    /* Is the current element in the list is first, keep it.
                     * Otherwise, remove */
                    else if (slotType == FT)
                    {
                        if (curSlot != curNode->slotData.begin())
                        {
                            curSlot = curNode->slotData.erase(curSlot);
                            continue;
                        }
                    }
                    /* Is the current element in the list is last, keep it.
                     * Otherwise, remove */
                    else if (slotType == LT)
                    {
                        std::list<ecmdSlotData>::iterator lastSlot = curSlot;
                        lastSlot++;
                        if (lastSlot != curNode->slotData.end())
                        {
                            curSlot = curNode->slotData.erase(curSlot);
                            continue;
                        }
                    }
                    /* If the current element in the list is even, keep it.
                     * Otherwise, remove */
                    else if (slotType == ET)
                    {
                        if ((curSlot->slotId % 2) == 1)
                        {
                            curSlot = curNode->slotData.erase(curSlot);
                            continue;
                        }
                    }
                    /* If the current element in the list is odd, keep it.
                     * Otherwise, remove */
                    else if (slotType == OT)
                    {
                        if ((curSlot->slotId % 2) == 0)
                        {
                            curSlot = curNode->slotData.erase(curSlot);
                            continue;
                        }
                    }
                }

                /* Walk through all the chip positions */
                std::list<ecmdChipData>::iterator curChip =
                    curSlot->chipData.begin();
                while (curChip != curSlot->chipData.end())
                {
                    /* If posType >= MULTI, they specified one of the special
                     * queries where items need to be removed */
                    if (posType >= MULTI)
                    {
                        /* Is the current element in the list of numbers the
                         * user provided, if not remove it */
                        if (posType == MULTI)
                        {
                            if (removeCurrentElement(curChip->pos,
                                                     ecmdUserArgs.pos))
                            {
                                curChip = curSlot->chipData.erase(curChip);
                                continue;
                            }
                        }
                        /* Is the current element in the list is first, keep it.
                         * Otherwise, remove */
                        else if (posType == FT)
                        {
                            if (curChip != curSlot->chipData.begin())
                            {
                                curChip = curSlot->chipData.erase(curChip);
                                continue;
                            }
                        }
                        /* Is the current element in the list is last, keep it.
                         * Otherwise, remove */
                        else if (posType == LT)
                        {
                            std::list<ecmdChipData>::iterator lastChip =
                                curChip;
                            lastChip++;
                            if (lastChip != curSlot->chipData.end())
                            {
                                curChip = curSlot->chipData.erase(curChip);
                                continue;
                            }
                        }
                        /* If the current element in the list is even, keep it.
                         * Otherwise, remove */
                        else if (posType == ET)
                        {
                            if ((curChip->pos % 2) == 1)
                            {
                                curChip = curSlot->chipData.erase(curChip);
                                continue;
                            }
                        }
                        /* If the current element in the list is odd, keep it.
                         * Otherwise, remove */
                        else if (posType == OT)
                        {
                            if ((curChip->pos % 2) == 0)
                            {
                                curChip = curSlot->chipData.erase(curChip);
                                continue;
                            }
                        }
                    }

                    /* Walk through all the chipUnits */
                    std::list<ecmdChipUnitData>::iterator curChipUnit =
                        curChip->chipUnitData.begin();
                    while (curChipUnit != curChip->chipUnitData.end())
                    {
                        /* If chipUnitNumType >= MULTI, they specified one of
                         * the special queries where items need to be removed */
                        if (chipUnitNumType >= MULTI)
                        {
                            /* Is the current element in the list of numbers the
                             * user provided, if not remove it */
                            if (chipUnitNumType == MULTI)
                            {
                                if (removeCurrentElement(
                                        curChipUnit->chipUnitNum,
                                        ecmdUserArgs.chipUnitNum))
                                {
                                    curChipUnit = curChip->chipUnitData.erase(
                                        curChipUnit);
                                    continue;
                                }
                            }
                            /* Is the current element in the list is first, keep
                             * it.  Otherwise, remove */
                            else if (chipUnitNumType == FT)
                            {
                                if (curChipUnit !=
                                    curChip->chipUnitData.begin())
                                {
                                    curChipUnit = curChip->chipUnitData.erase(
                                        curChipUnit);
                                    continue;
                                }
                            }
                            /* Is the current element in the list is last, keep
                             * it.  Otherwise, remove */
                            else if (chipUnitNumType == LT)
                            {
                                std::list<ecmdChipUnitData>::iterator
                                    lastChipUnit = curChipUnit;
                                lastChipUnit++;
                                if (lastChipUnit != curChip->chipUnitData.end())
                                {
                                    curChipUnit = curChip->chipUnitData.erase(
                                        curChipUnit);
                                    continue;
                                }
                            }
                            /* If the current element in the list is even, keep
                             * it.  Otherwise, remove */
                            else if (chipUnitNumType == ET)
                            {
                                if ((curChipUnit->chipUnitNum % 2) == 1)
                                {
                                    curChipUnit = curChip->chipUnitData.erase(
                                        curChipUnit);
                                    continue;
                                }
                            }
                            /* If the current element in the list is odd, keep
                             * it.  Otherwise, remove */
                            else if (chipUnitNumType == OT)
                            {
                                if ((curChipUnit->chipUnitNum % 2) == 0)
                                {
                                    curChipUnit = curChip->chipUnitData.erase(
                                        curChipUnit);
                                    continue;
                                }
                            }
                        }

                        /* Walk through the threads */
                        std::list<ecmdThreadData>::iterator curThread =
                            curChipUnit->threadData.begin();
                        while (curThread != curChipUnit->threadData.end())
                        {
                            /* If threadType >= MULTI, they specified one of the
                             * special queries where items need to be removed */
                            if (threadType >= MULTI)
                            {
                                /* Is the current element in the list of numbers
                                 * the user provided, if not remove it */
                                if (threadType == MULTI)
                                {
                                    if (removeCurrentElement(
                                            curThread->threadId,
                                            ecmdUserArgs.thread))
                                    {
                                        curThread =
                                            curChipUnit->threadData.erase(
                                                curThread);
                                        continue;
                                    }
                                }
                                /* Is the current element in the list is first,
                                 * keep it.  Otherwise, remove */
                                else if (threadType == FT)
                                {
                                    if (curThread !=
                                        curChipUnit->threadData.begin())
                                    {
                                        curThread =
                                            curChipUnit->threadData.erase(
                                                curThread);
                                        continue;
                                    }
                                }
                                /* Is the current element in the list is last,
                                 * keep it.  Otherwise, remove */
                                else if (threadType == LT)
                                {
                                    std::list<ecmdThreadData>::iterator
                                        lastThread = curThread;
                                    lastThread++;
                                    if (lastThread !=
                                        curChipUnit->threadData.end())
                                    {
                                        curThread =
                                            curChipUnit->threadData.erase(
                                                curThread);
                                        continue;
                                    }
                                }
                                /* If the current element in the list is even,
                                 * keep it.  Otherwise, remove */
                                else if (threadType == ET)
                                {
                                    if ((curThread->threadId % 2) == 1)
                                    {
                                        curThread =
                                            curChipUnit->threadData.erase(
                                                curThread);
                                        continue;
                                    }
                                }
                                /* If the current element in the list is odd,
                                 * keep it.  Otherwise, remove */
                                else if (threadType == OT)
                                {
                                    if ((curThread->threadId % 2) == 0)
                                    {
                                        curThread =
                                            curChipUnit->threadData.erase(
                                                curThread);
                                        continue;
                                    }
                                }
                            }

                            curThread++;
                        } /* while curThread */
                        if ((target.threadState !=
                             ECMD_TARGET_FIELD_UNUSED) &&
                            curChipUnit->threadData.empty())
                        {
                            curChipUnit =
                                curChip->chipUnitData.erase(curChipUnit);
                        }
                        else
                        {
                            curChipUnit++;
                        }
                    } /* while curChipUnit */

                    if ((target.chipUnitNumState !=
                         ECMD_TARGET_FIELD_UNUSED) &&
                        curChip->chipUnitData.empty())
                    {
                        curChip = curSlot->chipData.erase(curChip);
                    }
                    else
                    {
                        curChip++;
                    }
                } /* while curChip */

                /* Let's check to make sure there is something left here after
                 * we removed everything */
                if (((target.chipTypeState != ECMD_TARGET_FIELD_UNUSED) ||
                     (target.posState != ECMD_TARGET_FIELD_UNUSED)) &&
                    curSlot->chipData.empty())
                {
                    curSlot = curNode->slotData.erase(curSlot);
                }
                else
                {
                    curSlot++;
                }
            } /* while curSlot */

            /* Let's check to make sure there is something left here after we
             * removed everything */
            if ((target.slotState != ECMD_TARGET_FIELD_UNUSED) &&
                curNode->slotData.empty())
            {
                curNode = curCage->nodeData.erase(curNode);
            }
            else
            {
                curNode++;
            }
        } /* while curNode */

        /* Let's check to make sure there is something left here after we
         * removed everything */
        if ((target.nodeState != ECMD_TARGET_FIELD_UNUSED) &&
            curCage->nodeData.empty())
        {
            curCage = queryData.cageData.erase(curCage);
        }
        else
        {
            curCage++;
        }
    } /* while curCage */

    // lint +e713   @02a
    // lint +e820   @02a

    return rc;
}

uint32_t dllSpecificCommandArgs([[maybe_unused]] int* argc,
                                [[maybe_unused]] char** argv[])
{
    return ECMD_SUCCESS;
}
uint32_t dllCommonCommandArgs(int* argc, char** argv[])
{
    uint32_t rc = ECMD_SUCCESS;

    /* We need to pull out the targeting options here, and
     store them away for future use */
    char* curArg = ecmdParseOptionWithArgs(argc, argv, "-trace=");

    /* Grab the quiet mode flag */
    if (ecmdParseOption(argc, argv, "-quiet"))
    {
        ecmdGlobal_quiet = 1;
    }
    else
    {
        /* Check for quiet mode env variable */
        const char* quiet_mode = getenv("ECMD_QUIETMODE");
        if ((quiet_mode != NULL) && (strncmp("quiet", quiet_mode, 5) == 0))
        {
            ecmdGlobal_quiet = 1;
        }
    }

    /* Grab the quiet error mode flag */
    if (ecmdParseOption(argc, argv, "-quieterror"))
    {
        ecmdGlobal_quietError = 1;
    }

    /* Grab the coe mode flag */
    if (ecmdParseOption(argc, argv, "-coe"))
    {
        ecmdGlobal_continueOnError = 1;
    }

    /* Grab the exist loop flag */
    if (ecmdParseOption(argc, argv, "-exist"))
    {
        ecmdGlobal_looperMode = ECMD_EXIST_LOOP;
    }

    /*************************************/
    /* Parse command line targeting args */
    /*************************************/

    /* This is left in for backwards comptability, preference is to use the
     * option below */
    bool allFound = false;
    if (ecmdParseOption(argc, argv, "-all"))
    {
        ecmdUserArgs.cage = "all";
        ecmdUserArgs.node = "all";
        ecmdUserArgs.slot = "all";
        ecmdUserArgs.pos = "all";
        ecmdUserArgs.chipUnitNum = "all";
        ecmdUserArgs.thread = "all";
        allFound = true;
    }

    // all targets
    curArg = ecmdParseOptionWithArgs(argc, argv, "-a");
    if (curArg)
    {
        ecmdUserArgs.cage = curArg;
        ecmdUserArgs.node = curArg;
        ecmdUserArgs.slot = curArg;
        ecmdUserArgs.pos = curArg;
        ecmdUserArgs.chipUnitNum = curArg;
        ecmdUserArgs.thread = curArg;
        allFound = true;
    }

    // Target Short-Hand:  For -k, -n, -s, -p, -c, -t options, look for ':' in
    // case multiple
    //  target fields were put together in 1 arg
    uint32_t l_find = 0;
    std::string l_tmp_string;

    // cage - the "-k" was Larry's idea, I just liked it -
    curArg = ecmdParseOptionWithArgs(argc, argv, "-k");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -k at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.cage = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-k");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    // node
    curArg = ecmdParseOptionWithArgs(argc, argv, "-n");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -n at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.node = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-n");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    // slot
    curArg = ecmdParseOptionWithArgs(argc, argv, "-s");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -s at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.slot = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-s");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    // position
    curArg = ecmdParseOptionWithArgs(argc, argv, "-p");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -p at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.pos = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-p");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    // chipUnit
    curArg = ecmdParseOptionWithArgs(argc, argv, "-c");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -c at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.chipUnitNum = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-c");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    // thread
    curArg = ecmdParseOptionWithArgs(argc, argv, "-t");
    if (curArg)
    {
        if (allFound)
        {
            dllOutputError(
                "dllCommonCommandArgs - Cannot specify -a target parm and -t at the same time\n");
            return ECMD_INVALID_ARGS;
        }
        else
        {
            // For target expansion, look for ':'
            l_tmp_string = curArg;
            l_find = l_tmp_string.find_first_of(":");
            if (l_find == std::string::npos)
            {
                // No ":" found - just set ecmdUserArgs target directly
                ecmdUserArgs.thread = curArg;
            }
            else
            {
                // Found ":"; Make sure there's something before first ':'
                if (l_find == 0)
                {
                    dllOutputError(
                        "dllCommonCommandArgs - No Target Info Found Before First ':'\n");
                    return ECMD_INVALID_ARGS;
                }
                else
                {
                    // Call Expansion function to process arg with ':'
                    rc = ecmdTargetExpansion(l_tmp_string, "-t");
                    if (rc)
                        return rc;
                }
            }
        }
    }

    /* Call the dllSpecificFunction */
    rc = dllSpecificCommandArgs(argc, argv);

    return rc;
}

/* @brief used by dllCommonCommandArgs when ":" found, sets ecmdUserArgs */
uint32_t ecmdTargetExpansion(std::string arg_string, const char* input_target)
{
    uint32_t rc = ECMD_SUCCESS;
    std::string l_tmp_string;

    // We know curArg has ':', so tokenize the string
    std::vector<std::string> tokens;
    std::vector<std::string>::iterator tokit;
    ecmdParseTokens(arg_string, ":", tokens);

    for (tokit = tokens.begin(); tokit != tokens.end(); tokit++)
    {
        if (tokit == tokens.begin())
        {
            // the first arg belongs to the input_target passed in
            if (!strcmp(input_target, "-k"))
                ecmdUserArgs.cage = tokit->c_str();
            else if (!strcmp(input_target, "-n"))
                ecmdUserArgs.node = tokit->c_str();
            else if (!strcmp(input_target, "-s"))
                ecmdUserArgs.slot = tokit->c_str();
            else if (!strcmp(input_target, "-p"))
                ecmdUserArgs.pos = tokit->c_str();
            else if (!strcmp(input_target, "-c"))
                ecmdUserArgs.chipUnitNum = tokit->c_str();
            else if (!strcmp(input_target, "-t"))
                ecmdUserArgs.thread = tokit->c_str();
        }
        else
        {
            // Set this here for string operations below
            l_tmp_string = tokit->c_str();

            // Look for additional targets
            // If found, remove first char of l_tmp_string and then set
            // ecmdUserArgs

            if (!strncmp(tokit->c_str(), "k", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.cage = l_tmp_string;
            }
            else if (!strncmp(tokit->c_str(), "n", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.node = l_tmp_string;
            }
            else if (!strncmp(tokit->c_str(), "s", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.slot = l_tmp_string;
            }
            else if (!strncmp(tokit->c_str(), "p", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.pos = l_tmp_string;
            }
            else if (!strncmp(tokit->c_str(), "c", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.chipUnitNum = l_tmp_string;
            }
            else if (!strncmp(tokit->c_str(), "t", 1))
            {
                l_tmp_string.erase(0, 1);
                ecmdUserArgs.thread = l_tmp_string;
            }
            else
            {
                dllOutputError(
                    "ecmdTargetExpansion - Found non-target data after ':'\n");
                return ECMD_INVALID_ARGS;
            }
        } // end of if (tokit == tokens.begin() ) check
    } // end of tokens for loop

    return rc;
}

void dllPushCommandArgs()
{
    ecmdArgsStack.push_back(ecmdUserArgs);
    ecmdUserArgs.cage = ecmdUserArgs.node = ecmdUserArgs.slot =
        ecmdUserArgs.pos = ecmdUserArgs.chipUnitNum = ecmdUserArgs.thread = "";
}

void dllPopCommandArgs()
{
    if (!ecmdArgsStack.empty())
    {
        ecmdUserArgs = ecmdArgsStack.back();
        ecmdArgsStack.pop_back();
    }
}

uint8_t removeCurrentElement(int curPos, std::string userArgs)
{
    uint8_t l_remove = 1;
    std::string curSubstr;
    size_t curOffset = 0;
    size_t nextOffset = 0;
    size_t tmpOffset = 0;

    while (curOffset < userArgs.length())
    {
        nextOffset = userArgs.find(',', curOffset);
        if (nextOffset == std::string::npos)
        {
            nextOffset = userArgs.length();
        }

        curSubstr = userArgs.substr(curOffset, nextOffset - curOffset);

        if ((tmpOffset = curSubstr.find("..", 0)) < curSubstr.length())
        {
            int lowerBound = atoi(curSubstr.substr(0, tmpOffset).c_str());
            int upperBound = atoi(
                curSubstr.substr(tmpOffset + 2, curSubstr.length()).c_str());

            if (lowerBound <= curPos && curPos <= upperBound)
            {
                l_remove = 0;
                break;
            }
        }
        else
        {
            int curValidPos = atoi(curSubstr.c_str());
            if (curValidPos == curPos)
            {
                l_remove = 0;
                break;
            }
        }

        curOffset = nextOffset + 1;
    }

    return l_remove;
}

/* Returns true if all chars of str are decimal numbers */
bool isValidTargetString(std::string& str)
{
    bool ret = true;

    /* This is the code that allows hex input positions on the command line */
    /* Since all of the cmdline target args are checked via this function, it's
     * the easiest place */
    /* to convert any hex numbers to decimal.  This will allow all the rest of
     * the code after this */
    /* point to behave as it currently does with decimal numbers - JTA 09/11/07
     */
    size_t startPos = str.find("0x");
    size_t endPos;
    char decimalString[10];
    size_t matches, decimalNum;

    while (startPos != std::string::npos)
    {
        endPos = str.find_first_of(
            ",.", startPos); // These are the special seperators
        matches = sscanf(str.substr((startPos + 2), endPos).c_str(), "%zx",
                         &decimalNum);
        if (!matches)
        { // sscanf didn't find anything
            return false;
        }
        /* Turn our number into a string and stick it back into the target
         * string */
        sprintf(decimalString, "%zd", decimalNum);
        str.replace(startPos, (endPos - startPos), decimalString);

        /* Find the next one, starting at the end of our replace */
        startPos = str.find("0x", (startPos + strlen(decimalString)));
    }

    for (uint32_t x = 0; x < str.length(); x++)
    {
        if (isdigit(str[x]))
        {}
        else if (str[x] == ',')
        {}
        else if (str[x] == '.' && str[x + 1] == '.')
        {
            x++;
        }
        else
        {
            ret = false;
            break;
        }
    }

    return ret;
}

uint32_t dllGetGlobalVar(ecmdGlobalVarType_t i_type)
{
    uint32_t ret = 0;

    if (i_type == ECMD_GLOBALVAR_QUIETMODE)
    {
        ret = ecmdGlobal_quiet;
    }
    else if (i_type == ECMD_GLOBALVAR_QUIETERRORMODE)
    {
        ret = ecmdGlobal_quietError;
    }
    else if (i_type == ECMD_GLOBALVAR_COEMODE)
    {
        ret = ecmdGlobal_continueOnError;
    }
    else if (i_type == ECMD_GLOBALVAR_LOOPMODE)
    {
        ret = ecmdGlobal_looperMode;
    }
    else if (i_type == ECMD_GLOBALVAR_CMDLINEMODE)
    {
        ret = ecmdGlobal_cmdLineMode;
    }

    return ret;
}

uint32_t dllSetGlobalVar(ecmdGlobalVarType_t i_type, uint32_t i_value)
{
    uint32_t rc = ECMD_SUCCESS;

    if (i_type == ECMD_GLOBALVAR_QUIETMODE)
    {
        ecmdGlobal_quiet = i_value;
    }
    else if (i_type == ECMD_GLOBALVAR_QUIETERRORMODE)
    {
        ecmdGlobal_quietError = i_value;
    }
    else if (i_type == ECMD_GLOBALVAR_COEMODE)
    {
        ecmdGlobal_continueOnError = i_value;
    }
    else if (i_type == ECMD_GLOBALVAR_LOOPMODE)
    {
        ecmdGlobal_looperMode = i_value;
    }
    else if (i_type == ECMD_GLOBALVAR_CMDLINEMODE)
    {
        ecmdGlobal_cmdLineMode = i_value;
    }
    else
    {
        return ECMD_INVALID_ARGS;
    }

    return rc;
}

std::string dllParseReturnCode(uint32_t)
{
    return {};
}

/* TargetConfiged and TargetExist are the same function, except for having to
 * call a different interface */
/* Wrapping them this way, to call the internal queryTargetConfiguredExist is
 * the most efficient way */
bool dllQueryTargetConfigured(const ecmdChipTarget& target,
                              const ecmdQueryData* i_queryData)
{
    return queryTargetConfigExist(target, i_queryData, false);
}

bool dllQueryTargetExist(const ecmdChipTarget& target,
                         const ecmdQueryData* i_queryData)
{
    return queryTargetConfigExist(target, i_queryData, true);
}

bool queryTargetConfigExist(const ecmdChipTarget& target,
                            const ecmdQueryData* i_queryData, bool i_existQuery)
{
    uint32_t rc = ECMD_SUCCESS;
    bool ret = false;
    ecmdChipTarget queryTarget;
    const ecmdQueryData* l_queryData = i_queryData;
    ecmdQueryData* l_myQueryData = NULL;

    std::list<ecmdCageData>::const_iterator ecmdCurCage;
    std::list<ecmdNodeData>::const_iterator ecmdCurNode;
    std::list<ecmdSlotData>::const_iterator ecmdCurSlot;
    std::list<ecmdChipData>::const_iterator ecmdCurChip;
    std::list<ecmdChipUnitData>::const_iterator ecmdCurChipUnit;
    std::list<ecmdThreadData>::const_iterator ecmdCurThread;

    /* Do we need to do our own query ? */
    if (l_queryData == NULL)
    {
        l_myQueryData = new ecmdQueryData;
        queryTarget = target;

        /* Force the states to be right, if not set properly */
        if (queryTarget.cageState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.cageState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.nodeState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.nodeState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.slotState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.slotState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.chipTypeState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.chipTypeState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.posState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.posState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.chipUnitTypeState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.chipUnitTypeState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.chipUnitNumState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.chipUnitNumState = ECMD_TARGET_FIELD_VALID;
        if (queryTarget.threadState != ECMD_TARGET_FIELD_UNUSED)
            queryTarget.threadState = ECMD_TARGET_FIELD_VALID;

        if (i_existQuery)
        {
            rc = dllQueryExist(queryTarget, *l_myQueryData,
                               ECMD_QUERY_DETAIL_LOW);
        }
        else
        {
            rc = dllQueryConfig(queryTarget, *l_myQueryData,
                                ECMD_QUERY_DETAIL_LOW);
        }
        if (rc)
        {
            delete l_myQueryData;
            return ret;
        }
        l_queryData = l_myQueryData;
    }

    /* Now we have our data, let's start walking the data we have */
    for (ecmdCurCage = l_queryData->cageData.begin();
         ecmdCurCage != l_queryData->cageData.end(); ecmdCurCage++)
    {
        if (ecmdCurCage->cageId == target.cage)
        {
            if (target.nodeState == ECMD_TARGET_FIELD_UNUSED)
            {
                ret = true;
                break;
            }

            for (ecmdCurNode = ecmdCurCage->nodeData.begin();
                 ecmdCurNode != ecmdCurCage->nodeData.end(); ecmdCurNode++)
            {
                if (ecmdCurNode->nodeId == target.node)
                {
                    if (target.slotState == ECMD_TARGET_FIELD_UNUSED)
                    {
                        ret = true;
                        break;
                    }

                    for (ecmdCurSlot = ecmdCurNode->slotData.begin();
                         ecmdCurSlot != ecmdCurNode->slotData.end();
                         ecmdCurSlot++)
                    {
                        if (ecmdCurSlot->slotId == target.slot)
                        {
                            if (target.chipTypeState ==
                                    ECMD_TARGET_FIELD_UNUSED ||
                                target.posState == ECMD_TARGET_FIELD_UNUSED)
                            {
                                ret = true;
                                break;
                            }

                            for (ecmdCurChip = ecmdCurSlot->chipData.begin();
                                 ecmdCurChip != ecmdCurSlot->chipData.end();
                                 ecmdCurChip++)
                            {
                                if (((ecmdCurChip->chipType ==
                                      target.chipType) ||
                                     (ecmdCurChip->chipCommonType ==
                                      target.chipType) ||
                                     (ecmdCurChip->chipShortType ==
                                      target.chipType)) &&
                                    (ecmdCurChip->pos == target.pos))
                                {
                                    if (target.chipUnitNumState ==
                                        ECMD_TARGET_FIELD_UNUSED)
                                    {
                                        ret = true;
                                        break;
                                    }

                                    for (ecmdCurChipUnit =
                                             ecmdCurChip->chipUnitData.begin();
                                         ecmdCurChipUnit !=
                                         ecmdCurChip->chipUnitData.end();
                                         ecmdCurChipUnit++)
                                    {
                                        if (ecmdCurChipUnit->chipUnitNum ==
                                            target.chipUnitNum)
                                        {
                                            if (target.threadState ==
                                                ECMD_TARGET_FIELD_UNUSED)
                                            {
                                                ret = true;
                                                break;
                                            }

                                            for (ecmdCurThread =
                                                     ecmdCurChipUnit->threadData
                                                         .begin();
                                                 ecmdCurThread !=
                                                 ecmdCurChipUnit->threadData
                                                     .end();
                                                 ecmdCurThread++)
                                            {
                                                if (ecmdCurThread->threadId ==
                                                    target.thread)
                                                {
                                                    ret = true;
                                                    break;
                                                } /* curThreadId == tarThreadId
                                                   */
                                            } /* for ecmdCurThread */

                                            if (ret)
                                                break;
                                        } /* curChipUnitNum == tarChipUnitId */
                                    } /* for ecmdCurChipUnit */

                                    if (ret)
                                        break;
                                } /* curChipType == tarChipType && curChipPos ==
                                     tarChipPos */
                            } /* for ecmdCurChip */

                            if (ret)
                                break;
                        } /* curSlotId == tarSlotId */
                    } /* for ecmdCurSlot */

                    if (ret)
                        break;
                } /* curNodeId == tarNodeId */
            } /* for ecmdCurNode */

            if (ret)
                break;
        } /* curCageId == tarCageId */
    } /* for ecmdCurCage */

    if (l_myQueryData)
    {
        delete l_myQueryData;
    }

    return ret;
}

/**
 @brief Get Current Cmdline String
 @retval String representing current cmdline string being processed
*/
std::string dllGetCurrentCmdline()
{
    return ecmdGlobal_currentCmdline;
}

/**
 @brief Convert list of Cmdline Args to String and Save in Dll
 @param argc Command line arguments
 @param argv Command line arguments
*/
void dllSetCurrentCmdline(int argc, char* argv[])
{
    // new string coming in, so erase/clear what was there first
    ecmdGlobal_currentCmdline = "";

    // now create new string from argv[] array of size argc
    for (int i = 0; i < argc; i++)
    {
        ecmdGlobal_currentCmdline += argv[i];
        ecmdGlobal_currentCmdline += " ";
    }
}

void dllOutputError(const char* err)
{
    lg2::error("error {ERR}", "ERR", err);
}

void dllOutputWarning(const char* err)
{
    lg2::warning("warning {ERR}", "ERR", err);
}

void dllOutput(const char* msg)
{
    lg2::info("msg {MSG}", "MSG", msg);
}

uint32_t dllDelay(uint32_t, uint32_t msDelay)
{
    uint32_t rc = usleep(msDelay * 1000);
    if (rc != 0)
    {
        lg2::error("dllDelay usleep failed");
    }
    return rc;
}
} //extern "C"
