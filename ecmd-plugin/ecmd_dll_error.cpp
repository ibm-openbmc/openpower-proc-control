#include <ecmdDataBuffer.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>

#include <phosphor-logging/lg2.hpp>

#include <cstdint>
extern "C"
{
//---------------------------------------------------------------------
// optional methods the plugin can choose to add implementation
//---------------------------------------------------------------------
std::string dllGetErrorMsg(uint32_t, bool, bool, bool)
{
    return {};
}

uint32_t dllRegisterErrorMsg(uint32_t, const char*, const char*)
{
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorMsgs(uint32_t)
{
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorMsgsString(uint32_t, std::string)
{
    return ECMD_SUCCESS;
}

uint32_t dllGetErrorTarget(uint32_t, std::list<ecmdChipTarget>&, bool)
{
    return ECMD_SUCCESS;
}

uint32_t dllRegisterErrorTarget(uint32_t, const ecmdChipTarget&)
{
    return ECMD_SUCCESS;
}

uint32_t dllFlushRegisteredErrorTargets(uint32_t)
{
    return ECMD_SUCCESS;
}

} // extern "C"
