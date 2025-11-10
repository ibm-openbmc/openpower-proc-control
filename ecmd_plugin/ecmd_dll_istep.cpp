#include <ecmdDataBuffer.H>
#include <ecmdDllCapi.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <libipl.H>

#include <ecmd_util.hpp>
#include <istep_table.hpp>

#include <cstdint>
#include <iostream>

uint32_t executeIstep(uint16_t major, uint16_t minorStart, uint16_t minorEnd)
{
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
                std::cerr << "Unable to set IPL in interactive mode\n";
                return rc;
            }
        }
        else
        {
            std::cerr << "FAIL: istepPowerOn\n";
            return rc;
        }
    } // major ==0
    /* loop through each isteps */
    for (uint16_t minor = minorStart; minor <= minorEnd; minor++)
    {
        auto istepNameOpt = istep_table::getStepName(major, minor);
        if (!istepNameOpt)
        {
            std::cerr << "Invalid istep " << major << "." << minor << std::endl;
            continue;
        }
        IStepDestination destination = getDestination(major, minor);

        // This istep is NOOP
        if (destination == IStepDestination::EDBG_ISTEP_NOOP)
        {
            std::cout << "Requested istep %s is NOOP " << *istepNameOpt
                      << std::endl;
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
                        std::cerr << "FAIL: failed to set host state\n";
                        return rc;
                    }
                }
                std::cout << " PASS: istep  " << *istepNameOpt << std::endl;
            }
            else
            {
                std::cerr << "  FAIL: istep check Error " << *istepNameOpt
                          << std::endl;
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
    uint16_t indexBegin, minorStart = EDBG_INVALID_POSITION;
    uint16_t indexEnd, minorEnd = EDBG_INVALID_POSITION;
    uint16_t activeStep = EDBG_INVALID_ISTEP_NUM;

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
        if (isteps.getNumBitsSet(
                EDBG_FIRST_ISTEP_NUM,
                EDBG_LAST_ISTEP_NUM - EDBG_FIRST_ISTEP_NUM + 1))
        {
            rc = ECMD_INVALID_ARGS;
            std::cerr
                << "dllIStepsByNumber: No Steps in active range selected. "
                << "Range start: " << EDBG_FIRST_ISTEP_NUM
                << " end: " << EDBG_LAST_ISTEP_NUM;
            break; // exit do-loop
        }

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
                    printf(
                        "dllIStepsByNumber: Requested iStep Number %d is invalid.\n",
                        activeStep);
                    continue;
                }
                /* 1a) Lookup first index entry of this active step */
                indexBegin = getPosFirstMinorNumber(activeStep);
                minorStart = getIStepMinorNumber(indexBegin);

                /* 1b) Starting with indexBegin,
                 *     lookup last index entry of this active step */
                indexEnd = getPosLastMinorNumber(activeStep);
                minorEnd = getIStepMinorNumber(indexEnd);

                /* 2) Call iStepsHelper()  multiple times,
                 *    starting with starting index entry #
                 *    and ending with the ending entry #. */
                /* kick off isteps */
                rc = executeIstep(activeStep, minorStart, minorEnd);

            } /*  end of 'if' active step check */

        } /* end of for loop going through active i_steps */

    } while (0);

    return rc;
}

uint32_t dllIStepsByName(std::string /*stepName*/)
{
    return ECMD_SUCCESS;
}
