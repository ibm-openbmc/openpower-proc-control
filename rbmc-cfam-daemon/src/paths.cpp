// SPDX-License-Identifier: Apache-2.0

#include "paths.hpp"

namespace paths
{

std::filesystem::path getDeviceDir()
{
    return std::filesystem::path{"/dev"};
}

std::filesystem::path getFSIMasterDir()
{
    return std::filesystem::path{"/sys/class/fsi-master"};
}

} // namespace paths
