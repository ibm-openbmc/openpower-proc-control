// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

int main()
{
    sdbusplus::async::context ctx;
    std::unique_ptr<SysFS> sysfs = std::make_unique<SysFSImpl>();

    sdbusplus::server::manager_t objMgr{
        ctx, SiblingInterface::namespace_path::value};

    Application app{ctx, std::move(sysfs)};

    ctx.spawn([](sdbusplus::async::context& ctx) -> sdbusplus::async::task<> {
        ctx.request_name(SiblingInterface::interface);
        co_return;
    }(ctx));

    ctx.run();

    return 0;
}
