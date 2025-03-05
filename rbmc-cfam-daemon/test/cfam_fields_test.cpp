// SPDX-License-Identifier: Apache-2.0
#include "bmc_cfam.hpp"
#include "cfam_fields.hpp"

#include <format>
#include <ranges>

#include <gtest/gtest.h>

using namespace cfam;

TEST(CFAMFields, NoOverLappingFieldsTest)
{
    std::array<uint32_t, numScratchPadRegs> regs{};

    // Make sure there aren't any overlapping fields
    // in BMCCFAM::cfamFields.
    for (const auto& [field, meta] : BMCCFAM::cfamFields)
    {
        auto regIndex = std::to_underlying(meta.reg);

        for (auto i : std::views::iota(meta.start, meta.start + meta.numBits))
        {
            if (regs[regIndex] & (0x80000000 >> i))
            {
                ADD_FAILURE()
                    << "Reg " << regIndex << " field "
                    << static_cast<int>(field)
                    << " has overlapping bits at offset " << i << "\n";
            }
            regs[regIndex] |= (0x80000000 >> i);
        }
    }

    auto summary = std::ranges::fold_left(
        regs, "The used bits are:", [](const auto& front, const auto& field) {
            return std::format("{} {:#010x}", front, field);
        });
    std::cout << summary << '\n';
}

// Extract fields from a register
TEST(CFAMFields, GetFieldValueTest)
{
    uint32_t data{0x123ABCDE};

    EXPECT_EQ(getFieldValue(data, 0, 12), 0x123);
    EXPECT_EQ(getFieldValue(data, 12, 3), 0x5);
    EXPECT_EQ(getFieldValue(data, 16, 1), 0x1);
    EXPECT_EQ(getFieldValue(data, 17, 1), 0x0);
    EXPECT_EQ(getFieldValue(data, 18, 5), 0x1E);
    EXPECT_EQ(getFieldValue(data, 24, 4), 0xD);
    EXPECT_EQ(getFieldValue(data, 0, 32), data);

    // Overflow 32 bits
    EXPECT_THROW(getFieldValue(data, 30, 5), std::invalid_argument);
}

// Test extracting fields from a map of registers.
TEST(CFAMFields, GetFieldValueRegsTest)
{
    RegMap regs{{ScratchPadReg::two, 0x80000000},
                {ScratchPadReg::three, 0x00500000}};

    // The actual extraction is tested in GetFieldValueTest
    FieldMetadata meta1{ScratchPadReg::two, 0, 1, "a"};
    FieldMetadata meta2{ScratchPadReg::two, 9, 5, "a"};
    FieldMetadata meta3{ScratchPadReg::three, 9, 5, "a"};
    FieldMetadata meta4{ScratchPadReg::four, 0, 1, "a"};

    EXPECT_EQ(getFieldValue(regs, meta1), 1);
    EXPECT_EQ(getFieldValue(regs, meta2), 0);
    EXPECT_EQ(getFieldValue(regs, meta3), 0x14);

    // ScratchPadReg::four doesn't exist.
    EXPECT_THROW(getFieldValue(regs, meta4), std::invalid_argument);
}

// Test creating various reg/data/mask ModifyOps
TEST(CFAMFields, CreateModifyDataTest)
{
    // input = metadata, value of field
    using Input = std::tuple<FieldMetadata, uint32_t>;

    // List of testcases with the inputs and then the expected ModifyOp values
    const std::vector<std::tuple<Input, ModifyOp>> testcases{

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 0, 32, "a"},
                         0x11111111},
                   ModifyOp{ScratchPadReg::three, 0x11111111, 0xFFFFFFFF}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 0, 1, "a"}, 0x0},
                   ModifyOp{ScratchPadReg::three, 0x0, 0x80000000}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 0, 1, "a"}, 0x1},
                   ModifyOp{ScratchPadReg::three, 0x80000000, 0x80000000}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 31, 1, "a"}, 0x1},
                   ModifyOp{ScratchPadReg::three, 0x1, 0x00000001}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 31, 1, "a"}, 0x0},
                   ModifyOp{ScratchPadReg::three, 0x0, 0x00000001}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::three, 0, 1, "a"}, 0x1},
                   ModifyOp{ScratchPadReg::three, 0x80000000, 0x80000000}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::one, 4, 8, "a"}, 0x55},
                   ModifyOp{ScratchPadReg::one, 0x05500000, 0xFF00000}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::two, 19, 3, "a"}, 0x5},
                   ModifyOp{ScratchPadReg::two, 0x00001400, 0x00001C00}},

        std::tuple{Input{FieldMetadata{ScratchPadReg::two, 11, 20, "a"},
                         0x34343},
                   ModifyOp{ScratchPadReg::two, 0x00068686, 0x001FFFFE}}};

    for (const auto& [metaAndValue, expected] : testcases)
    {
        const auto& meta = std::get<0>(metaAndValue);
        const auto& inputValue = std::get<1>(metaAndValue);

        auto actual = createModifyOp(meta, inputValue);

        EXPECT_EQ(expected.reg, actual.reg);
        EXPECT_EQ(expected.data, actual.data);
        EXPECT_EQ(expected.mask, actual.mask);
    }
}
