# Target description
SOURCES = buffer.c socket_loop.c smq.c list.c log.c net_test.c 
INCLUDEPATH = ../../src/
TARGET = net-test

vpath %.c ../../src/core ../../src/server
vpath %.h ../../src/core 

# Default config
include ../../makefiles/config.mk

DEFINES += USE_LIST_TYPEINFO USE_TERMINAL_COLORS USE_LOG_TIME_MARKERS

# Default rules
include ../../makefiles/rules.mk
