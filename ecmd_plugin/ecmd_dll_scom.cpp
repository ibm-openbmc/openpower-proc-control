#include <ecmdDataBuffer.H>
#include <ecmdDllCapi.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>

#include <cstdint>
#include <iostream>

uint32_t dllGetScom(ecmdChipTarget& /*target*/, uint64_t address,
                    ecmdDataBuffer& /*data*/)
{
    std::cout << "ecmd_dll_scom: dllGetScom address 0x" << std::hex << address
              << std::endl;
    return ECMD_SUCCESS;
}

uint32_t dllPutScom(const ecmdChipTarget& /*target*/, uint64_t address,
                    const ecmdDataBuffer& /*data*/)
{
    std::cout << "ecmd_dll_scom: dllPutScom address 0x" << std::hex << address
              << std::endl;
    return ECMD_SUCCESS;
}
