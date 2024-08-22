// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "sysfs.hpp"

#include <gmock/gmock.h>

class MockSysFS : public SysFS
{
  public:
    MOCK_METHOD((std::expected<uint32_t, int>), read,
                (const std::filesystem::path&), (const override));

    MOCK_METHOD(int, write, (const std::filesystem::path&, uint32_t),
                (const override));
};
