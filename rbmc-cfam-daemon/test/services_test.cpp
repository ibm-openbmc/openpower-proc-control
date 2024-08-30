// SPDX-License-Identifier: Apache-2.0
#include "services.hpp"

#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

fs::path writeOsRelease(const char* data)
{
    char path[] = "/tmp/osReleaseXXXXXX";

    auto rc = mkdtemp(path);
    if (rc == nullptr)
    {
        throw std::runtime_error("Could not create temp dir");
    }

    fs::path file{path};
    file /= "os-release";
    std::ofstream stream{file};
    stream << data;

    return file;
}

TEST(UtilsTest, TestGetVersionID)
{
    const auto osRelease = R"-(
ID=MY_ID
NAME="BMC (OpenBMC for Foo Systems)"
VERSION="fw2842.00-8"
VERSION_ID=fw2842.00-8-2842.20240425 (FOO)
)-";

    auto file = writeOsRelease(osRelease);

    EXPECT_EQ(Services::getFWVersionID(file),
              std::string{"fw2842.00-8-2842.20240425 (FOO)"});

    fs::remove_all(file.parent_path());
}

TEST(UtilsTest, TestGetVersionIDWithQuotes)
{
    const auto osRelease = R"-(
ID=MY_ID
NAME="BMC (OpenBMC for Foo Systems)"
VERSION="fw2842.00-8"
VERSION_ID="fw2842.00-8-2842.20240425 (FOO)"
)-";

    auto file = writeOsRelease(osRelease);

    EXPECT_EQ(Services::getFWVersionID(file),
              std::string{"fw2842.00-8-2842.20240425 (FOO)"});

    fs::remove_all(file.parent_path());
}

TEST(UtilsTest, TestNoVersionID)
{
    const auto osRelease = R"-(
ID=MY_ID
NAME="BMC (OpenBMC for Foo Systems)"
VERSION="fw2842.00-8"
CPE_NAME="cpe:/o:openembedded:openbmc-foo:fw2842.00-8"
)-";

    auto file = writeOsRelease(osRelease);

    EXPECT_THROW(Services::getFWVersionID(file), std::runtime_error);

    fs::remove_all(file.parent_path());
}
