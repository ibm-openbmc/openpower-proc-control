// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <sdbusplus/async.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/common.hpp>

#include <filesystem>

/**
 * @class Services
 *
 * Provides APIs to retrieve various pieces of data from the system.
 * Also provides methods to make callbacks on certain D-Bus changes.
 */
class Services
{
  public:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    using BMCStateCallback = std::function<void(BMCState)>;
    using RoleCallback = std::function<void(Role)>;
    using RedEnabledCallback = std::function<void(bool)>;
    using FailoversPausedCallback = std::function<void(bool)>;

    Services() = delete;
    ~Services() = default;
    Services(const Services&) = delete;
    Services& operator=(const Services&) = delete;
    Services(Services&&) = delete;
    Services& operator=(Services&&) = delete;

    /**
     * @brief Constructor
     *
     * Starts up its D-Bus watches.
     *
     * @param[in] ctx - The async context
     * @param[in] stateCallback - Function to run when the BMC state changes
     * @param[in] roleCallback - Function to run when the role changes
     * @param[in] redEnabledCallback - Function to run when the
     *                                 redundancyEnabled prop changes
     * @param[in] failoversPausedCallback - Function to run when this prop
     *                                      changes
     */
    Services(sdbusplus::async::context& ctx, BMCStateCallback&& stateCallback,
             RoleCallback&& roleCallback,
             RedEnabledCallback&& redEnabledCallback,
             FailoversPausedCallback&& failoversPausedCallback);

    /**
     * @brief Reads the CurrentBMCState property
     *
     * @return - The BMC state
     */
    sdbusplus::async::task<BMCState> getBMCState();

    /**
     * @brief Reads the role and redundancyEnabled properties
     *
     * @return - A tuple of the role and redundancyEnabled property
     */
    sdbusplus::async::task<std::tuple<Services::Role, bool, bool>>
        getRedundancyProps();

    /**
     * @brief Reads the VERSION_ID field out of the version file.
     *
     * It would get this from /etc/os-release on a real system.
     *
     * @param[in] file - The file to read from.
     *
     * @return - The version id
     */
    static std::string getFWVersionID(const std::filesystem::path& file);

    /**
     * @brief Returns this BMC's position.
     *
     * @return - The position
     */
    uint32_t getBMCPosition();

  private:
    /**
     * @brief Starts up the async watches on the D-Bus properties
     *        being monitored.
     */
    void startup();

    /**
     * @brief Watches for PropertiesChanged signals on CurrentBMCState
     *        and invokes the callback on a change.
     *
     * @return - The async task object
     */
    sdbusplus::async::task<> watchBMCStateProp();

    /**
     * @brief Watches for PropertiesChanged signals on the two
     *        interesting redundancy interface properties
     *        and invokes the callback on a change.
     *
     * @return - The async task object
     */
    sdbusplus::async::task<> watchRedundancyProps();

    /**
     * @brief Watches for InterfacesAdded signals on the path that
     *        the state and redundancy interfaces reside and invokes
     *        the appropriate callbacks when the properties are added.
     */
    sdbusplus::async::task<> watchBMCInterfaceAdded();

    /**
     * @brief The async context object
     */
    sdbusplus::async::context& ctx;

    /**
     * @brief The callback function for CurrentBMCState
     */
    BMCStateCallback bmcStateCallback;

    /**
     * @brief The callback function for Role
     */
    RoleCallback roleCallback;

    /**
     * @brief The callback function for RedundancyEnabled
     */
    RedEnabledCallback redEnabledCallback;

    /**
     * @brief The callback function for FailoversPaused
     */
    FailoversPausedCallback failoversPausedCallback;
};
