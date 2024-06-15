# ---------------------------------------------------------
# Makefile for Mondadori Picking - Gestione Barcode Gryphon
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = barcode_scarti

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
OBJS = 

# sorgenti componenti il progetto
SRCS = barcode.c 


# libraries
LIBS =$(EP_LIB) $(MSQ_LIB) $(ZMQ_LIBS)

# includes 
INCLUDES = $(EP_INC) $(MSQ_INC)

# Linker flags 
LFLAGS = $(LIBS)


all: $(EXE_DIR)/$(PRJ_NAME)





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



barcode.o: barcode.c barcode.h $(COMMON_DEPS)

# Compile rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo ------ Linking $(PRJ_NAME)
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo ------ Terminated

clean:
	@echo ------ Cleaning ...
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo ------ Terminated


