// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

sdbusplus::async::task<> Application::run()
{
    using namespace std::chrono_literals;
    CFAMAccess link1{1, *sysfs.get()};

    co_await localBMC.start();

    while (!ctx.stop_requested())
    {
        // Eventually what's off of link 1 may not be a BMC and we'll need
        // a way to know that.  For now, assume it's the sibling BMC's CFAM
        // if it's there.
        if (!siblingBMC && link1.exists())
        {
            siblingBMC = std::make_unique<SiblingBMC>(ctx, 1, *sysfs.get());
        }

        if (siblingBMC)
        {
            siblingBMC->read();
            localBMC.setSiblingCommsOK(siblingBMC->ok());
        }

        co_await sdbusplus::async::sleep_for(ctx, 2s);
    }

    co_return;
}
