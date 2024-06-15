# ---------------------------------------------------------
# A&L Easy Picking
# Makefile - Ricezione da files ascii
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = receive 

# Program debug flags
# debug version
DEFDEBUG = -g -DDEBUG -DTRACE

# release version
#DEFDEBUG = -O 

# user defines
USR_DEF = 

# include file contenente le definizioni generiche
include ../common/picking.mak


# object files componenti il progetto
OBJS =  cmp_time.c

# sorgenti componenti il progetto
SRCS = receive.c recvar.c recutil.c


# libraries
LIBS =$(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GLIB_LIB) $(ZMQ_LIBS)

# includes 
INCLUDES = -I. $(EP_INC) $(MSQ_INC) $(PSQL_INC) $(GLIB_CFLAGS)

# Linker flags 
LFLAGS = $(LIBS)

# Default compile rule
.c.o:
	@echo ------ Compiling $<
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $<

# main target
all: $(EXE_DIR)/$(PRJ_NAME) $(EXE_DIR)/attribuzioni





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



receive.o: receive.c receive.h recext.h recstruct.h recfun.h $(COMMON_DEPS)

recvar.o: recvar.c receive.h recext.h recstruct.h recfun.h $(COMMON_DEPS)

recutil.o: recutil.c receive.h recext.h recstruct.h recfun.h $(COMMON_DEPS)

# Compile rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'

# Compile rules
$(EXE_DIR)/attribuzioni: attribuzioni.o $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) attribuzioni.o recvar.o recutil.o $(OBJS) $(LFLAGS) -o $(EXE_DIR)/attribuzioni
	@echo '------ Terminated'

clean:
	@echo '------ Cleaning ...'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'


