#include <ecmdDataBuffer.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <libipl.H>

#include <ecmd_util.hpp>
#include <istep_table.hpp>
#include <phosphor-logging/lg2.hpp>

extern "C"
{
uint32_t executeIstep(uint16_t major, uint16_t minorStart, uint16_t minorEnd)
{
    lg2::info(
        "executeIstep major={MAJOR} minorstart={MINOR_START} minorend={MINOR_END} ",
        "MAJOR", major, "MINOR_START", minorStart, "MINOR_END", minorEnd);
    using namespace istep_table;
    uint32_t rc = ECMD_SUCCESS;

    // If istep is 0 then, run chassis on and other workaround steps before
    // kick off ipl_run_major_minor() in loop
    if (major == 0)
    {
        /* istep power on */
        rc = ecmd_util::istepPowerOn();
        if (!rc)
        {
            // Set IPL mode to interactive
            rc = ipl_init(IPL_HOSTBOOT);
            if (rc)
            {
                lg2::error("Unable to set IPL in interactive mode");
                return rc;
            }
        }
        else
        {
            lg2::error("FAIL: istepPowerOn");
            return rc;
        }
    } // major ==0
    /* loop through each isteps */
    for (uint16_t minor = minorStart; minor <= minorEnd; minor++)
    {
        auto istepNameOpt = istep_table::getStepName(major, minor);
        if (!istepNameOpt)
        {
            lg2::error("Invalid istep major={MAJOR} minor={MINOR}", "MAJOR",
                       major, "MINOR", minor);
            continue;
        }
        IStepDestination destination = getDestination(major, minor);

        // This istep is NOOP
        if (destination == IStepDestination::EDBG_ISTEP_NOOP)
        {
            lg2::error("Requested istep {STEP} is noop", "STEP", *istepNameOpt);
        }
        else
        {
            /* kick off isteps */
            rc = ipl_run_major_minor(major, minor);
            if (!rc)
            {
                if (major == 6 && minor == 4)
                {
                    // if the istep reaches 6.4 then, set the Host state to
                    // running!
                    rc = ecmd_util::setHostStateToRunning();
                    if (rc != ECMD_SUCCESS)
                    {
                        lg2::error("FAIL: failed to set host state");
                        return rc;
                    }
                }
                lg2::error("PASS: istep {STEP}", "STEP", *istepNameOpt);
            }
            else
            {
                lg2::error("FAIL: istep check Error {STEP}", "STEP",
                           *istepNameOpt);
                return rc;
            }
        }
    } // end for
    return rc;
}

uint32_t dllIStepsByNumber(const ecmdDataBuffer& isteps)
{
    using namespace istep_table;

    uint32_t rc = ECMD_SUCCESS;

    do // Start of single exit point loop.
    {
        /****************************************************************************************/
        /* First, check to make sure at least 1 bit is active in i_steps
         * databuffer, then....   */
        /* For each valid bit in the i_steps databuffer: */
        /*   1)  find the index entry #s that start that step and end that
         * step */
        /*   2)  Call iStepsHelper()  multiple times, starting with starting
         * index entry #    */
        /*        and ending with the ending entry #. */
        /****************************************************************************************/
        // check to see if at least 1 bit in the range is active
        uint32_t count = isteps.getNumBitsSet(
            EDBG_FIRST_ISTEP_NUM,
            EDBG_LAST_ISTEP_NUM - EDBG_FIRST_ISTEP_NUM + 1);
        if (count == 0)
        {
            rc = ECMD_INVALID_ARGS;
            lg2::error("No Steps in active range selected {FIRST} to {LAST}",
                       "FIRST", EDBG_FIRST_ISTEP_NUM, "LAST",
                       EDBG_LAST_ISTEP_NUM);
            break; // exit do-loop
        }
        else
        {
            lg2::debug("number of isteps bits set is {COUNT}", "COUNT", count);
        }

        uint16_t activeStep = EDBG_INVALID_ISTEP_NUM;
        /* Large 'for' loop that goes through i_steps looking for active
         * steps start at begining of range */
        for (activeStep = EDBG_FIRST_ISTEP_NUM;
             activeStep <= EDBG_LAST_ISTEP_NUM && rc == ECMD_SUCCESS;
             ++activeStep)
        {
            if (isteps.isBitSet(activeStep)) // this is an active step
            {
                /*  look IPLTable for the existence of this istep number */
                if (false == isValid(activeStep))
                {
                    /* this is only warning, as the value is in the range,
                     * but isn't being used */
                    lg2::error("Requested iStep Number {STEP} is invalid",
                               "STEP", activeStep);
                    continue;
                }

                /* 1a) Lookup first index entry of this active step */
                uint16_t indexBegin = getPosFirstMinorNumber(activeStep);
                uint16_t minorStart = getIStepMinorNumber(indexBegin);

                /* 1b) Starting with indexBegin,
                 *     lookup last index entry of this active step */
                uint16_t indexEnd = getPosLastMinorNumber(activeStep);
                uint16_t minorEnd = getIStepMinorNumber(indexEnd);

                /* 2) Call iStepsHelper()  multiple times,
                 *    starting with starting index entry #
                 *    and ending with the ending entry #. */
                /* kick off isteps */
                rc = executeIstep(activeStep, minorStart, minorEnd);
                if (rc)
                {
                    lg2::error("error in executeIstep");
                    break;
                }
            } /*  end of 'if' active step check */
        } /* end of for loop going through active i_steps */
    } while (0);

    return rc;
}

uint32_t dllIStepsByName(std::string stepName)
{
    lg2::info("dllIStepsByName name {STEP} not implemented", "STEP", stepName);
    return ECMD_SUCCESS;
}

uint32_t dllIStepsByNameMultiple(std::list<std::string>)
{
    lg2::info("dllIStepsByNameMultiple not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllIStepsByNameRange(std::string begin, std::string end)
{
    // TODO: Refer to edbgEcmdDll.C and add relevant code here
    lg2::info("dllIStepsByNameRange {BEGIN} to {END}", "BEGIN", begin, "END",
              end);
    lg2::error("dllIStepsByNameRange not implemented");
    return ECMD_FUNCTION_NOT_SUPPORTED;
}
} // extern "C"
