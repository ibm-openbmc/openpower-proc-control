// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "cfam_types.hpp"
#include "driver.hpp"

#include <unistd.h>

#include <expected>
#include <filesystem>
#include <set>

/**
 * @class CFAMAccess
 *
 * This class contains methods to read and write the four
 * scratchpad registers in the Gemini Mailbox block of a CFAM.
 */
class CFAMAccess
{
  public:
    using RegMapExpected = std::expected<cfam::RegMap, int>;

    CFAMAccess() = delete;
    ~CFAMAccess() = default;

    /**
     * @brief Constructor
     *
     * @param[in] link - The BMC's FSI link the CFAM is on.
     * @param[in] driver - A mockable object for accessing the driver.
     */
    CFAMAccess(size_t link, Driver& driver) : link(link), driver(driver) {}

    /**
     * @brief Reads all scratchpad registers passed into the function.
     *
     * @param[in] regNames - the registers to read.
     *
     * @return The map of register names to values, or an std::unexpected
     *         containing an error code if something fails.
     */
    RegMapExpected
        readScratchRegs(const std::set<cfam::ScratchPadReg>& regNames);

    /**
     * @brief Reads a register
     *
     * @param[in] reg - The register to read
     *
     * @return The register value, or an error code if something failed
     */
    std::expected<uint32_t, int> readScratchReg(cfam::ScratchPadReg reg);

    /**
     * @brief Writes a register
     *
     * @param[in] reg - The register to write
     * @param[in] data - The data to write
     *
     * @return 0 if successful, nonzero else
     */
    int writeScratchReg(cfam::ScratchPadReg reg, uint32_t data);

    /**
     * @brief Writes a register field by doing a read/modify/write
     *
     * @param[in] op - The reg/start/length data
     *
     * @return 0 if successful, nonzero else
     */
    int writeScratchRegWithMask(const cfam::ModifyOp& op);

    /**
     * @brief If the CFAM device exists in the filesystem
     *
     * @return a bool indicating if it exists.
     */
    bool exists() const;

  private:
    /**
     * @brief Makes the driver path to use to access the register
     *
     * @param[in] reg - The register to find the path for
     *
     * @return path - The path to the driver file
     */
    std::filesystem::path getRegisterPath(cfam::ScratchPadReg reg) const;

    /**
     * @brief The BMC's FSI link the CFAM is on
     */
    size_t link;

    /**
     * @brief Driver object
     */
    Driver& driver;
};
