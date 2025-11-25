#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <unistd.h>
extern "C"
{
#include <libpdbg.h>
}
#include <phosphor-logging/lg2.hpp>

extern "C"
{
uint32_t dllInitDll()
{
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
    return ECMD_SUCCESS;
}
uint32_t dllLoadDll(const char*, uint32_t)
{
    return dllInitDll();
}

uint32_t dllUnloadDll()
{
    uint32_t rc = 0;
    rc = dllFreeDll();
    return rc;
}

void dllOutputError(const char* err)
{
    lg2::error("error {ERR}", "ERR", err);
}

void dllOutputWarning(const char* err)
{
    lg2::warning("warning {ERR}", "ERR", err);
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
//---------------------------------------------------------------------
// optional methods the plugin can choose to add implementation
//---------------------------------------------------------------------
void dllOutput(const char*) {}

uint32_t dllGetGlobalVar(ecmdGlobalVarType_t)
{
    return ECMD_SUCCESS;
}
uint32_t dllQueryDllInfo(ecmdDllInfo&)
{
    return ECMD_SUCCESS;
}

uint32_t dllCheckDllVersion(const char*)
{
    return ECMD_SUCCESS;
}

bool dllQueryVersionGreater(const char*)
{
    return false;
}

std::string dllGetCurrentCmdline()
{
    return {};
}

void dllSetCurrentCmdline(int, [[maybe_unused]] char* argv[]) {}

uint32_t dllSetGlobalVar(ecmdGlobalVarType_t, uint32_t)
{
    return ECMD_SUCCESS;
}

uint32_t dllCommonCommandArgs(int*, [[maybe_unused]] char** argv[])
{
    return ECMD_SUCCESS;
}
} // extern "C"
