// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

sdbusplus::async::task<> Application::run()
{
    using namespace std::chrono_literals;

    while (!ctx.stop_requested())
    {
        // TODO: Read CFAMs
        co_await sdbusplus::async::sleep_for(ctx, 2s);
    }

    co_return;
}
