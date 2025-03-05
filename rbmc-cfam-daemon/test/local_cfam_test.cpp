// SPDX-License-Identifier: Apache-2.0
#include "local_cfam.hpp"
#include "mock_driver.hpp"
#include "test_utils.hpp"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class LocalCFAMTest : public CFAMSDevice
{};

TEST_F(LocalCFAMTest, GetMaxValueTest)
{
    EXPECT_EQ(local_cfam_util::getMaxValue(4), 0xF);
    EXPECT_EQ(local_cfam_util::getMaxValue(8), 0xFF);
    EXPECT_EQ(local_cfam_util::getMaxValue(9), 0x1FF);
    EXPECT_EQ(local_cfam_util::getMaxValue(24), 0x00FFFFFF);
}

TEST_F(LocalCFAMTest, TestWriteApiVersion)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0xFF000000, 0xFF000000))
        .WillOnce(Return(0));

    cfam.writeApiVersion(0xFF);
}

TEST_F(LocalCFAMTest, TestWriteBMCPosition)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00800000, 0x00800000))
        .WillOnce(Return(0));

    cfam.writeBMCPosition(1);
}

TEST_F(LocalCFAMTest, TestWriteRole)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00400000, 0x00600000))
        .WillOnce(Return(0));

    cfam.writeRole(LocalCFAM::Role::Passive);
}

TEST_F(LocalCFAMTest, TestWriteRedundancyEnabled)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00100000, 0x00100000))
        .WillOnce(Return(0));

    cfam.writeRedundancyEnabled(true);
}

TEST_F(LocalCFAMTest, TestWriteFailoversPaused)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00080000, 0x00080000))
        .WillOnce(Return(0));

    cfam.writeFailoversPaused(true);
}
TEST_F(LocalCFAMTest, TestWriteProvisioned)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00040000, 0x00040000))
        .WillOnce(Return(0));

    cfam.writeProvisioned(true);
}

TEST_F(LocalCFAMTest, TestWriteBMCState)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00018000, 0x00038000))
        .WillOnce(Return(0));

    cfam.writeBMCState(LocalCFAM::BMCState::Quiesced);
}

TEST_F(LocalCFAMTest, TestWriteSiblingCommsOK)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00004000, 0x00004000))
        .WillOnce(Return(0));

    cfam.writeSiblingCommsOK(true);
}

TEST_F(LocalCFAMTest, TestWriteFWVersion)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, 1, 0x12345678, 0xFFFFFFFF))
        .WillOnce(Return(0));

    cfam.writeFWVersion(0x12345678);
}

TEST_F(LocalCFAMTest, TestIncHeartbeat)
{
    using Expected = std::expected<uint32_t, int>;
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, read(link0Device, 0))
        .WillOnce(Return(Expected(0x000000FE))); // Get initial HB

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x000000FF, 0xFF))
        .WillOnce(Return(0));
    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00000000, 0xFF))
        .WillOnce(Return(0));
    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00000001, 0xFF))
        .WillOnce(Return(0));

    cfam.incHeartbeat();
    cfam.incHeartbeat();
    cfam.incHeartbeat();
}

TEST_F(LocalCFAMTest, TestWriteFails)
{
    MockDriver driver;
    LocalCFAM cfam{0, driver};

    EXPECT_CALL(driver, writeWithMask(link0Device, _, _, _))
        .WillRepeatedly(Return(-1));

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
