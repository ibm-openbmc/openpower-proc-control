#pragma once
#include <targeting/target.H>

namespace ecmd_util
{
TARGETING::TargetPtr getProcTargetByPos(uint32_t posValue);

bool isChassisOn();

int startAttnHandler();

// Trigger obmcutil hostrebootoff->chassison->wait for chassison()
int istepPowerOn();

// Set host state to running
int setHostStateToRunning();
} // namespace ecmd_util
