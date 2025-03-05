// SPDX-License-Identifier: Apache-2.0
#include "mock_driver.hpp"
#include "sibling_cfam.hpp"
#include "test_utils.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

class SiblingCFAMTest : public CFAMSDevice
{};

TEST_F(SiblingCFAMTest, TestReads)
{
    using Expected = std::expected<uint32_t, int>;
    MockDriver driver;

    SiblingCFAM sibling{1, driver};

    EXPECT_CALL(driver, read(link1Device, 0))
        .WillOnce(Return(Expected(0x01DDFFFF)));
    EXPECT_CALL(driver, read(link1Device, 1))
        .WillOnce(Return(Expected(0x12345678)));

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

TEST_F(SiblingCFAMTest, TestReadFail)
{
    MockDriver driver;

    SiblingCFAM sibling{1, driver};

    EXPECT_CALL(driver, read(link1Device, 0))
        .WillRepeatedly(Return(std::unexpected{1}));

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
