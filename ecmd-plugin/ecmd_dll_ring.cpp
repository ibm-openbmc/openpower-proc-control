#include <ecmdDataBuffer.H>
#include <ecmdStructs.H>

#include <phosphor-logging/lg2.hpp>

#include <cstdint>
extern "C"
{
//---------------------------------------------------------------------
// optional methods the plugin can choose to add implementation
//---------------------------------------------------------------------
bool dllIsRingCacheEnabled(const ecmdChipTarget&)
{
    return false;
}
}
