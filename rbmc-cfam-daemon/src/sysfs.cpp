// SPDX-License-Identifier: Apache-2.0
#include "sysfs.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <cstring>

struct Closer
{
    explicit Closer(int fd) : fd(fd) {}
    ~Closer()
    {
        if (fd != -1)
        {
            close(fd);
        }
    }

    Closer() = delete;

    int fd = -1;
};

std::expected<uint32_t, int> SysFSImpl::read(
    const std::filesystem::path& file) const
{
    try
    {
        if (!std::filesystem::exists(file))
        {
            lg2::error("Read: Scratch register file {FILE} doesn't exist!",
                       "FILE", file);
            return std::unexpected{ENOENT};
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Call to filesystem::exists failed on {FILE}: {ERR}", "FILE",
                   file, "ERR", e);
        return std::unexpected{ENOENT};
    }

    int fd = open(file.c_str(), O_RDONLY, O_SYNC);
    if (fd < 0)
    {
        int err = errno;
        lg2::error("open: Reading file {FILE} failed with error {ERROR}",
                   "FILE", file, "ERROR", strerror(err));
        return std::unexpected{err};
    }

    Closer c{fd};
    uint32_t value = 0;

    ssize_t rc = ::read(fd, &value, sizeof(value));
    if (rc < 0)
    {
        int err = errno;
        lg2::error("Reading file {FILE} failed with error {ERROR}", "FILE",
                   file, "ERROR", strerror(err));
        return std::unexpected{err};
    }

    if (rc != 4)
    {
        lg2::error("Reading file {FILE} only read {BYTES} bytes. Expected 4.",
                   "FILE", file, "BYTES", rc);
        return std::unexpected{EIO};
    }

    return value;
}

int SysFSImpl::write(const std::filesystem::path& file, uint32_t value) const
{
    try
    {
        if (!std::filesystem::exists(file))
        {
            lg2::error("Write: Scratch register file {FILE} doesn't exist!",
                       "FILE", file);
            return ENOENT;
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Call to filesystem::exists failed on {FILE}: {ERR}", "FILE",
                   file, "ERR", e);
        return ENOENT;
    }

    int fd = open(file.c_str(), O_WRONLY, O_SYNC);
    if (fd < 0)
    {
        int err = errno;
        lg2::error("open: Writing file {FILE} failed with error {ERROR}",
                   "FILE", file, "ERROR", strerror(err));
        return err;
    }

    Closer c{fd};

    ssize_t rc = ::write(fd, &value, sizeof(value));
    if (rc < 0)
    {
        int err = errno;
        lg2::error("Writing FILE {FILE} failed with errno {ERROR}", "FILE",
                   file, "ERROR", strerror(err));
        return err;
    }

    if (rc != 4)
    {
        lg2::error("Writing FILE {FILE} only wrote {BYTES} bytes. Expected 4.",
                   "FILE", file, "BYTES", rc);
        return EIO;
    }

    return 0;
}
