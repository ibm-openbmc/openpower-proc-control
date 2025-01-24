// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "driver.hpp"
#include "local_bmc.hpp"
#include "sibling_bmc.hpp"

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
     *
     * @param ctx - The async context object
     * @param fs - The Driver object
     */
    Application(sdbusplus::async::context& ctx, std::unique_ptr<Driver> d) :
        ctx(ctx), driver(std::move(d)), localBMC(ctx, *driver.get())
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

    /**
     * @brief Driver object for the CFAM.
     */
    std::unique_ptr<Driver> driver;

    /**
     * @brief Handles logic for the local BMC.
     */
    LocalBMC localBMC;

    /**
     * @brief Handles logic for the sibling BMC.
     *
     * Only created if sibling is present.
     */
    std::unique_ptr<SiblingBMC> siblingBMC;
};
