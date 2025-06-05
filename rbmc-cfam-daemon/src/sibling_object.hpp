// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <xyz/openbmc_project/Software/Version/server.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/server.hpp>
#include <xyz/openbmc_project/State/BMC/server.hpp>
#include <xyz/openbmc_project/State/Decorator/Heartbeat/server.hpp>

using RedIntf = sdbusplus::server::xyz::openbmc_project::state::bmc::Redundancy;
using HBIntf =
    sdbusplus::server::xyz::openbmc_project::state::decorator::Heartbeat;
using VersionIntf = sdbusplus::server::xyz::openbmc_project::software::Version;
using StateIntf = sdbusplus::server::xyz::openbmc_project::state::BMC;

using SiblingInterfaces =
    sdbusplus::server::object_t<RedIntf, HBIntf, VersionIntf, StateIntf>;

/**
 * @class SiblingObject
 *
 * The D-Bus object that holds the sibling BMC's values.
 *
 * Note: This isn't the async version because the code needs to
 * conditionally specify if a PC signal should be emitted,
 * and the only way to do that with the async version is with
 * a template param, e.g. obj->role<true>(val)
 */
class SiblingObject : public SiblingInterfaces
{
  public:
    SiblingObject(const SiblingObject&) = delete;
    SiblingObject& operator=(const SiblingObject&) = delete;
    SiblingObject(SiblingObject&&) = delete;
    SiblingObject& operator=(SiblingObject&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] bus - The sdbusplus bus object
     * @param[in] path - Object path
     */
    SiblingObject(sdbusplus::bus_t& bus, const std::string& path) :
        SiblingInterfaces(bus, path.c_str(), action::defer_emit)
    {}

    // Don't allow writes
    bool disableRedundancyOverride(bool) override
    {
        throw sdbusplus::xyz::openbmc_project::Common::Error::Unavailable();
    }

    // Don't allow writes
    Transition requestedBMCTransition(Transition) override
    {
        throw sdbusplus::xyz::openbmc_project::Common::Error::Unavailable();
    }
};
