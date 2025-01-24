// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <filesystem>

namespace paths
{

/**
 * @brief Returns the path to /dev so that in unit test
 *        a different path can be returned.
 */
std::filesystem::path getDeviceDir();

/**
 * @brief Returns the path to /sys/class/fsi-master so that
 *        in unit test a different path can be returned.
 */
std::filesystem::path getFSIMasterDir();

} // namespace paths
