#include <ecmd_dll_capi.H>

#include <ecmdDataBuffer.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>

#include <phosphor-logging/lg2.hpp>

#include <cstdint>
extern "C"
{
std::string dllGetErrorMsg(uint32_t, bool, bool, bool)
{
    lg2::error("dllGetErrorMsg is not implemented");
    return {};
}

uint32_t dllRegisterErrorMsg(uint32_t, const char*, const char*)
{
    lg2::error("dllRegisterErrorMsg is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorMsgs(uint32_t)
{
    lg2::error("dllFlushRegisteredErrorMsgs is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorMsgsString(uint32_t, std::string)
{
    lg2::error("dllFlushRegisteredErrorMsgsString is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllGetErrorTarget(uint32_t, std::list<ecmdChipTarget>&, bool)
{
    lg2::error("dllGetErrorTarget is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllRegisterErrorTarget(uint32_t, const ecmdChipTarget&)
{
    lg2::error("dllRegisterErrorTarget is not implemented");
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorTargets(uint32_t)
{
    lg2::error("dllFlushRegisteredErrorTargets is not implemented");
    return ECMD_SUCCESS;
}

} //extern "C"
