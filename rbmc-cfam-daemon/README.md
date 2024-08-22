# RBMC CFAM Daemon

Each BMC in IBM's redundant BMC system has an FSI connection to its own CFAM-S
(CFAM-S = a standalone CFAM chip) as well as the sibling BMC's CFAM-S. The four
scratchpad registers in the Gemini mailbox block in these CFAMs will be used for
communication between the BMCs.

The local BMC will write the registers in its own CFAM. The sibling BMC would
then read these to obtain the information.

On startup the rbmc-cfamd application on the local BMC will collect the
information the sibling needs and write it into its own CFAM. It will watch
D-Bus in case it needs to update a field on a change.

It will read the sibling's CFAM every 2 seconds. A D-Bus interface will be
created to make these values available to other applications.

The `rbmc-cfam-daemon` meson option will enable building this code.

## CFAM-S ScratchPad Register Layout

| Field            | Register | Start Bit | Length |
| ---------------- | -------- | --------- | ------ |
| API Version      | 1        | 0         | 8      |
| BMC Position     | 1        | 8         | 1      |
| Role             | 1        | 9         | 2      |
| Red Enabled      | 1        | 11        | 1      |
| Failovers Paused | 1        | 12        | 1      |
| Provisioned      | 1        | 13        | 1      |
| BMC State        | 1        | 14        | 3      |
| Sibling Comms OK | 1        | 17        | 1      |
| Reserved         | 1        | 18        | 6      |
| Heartbeat        | 1        | 24        | 8      |
| FW Version       | 2        | 0         | 32     |
| Reserved         | 3        | 0         | 32     |
| Reserved         | 4        | 0         | 32     |

### Field Definitions

**API Version**: The version of the data layout.

**BMC Position**: The position the BMC thinks it is, such as 0 or 1. Each BMC
should have a unique value or something is wrong.

**Role**: The BMC's role value.

**Red Enabled**: If redundancy is enabled. In general this means file syncing is
active and failovers are possible unless they are paused. This field is only
valid when the BMC is active.

**Failovers Paused**: When redundancy is still enabled, i.e. file syncs are
still occurring, but failovers are not allowed. This could be set for example
during a boot. This field is only valid when the BMC is active.

**Provisioned**: If this BMC has been provisioned.

**BMC State**: The current state of the BMC. Matches the CurrentBMCState
property from the BMC state daemon.

**Sibling Comms OK**: If this BMC can read the sibling BMC's CFAM.

**Heartbeat**: This field is incremented by one every time the CFAM application
receives the Heartbeat signal from the RBMC state manager application. This can
be used by the sibling to know the management daemon is alive.

**FW Version**: A hash of the VERSION_ID field in /etc/os-release. This is used
to tell if the BMCs are running the same version of code.
