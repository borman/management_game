# Target description
SOURCES = $(notdir $(wildcard src/*.c))
TARGET = management-server
INCLUDEPATH = src

vpath %.c src
vpath %.h src

# Default config
include makefiles/config.mk

# Prefix each log message with current time
DEFINES += USE_LOG_TIME_MARKERS
# Do runtime type checks for list operations
DEFINES += USE_LIST_TYPEINFO
# Print messages in color
DEFINES += USE_TERMINAL_COLORS

# Default rules
include makefiles/rules.mk
