// SPDX-License-Identifier: Apache-2.0
#include "sibling_bmc.hpp"

#include <phosphor-logging/lg2.hpp>

#include <format>

void SiblingBMC::read()
{
    bool createdIface = false;

    if (!cfam.isReady())
    {
        if (ready)
        {
            lg2::info("Sibling CFAM device changed to not ready");
            ready = false;
        }

        if (siblingInterface)
        {
            lg2::info(
                "Removing sibling interface from D-Bus due to CFAM not ready");
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
        if (siblingInterface)
        {
            lg2::info("Removing sibling API from D-Bus due to CFAM error");
            siblingInterface.reset();
        }
        return;
    }

    if (!siblingInterface)
    {
        lg2::info("Creating Sibling D-Bus interface");

        auto objectPath =
            sdbusplus::message::object_path{
                SiblingInterface::namespace_path::value} /
            SiblingInterface::namespace_path::bmc;

        siblingInterface = std::make_unique<SiblingInterface>(
            ctx.get_bus(), objectPath.str.c_str(),
            SiblingInterface::action::defer_emit);

        createdIface = true;
    }

    siblingInterface->communicationOK(cfam.getSiblingCommsOK(), createdIface);
    siblingInterface->bmcPosition(cfam.getBMCPosition(), createdIface);
    siblingInterface->provisioned(cfam.getProvisioned(), createdIface);
    siblingInterface->role(cfam.getRole(), createdIface);
    siblingInterface->redundancyEnabled(cfam.getRedundancyEnabled(),
                                        createdIface);
    siblingInterface->failoversPaused(cfam.getFailoversPaused(), createdIface);
    siblingInterface->bmcState(cfam.getBMCState(), createdIface);

    auto version = std::format("{:X}", cfam.getFWVersion());
    siblingInterface->fwVersion(version, createdIface);

    // Must detect a heartbeat change to consider it active, so it won't
    // be active until at least the second time though.
    auto heartbeat = cfam.getHeartbeat();
    auto alive = lastHeartbeat.has_value() &&
                 (heartbeat != lastHeartbeat.value());
    siblingInterface->heartbeat(alive, createdIface);
    lastHeartbeat = heartbeat;

    if (createdIface)
    {
        siblingInterface->emit_object_added();
    }
}
