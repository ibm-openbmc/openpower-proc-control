// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "sibling_cfam.hpp"
#include "sibling_object.hpp"

#include <sdbusplus/async.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/Sibling/server.hpp>
#include <xyz/openbmc_project/State/Decorator/Availability/server.hpp>

using SiblingIntf =
    sdbusplus::xyz::openbmc_project::State::BMC::Redundancy::server::Sibling;
using SiblingInterface = sdbusplus::server::object_t<SiblingIntf>;

using AvailInterface =
    sdbusplus::server::xyz::openbmc_project::state::decorator::Availability;

/**
 * @class SiblingBMC
 *
 * Represents the sibling BMC.  Responsible for holding the Sibling D-Bus
 * interface, which is populated from fields in the sibling's CFAM.
 * The interface's properties are updated after calls to read().
 */
class SiblingBMC
{
  public:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    ~SiblingBMC() = default;
    SiblingBMC(const SiblingBMC&) = delete;
    SiblingBMC& operator=(const SiblingBMC&) = delete;
    SiblingBMC(SiblingBMC&&) = delete;
    SiblingBMC& operator=(SiblingBMC&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] ctx - The async context object
     * @param[in] link - The FSI link for the CFAM
     * @param[in] driver - The driver object
     * @param[in] availIface - Availability D-Bus iface obj
     */
    SiblingBMC(sdbusplus::async::context& ctx, size_t link, Driver& driver,
               AvailInterface& availIface) :
        ctx(ctx), availInterface(availIface), cfam(link, driver)
    {}

    /**
     * @brief Read's the sibling's CFAM scratchpad registers and puts the
     *        values on D-Bus.
     *
     * More specifically, it will:
     *
     * 1) Read all CFAM registers that have fields defined
     * 2) Create and/or update the Sibling D-Bus interface with these values.
     * 3) If there are errors, remove the interface from D-Bus so nobody
     *    can read stale values.
     */
    sdbusplus::async::task<> read();

    /**
     * @brief Says if it could successfully read the CFAM.
     *
     * @return bool - if the CFAM could be read without an error.
     */
    bool ok()
    {
        return cfam.isReady() && !cfam.hasError();
    }

    /**
     * @brief Returns the D-Bus object path for the sibling BMC.
     */
    static std::string getObjectPath()
    {
        return std::string{RedIntf::namespace_path::value} + '/' +
               RedIntf::namespace_path::sibling_bmc;
    }

  private:
    /**
     * @brief Checks if CFAM is ready to access.
     *
     * Will retry a few times if it isn't.
     *
     * @return bool - If ready or not
     */
    sdbusplus::async::task<bool> checkCFAMReady();

    /**
     * @brief The context object
     */
    sdbusplus::async::context& ctx;

    /**
     * @brief The Sibling D-Bus interface
     */
    std::unique_ptr<SiblingInterface> siblingInterface;

    /**
     * @brief The Sibling D-Bus object
     */
    std::unique_ptr<SiblingObject> siblingObject;

    /**
     * @brief D-Bus The Availability D-Bus interface
     *
     * Holds the Available property that indicates if the sibling
     * BMC's CFAM FSI device is present.
     */
    AvailInterface& availInterface;

    /**
     * @brief The last heartbeat value read from the CFAM.
     */
    std::optional<uint32_t> lastHeartbeat;

    /**
     * @brief If the sibling CFAM is ready, meaning its driver
     *        files are there.
     *
     *        Used as a flag to bound tracing.
     */
    bool ready = false;

    /**
     * @brief The SiblingCFAM object to handle the reads.
     */
    SiblingCFAM cfam;
};
