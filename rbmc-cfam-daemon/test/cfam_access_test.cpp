// SPDX-License-Identifier: Apache-2.0
#include "cfam_access.hpp"
#include "mock_driver.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

// Test CFAMAccess::readScratchReg()
TEST(CFAMAccess, ReadTest)
{
    MockDriver driver;

    std::expected<uint32_t, int> expected = 0x12345678;
    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};

    // First read works, next one fails
    EXPECT_CALL(driver, read(reg1)).WillOnce(Return(expected));
    EXPECT_CALL(driver, read(reg2)).WillOnce(Return(std::unexpected<int>{2}));

    CFAMAccess cfam{0, driver};

    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::one), 0x12345678);
    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::two),
              std::unexpected<int>{2});
}

// Test CFAMAccess::readScratchRegs
TEST(CFAMAccess, ReadScratchRegsTest)
{
    std::set<cfam::ScratchPadReg> regs{
        cfam::ScratchPadReg::one, cfam::ScratchPadReg::two,
        cfam::ScratchPadReg::three, cfam::ScratchPadReg::four};

    std::array<std::filesystem::path, 4> paths{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1",
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2",
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch3",
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch4"};

    std::array<uint32_t, 4> readValues{0x11111111, 0x22222222, 0x33333333,
                                       0x44444444};

    // Reads work
    {
        MockDriver driver;
        EXPECT_CALL(driver, read(paths[0])).WillOnce(Return(readValues[0]));
        EXPECT_CALL(driver, read(paths[1])).WillOnce(Return(readValues[1]));
        EXPECT_CALL(driver, read(paths[2])).WillOnce(Return(readValues[2]));
        EXPECT_CALL(driver, read(paths[3])).WillOnce(Return(readValues[3]));

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
        EXPECT_CALL(driver, read(paths[0])).WillOnce(Return(readValues[0]));
        EXPECT_CALL(driver, read(paths[1])).WillOnce(Return(readValues[1]));
        EXPECT_CALL(driver, read(paths[2]))
            .WillOnce(Return(std::unexpected<int>{2}));

        CFAMAccess cfam{0, driver};
        auto results = cfam.readScratchRegs(regs);
        EXPECT_EQ(results, std::unexpected<int>(2));
    }
}

// Test CFAMAccess::writeScratchReg()
TEST(CFAMAccess, WriteTest)
{
    MockDriver driver;

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};

    // First write works, next one fails
    EXPECT_CALL(driver, write(reg1, 0x12345678)).WillOnce(Return(0));
    EXPECT_CALL(driver, write(reg2, 0)).WillOnce(Return(-1));

    CFAMAccess cfam{0, driver};

    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678), 0);
    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::two, 0), -1);
}

// Test CFAMAccess::writeScratchRegWithMask()
TEST(CFAMAccess, WriteWithMaskTest)
{
    MockDriver driver;

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};
    std::filesystem::path reg3{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch3"};

    std::expected<uint32_t, int> expected = 0x12345678;

    // A working read/write
    EXPECT_CALL(driver, read(reg1)).WillOnce(Return(expected));
    EXPECT_CALL(driver, write(reg1, 0x12AAAA78)).WillOnce(Return(0));

    // Read fails
    EXPECT_CALL(driver, read(reg2)).WillOnce(Return(std::unexpected<int>{2}));

    // Read works, write fails
    EXPECT_CALL(driver, read(reg3)).WillOnce(Return(expected));
    EXPECT_CALL(driver, write(reg3, 0x12AAAA78)).WillOnce(Return(3));

    CFAMAccess cfam{0, driver};

    cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 0);

    // Read fails
    op.reg = cfam::ScratchPadReg::two;
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 2);

    // Write fails
    op.reg = cfam::ScratchPadReg::three;
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 3);
}
