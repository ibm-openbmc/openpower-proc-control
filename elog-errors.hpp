// This file was autogenerated.  Do not edit!
// See elog-gen.py for more details
#pragma once

#include <string>
#include <tuple>
#include <type_traits>
#include <sdbusplus/exception.hpp>
#include <phosphor-logging/log.hpp>
#include <phosphor-logging/elog.hpp>

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace Error
{
    struct GPIO;
} // namespace Error
} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

namespace sdbusplus
{
namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace Error
{
    struct ReadFailure;
} // namespace Error
} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org
} // namespace sdbusplus

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace Error
{
    struct Inventory;
} // namespace Error
} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace Error
{
    struct IIC;
} // namespace Error
} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

namespace sdbusplus
{
namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace Error
{
    struct WriteFailure;
} // namespace Error
} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org
} // namespace sdbusplus

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace Error
{
    struct Device;
} // namespace Error
} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

namespace sdbusplus
{
namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace Error
{
    struct OpenFailure;
} // namespace Error
} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org
} // namespace sdbusplus

namespace sdbusplus
{
namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace Error
{
    struct SeekFailure;
} // namespace Error
} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org
} // namespace sdbusplus

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace Error
{
    struct IPMISensor;
} // namespace Error
} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus


namespace phosphor
{

namespace logging
{

namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace _Device
{

struct CALLOUT_ERRNO
{
    static constexpr auto str = "CALLOUT_ERRNO=%d";
    static constexpr auto str_short = "CALLOUT_ERRNO";
    using type = std::tuple<std::decay_t<decltype(str)>,int32_t>;
    explicit constexpr CALLOUT_ERRNO(int32_t a) : _entry(entry(str, a)) {};
    type _entry;
};
struct CALLOUT_DEVICE_PATH
{
    static constexpr auto str = "CALLOUT_DEVICE_PATH=%s";
    static constexpr auto str_short = "CALLOUT_DEVICE_PATH";
    using type = std::tuple<std::decay_t<decltype(str)>,const char*>;
    explicit constexpr CALLOUT_DEVICE_PATH(const char* a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _Device

struct Device : public sdbusplus::exception_t
{
    static constexpr auto errName = "xyz.openbmc_project.Common.Callout.Device";
    static constexpr auto errDesc = "Generic device callout";
    static constexpr auto L = level::INFO;
    using CALLOUT_ERRNO = _Device::CALLOUT_ERRNO;
    using CALLOUT_DEVICE_PATH = _Device::CALLOUT_DEVICE_PATH;
    using metadata_types = std::tuple<CALLOUT_ERRNO, CALLOUT_DEVICE_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz


namespace details
{

template <>
struct map_exception_type<sdbusplus::xyz::openbmc_project::Common::Callout::Error::Device>
{
    using type = xyz::openbmc_project::Common::Callout::Device;
};

}

namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace _GPIO
{

struct CALLOUT_GPIO_NUM
{
    static constexpr auto str = "CALLOUT_GPIO_NUM=%u";
    static constexpr auto str_short = "CALLOUT_GPIO_NUM";
    using type = std::tuple<std::decay_t<decltype(str)>,uint32_t>;
    explicit constexpr CALLOUT_GPIO_NUM(uint32_t a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _GPIO

struct GPIO : public sdbusplus::exception_t
{
    static constexpr auto errName = "xyz.openbmc_project.Common.Callout.GPIO";
    static constexpr auto errDesc = "Callout GPIO pin";
    static constexpr auto L = level::INFO;
    using CALLOUT_GPIO_NUM = _GPIO::CALLOUT_GPIO_NUM;
    using CALLOUT_ERRNO = xyz::openbmc_project::Common::Callout::Device::CALLOUT_ERRNO;
    using CALLOUT_DEVICE_PATH = xyz::openbmc_project::Common::Callout::Device::CALLOUT_DEVICE_PATH;
    using metadata_types = std::tuple<CALLOUT_GPIO_NUM, CALLOUT_ERRNO, CALLOUT_DEVICE_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz


namespace details
{

template <>
struct map_exception_type<sdbusplus::xyz::openbmc_project::Common::Callout::Error::GPIO>
{
    using type = xyz::openbmc_project::Common::Callout::GPIO;
};

}

namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace _IIC
{

struct CALLOUT_IIC_BUS
{
    static constexpr auto str = "CALLOUT_IIC_BUS=%s";
    static constexpr auto str_short = "CALLOUT_IIC_BUS";
    using type = std::tuple<std::decay_t<decltype(str)>,const char*>;
    explicit constexpr CALLOUT_IIC_BUS(const char* a) : _entry(entry(str, a)) {};
    type _entry;
};
struct CALLOUT_IIC_ADDR
{
    static constexpr auto str = "CALLOUT_IIC_ADDR=0x%hx";
    static constexpr auto str_short = "CALLOUT_IIC_ADDR";
    using type = std::tuple<std::decay_t<decltype(str)>,uint16_t>;
    explicit constexpr CALLOUT_IIC_ADDR(uint16_t a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _IIC

struct IIC : public sdbusplus::exception_t
{
    static constexpr auto errName = "xyz.openbmc_project.Common.Callout.IIC";
    static constexpr auto errDesc = "Callout IIC device";
    static constexpr auto L = level::INFO;
    using CALLOUT_IIC_BUS = _IIC::CALLOUT_IIC_BUS;
    using CALLOUT_IIC_ADDR = _IIC::CALLOUT_IIC_ADDR;
    using CALLOUT_ERRNO = xyz::openbmc_project::Common::Callout::Device::CALLOUT_ERRNO;
    using CALLOUT_DEVICE_PATH = xyz::openbmc_project::Common::Callout::Device::CALLOUT_DEVICE_PATH;
    using metadata_types = std::tuple<CALLOUT_IIC_BUS, CALLOUT_IIC_ADDR, CALLOUT_ERRNO, CALLOUT_DEVICE_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz


namespace details
{

template <>
struct map_exception_type<sdbusplus::xyz::openbmc_project::Common::Callout::Error::IIC>
{
    using type = xyz::openbmc_project::Common::Callout::IIC;
};

}

namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace _Inventory
{

struct CALLOUT_INVENTORY_PATH
{
    static constexpr auto str = "CALLOUT_INVENTORY_PATH=%s";
    static constexpr auto str_short = "CALLOUT_INVENTORY_PATH";
    using type = std::tuple<std::decay_t<decltype(str)>,const char*>;
    explicit constexpr CALLOUT_INVENTORY_PATH(const char* a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _Inventory

struct Inventory : public sdbusplus::exception_t
{
    static constexpr auto errName = "xyz.openbmc_project.Common.Callout.Inventory";
    static constexpr auto errDesc = "Inventory item callout";
    static constexpr auto L = level::INFO;
    using CALLOUT_INVENTORY_PATH = _Inventory::CALLOUT_INVENTORY_PATH;
    using metadata_types = std::tuple<CALLOUT_INVENTORY_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz


namespace details
{

template <>
struct map_exception_type<sdbusplus::xyz::openbmc_project::Common::Callout::Error::Inventory>
{
    using type = xyz::openbmc_project::Common::Callout::Inventory;
};

}

namespace xyz
{
namespace openbmc_project
{
namespace Common
{
namespace Callout
{
namespace _IPMISensor
{

struct CALLOUT_IPMI_SENSOR_NUM
{
    static constexpr auto str = "CALLOUT_IPMI_SENSOR_NUM=%u";
    static constexpr auto str_short = "CALLOUT_IPMI_SENSOR_NUM";
    using type = std::tuple<std::decay_t<decltype(str)>,uint32_t>;
    explicit constexpr CALLOUT_IPMI_SENSOR_NUM(uint32_t a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _IPMISensor

struct IPMISensor : public sdbusplus::exception_t
{
    static constexpr auto errName = "xyz.openbmc_project.Common.Callout.IPMISensor";
    static constexpr auto errDesc = "Callout IPMI sensor";
    static constexpr auto L = level::INFO;
    using CALLOUT_IPMI_SENSOR_NUM = _IPMISensor::CALLOUT_IPMI_SENSOR_NUM;
    using metadata_types = std::tuple<CALLOUT_IPMI_SENSOR_NUM>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace Callout
} // namespace Common
} // namespace openbmc_project
} // namespace xyz


namespace details
{

template <>
struct map_exception_type<sdbusplus::xyz::openbmc_project::Common::Callout::Error::IPMISensor>
{
    using type = xyz::openbmc_project::Common::Callout::IPMISensor;
};

}

namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace _OpenFailure
{

struct ERRNO
{
    static constexpr auto str = "ERRNO=%d";
    static constexpr auto str_short = "ERRNO";
    using type = std::tuple<std::decay_t<decltype(str)>,int32_t>;
    explicit constexpr ERRNO(int32_t a) : _entry(entry(str, a)) {};
    type _entry;
};
struct PATH
{
    static constexpr auto str = "PATH=%s";
    static constexpr auto str_short = "PATH";
    using type = std::tuple<std::decay_t<decltype(str)>,const char*>;
    explicit constexpr PATH(const char* a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _OpenFailure

struct OpenFailure : public sdbusplus::exception_t
{
    static constexpr auto errName = "org.open_power.Proc.CFAM.OpenFailure";
    static constexpr auto errDesc = "Failed to open the device.";
    static constexpr auto L = level::INFO;
    using ERRNO = _OpenFailure::ERRNO;
    using PATH = _OpenFailure::PATH;
    using metadata_types = std::tuple<ERRNO, PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org


namespace details
{

template <>
struct map_exception_type<sdbusplus::org::open_power::Proc::CFAM::Error::OpenFailure>
{
    using type = org::open_power::Proc::CFAM::OpenFailure;
};

}

namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace _ReadFailure
{


}  // namespace _ReadFailure

struct ReadFailure : public sdbusplus::exception_t
{
    static constexpr auto errName = "org.open_power.Proc.CFAM.ReadFailure";
    static constexpr auto errDesc = "Failed to read from the device.";
    static constexpr auto L = level::INFO;
    using CALLOUT_ERRNO = xyz::openbmc_project::Common::Callout::Device::CALLOUT_ERRNO;
    using CALLOUT_DEVICE_PATH = xyz::openbmc_project::Common::Callout::Device::CALLOUT_DEVICE_PATH;
    using metadata_types = std::tuple<CALLOUT_ERRNO, CALLOUT_DEVICE_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org


namespace details
{

template <>
struct map_exception_type<sdbusplus::org::open_power::Proc::CFAM::Error::ReadFailure>
{
    using type = org::open_power::Proc::CFAM::ReadFailure;
};

}

namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace _WriteFailure
{


}  // namespace _WriteFailure

struct WriteFailure : public sdbusplus::exception_t
{
    static constexpr auto errName = "org.open_power.Proc.CFAM.WriteFailure";
    static constexpr auto errDesc = "Failed to write to the device.";
    static constexpr auto L = level::INFO;
    using CALLOUT_ERRNO = xyz::openbmc_project::Common::Callout::Device::CALLOUT_ERRNO;
    using CALLOUT_DEVICE_PATH = xyz::openbmc_project::Common::Callout::Device::CALLOUT_DEVICE_PATH;
    using metadata_types = std::tuple<CALLOUT_ERRNO, CALLOUT_DEVICE_PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org


namespace details
{

template <>
struct map_exception_type<sdbusplus::org::open_power::Proc::CFAM::Error::WriteFailure>
{
    using type = org::open_power::Proc::CFAM::WriteFailure;
};

}

namespace org
{
namespace open_power
{
namespace Proc
{
namespace CFAM
{
namespace _SeekFailure
{

struct ERRNO
{
    static constexpr auto str = "ERRNO=%d";
    static constexpr auto str_short = "ERRNO";
    using type = std::tuple<std::decay_t<decltype(str)>,int32_t>;
    explicit constexpr ERRNO(int32_t a) : _entry(entry(str, a)) {};
    type _entry;
};
struct ADDRESS
{
    static constexpr auto str = "ADDRESS=0x%0x";
    static constexpr auto str_short = "ADDRESS";
    using type = std::tuple<std::decay_t<decltype(str)>,uint16_t>;
    explicit constexpr ADDRESS(uint16_t a) : _entry(entry(str, a)) {};
    type _entry;
};
struct OFFSET
{
    static constexpr auto str = "OFFSET=%d";
    static constexpr auto str_short = "OFFSET";
    using type = std::tuple<std::decay_t<decltype(str)>,uint16_t>;
    explicit constexpr OFFSET(uint16_t a) : _entry(entry(str, a)) {};
    type _entry;
};
struct PATH
{
    static constexpr auto str = "PATH=%s";
    static constexpr auto str_short = "PATH";
    using type = std::tuple<std::decay_t<decltype(str)>,const char*>;
    explicit constexpr PATH(const char* a) : _entry(entry(str, a)) {};
    type _entry;
};

}  // namespace _SeekFailure

struct SeekFailure : public sdbusplus::exception_t
{
    static constexpr auto errName = "org.open_power.Proc.CFAM.SeekFailure";
    static constexpr auto errDesc = "Failed to seek on the device.";
    static constexpr auto L = level::INFO;
    using ERRNO = _SeekFailure::ERRNO;
    using ADDRESS = _SeekFailure::ADDRESS;
    using OFFSET = _SeekFailure::OFFSET;
    using PATH = _SeekFailure::PATH;
    using metadata_types = std::tuple<ERRNO, ADDRESS, OFFSET, PATH>;

    const char* name() const noexcept
    {
        return errName;
    }

    const char* description() const noexcept
    {
        return errDesc;
    }

    const char* what() const noexcept
    {
        return errName;
    }
};

} // namespace CFAM
} // namespace Proc
} // namespace open_power
} // namespace org


namespace details
{

template <>
struct map_exception_type<sdbusplus::org::open_power::Proc::CFAM::Error::SeekFailure>
{
    using type = org::open_power::Proc::CFAM::SeekFailure;
};

}


} // namespace logging

} // namespace phosphor
