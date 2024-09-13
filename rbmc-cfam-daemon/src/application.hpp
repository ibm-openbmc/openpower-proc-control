// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <sdbusplus/async.hpp>

class Application
{
  public:
    Application() = delete;
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /**
     * @brief Constructor
     * @param ctx - The async context object
     */
    explicit Application(sdbusplus::async::context& ctx) : ctx(ctx)
    {
        ctx.spawn(run());
    }

  private:
    /**
     * @brief Starts the CFAM-S read loop.
     */
    sdbusplus::async::task<> run();

    /**
     * @brief The async context object
     */
    sdbusplus::async::context& ctx;
};
