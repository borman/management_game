# Target description
MODULES = core server
SOURCE_DIRS = $(addprefix src/,$(MODULES))
SOURCES = $(notdir $(wildcard $(addsuffix /*.c,$(SOURCE_DIRS))))
TARGET = management-server
INCLUDEPATH = src

vpath %.c $(SOURCE_DIRS)
vpath %.h $(SOURCE_DIRS)

# Default config
include makefiles/config.mk

DEFINES += _POSIX_C_SOURCE=201011

# Prefix each log message with current time
DEFINES += USE_LOG_TIME_MARKERS
# Do runtime type checks for list operations
DEFINES += USE_LIST_TYPEINFO
# Print messages in color
DEFINES += USE_TERMINAL_COLORS

# Default rules
include makefiles/rules.mk
