# Target description
SOURCES = $(notdir $(wildcard src/*.c))
TARGET = management-server
INCLUDEPATH = src

vpath %.c src
vpath %.h src

# Default config
include makefiles/config.mk

DEFINES += USE_LIST_TYPEINFO USE_TERMINAL_COLORS

# Default rules
include makefiles/rules.mk
