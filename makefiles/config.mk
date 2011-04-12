ifeq ($(USE_GCC),1)
CC = gcc
CXX = g++
LINK = g++
OFLAGS += 
CFLAGS += -pipe -std=c90 -ansi -pedantic -Wall -g $(OFLAGS)
CXXFLAGS += -pipe -pedantic -Wall -Wextra -g $(OFLAGS)
else
CC = clang
CXX = clang++
LINK = clang++
OFLAGS += 
CFLAGS += -Qunused-arguments -pipe -std=c90 -ansi -pedantic -Wall -g $(OFLAGS)
CXXFLAGS += -pipe -pedantic -Wall -Wextra -g $(OFLAGS)
endif
LDFLAGS = -lm

VERBOSE_MAKE ?= 0

ifeq ($(VERBOSE_MAKE),1)
	A := 
else
	A := @
endif
