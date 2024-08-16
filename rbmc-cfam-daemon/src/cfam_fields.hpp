// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "cfam_types.hpp"

#include <cstddef>
#include <cstdint>

namespace cfam
{

/**
 * Metadata about a field in the CFAM-S scratchpad.
 */
struct FieldMetadata
{
    ScratchPadReg reg;
    size_t start;
    size_t numBits;
};

/**
 * @brief Extracts a field from the data passed in.
 *
 * Throws an exception if start + numBits goes past 32.
 *
 * @param[in] data - The data to extra the field from
 * @param[in] start - The start bit
 * @param[in] numBits - The number of bits to extract
 *
 * @return The extracted field in a uint32_t.
 */
uint32_t getFieldValue(uint32_t data, size_t start, size_t numBits);

/**
 * @brief Extracts a field from a map of register names/values using
 *        the metadata about that field.
 *
 * @param[in] regs - Map of reg name -> reg values
 * @param[in] meta - The metadata (reg, start, length) about the field
 *
 * @return The extracted field in a uint32_t.
 */
uint32_t getFieldValue(const RegMap& regs, const FieldMetadata& meta);

/**
 * @brief Creates a ModifyOp struct that is used to do a read/modify/write
 *        to write the field value passed in.
 *
 * For example, if a field was 16 bits starting at bit 8 and
 * the field value to write is 0x00001234:
 *
 *     ModifyOp.reg = <the register to write to, from the metadata>
 *     ModifyOp.data = 0x00123400
 *     ModifyOp.mask = 0x00FFFF00
 *
 * @param[in] meta - The metadata (reg, start, length) about the field
 * @param[in] fieldValue - The value of the field to write
 *
 * @return The ModifyOp struct used to do the write
 *
 */
ModifyOp createModifyOp(const FieldMetadata& meta, uint32_t fieldValue);

} // namespace cfam
