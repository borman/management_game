ifeq ($(USE_GCC),1)
CC = gcc
CFLAGS = -pipe -std=c90 -ansi -pedantic -Wall -g
else
CC = clang
CFLAGS = -Qunused-arguments -pipe -std=c90 -ansi -pedantic -Wall -g
endif
LDFLAGS = -lm

VERBOSE_MAKE ?= 0

ifeq ($(VERBOSE_MAKE),1)
	A := 
else
	A := @
endif
