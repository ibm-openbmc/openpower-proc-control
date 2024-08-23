// SPDX-License-Identifier: Apache-2.0
#include "local_cfam.hpp"

#include <phosphor-logging/lg2.hpp>

#include <ranges>

namespace local_cfam_util
{
uint32_t getMaxValue(size_t numBits)
{
    uint32_t maxValue = 0;

    size_t start = 32 - numBits;
    for (auto i : std::views::iota(start, start + numBits))
    {
        maxValue |= (0x80000000 >> i);
    }

    return maxValue;
}
} // namespace local_cfam_util

int LocalCFAM::writeField(Field field, uint32_t value)
{
    auto op = cfam::createModifyOp(cfamFields.at(field), value);

    return cfamAccess.writeScratchRegWithMask(op);
}

void LocalCFAM::writeApiVersion(uint8_t version)
{
    auto rc = writeField(Field::apiVersion, version);
    if (rc != 0)
    {
        lg2::error("Failed writing API version {VERSION} in local CFAM",
                   "VERSION", version);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeFWVersion(uint32_t version)
{
    auto rc = writeField(Field::fwVersion, version);
    if (rc != 0)
    {
        lg2::error("Failed writing FW version {VERSION} in local CFAM",
                   "VERSION", lg2::hex, version);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeBMCPosition(uint8_t position)
{
    auto rc = writeField(Field::bmcPosition, position);
    if (rc != 0)
    {
        lg2::error("Failed writing BMC position {POSITION} in local CFAM",
                   "POSITION", position);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeProvisioned(bool isProvisioned)
{
    auto rc = writeField(Field::provisioned, isProvisioned);
    if (rc != 0)
    {
        lg2::error("Failed writing provisioned {PROVISIONED} in local CFAM",
                   "PROVISIONED", isProvisioned);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeRedundancyEnabled(bool enabled)
{
    auto rc = writeField(Field::redundancyEnabled, enabled);
    if (rc != 0)
    {
        lg2::error("Failed writing redundancy enabled {ENABLED} in local CFAM",
                   "ENABLED", enabled);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeFailoversPaused(bool paused)
{
    auto rc = writeField(Field::failoversPaused, paused);
    if (rc != 0)
    {
        lg2::error("Failed writing failovers paused {PAUSED} in local CFAM",
                   "PAUSED", paused);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeRole(LocalCFAM::Role role)
{
    auto rc = writeField(Field::role, static_cast<uint32_t>(role));
    if (rc != 0)
    {
        lg2::error("Failed writing role {ROLE} in local CFAM", "ROLE", role);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeBMCState(LocalCFAM::BMCState state)
{
    auto rc = writeField(Field::bmcState, static_cast<uint32_t>(state));
    if (rc != 0)
    {
        lg2::error("Failed writing BMC state {STATE} in local CFAM", "STATE",
                   state);
        throw std::system_error(rc, std::generic_category());
    }
}

void LocalCFAM::writeSiblingCommsOK(bool ok)
{
    auto rc = writeField(Field::siblingCommsOK, ok);
    if (rc != 0)
    {
        lg2::error("Failed writing Sibling OK field {OK} in local CFAM", "OK",
                   ok);
        throw std::system_error(rc, std::generic_category());
    }
}

std::expected<uint32_t, int> LocalCFAM::readField(Field field)
{
    auto data = cfamAccess.readScratchReg(cfamFields.at(field).reg);

    if (!data.has_value())
    {
        return std::unexpected{data.error()};
    }

    return cfam::getFieldValue(*data, cfamFields.at(field).start,
                               cfamFields.at(field).numBits);
}

void LocalCFAM::incHeartbeat()
{
    static bool firstHeartbeat = true;

    // The first time through, read the initial value from the hardware.
    if (firstHeartbeat)
    {
        firstHeartbeat = false;

        auto value = readField(Field::heartbeat);
        if (value.has_value())
        {
            heartbeat = *value;
        }
        else
        {
            lg2::error("Failed reading heartbeat from local CFAM, error {ERR}",
                       "ERR", value.error());
            throw std::system_error(value.error(), std::generic_category());
        }
    }

    heartbeat = (heartbeat == maxHeartbeatValue) ? 0 : heartbeat + 1;

    auto rc = writeField(Field::heartbeat, heartbeat);
    if (rc != 0)
    {
        lg2::error("Failed writing BMC heartbeat {HEARTBEAT} in local CFAM",
                   "HEARTBEAT", lg2::hex, heartbeat);
        throw std::system_error(rc, std::generic_category());
    }
}
