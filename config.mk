# Compiler settings
CC      ?= cc
CXX     ?= c++

CFLAGS  += -Wall -std=gnu99 -O2 \
		   -Wno-unused-result

CXXFLAGS ?= -g -O2

CPPFLAGS ?= -D_GNU_SOURCE

LDFLAGS ?= -lm
LEX     ?= flex
LFLAGS  ?=
YACC    ?= bison
YFLAGS  ?=
LD      ?= ld
PERL    ?= perl -w
MKDIR   ?= mkdir

# Install Paths
PREFIX  := /usr
BINDIR  := $(PREFIX)/bin
MANDIR  := $(PREFIX)/share/man
MAN1DIR := $(MANDIR)/man1
DOCDIR  := $(PREFIX)/share/doc/$(EXE)

# Show all commands executed by the Makefile
V ?= n

CONFIG_DEBUG ?= y

# if 'y', then the jit cpu core will be included
#
# Requires libjit
CONFIG_JIT ?= n

# Three different backend choices, which control the display and audio.
# This has to be chosen at compile time.
#
# The emscripten makes use of the sdl backend along with some extra settings
CONFIG_BACKEND ?= SDL
# CONFIG_BACKEND := PROTURA
# CONFIG_BACKEND := EMSCRIPTEN

