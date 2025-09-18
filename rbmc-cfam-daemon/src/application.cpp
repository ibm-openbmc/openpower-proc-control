// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

sdbusplus::async::task<> Application::run()
{
    using namespace std::chrono_literals;
    CFAMAccess link1{1, *driver.get()};

    co_await localBMC.start();

    while (!ctx.stop_requested())
    {
        if (!siblingBMC && link1.exists())
        {
            siblingBMC = std::make_unique<SiblingBMC>(ctx, 1, *driver.get(),
                                                      availInterface);
        }

        if (siblingBMC)
        {
            co_await siblingBMC->read();
            localBMC.setSiblingCommsOK(siblingBMC->ok());
        }

        co_await sdbusplus::async::sleep_for(ctx, 2s);
    }

    co_return;
}
