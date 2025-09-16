// SPDX-License-Identifier: Apache-2.0
#include "cfam_access.hpp"

#include "paths.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <thread>
#include <utility>

namespace fs = std::filesystem;
constexpr size_t maxAttempts = 10;

bool CFAMAccess::exists()
{
    if (devicePath.empty())
    {
        devicePath = findDevicePath();
    }
    return !devicePath.empty() && std::filesystem::exists(devicePath);
}

fs::path CFAMAccess::findDevicePath() const
{
    auto path = paths::getFSIMasterDir() /
                std::format("fsi{}/slave@00:00/{:02}:00:00:13", link, link);

    if (!fs::exists(path))
    {
        lg2::debug("{PATH} doesn't exist!", "PATH", path);
        return {};
    }

    try
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.path().filename().string().starts_with("mbox-cfam-s"))
            {
                // Return the path in /dev
                return paths::getDeviceDir() / entry.path().filename();
            }
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Failure iterating {PATH}: {ERROR}", "PATH", path, "ERROR",
                   e);
    }
    return {};
}

std::expected<uint32_t, int> CFAMAccess::readScratchReg(cfam::ScratchPadReg reg)
{
    size_t attempts = 0;
    std::expected<uint32_t, int> result;

    while (attempts < maxAttempts)
    {
        result = driver.read(devicePath, std::to_underlying(reg));
        if (!result.has_value())
        {
            lg2::error(
                "readScratchReg failed on {PATH} reg {REG} with errno {ERROR}",
                "PATH", devicePath, "REG", std::to_underlying(reg), "ERROR",
                result.error());
            attempts++;

            if (attempts < maxAttempts)
            {
                lg2::warning("Retrying read");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        else
        {
            break;
        }
    }

    return result;
}

CFAMAccess::RegMapExpected CFAMAccess::readScratchRegs(
    const std::set<cfam::ScratchPadReg>& regNames)
{
    cfam::RegMap regs;

    for (const auto& reg : regNames)
    {
        auto data = readScratchReg(reg);
        if (!data.has_value())
        {
            lg2::error("Error {ERR} on reg {REG}", "ERR", data.error(), "REG",
                       reg);
            return std::unexpected{data.error()};
        }

        regs[reg] = data.value();
    }

    return regs;
}

int CFAMAccess::writeScratchReg(cfam::ScratchPadReg reg, uint32_t data)
{
    size_t attempts = 0;
    int rc = 0;

    while (attempts < maxAttempts)
    {
        rc = driver.write(devicePath, std::to_underlying(reg), data);

        if (rc != 0)
        {
            lg2::error(
                "writeScratchReg failed on {PATH} reg {REG} with error {ERROR}",
                "PATH", devicePath, "REG", std::to_underlying(reg), "ERROR",
                rc);
            attempts++;

            if (attempts < maxAttempts)
            {
                lg2::warning("Retrying write");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        else
        {
            break;
        }
    }

    return rc;
}

int CFAMAccess::writeScratchRegWithMask(const cfam::ModifyOp& op)
{
    size_t attempts = 0;
    int rc = 0;

    while (attempts < maxAttempts)
    {
        rc = driver.writeWithMask(devicePath, std::to_underlying(op.reg),
                                  op.data, op.mask);
        if (rc != 0)
        {
            lg2::error(
                "writeScratchRegWithMask failed on {PATH} reg {REG} with errno {ERROR}",
                "PATH", devicePath, "REG", std::to_underlying(op.reg), "ERROR",
                rc);
            attempts++;

            if (attempts < maxAttempts)
            {
                lg2::warning("Retrying writeWithMask");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        else
        {
            break;
        }
    }

    return rc;
}
