// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

int main()
{
    sdbusplus::async::context ctx;
    std::unique_ptr<SysFS> sysfs = std::make_unique<SysFSImpl>();

    Application app{ctx, std::move(sysfs)};

    ctx.run();

    return 0;
}
