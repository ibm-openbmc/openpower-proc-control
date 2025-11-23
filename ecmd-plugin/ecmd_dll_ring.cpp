#include <ecmdDataBuffer.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <ecmd_dll_capi.H>

#include <phosphor-logging/lg2.hpp>

#include <cstdint>
extern "C"
{
bool dllIsRingCacheEnabled(const ecmdChipTarget&)
{
    lg2::error("dllIsRingCacheEnabled is not implemented");
    return false;
}
}
