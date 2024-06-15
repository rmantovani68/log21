# ---------------------------------------------------------
# Makefile for Easy Picking - Ricezione dati Bartolini
# ---------------------------------------------------------
# Copyright CNI srl 1995-2001

# --------------------------
# Project Name (executable)
# --------------------------
PRJ_NAME = test

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
USR_DEF =  -DHAVE_CONFIG_H -DTRACE 

# include file contenente le definizioni generiche
include ../common/picking.mak


# object files componenti il progetto
OBJS = $(GEN_INC)/dbfun.o $(GEN_INC)/ca_file.o cmp_time.o $(GEN_INC)/support.o

# sorgenti componenti il progetto
SRCS = main.c mainfun.c mainproc.c mainutil.c mainvar.c callbacks.c interface.c 

# libraries
LIBS = $(GNOME_LIB) $(GTK_LIB) $(MSQ_LIB) $(PSQL_LIB)

# includes 
INCLUDES = -I. -I$(GEN_INC) $(GNOME_CFLAGS) $(GTK_CFLAGS) $(MSQ_INC) $(PSQL_INC)

# Linker flags 
LFLAGS = $(LIBS)

# Default compile rule
.c.o:
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $<

# main target
all: $(EXE_DIR)/$(PRJ_NAME) 

# dependencies
main.o: main.c $(COMMON_DEPS) mainfun.h mainext.h mainstruct.h

mainfun.o: mainfun.c $(COMMON_DEPS) mainfun.h mainext.h mainstruct.h

mainproc.o: mainproc.c $(COMMON_DEPS) mainfun.h mainext.h mainstruct.h

mainutil.o: mainutil.c $(COMMON_DEPS) mainfun.h mainext.h mainstruct.h

mainvar.o: mainvar.c $(COMMON_DEPS) mainfun.h mainext.h mainstruct.h

callbacks.o: callbacks.c callbacks.h 

interface.o: interface.c interface.h 

support.o: support.c support.h 


# build rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'

clean:
	@echo '------ Clean ...'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'
