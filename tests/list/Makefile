# Target description
SOURCES = list.c log.c list_test.c 
INCLUDEPATH = ../../src/
TARGET = list-test

vpath %.c ../../src/core ../../src/server
vpath %.h ../../src/core 

# Default config
include ../../makefiles/config.mk

DEFINES += USE_LIST_TYPEINFO USE_TERMINAL_COLORS

# Default rules
include ../../makefiles/rules.mk
