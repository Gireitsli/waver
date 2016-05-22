#==========================================
#
#      ___           ___           ___     
#     /\__\         /\  \         /\__\    
#    /:/ _/_       /::\  \       /:/  /    
#   /:/ /\__\     /:/\:\  \     /:/  /     
#  /:/ /:/ _/_   /::\~\:\  \   /:/__/  ___ 
# /:/_/:/ /\__\ /:/\:\ \:\__\  |:|  | /\__\
# \:\/:/ /:/  / \/__\:\/:/  /  |:|  |/:/  /
#  \::/_/:/  /       \::/  /   |:|__/:/  / 
#   \:\/:/  /        /:/  /     \::::/__/  
#    \::/  /        /:/  /       ~~~~      
#     \/__/         \/__/                  
#
# WAV creator
# ===================
#
# Purpose: Makefile to create a debug
#          and a release build of the
#          waver program.
#
#          - Targets to install or
#            uninstall the waver
#            program system wide
#            are also provided.
#            (MUST be run as root
#            or sudoer.)
#
#
# Author:  P. Leibundgut <pl@vqe.ch>
#
#
# Date:    05/2016
#
#==========================================

# Directories
INCDIR    = ./include
SRCDIR    = ./src
OBJDIR    = ./obj
BINDIR    = ./bin
DBGBINDIR = $(BINDIR)/debug
RELBINDIR = $(BINDIR)/release


# Files
HDR  = $(INCDIR)/cpuinfo.h
HDR += $(INCDIR)/mtimer.h
HDR += $(INCDIR)/waver.h

SRC = $(SRCDIR)/waver.c

OBJDBG = $(subst $(SRCDIR),$(OBJDIR),$(SRC:.c=_dbg.o))
OBJREL = $(subst $(SRCDIR),$(OBJDIR),$(SRC:.c=_rel.o))

BINNAME = waver
BINDBG = $(DBGBINDIR)/$(BINNAME)
BINREL = $(RELBINDIR)/$(BINNAME)


# vpath variable for pattern rules to look into the
# directories specified in vpath as well when 
# searching the dependency files.
VPATH = $(SRCDIR)


# system installation locations ...
PREFIX      = /usr/local
INSTALLDIR  = $(PREFIX)/bin


# Compiler
CC  = gcc


# Linker
LD  = gcc


# Compiler flags
CFDBG  = -std=gnu99 -Wall -ggdb3 -O0
CFREL  = -std=gnu99 -Wall -march=native -funroll-loops -O3

INCLUDES = -I$(INCDIR)


# Linker flags
LF  =


# Libraries
LB  = -lpthread


# phony targets
.PHONY: all install uninstall clean


# all the files/directories we want in the end.
all: $(BINDBG) $(BINREL)


# Directory targets
$(OBJDIR):
	mkdir $@

$(BINDIR):
	mkdir $@

$(DBGBINDIR): $(BINDIR)
	mkdir $@

$(RELBINDIR): $(BINDIR)
	mkdir $@


# Binaries
$(BINDBG): $(OBJDBG) $(DBGBINDIR)
	$(LD) -o $@ $(OBJDBG) $(LB)

$(BINREL): $(OBJREL) $(RELBINDIR)
	$(LD) -o $@ $(OBJREL) $(LB)


# Pattern rules to compile the sources
$(OBJDIR)/%_dbg.o: %.c $(HDR) $(OBJDIR)
	$(CC) $(CFDBG) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%_rel.o: %.c $(HDR) $(OBJDIR)
	$(CC) $(CFREL) $(INCLUDES) -c $< -o $@


# run this target as root or sudoer
install: $(BINREL)
	install -m 755 -s -o root -g root $< $(INSTALLDIR)
	chown -R $(shell echo $$SUDO_USER) $(OBJDIR) $(BINDIR)
	chgrp -R $(shell echo $$SUDO_USER) $(OBJDIR) $(BINDIR)

# run this target as root or sudoer
uninstall: $(INSTALLDIR)/$(BINNAME)
	rm -f $<


clean:
	rm -r $(OBJDIR) $(BINDIR)

