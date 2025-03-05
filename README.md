# openpower-proc-control

Contains:

1. Procedures that interact with the OpenPower nest chipset.
2. An application that
   [interfaces with CFAM-S chips](rbmc-cfam-daemon/README.md) on certain
   systems.

## To Build

To build this package, do the following steps:

    1. meson setup builddir
    2. meson compile -C builddir

To build with phal feature:

    1. meson setup builddir -Dphal=enabled -Dopenfsi=enabled
    2. meson compile -C builddir

To clean the repository run `ninja -C builddir/ clean`.
