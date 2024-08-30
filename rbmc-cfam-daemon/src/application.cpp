// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

sdbusplus::async::task<> Application::run()
{
    using namespace std::chrono_literals;

    co_await localBMC.start();

    while (!ctx.stop_requested())
    {
        // TODO: Read sibling CFAM
        co_await sdbusplus::async::sleep_for(ctx, 2s);
    }

    co_return;
}
