// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

int main()
{
    sdbusplus::async::context ctx;
    std::unique_ptr<Driver> driver = std::make_unique<DriverImpl>();

    sdbusplus::server::manager_t objMgr{
        ctx, SiblingInterface::namespace_path::value};

    Application app{ctx, std::move(driver)};

    // Wait for the local CFAM before claiming the D-Bus name.
    app.waitForLocalCFAM();

    ctx.spawn([](sdbusplus::async::context& ctx) -> sdbusplus::async::task<> {
        ctx.request_name(SiblingInterface::interface);
        co_return;
    }(ctx));

    ctx.run();

    return 0;
}
