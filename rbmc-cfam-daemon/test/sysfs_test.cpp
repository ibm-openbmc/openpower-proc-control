// SPDX-License-Identifier: Apache-2.0
#include "sysfs.hpp"

#include <format>
#include <fstream>

#include <gtest/gtest.h>

class SysFSTest : public ::testing::Test
{
  protected:
    static void SetUpTestCase()
    {
        char d[] = "/tmp/sysfstestXXXXXX";
        dir = mkdtemp(d);
    }

    static void TearDownTestCase()
    {
        std::filesystem::remove_all(dir);
    }

    void initFile(std::string_view name, uint32_t value)
    {
        auto path = dir / name;
        FILE* fp = fopen(path.c_str(), "w");
        if (fp == nullptr)
        {
            throw std::runtime_error(
                std::format("Failed to open {}", path.string()));
        }

        auto rc = fwrite(&value, 4, 1, fp);
        fclose(fp);
        if (rc != 1)
        {
            throw std::runtime_error(
                std::format("Failed writing {}", path.string()));
        }
    }

    static std::filesystem::path dir;
};

std::filesystem::path SysFSTest::dir;

TEST_F(SysFSTest, SysFSTest)
{
    SysFSImpl sysfs;
    auto path = SysFSTest::dir / "scratch1";

    // Create the file
    initFile(path.c_str(), 0x00000000);

    EXPECT_EQ(sysfs.write(path, 0x12345678), 0);

    EXPECT_EQ(sysfs.read(path), 0x12345678);

    path = SysFSTest::dir / "scratch2";

    // Doesn't exist
    EXPECT_EQ(sysfs.read(path), std::unexpected<int>{ENOENT});

    // Doesn't exist
    path = SysFSTest::dir / "scratch2";
    EXPECT_EQ(sysfs.write(path, 0x12345678), ENOENT);
}