// SPDX-License-Identifier: Apache-2.0
#include "local_cfam.hpp"
#include "mock_sysfs.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

TEST(LocalCFAMTest, GetMaxValueTest)
{
    EXPECT_EQ(local_cfam_util::getMaxValue(4), 0xF);
    EXPECT_EQ(local_cfam_util::getMaxValue(8), 0xFF);
    EXPECT_EQ(local_cfam_util::getMaxValue(9), 0x1FF);
    EXPECT_EQ(local_cfam_util::getMaxValue(24), 0x00FFFFFF);
}

TEST(LocalCFAMTest, TestWriteApiVersion)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0xFF000000)).WillOnce(Return(0));

    cfam.writeApiVersion(0xFF);
}

TEST(LocalCFAMTest, TestWriteBMCPosition)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00800000)).WillOnce(Return(0));

    cfam.writeBMCPosition(1);
}

TEST(LocalCFAMTest, TestWriteRole)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00400000)).WillOnce(Return(0));

    cfam.writeRole(LocalCFAM::Role::Passive);
}

TEST(LocalCFAMTest, TestWriteRedundancyEnabled)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00100000)).WillOnce(Return(0));

    cfam.writeRedundancyEnabled(true);
}

TEST(LocalCFAMTest, TestWriteFailoversPaused)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00080000)).WillOnce(Return(0));

    cfam.writeFailoversPaused(true);
}
TEST(LocalCFAMTest, TestWriteProvisioned)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00040000)).WillOnce(Return(0));

    cfam.writeProvisioned(true);
}

TEST(LocalCFAMTest, TestWriteBMCState)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00018000)).WillOnce(Return(0));

    cfam.writeBMCState(LocalCFAM::BMCState::Quiesced);
}

TEST(LocalCFAMTest, TestWriteSiblingCommsOK)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x00004000)).WillOnce(Return(0));

    cfam.writeSiblingCommsOK(true);
}

TEST(LocalCFAMTest, TestWriteFWVersion)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};

    EXPECT_CALL(sysfs, read(reg)).WillOnce(Return(Expected(0)));
    EXPECT_CALL(sysfs, write(reg, 0x12345678)).WillOnce(Return(0));

    cfam.writeFWVersion(0x12345678);
}

TEST(LocalCFAMTest, TestIncHeartbeat)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg))
        .WillOnce(Return(Expected(0x000000FE)))  // Get initial HB
        .WillOnce(Return(Expected(0x000000FE)))  // RMW 1
        .WillOnce(Return(Expected(0x000000FF)))  // RMW 2
        .WillOnce(Return(Expected(0x00000000))); // RMW 3
    EXPECT_CALL(sysfs, write(reg, 0x000000FF)).WillOnce(Return(0));
    EXPECT_CALL(sysfs, write(reg, 0x00000000)).WillOnce(Return(0));
    EXPECT_CALL(sysfs, write(reg, 0x00000001)).WillOnce(Return(0));

    cfam.incHeartbeat();
    cfam.incHeartbeat();
    cfam.incHeartbeat();
}

TEST(LocalCFAMTest, TestWriteFails)
{
    MockSysFS sysfs;
    LocalCFAM cfam{0, sysfs};
    std::filesystem::path reg{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg)).WillRepeatedly(Return(std::unexpected{1}));

    EXPECT_THROW(cfam.writeApiVersion(0xFF), std::system_error);
    EXPECT_THROW(cfam.writeRole(LocalCFAM::Role::Passive), std::system_error);
    EXPECT_THROW(cfam.writeRedundancyEnabled(true), std::system_error);
    EXPECT_THROW(cfam.writeFailoversPaused(true), std::system_error);
    EXPECT_THROW(cfam.writeProvisioned(true), std::system_error);
    EXPECT_THROW(cfam.writeBMCState(LocalCFAM::BMCState::Quiesced),
                 std::system_error);
    EXPECT_THROW(cfam.writeSiblingCommsOK(true), std::system_error);

    EXPECT_THROW(cfam.incHeartbeat(), std::system_error);
}
