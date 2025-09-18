// SPDX-License-Identifier: Apache-2.0
#include "sibling_bmc.hpp"

#include <phosphor-logging/lg2.hpp>

#include <chrono>
#include <format>

sdbusplus::async::task<bool> SiblingBMC::checkCFAMReady()
{
    constexpr size_t maxAttempts = 10;
    size_t attempts = 0;

    while (attempts < maxAttempts)
    {
        if (cfam.isReady())
        {
            co_return true;
        }

        attempts++;
        if (attempts < maxAttempts)
        {
            if (ready)
            {
                lg2::warning("Sibling BMC CFAM not ready. Retrying");
            }
            co_await sdbusplus::async::sleep_for(ctx,
                                                 std::chrono::milliseconds(50));
        }
        else
        {
            if (ready)
            {
                lg2::error("Giving up");
            }
        }
    }
    co_return false;
}

sdbusplus::async::task<> SiblingBMC::read()
{
    bool createdObject = false;

    if (!co_await checkCFAMReady())
    {
        if (ready)
        {
            lg2::info("Sibling CFAM device changed to not ready");
            ready = false;
        }

        if (siblingObject)
        {
            lg2::info(
                "Removing sibling object from D-Bus due to CFAM not ready");
            siblingObject.reset();

            siblingInterface.reset();
        }

        availInterface.available(false);

        co_return;
    }

    availInterface.available(true);

    if (!ready)
    {
        lg2::info("Sibling CFAM device changed to ready");
        ready = true;
    }

    cfam.readAll();

    if (cfam.hasError())
    {
        if (siblingObject)
        {
            lg2::info("Removing sibling object from D-Bus due to CFAM error");
            siblingObject.reset();

            siblingInterface.reset();
        }
        co_return;
    }

    // Must detect a heartbeat change to consider it alive, so it won't
    // be alive until at least the second time though.
    auto heartbeat = cfam.getHeartbeat();
    auto alive = lastHeartbeat.has_value() &&
                 (heartbeat != lastHeartbeat.value());
    lastHeartbeat = heartbeat;

    if (!alive)
    {
        if (siblingObject)
        {
            lg2::info("Removing sibling object because heartbeat stopped");
            siblingObject.reset();

            siblingInterface.reset();
        }
        co_return;
    }

    if (!siblingObject)
    {
        lg2::info("Creating Sibling D-Bus interfaces");

        siblingObject = std::make_unique<SiblingObject>(
            ctx.get_bus(), getObjectPath().c_str());

        siblingInterface = std::make_unique<SiblingInterface>(
            ctx.get_bus(), getObjectPath().c_str(),
            SiblingInterface::action::defer_emit);

        createdObject = true;
    }

    siblingObject->role(cfam.getRole(), createdObject);
    siblingObject->redundancyEnabled(cfam.getRedundancyEnabled(),
                                     createdObject);
    siblingObject->failoversAllowed(cfam.getFailoversAllowed(), createdObject);
    siblingObject->currentBMCState(cfam.getBMCState(), createdObject);
    siblingObject->failoverImminent(cfam.getFailoverImminent(), createdObject);
    siblingObject->failoverInProgress(cfam.getFailoverInProgress(),
                                      createdObject);

    auto version = std::format("{:08X}", cfam.getFWVersion());
    siblingObject->version(version, createdObject);
    siblingObject->active(alive, createdObject);

    if (createdObject)
    {
        siblingObject->emit_object_added();
    }

    siblingInterface->communicationOK(cfam.getSiblingCommsOK(), createdObject);
    siblingInterface->bmcPosition(cfam.getBMCPosition(), createdObject);
    siblingInterface->provisioned(cfam.getProvisioned(), createdObject);
    siblingInterface->role(cfam.getRole(), createdObject);
    siblingInterface->redundancyEnabled(cfam.getRedundancyEnabled(),
                                        createdObject);
    siblingInterface->failoversAllowed(cfam.getFailoversAllowed(),
                                       createdObject);
    siblingInterface->bmcState(cfam.getBMCState(), createdObject);

    siblingInterface->fwVersion(version, createdObject);

    siblingInterface->heartbeat(alive, createdObject);

    if (createdObject)
    {
        siblingInterface->emit_object_added();
    }
}
