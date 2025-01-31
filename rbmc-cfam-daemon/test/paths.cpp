// SPDX-License-Identifier: Apache-2.0

#include "paths.hpp"

namespace paths
{

std::filesystem::path getDeviceDir()
{
    static std::string path;

    if (path.empty())
    {
        char dir[] = "/tmp/deviceXXXXXX";
        path = mkdtemp(dir);
    }
    return path;
}

std::filesystem::path getFSIMasterDir()
{
    static std::string path;

    if (path.empty())
    {
        char dir[] = "/tmp/fsimasterXXXXXX";
        path = mkdtemp(dir);
    }
    return path;
}

} // namespace paths
