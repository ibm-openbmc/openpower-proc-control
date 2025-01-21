// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "local_cfam.hpp"
#include "services.hpp"
#include "sysfs.hpp"

#include <sdbusplus/async/context.hpp>

/**
 * @class LocalBMC
 *
 * Represents the local BMC.  This class handles writing
 * data about this BMC into its own CFAM so that the sibling
 * BMC can read it.
 */
class LocalBMC
{
  public:
    LocalBMC() = delete;
    ~LocalBMC() = default;
    LocalBMC(const LocalBMC&) = delete;
    LocalBMC& operator=(const LocalBMC&) = delete;
    LocalBMC(LocalBMC&&) = delete;
    LocalBMC& operator=(LocalBMC&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] ctx - The async context object
     * @param[in] sysfs - The sysfs object
     */
    LocalBMC(sdbusplus::async::context& ctx, SysFS& sysfs) :
        ctx(ctx), cfam(0, sysfs)
    {}

    /**
     * @brief Kicks off initializing the local CFAM.
     *
     * This will write the fields into the CFAM, and start watches
     * for when D-Bus properties change that trigger more writes.
     */
    sdbusplus::async::task<> start();

    /**
     * @brief Sets the 'sibling communications OK' bit in the local CFAM.
     *
     * This represents if this local BMC could read the sibling's CFAM or not.
     *
     * @param[in] ok - If these communications are good or not.
     */
    void setSiblingCommsOK(bool ok);

  private:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    /**
     * @brief Writes the API version field into the CFAM
     */
    void writeApiVersion();

    /**
     * @brief Writes the FW version field into the CFAM
     */
    void writeFWVersion();

    /**
     * @brief Writes the BMC position field into the CFAM
     */
    void writeBMCPosition();

    /**
     * @brief Writes the Provisioned field into the CFAM
     */
    void writeProvisioned();

    /**
     * @brief Writes the Sibling Comms OK field into the CFAM
     */
    inline void writeSiblingCommsNotOK();

    /**
     * @brief Writes the Redundancy Enabled field into the CFAM
     */
    sdbusplus::async::task<> writeRedundancyProps();

    /**
     * @brief Writes the BMC state field into the CFAM
     */
    sdbusplus::async::task<> writeBMCState();

    /**
     * @brief Watches for heartbeat signals to increment the
     *        heartbeat field in the CFAM.
     */
    sdbusplus::async::task<> watchHeartBeat();

    /**
     * @brief Callback function for when the BMC state D-Bus property
     *        changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param[in] state - Value to write
     */
    void bmcStateChanged(BMCState state);

    /**
     * @brief Callback function for when the role D-Bus property
     *        changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param[in] role - Value to write
     */
    void roleChanged(Role role);

    /**
     * @brief Callback function for when the redundancy enabled
     * D-Bus property changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param[in] enabled - Value to write
     */
    void redEnabledChanged(bool enabled);

    /**
     * @brief Callback function for when the failovers paused
     * D-Bus property changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param[in] paused - Value to write
     */
    void failoversPausedChanged(bool paused);

    /**
     * @brief The context object
     */
    sdbusplus::async::context& ctx;

    /**
     * @brief The LocalCFAM object to handle the writes
     */
    LocalCFAM cfam;

    /**
     * @brief Object to read data from the system.
     */
    std::unique_ptr<Services> services;

    /**
     * @brief The current sibling communications OK value.
     *
     * Used so that the CFAM doesn't have to be written
     * on every poll.
     */
    bool siblingOK{false};
};
