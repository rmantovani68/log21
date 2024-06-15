# ---------------------------------------------------------
# Makefile for Easy Picking 3.0
# Gestione Pesatura Colli / Fine Linea
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

# --------------------------
# Project Name (executable)
# --------------------------
PRJ_NAME = scarti

# --------------------------
# Program debug flags
# --------------------------

# debug version
DEFDEBUG = -g -DDEBUG

# release version
#DEFDEBUG = -O 

# --------------------------
# user defines
# --------------------------

# User Definitions
#USR_DEF =  -DHAVE_CONFIG_H -DTRACE -DTRACE_PLC
USR_DEF =  -DHAVE_CONFIG_H -DTRACE
#USR_DEF =  -DHAVE_CONFIG_H -DTRACE

# include file contenente le definizioni generiche
include ../common/picking.mak


# object files componenti il progetto
OBJS = 

# sorgenti componenti il progetto
SRCS = main.c mainfun.c mainproc.c mainutil.c mainvar.c callbacks.c interface.c

# libraries
LIBS = $(GNOME_LIB) $(GTK_LIB) $(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(ZMQ_LIBS)

# includes 
INCLUDES = -I. $(GNOME_CFLAGS) $(GTK_CFLAGS) $(EP_INC) $(MSQ_INC) $(PSQL_INC)

# Linker flags 
LFLAGS = $(LIBS)



all: $(EXE_DIR)/$(PRJ_NAME) 





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



main.o: main.c $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

mainfun.o: mainfun.c $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

mainproc.o: mainproc.c $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

mainutil.o: mainutil.c $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

mainvar.o: mainvar.c $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

callbacks.o: callbacks.c callbacks.h $(COMMON_DEPS) main.h mainfun.h mainext.h mainstruct.h 

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


