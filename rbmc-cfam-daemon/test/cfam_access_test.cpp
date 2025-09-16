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
    // Passes
    {
        MockDriver driver;

        EXPECT_CALL(driver, read(link0Device, 0)).WillOnce(Return(0x12345678));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::one), 0x12345678);
    }

    // 9 fails, then last retry works
    {
        MockDriver driver;

        EXPECT_CALL(driver, read(link0Device, 1))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(std::unexpected<int>{2}))
            .WillOnce(Return(0x12345678));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::two), 0x12345678);
    }

    // All retries fail
    {
        MockDriver driver;

        EXPECT_CALL(driver, read(link0Device, 1))
            .WillRepeatedly(Return(std::unexpected<int>{2}));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::two),
                  std::unexpected(2));
    }
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

    // One register fails all retries
    {
        MockDriver driver;
        EXPECT_CALL(driver, read(link0Device, 0))
            .WillOnce(Return(readValues[0]));
        EXPECT_CALL(driver, read(link0Device, 1))
            .WillOnce(Return(readValues[1]));
        EXPECT_CALL(driver, read(link0Device, 2))
            .WillRepeatedly(Return(std::unexpected<int>{2}));

        CFAMAccess cfam{0, driver};
        auto results = cfam.readScratchRegs(regs);
        EXPECT_EQ(results, std::unexpected<int>(2));
    }
}

// Test CFAMAccess::writeScratchReg()
TEST_F(CFAMAccessTest, WriteTest)
{
    // Write works
    {
        MockDriver driver;

        // First write works
        EXPECT_CALL(driver, write(link0Device, 0, 0x12345678))
            .WillOnce(Return(0));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678),
                  0);
    }

    // 9 fails, last retry works
    {
        MockDriver driver;

        EXPECT_CALL(driver, write(link0Device, 0, 0x12345678))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(1))
            .WillOnce(Return(0));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678),
                  0);
    }

    // All attempts fail
    {
        MockDriver driver;

        EXPECT_CALL(driver, write(link0Device, 0, 0x12345678))
            .WillRepeatedly(Return(1));

        CFAMAccess cfam{0, driver};

        EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678),
                  1);
    }
}

// Test CFAMAccess::writeScratchRegWithMask()
TEST_F(CFAMAccessTest, WriteWithMaskTest)
{
    // Write works
    {
        MockDriver driver;

        EXPECT_CALL(driver,
                    writeWithMask(link0Device, 0, 0x00AAAA00, 0x00FFFF00))
            .WillOnce(Return(0));

        CFAMAccess cfam{0, driver};

        cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
        EXPECT_EQ(cfam.writeScratchRegWithMask(op), 0);
    }

    // 9 fails, last retry works
    {
        MockDriver driver;

        EXPECT_CALL(driver,
                    writeWithMask(link0Device, 0, 0x00AAAA00, 0x00FFFF00))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(0));

        CFAMAccess cfam{0, driver};

        cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
        EXPECT_EQ(cfam.writeScratchRegWithMask(op), 0);
    }

    // All attempts fail
    {
        MockDriver driver;

        EXPECT_CALL(driver,
                    writeWithMask(link0Device, 0, 0x00AAAA00, 0x00FFFF00))
            .WillRepeatedly(Return(-1));

        CFAMAccess cfam{0, driver};

        cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
        EXPECT_EQ(cfam.writeScratchRegWithMask(op), -1);
    }
}
