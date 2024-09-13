// SPDX-License-Identifier: Apache-2.0
#include "application.hpp"

int main()
{
    sdbusplus::async::context ctx;

    Application app{ctx};

    ctx.run();

    return 0;
}
