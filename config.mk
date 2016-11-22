# Compiler settings
CC      ?= cc
CXX     ?= c++

CFLAGS  += -Wall -g -pg -std=c99 -O2 \
		   -Wno-unused-result
CXXFLAGS ?= -g -O2 -pg

CPPFLAGS ?= -D_GNU_SOURCE

LDFLAGS ?= -lm -pg
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

