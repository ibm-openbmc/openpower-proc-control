// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "driver.hpp"

#include <gmock/gmock.h>

class MockDriver : public Driver
{
  public:
    MOCK_METHOD((std::expected<uint32_t, int>), read,
                (const std::filesystem::path&, uint32_t), (const override));

    MOCK_METHOD(int, write, (const std::filesystem::path&, uint32_t, uint32_t),
                (const override));

    MOCK_METHOD(int, writeWithMask,
                (const std::filesystem::path&, uint32_t, uint32_t, uint32_t),
                (const override));
};
