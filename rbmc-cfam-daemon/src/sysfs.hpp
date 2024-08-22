// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <expected>
#include <filesystem>

/**
 * @class SysFS
 *
 * Abstract base class to provide read and write functions.
 */
class SysFS
{
  public:
    SysFS() = default;
    virtual ~SysFS() = default;

    /**
     * @brief Reads a CFAM register via a sysfs file
     *
     * @param[in] file - The sysfs file to read
     *
     * @return - An std::expected value of either the register/file
     *           data or an integer error code if something fails.
     */
    virtual std::expected<uint32_t, int> read(
        const std::filesystem::path& file) const = 0;

    /**
     * @brief Writes a CFAM register via a sysfs file
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
 * @class SysFSImpl
 *
 * The implementation of the SysFS class.
 */
class SysFSImpl : public SysFS
{
  public:
    SysFSImpl() = default;

    /**
     * @copydoc SysFS::read
     */
    std::expected<uint32_t, int> read(
        const std::filesystem::path& file) const override;

    /**
     * @copydoc SysFS::write
     */
    int write(const std::filesystem::path& file, uint32_t value) const override;
};
