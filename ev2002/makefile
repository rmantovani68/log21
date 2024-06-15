# ---------------------------------------------------------
# A&L Easy Picking
# Makefile - Gestione Bilancia
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = ev2002

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
OBJS =  cmp_time.o

# sorgenti componenti il progetto
SRCS = ev2002.c 

# libraries
LIBS =$(EP_LIB) $(MSQ_LIB) $(GLIB_LIBS) $(ZMQ_LIBS) -lm

# includes 
INCLUDES = $(EP_INC) $(MSQ_INC) $(GLIB_CFLAGS)

# Linker flags 
LFLAGS = $(LIBS)


all: $(EXE_DIR)/$(PRJ_NAME) 

# dependencies

cmp_time.h:
	$(__CMP_TIME__H__)

cmp_time.c: $(SRCS:.c=.o)
	$(__CMP_TIME__C__)

cmp_time.o: cmp_time.c

ev2002.o: ev2002.c ev2002.h cmp_time.h $(COMMON_DEPS)

# Compile rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo ------ Linking $<
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo ------ Terminated

txser.o: txser.c $(COMMON_DEPS)

rxser.o: rxser.c $(COMMON_DEPS)

# 
txser: txser.o
	@echo ------ Linking $<
	@$(CC) txser.o $(LFLAGS) -o txser
	@echo ------ Terminated

# Compile rules
rxser: rxser.o
	@echo ------ Linking $<
	@$(CC) rxser.o $(LFLAGS) -o rxser
	@echo ------ Terminated

clean:
	@echo ------ Cleaning ...
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo ------ Terminated


