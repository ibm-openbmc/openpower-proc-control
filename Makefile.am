AM_DEFAULT_SOURCE_EXT = .cpp

systemdsystemunit_DATA = \
  pcie-poweroff@.service \
  xyz.openbmc_project.Control.Host.NMI.service \
  nmi.service

bin_PROGRAMS = \
	openpower-proc-control \
	openpower-proc-nmi

openpower_proc_control_SOURCES = \
	proc_control.cpp \
	cfam_access.cpp \
	filedescriptor.cpp \
	registration.cpp \
	targeting.cpp \
	openpower_procedures.cpp \
	ext_interface.cpp

openpower_proc_nmi_SOURCES = \
	nmi_main.cpp \
	nmi_interface.cpp

CLEANFILES = openpower_procedures.cpp

openpower_proc_control_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) \
                                 $(PHOSPHOR_DBUS_INTERFACES_LIBS) \
                                 $(OPENPOWER_DBUS_INTERFACES_LIBS) \
                                 $(SDBUSPLUS_LIBS) \
                                 -lstdc++fs

openpower_proc_control_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS) \
                                  $(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
                                  $(OPENPOWER_DBUS_INTERFACES_CFLAGS) \
                                  $(SDBUSPLUS_CFLAGS)

openpower_proc_nmi_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) \
                             $(PHOSPHOR_DBUS_INTERFACES_LIBS) \
                             $(SDBUSPLUS_LIBS)

openpower_proc_nmi_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS) \
                              $(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
                              $(SDBUSPLUS_CFLAGS)

SUBDIRS = test

-include Makefile.generated
