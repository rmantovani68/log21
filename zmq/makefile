# ---------------------------------------------------------
# Makefile for Easy Picking - Main Process
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

# --------------------------
# Program debug flags
# --------------------------

# debug version
DEFDEBUG = -g -DDEBUG -DTRACE

# release version
#DEFDEBUG = -O 

# --------------------------
# user defines
# --------------------------

# User Definitions
USR_DEF =  -DTRACE 

# include file contenente le definizioni generiche
include ../common/picking.mak

# libraries
LIBS = $(ZMQ_LIBS)

# includes 
INCLUDES = -I. $(ZMQ_CFLAGS)

# main target
all: hwserver hwclient

# dependencies

hwserver.o: hwserver.c

hwclient.o: hwclient.c

# build rules
hwserver: hwserver.o
	@echo 'Linking ... $@'
	@$(CC) hwserver.o $(LIBS) -o hwserver
	@echo 'Terminated'

hwclient: hwclient.o
	@echo 'Linking ... $@'
	@$(CC) hwclient.o $(LIBS) -o hwclient
	@echo 'Terminated'

clean:
	@echo '------ Clean ...'
	@rm -f hwclient hwserver *.o
	@echo '------ Terminated'


