include $(APPDIR)/Make.defs

# M+++ built-in application info

PROGNAME  = $(CONFIG_EXAMPLES_M_THREE_PLUS_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_M_THREE_PLUS_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_M_THREE_PLUS_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_M_THREE_PLUS)

# M+++ init file

MAINSRC = server.c

# Build with WebAssembly when CONFIG_INTERPRETERS_WAMR is enabled

WASM_BUILD = y

# Mode of WebAssembly Micro Runtime

WAMR_MODE  = AOT

include $(APPDIR)/Application.mk
