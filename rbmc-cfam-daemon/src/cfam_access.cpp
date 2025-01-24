// SPDX-License-Identifier: Apache-2.0
#include "cfam_access.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <unordered_map>

namespace fs = std::filesystem;

const std::unordered_map<cfam::ScratchPadReg, std::string> scratchPadRegNames{
    {cfam::ScratchPadReg::one, "scratch1"},
    {cfam::ScratchPadReg::two, "scratch2"},
    {cfam::ScratchPadReg::three, "scratch3"},
    {cfam::ScratchPadReg::four, "scratch4"}};

fs::path CFAMAccess::getRegisterPath(cfam::ScratchPadReg reg) const
{
    return std::format("/sys/class/fsi-master/fsi{}/slave@00:00/{}", link,
                       scratchPadRegNames.at(reg));
}

bool CFAMAccess::exists() const
{
    auto path = getRegisterPath(cfam::ScratchPadReg::one);

    try
    {
        return fs::exists(path);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed calling filesystem::exists on {FILE}: {ERR}", "FILE",
                   path, "ERR", e);
    }
    return false;
}

std::expected<uint32_t, int> CFAMAccess::readScratchReg(cfam::ScratchPadReg reg)
{
    return driver.read(getRegisterPath(reg));
}

CFAMAccess::RegMapExpected
    CFAMAccess::readScratchRegs(const std::set<cfam::ScratchPadReg>& regNames)
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
    return driver.write(getRegisterPath(reg), data);
}

int CFAMAccess::writeScratchRegWithMask(const cfam::ModifyOp& op)
{
    auto regData = readScratchReg(op.reg);
    if (!regData.has_value())
    {
        return regData.error();
    }

    // Clear all bits in the field
    *regData &= ~op.mask;

    // Now set the bits
    *regData |= (op.data & op.mask);

    return writeScratchReg(op.reg, *regData);
}
