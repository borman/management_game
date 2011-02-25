ifeq ($(USE_GCC),1)
CC = gcc
CXX = g++
LINK = g++
CFLAGS = -pipe -std=c90 -ansi -pedantic -Wall -g
CXXFLAGS = -pipe -pedantic -Wall -Wextra -g
else
CC = clang
CXX = clang++
LINK = clang++
CFLAGS = -Qunused-arguments -pipe -std=c90 -ansi -pedantic -Wall -g
CXXFLAGS = -pipe -pedantic -Wall -Wextra -g
endif
LDFLAGS = -lm

VERBOSE_MAKE ?= 0

ifeq ($(VERBOSE_MAKE),1)
	A := 
else
	A := @
endif
