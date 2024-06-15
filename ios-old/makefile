# ---------------------------------------------------------
# A&L Easy Picking
# Makefile - Gestione IOS
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = ios

# Program debug flags
# debug version
DEFDEBUG = -g -DDEBUG -DTRACE

# release version
#DEFDEBUG = -O 

# user defines
#USR_DEF = -DHAS_IOS_CNI
USR_DEF =

# include file contenente le definizioni generiche
include ../common/picking.mak


# object files componenti il progetto
OBJS = 

# sorgenti componenti il progetto
SRCS = ios.c 


# libraries
#LIBS =$(EP_LIB) $(MSQ_LIB) $(DEVCNI_LIB)  $(ZMQ_LIBS)
LIBS =$(EP_LIB) $(MSQ_LIB) $(GLIB_LIBS) -lm $(ZMQ_LIBS)

# includes 
#INCLUDES = $(EP_INC) $(MSQ_INC) $(DEVCNI_INC)
INCLUDES = $(GLIB_CFLAGS) $(EP_INC) $(MSQ_INC)

# Linker flags 
LFLAGS = $(LIBS)

# Default compile rule
.c.o:
	@echo ------ Compiling $<
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $<


all: $(EXE_DIR)/$(PRJ_NAME) $(EXE_DIR)/iosver $(EXE_DIR)/iosdump $(EXE_DIR)/setlampada 





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



ios.o: ios.c ios.h $(COMMON_DEPS)
iosdump.o: iosdump.c
iosset.o: iosset.c
iossetbit.o: iossetbit.c
iosver.o: iosver.c 
setlampada.o: setlampada.c 

# Compile rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'

$(EXE_DIR)/iosver: iosver.o
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) iosver.o $(LFLAGS) -o $(EXE_DIR)/iosver
	@echo '------ Terminated'

$(EXE_DIR)/iosdump: iosdump.o
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) iosdump.o $(LFLAGS) -o $(EXE_DIR)/iosdump
	@echo '------ Terminated'

$(EXE_DIR)/iosset: iosset.o
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) iosset.o $(OBJS) $(LFLAGS) -o $(EXE_DIR)/iosset
	@echo '------ Terminated'

$(EXE_DIR)/iossetbit: iossetbit.o
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) iossetbit.o $(OBJS) $(LFLAGS) -o $(EXE_DIR)/iossetbit
	@echo '------ Terminated'

$(EXE_DIR)/setlampada: setlampada.o
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) setlampada.o $(LFLAGS) -o $(EXE_DIR)/setlampada
	@echo '------ Terminated'

clean:
	@echo '------ Cleaning ...'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'


