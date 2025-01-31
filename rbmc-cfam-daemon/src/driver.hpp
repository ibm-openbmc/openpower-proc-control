// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <expected>
#include <filesystem>

/**
 * @class Driver
 *
 * Abstract base class to provide CFAM-S read and write functions.
 */
class Driver
{
  public:
    Driver() = default;
    virtual ~Driver() = default;

    /**
     * @brief Reads a CFAM scratchpad register
     *
     * @param[in] device - The device file in /dev
     * @param[in] scratchReg - The register (0-3) to read
     *
     * @return - An std::expected value of either the register
     *           data or an integer error code if something fails.
     */
    virtual std::expected<uint32_t, int> read(
        const std::filesystem::path& device, uint32_t scratchReg) const = 0;

    /**
     * @brief Writes a CFAM scratchpad register
     *
     * @param[in] device - The device file in /dev
     * @param[in] scratchReg - The register (0-3) to write
     * @param[in] value - The value to write
     *
     * @return - 0 if success, nonzero else
     */
    virtual int write(const std::filesystem::path& device, uint32_t scratchReg,
                      uint32_t value) const = 0;

    /**
     * @brief Writes a CFAM scratchpad register with a mask
     *
     * @param[in] device - The device file in /dev
     * @param[in] scratchReg - The register (0-3) to write
     * @param[in] value - The value to write
     * @param[in] mask - The mask to use
     *
     * @return - 0 if success, nonzero else
     */
    virtual int writeWithMask(const std::filesystem::path& device,
                              uint32_t scratchReg, uint32_t value,
                              uint32_t mask) const = 0;
};

/**
 * @class DriverImpl
 *
 * The implementation of the Driver class.
 */
class DriverImpl : public Driver
{
  public:
    DriverImpl() = default;

    /**
     * @copydoc Driver::read
     */
    std::expected<uint32_t, int> read(const std::filesystem::path& device,
                                      uint32_t scratchReg) const override;

    /**
     * @copydoc Driver::write
     */
    int write(const std::filesystem::path& device, uint32_t scratchReg,
              uint32_t value) const override;

    /**
     * @copydoc Driver::writeWithMask
     */
    int writeWithMask(const std::filesystem::path& device, uint32_t scratchReg,
                      uint32_t value, uint32_t mask) const override;

  private:
    /**
     * @brief Says if the reg passed in is a valid scratchpad register.
     *
     * @param[in] scratchReg - The register
     *
     * @return If valid
     */
    inline bool isRegValid(uint32_t scratchReg) const
    {
        return scratchReg < 4;
    }
};
