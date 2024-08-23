// SPDX-License-Identifier: Apache-2.0
#include "mock_sysfs.hpp"
#include "sibling_cfam.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

TEST(SiblingCFAMTest, TestReads)
{
    using Expected = std::expected<uint32_t, int>;
    MockSysFS sysfs;

    SiblingCFAM sibling{1, sysfs};

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi1/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi1/slave@00:00/scratch2"};

    EXPECT_CALL(sysfs, read(reg1)).WillOnce(Return(Expected(0x01DDFFFF)));
    EXPECT_CALL(sysfs, read(reg2)).WillOnce(Return(Expected(0x12345678)));

    // Before first read, in error state
    EXPECT_TRUE(sibling.hasError());
    EXPECT_THROW(sibling.getApiVersion(), std::runtime_error);

    sibling.readAll();

    EXPECT_FALSE(sibling.hasError());

    EXPECT_EQ(sibling.getApiVersion(), 0x01);
    EXPECT_EQ(sibling.getBMCPosition(), 1);
    EXPECT_EQ(sibling.getProvisioned(), true);
    EXPECT_EQ(sibling.getRedundancyEnabled(), true);
    EXPECT_EQ(sibling.getFailoversPaused(), true);
    EXPECT_EQ(sibling.getBMCState(), SiblingCFAM::BMCState::Quiesced);
    EXPECT_EQ(sibling.getRole(), SiblingCFAM::Role::Passive);
    EXPECT_EQ(sibling.getSiblingCommsOK(), true);
    EXPECT_EQ(sibling.getHeartbeat(), 0xFF);
    EXPECT_EQ(sibling.getFWVersion(), 0x12345678);
}

TEST(SiblingCFAMTest, TestReadFail)
{
    MockSysFS sysfs;

    SiblingCFAM sibling{1, sysfs};

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi1/slave@00:00/scratch1"};

    EXPECT_CALL(sysfs, read(reg1)).WillRepeatedly(Return(std::unexpected{1}));

    sibling.readAll();

    EXPECT_TRUE(sibling.hasError());

    EXPECT_THROW(sibling.getApiVersion(), std::runtime_error);
    EXPECT_THROW(sibling.getFWVersion(), std::runtime_error);
    EXPECT_THROW(sibling.getBMCPosition(), std::runtime_error);
    EXPECT_THROW(sibling.getProvisioned(), std::runtime_error);
    EXPECT_THROW(sibling.getRedundancyEnabled(), std::runtime_error);
    EXPECT_THROW(sibling.getFailoversPaused(), std::runtime_error);
    EXPECT_THROW(sibling.getBMCState(), std::runtime_error);
    EXPECT_THROW(sibling.getRole(), std::runtime_error);
    EXPECT_THROW(sibling.getSiblingCommsOK(), std::runtime_error);
    EXPECT_THROW(sibling.getHeartbeat(), std::runtime_error);
}
