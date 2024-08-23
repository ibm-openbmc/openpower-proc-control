// SPDX-License-Identifier: Apache-2.0
#include "sibling_cfam.hpp"

#include <phosphor-logging/lg2.hpp>

void SiblingCFAM::readAll()
{
    auto regs = cfamAccess.readScratchRegs(usedRegs);

    if (!regs.has_value())
    {
        error = regs.error();
        lg2::error("Failure in readAllScratchRegs: {ERR}", "ERR", *error);
        return;
    }

    error = std::nullopt;
    apiVersion = cfam::getFieldValue(*regs, cfamFields.at(Field::apiVersion));

    fwVersion = cfam::getFieldValue(*regs, cfamFields.at(Field::fwVersion));

    bmcPosition = cfam::getFieldValue(*regs, cfamFields.at(Field::bmcPosition));

    provisioned = cfam::getFieldValue(*regs, cfamFields.at(Field::provisioned));

    heartbeat = cfam::getFieldValue(*regs, cfamFields.at(Field::heartbeat));

    redundancyEnabled =
        cfam::getFieldValue(*regs, cfamFields.at(Field::redundancyEnabled));

    failoversPaused =
        cfam::getFieldValue(*regs, cfamFields.at(Field::failoversPaused));

    auto r = cfam::getFieldValue(*regs, cfamFields.at(Field::role));
    role = static_cast<Role>(r);

    auto s = cfam::getFieldValue(*regs, cfamFields.at(Field::bmcState));
    bmcState = static_cast<BMCState>(s);

    siblingCommsOK =
        cfam::getFieldValue(*regs, cfamFields.at(Field::siblingCommsOK));
}

uint8_t SiblingCFAM::getApiVersion() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return apiVersion;
}

uint32_t SiblingCFAM::getFWVersion() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return fwVersion;
}

uint32_t SiblingCFAM::getBMCPosition() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return bmcPosition;
}

bool SiblingCFAM::getProvisioned() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return provisioned;
}

bool SiblingCFAM::getRedundancyEnabled() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return redundancyEnabled;
}

bool SiblingCFAM::getFailoversPaused() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return failoversPaused;
}

SiblingCFAM::BMCState SiblingCFAM::getBMCState() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return bmcState;
}

SiblingCFAM::Role SiblingCFAM::getRole() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return role;
}

bool SiblingCFAM::getSiblingCommsOK() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }
    return siblingCommsOK;
}

uint32_t SiblingCFAM::getHeartbeat() const
{
    if (error)
    {
        throw std::runtime_error{"CFAM fields not available"};
    }

    return heartbeat;
}
