// SPDX-License-Identifier: Apache-2.0
#include "cfam_access.hpp"
#include "mock_driver.hpp"
#include "test_utils.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

class CFAMAccessTest : public CFAMSDevice
{};

// Test CFAMAccess::readScratchReg()
TEST_F(CFAMAccessTest, ReadTest)
{
    MockDriver driver;
    std::expected<uint32_t, int> expected = 0x12345678;

    // First read works, next one fails
    EXPECT_CALL(driver, read(link0Device, 0)).WillOnce(Return(expected));
    EXPECT_CALL(driver, read(link0Device, 1))
        .WillOnce(Return(std::unexpected<int>{2}));

    CFAMAccess cfam{0, driver};

    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::one), 0x12345678);
    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::two),
              std::unexpected<int>{2});
}

// Test CFAMAccess::readScratchRegs
TEST_F(CFAMAccessTest, ReadScratchRegsTest)
{
    std::set<cfam::ScratchPadReg> regs{
        cfam::ScratchPadReg::one, cfam::ScratchPadReg::two,
        cfam::ScratchPadReg::three, cfam::ScratchPadReg::four};

    std::array<uint32_t, 4> readValues{0x11111111, 0x22222222, 0x33333333,
                                       0x44444444};

    // Reads work
    {
        MockDriver driver;
        EXPECT_CALL(driver, read(link0Device, 0))
            .WillOnce(Return(readValues[0]));
        EXPECT_CALL(driver, read(link0Device, 1))
            .WillOnce(Return(readValues[1]));
        EXPECT_CALL(driver, read(link0Device, 2))
            .WillOnce(Return(readValues[2]));
        EXPECT_CALL(driver, read(link0Device, 3))
            .WillOnce(Return(readValues[3]));

        CFAMAccess cfam{0, driver};
        auto results = cfam.readScratchRegs(regs);

        cfam::RegMap expectedResults{
            {cfam::ScratchPadReg::one, readValues[0]},
            {cfam::ScratchPadReg::two, readValues[1]},
            {cfam::ScratchPadReg::three, readValues[2]},
            {cfam::ScratchPadReg::four, readValues[3]},
        };

        EXPECT_EQ(results, expectedResults);
    }

    // A read fails
    {
        MockDriver driver;
        EXPECT_CALL(driver, read(link0Device, 0))
            .WillOnce(Return(readValues[0]));
        EXPECT_CALL(driver, read(link0Device, 1))
            .WillOnce(Return(readValues[1]));
        EXPECT_CALL(driver, read(link0Device, 2))
            .WillOnce(Return(std::unexpected<int>{2}));

        CFAMAccess cfam{0, driver};
        auto results = cfam.readScratchRegs(regs);
        EXPECT_EQ(results, std::unexpected<int>(2));
    }
}

// Test CFAMAccess::writeScratchReg()
TEST_F(CFAMAccessTest, WriteTest)
{
    MockDriver driver;

    // First write works, next one fails
    EXPECT_CALL(driver, write(link0Device, 0, 0x12345678)).WillOnce(Return(0));
    EXPECT_CALL(driver, write(link0Device, 1, 0)).WillOnce(Return(-1));

    CFAMAccess cfam{0, driver};

    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678), 0);
    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::two, 0), -1);
}

// Test CFAMAccess::writeScratchRegWithMask()
TEST_F(CFAMAccessTest, WriteWithMaskTest)
{
    MockDriver driver;

    EXPECT_CALL(driver, writeWithMask(link0Device, 0, 0x00AAAA00, 0x00FFFF00))
        .WillOnce(Return(0));

    EXPECT_CALL(driver, writeWithMask(link0Device, 1, 0x00AAAA00, 0x00FFFF00))
        .WillOnce(Return(-1));

    CFAMAccess cfam{0, driver};

    // Good
    cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 0);

    // Fails
    op.reg = cfam::ScratchPadReg::two;
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), -1);
}
