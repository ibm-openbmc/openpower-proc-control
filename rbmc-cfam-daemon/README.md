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
