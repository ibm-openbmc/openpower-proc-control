// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "bmc_cfam.hpp"

#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/common.hpp>

#include <optional>

/**
 * @class SiblingCFAM
 *
 * Represents the CFAM on the sibling BMC's card.
 *
 * Handles reading fields from the CFAM.  The getter functions
 * return the most recent value since the last readAll() call.
 * If that call failed, they will all throw an exception until
 * there is a good read again.
 */
class SiblingCFAM : public BMCCFAM
{
  public:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    SiblingCFAM() = delete;
    ~SiblingCFAM() override = default;
    SiblingCFAM(const SiblingCFAM&) = delete;
    SiblingCFAM& operator=(const SiblingCFAM&) = delete;
    SiblingCFAM(SiblingCFAM&&) = delete;
    SiblingCFAM& operator=(SiblingCFAM&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] link - The link the CFAM is on
     * @param[in] driver - The driver object
     */
    SiblingCFAM(size_t link, Driver& driver) :
        BMCCFAM(link, driver), usedRegs(BMCCFAM::getUsedRegs())
    {}

    /**
     * @brief Returns the API version field
     *
     * Will throw if there is a hardware error
     */
    uint8_t getApiVersion() const;

    /**
     * @brief Returns the firmware version field
     *
     * Will throw if there is a hardware error
     */
    uint32_t getFWVersion() const;

    /**
     * @brief Returns the BMC position field
     *
     * Will throw if there is a hardware error
     */
    uint32_t getBMCPosition() const;

    /**
     * @brief Returns the provisioned field
     *
     * Will throw if there is a hardware error
     */
    bool getProvisioned() const;

    /**
     * @brief Returns the redundancy enabled field
     *
     * Will throw if there is a hardware error
     */
    bool getRedundancyEnabled() const;

    /**
     * @brief Returns the failovers paused field
     *
     * Will throw if there is a hardware error
     */
    bool getFailoversPaused() const;

    /**
     * @brief Returns the BMC state field
     *
     * Will throw if there is a hardware error
     */
    BMCState getBMCState() const;

    /**
     * @brief Returns the role field
     *
     * Will throw if there is a hardware error
     */
    Role getRole() const;

    /**
     * @brief Returns the sibling comms OK field
     *
     * Will throw if there is a hardware error
     */
    bool getSiblingCommsOK() const;

    /**
     * @brief Returns the heartbeat field
     *
     * Will throw if there is a hardware error
     */
    uint32_t getHeartbeat() const;

    /**
     * @brief Says if there is an error accessing the
     *        hardware.
     */
    inline bool hasError() const
    {
        return error.has_value();
    }

    /**
     * @brief Reads all of the scratch registers that contain
     *        defined fields.
     */
    void readAll();

  private:
    /** @brief Latest API version value */
    uint32_t apiVersion{};

    /** @brief Latest BMC position value */
    uint32_t bmcPosition{};

    /** @brief Latest firmware version value */
    uint32_t fwVersion{};

    /** @brief Latest provisioned value */
    bool provisioned{};

    /** @brief Latest role value */
    Role role{};

    /** @brief Latest redundancy enabled value */
    bool redundancyEnabled{};

    /** @brief Latest failovers paused value */
    bool failoversPaused{};

    /** @brief Latest BMC state value */
    BMCState bmcState;

    /** @brief Latest heartbeat value */
    uint32_t heartbeat{};

    /** @brief Latest sibling comms OK value */
    bool siblingCommsOK{};

    /** @brief The set of scratchpad regs that contain field */
    std::set<cfam::ScratchPadReg> usedRegs;

    /** @brief If the last hardware access attempt returned an error */
    std::optional<int> error{ENOENT};
};
