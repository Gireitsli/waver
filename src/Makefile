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
# Author:  leibupro
#
# Date:    09/2015
#
#==========================================

# #########################################
#        MAKEFILE FOR DEBUG BUILDS        #
# #########################################

# Compiler
CC  = gcc

# Linker
LD  = gcc

# Compiler flags
CF  = -std=gnu99 -Wall -ggdb3 -O0

# Linker flags
LF  =

# Libraries
LB  =

INSTALL = install
PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin


# link results in binary
waver:	waver.o
	$(LD) -o waver waver.o $(LB)

# compile results in object file
waver.o: waver.c waver.h mtimer.h
	$(CC) $(CF) -c waver.c

# run as root or sudoer
install: installbin

installbin: waver
	$(INSTALL) -m 755 -s -o root -g root waver $(BINDIR)
	rm *.o waver

# run as root or sudoer
uninstall: $(BINDIR)/waver
	rm -f $(BINDIR)/waver

clean:
	rm *.o waver

