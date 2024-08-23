// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "bmc_cfam.hpp"
#include "sysfs.hpp"

#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/common.hpp>

#include <expected>

namespace local_cfam_util
{
/**
 * @brief Returns the largest number possible based for the
 *        number of bits passed in.
 *
 * For example, numbits = 8, max value = 0xFF
 *
 * @param[in] numBits - The number of bits in the value
 *
 * @return - The max value possible
 */
uint32_t getMaxValue(size_t numBits);

} // namespace local_cfam_util

/**
 * @class LocalCFAM
 *
 * Represents the CFAM on the local BMC's card.
 *
 * It handles writing fields into the CFAM.
 */
class LocalCFAM : public BMCCFAM
{
  public:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    LocalCFAM() = delete;
    ~LocalCFAM() override = default;
    LocalCFAM(const LocalCFAM&) = delete;
    LocalCFAM& operator=(const LocalCFAM&) = delete;
    LocalCFAM(LocalCFAM&&) = delete;
    LocalCFAM& operator=(LocalCFAM&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] link - The link the CFAM is on
     * @param[in] sysfs - The sysfs object
     */
    LocalCFAM(size_t link, SysFS& sysfs) :
        BMCCFAM(link, sysfs), maxHeartbeatValue(local_cfam_util::getMaxValue(
                                  cfamFields.at(Field::heartbeat).numBits))
    {}

    /**
     * @brief Writes the API version field into the CFAM
     *
     * @param[in] version - The version to write
     */
    void writeApiVersion(uint8_t version);

    /**
     * @brief Writes the firmware version field into the CFAM
     *
     * @param[in] version - The version to write
     */
    void writeFWVersion(uint32_t version);

    /**
     * @brief Writes the BMC position field into the CFAM
     *
     * @param[in] position - The position to write
     */
    void writeBMCPosition(uint8_t position);

    /**
     * @brief Writes the provisioned field into the CFAM
     *
     * @param[in] isProvisioned - If the BMC is provisioned
     */
    void writeProvisioned(bool isProvisioned);

    /**
     * @brief Writes the redundancy enabled field into the CFAM
     *
     * @param[in] enabled - If redundancy is enabled
     */
    void writeRedundancyEnabled(bool enabled);

    /**
     * @brief Writes the failovers paused field into the CFAM
     *
     * @param[in] paused - If paused
     */
    void writeFailoversPaused(bool paused);

    /**
     * @brief Writes the role field into the CFAM
     *
     * @param[in] role - The role
     */
    void writeRole(Role role);

    /**
     * @brief Writes the BMC state field into the CFAM
     *
     * @param[in] state - The state
     */
    void writeBMCState(BMCState state);

    /**
     * @brief Writes the sibling comms OK field into the CFAM
     *
     * @param[in] ok - If communications are OK
     */
    void writeSiblingCommsOK(bool ok);

    /**
     * @brief Increments the heartbeat field in the CFAM
     */
    void incHeartbeat();

  private:
    /**
     * @brief Writes the field to the CFAM
     *
     * @param[in] field - The field to write
     *
     * @param[in] value - The value to write (right justified)
     *
     * @return int - 0 if successful, nonzero else
     */
    int writeField(Field field, uint32_t value);

    /**
     * @brief Reads a field from the CFAM
     *
     * @param[in] field - the field to read
     *
     * @return - The value read, or an unexpected with the error value.
     */
    std::expected<uint32_t, int> readField(Field field);

    /**
     * @brief The current heartbeat value.
     */
    uint32_t heartbeat{};

    /**
     * @brief The largest value the heartbeat field can be
     *        based on the field size.
     */
    uint32_t maxHeartbeatValue;
};
