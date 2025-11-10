#include <ecmdDataBuffer.H>
#include <ecmdDllCapi.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <hwaccess/hw_access_intf.H>
#include <targeting/target.H>

#include <ecmd_util.hpp>

#include <cstdint>
#include <iostream>

uint32_t dllGetCfamRegister(const ecmdChipTarget& target, uint32_t address,
                            ecmdDataBuffer& ecmdData)
{
    std::cout << "ecmd_dll_cfam: dllGetCfamRegister address 0x" << std::hex
              << address << std::endl;
    int rc = ECMD_FAILURE;
    try
    {
        auto procTarget = ecmd_util::getProcTargetByPos(target.pos);
        uint32_t data = 0;
        if (!procTarget)
        {
            std::cerr << "Failed to find processor target for position "
                      << target.pos << "\n";
            return ECMD_FAILURE;
        }
        int rc =
            hwaccess::HwAccessIntf::getCfamRegister(procTarget, address, data);
        if (rc)
        {
            std::cout << "Failed in getCfamRegister \n";
            return rc;
        }
        ecmdData.setBitLength(32);
        ecmdData.setWord(0, data);
        return ECMD_SUCCESS;
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return rc;
}

uint32_t dllPutCfamRegister(const ecmdChipTarget& target, uint32_t address,
                            const ecmdDataBuffer& data)
{
    std::cout << "ecmd_dll_cfam: dllPutCfamRegister address 0x" << std::hex
              << address << std::endl;
    int rc = ECMD_FAILURE;
    try
    {
        auto procTarget = ecmd_util::getProcTargetByPos(target.pos);
        if (!procTarget)
        {
            std::cerr << "Failed to find processor target for position "
                      << target.pos << "\n";
            return ECMD_FAILURE;
        }
        int rc = hwaccess::HwAccessIntf::putCfamRegister(procTarget, address,
                                                         data.getWord(0));
        if (rc)
        {
            std::cout << "Failed in getCfamRegister \n";
            return rc;
        }
        return ECMD_SUCCESS;
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return rc;
}
