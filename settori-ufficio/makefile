# ---------------------------------------------------------
# A&L Easy Picking
# Makefile - Gestione Settori
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = settori

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
SRCS = settori.c settfun.c settvar.c


# libraries
LIBS =$(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GLIB_LIBS) $(ZMQ_LIBS)

# includes 
INCLUDES = -I. $(EP_INC) $(MSQ_INC) $(PSQL_INC) $(GLIB_CFLAGS)

# Linker flags 
LFLAGS = $(LIBS)

# main target
all: $(EXE_DIR)/$(PRJ_NAME) 





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



settori.o: settori.c settext.h settstruct.h settfun.h $(COMMON_DEPS)

settvar.o: settvar.c settext.h settstruct.h settfun.h $(COMMON_DEPS)

settfun.o: settfun.c settext.h settstruct.h settfun.h $(COMMON_DEPS)

# Compile rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o) $(OBJS)
	@echo '------ Creating compile time object file'
	$(__CMP_TIME__)
	@echo '------ Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo '------ Linking ...'
	@$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'

clean:
	@echo '------ Cleaning ...'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'


