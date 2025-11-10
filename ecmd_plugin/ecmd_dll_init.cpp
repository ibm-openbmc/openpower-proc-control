#include <ecmdDllCapi.H>
#include <ecmdReturnCodes.H>
#include <targeting/target_service.H>

constexpr std::string_view envVar = "ECMD_DLL_FILE";
constexpr std::string_view defaultPath = "/usr/lib/libecmd_plugin.so";

// Load the device tree and initialise the targets
static int initTargets(void)
{
    uint32_t rc = ECMD_SUCCESS;

    try
    {
        /*TODO p12-refactor: fix device tree path*/
        TARGETING::TargetService::instance().init("/tmp/targeting_test.dtb");
    }
    catch (std::exception& ex)
    {
        rc = ECMD_TARGET_NOT_CONFIGURED;
        std::cout << "exception raised " << ex.what() << std::endl;
    }
    return rc;
}

uint32_t dllInitDll()
{
    uint32_t rc = ECMD_SUCCESS;
    rc = initTargets();
    if (rc)
    {
        std::cerr << "Failed to dllInitDll \n";
        return rc;
    }

    if (std::getenv(envVar.data()) == nullptr)
    {
        if (setenv(envVar.data(), defaultPath.data(), 1) != 0)
        {
            std::string err =
                std::format("Failed to set environment variable {} to {}",
                            envVar, defaultPath);
            std::cout << err << std::endl;
            return ECMD_INVALID_DLL_FILENAME;
        }
        std::cout << std::format("Set {}={}\n", envVar, defaultPath);
    }
    else
    {
        std::cout << std::format("{} already set to {}\n", envVar,
                                 std::getenv(envVar.data()));
    }
    return rc;
}

uint32_t dllFreeDll()
{
    return ECMD_SUCCESS;
}
