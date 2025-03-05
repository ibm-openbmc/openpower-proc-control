// SPDX-License-Identifier: Apache-2.0
#include "cfam_access.hpp"

#include "paths.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <utility>

namespace fs = std::filesystem;

bool CFAMAccess::exists()
{
    if (devicePath.empty())
    {
        findDevicePath();
    }
    return !devicePath.empty();
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
    return driver.read(devicePath, std::to_underlying(reg));
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
    return driver.write(devicePath, std::to_underlying(reg), data);
}

int CFAMAccess::writeScratchRegWithMask(const cfam::ModifyOp& op)
{
    return driver.writeWithMask(devicePath, std::to_underlying(op.reg), op.data,
                                op.mask);
}
