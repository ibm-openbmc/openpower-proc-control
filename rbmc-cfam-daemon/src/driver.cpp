// SPDX-License-Identifier: Apache-2.0
#include "driver.hpp"

#include "uapi/fsi.h"

#include <fcntl.h>
#include <sys/ioctl.h>
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

std::expected<uint32_t, int> DriverImpl::read(
    const std::filesystem::path& device, uint32_t scratchReg) const
{
    if (!isRegValid(scratchReg))
    {
        return EINVAL;
    }

    try
    {
        if (!std::filesystem::exists(device))
        {
            lg2::error("Read: Device {DEVICE} doesn't exist!", "DEVICE",
                       device);
            return std::unexpected{ENOENT};
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Call to filesystem::exists failed on {DEVICE}: {ERR}",
                   "DEVICE", device, "ERR", e);
        return std::unexpected{ENOENT};
    }

    int fd = open(device.c_str(), O_RDONLY);
    if (fd < 0)
    {
        int err = errno;
        lg2::error("Read: Opening {DEVICE} failed with error {ERROR}", "DEVICE",
                   device, "ERROR", strerror(err));
        return std::unexpected{err};
    }

    Closer c{fd};

    mbox_access access;
    access.reg = scratchReg;
    int rc = ioctl(fd, FSI_MBOX_READ, &access);
    if (rc < 0)
    {
        int err = errno;
        lg2::error("Read: ioctl for {DEVICE} failed with error {ERROR}",
                   "DEVICE", device, "ERROR", strerror(err));
        return std::unexpected{err};
    }

    return access.data;
}

int DriverImpl::write(const std::filesystem::path& device, uint32_t scratchReg,
                      uint32_t value) const
{
    if (!isRegValid(scratchReg))
    {
        return EINVAL;
    }

    try
    {
        if (!std::filesystem::exists(device))
        {
            lg2::error("Write: Device {DEVICE} doesn't exist!", "DEVICE",
                       device);
            return ENOENT;
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Call to filesystem::exists failed on {DEVICE}: {ERR}",
                   "DEVICE", device, "ERR", e);
        return ENOENT;
    }

    int fd = open(device.c_str(), O_WRONLY);
    if (fd < 0)
    {
        int err = errno;
        lg2::error("Write: Opening {DEVICE} failed with error {ERROR}",
                   "DEVICE", device, "ERROR", strerror(err));
        return err;
    }

    Closer c{fd};

    mbox_access access;
    access.reg = scratchReg;
    access.data = value;

    int rc = ioctl(fd, FSI_MBOX_WRITE, &access);
    if (rc < 0)
    {
        int err = errno;
        lg2::error("Write: ioctl for {DEVICE} failed with errno {ERROR}",
                   "DEVICE", device, "ERROR", strerror(err));
        return err;
    }

    return 0;
}
int DriverImpl::writeWithMask(const std::filesystem::path& device,
                              uint32_t scratchReg, uint32_t value,
                              uint32_t mask) const
{
    if (!isRegValid(scratchReg))
    {
        return EINVAL;
    }

    try
    {
        if (!std::filesystem::exists(device))
        {
            lg2::error("WriteWithMask: Device {DEVICE} doesn't exist!",
                       "DEVICE", device);
            return ENOENT;
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Call to filesystem::exists failed on {DEVICE}: {ERR}",
                   "DEVICE", device, "ERR", e);
        return ENOENT;
    }

    int fd = open(device.c_str(), O_RDWR);
    if (fd < 0)
    {
        int err = errno;
        lg2::error("WriteWithMask: Opening {DEVICE} failed with error {ERROR}",
                   "DEVICE", device, "ERROR", strerror(err));
        return err;
    }

    Closer c{fd};

    mbox_access_rmw rmw;
    rmw.access.reg = scratchReg;
    rmw.access.data = value;
    rmw.mask = mask;

    int rc = ioctl(fd, FSI_MBOX_RMW, &rmw);
    if (rc < 0)
    {
        int err = errno;
        lg2::error(
            "WriteWithMask: ioctl for {DEVICE} failed with errno {ERROR}",
            "DEVICE", device, "ERROR", strerror(err));
        return err;
    }

    return 0;
}
