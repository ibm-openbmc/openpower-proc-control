// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <expected>
#include <filesystem>

/**
 * @class Driver
 *
 * Abstract base class to provide read and write functions.
 */
class Driver
{
  public:
    Driver() = default;
    virtual ~Driver() = default;

    /**
     * @brief Reads a CFAM scratch register
     *
     * @param[in] file - The sysfs file to read
     *
     * @return - An std::expected value of either the register/file
     *           data or an integer error code if something fails.
     */
    virtual std::expected<uint32_t, int>
        read(const std::filesystem::path& file) const = 0;

    /**
     * @brief Writes a CFAM register
     *
     * @param[in] file - The sysfs file to write
     * @param[in] value - The value to write
     *
     * @return - 0 if success, nonzero else
     */
    virtual int write(const std::filesystem::path& file,
                      uint32_t value) const = 0;
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
    std::expected<uint32_t, int>
        read(const std::filesystem::path& file) const override;

    /**
     * @copydoc Driver::write
     */
    int write(const std::filesystem::path& file, uint32_t value) const override;
};
