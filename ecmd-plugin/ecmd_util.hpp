#pragma once
#include <string>

namespace ecmd_util
{

bool isChassisOn();

int startAttnHandler();

// Trigger obmcutil hostrebootoff->chassison->wait for chassison()
int istepPowerOn();

// Set host state to running
int setHostStateToRunning();
} // namespace ecmd_util
