// SPDX-License-Identifier: Apache-2.0
#include "sibling_bmc.hpp"

#include <phosphor-logging/lg2.hpp>

#include <format>

void SiblingBMC::read()
{
    bool createdObject = false;

    if (!cfam.isReady())
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
        return;
    }

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
        return;
    }

    if (!siblingObject)
    {
        lg2::info("Creating Sibling D-Bus interfaces");

        auto objectPath =
            sdbusplus::message::object_path{RedIntf::namespace_path::value} /
            RedIntf::namespace_path::sibling_bmc;

        siblingObject =
            std::make_unique<SiblingObject>(ctx.get_bus(), objectPath.str);

        siblingInterface = std::make_unique<SiblingInterface>(
            ctx.get_bus(), objectPath.str.c_str(),
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

    // Must detect a heartbeat change to consider it active, so it won't
    // be active until at least the second time though.
    auto heartbeat = cfam.getHeartbeat();
    auto alive = lastHeartbeat.has_value() &&
                 (heartbeat != lastHeartbeat.value());
    siblingObject->active(alive, createdObject);
    lastHeartbeat = heartbeat;

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
