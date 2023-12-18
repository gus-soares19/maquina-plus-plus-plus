include $(APPDIR)/Make.defs

# M+++ built-in application info

PROGNAME  = $(CONFIG_EXAMPLES_M3P_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_M3P_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_M3P_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_M3P)

# M+++ init file

MAINSRC = server.c

include $(APPDIR)/Application.mk
