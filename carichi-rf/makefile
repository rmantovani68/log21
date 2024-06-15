# ------------------------------------------------------------
# Makefile for EMI - Gestione Ordini RF - TERMINALE CARRELLO
# Copyright A&L srl 2003-2004
# ------------------------------------------------------------

# --------------------------
# Project Name (executable)
# --------------------------
PRJ_NAME = rf

# --------------------------
# Program debug flags
# --------------------------
# debug version
DEFDEBUG = -g -DDEBUG -DTRACE

#--------------------------------------------------
# sorgenti componenti il progetto
#--------------------------------------------------
SRCS = rf.c rfvar.c rffun.c rfbox.c

# --------------------------
# user defines
# --------------------------
USR_DEF = 

#--------------------------------------------------
# include file contenente le definizioni generiche
#--------------------------------------------------
include ../common/picking.mak

#--------------------------------------------------
# object files componenti il progetto
#--------------------------------------------------
OBJS = cmp_time.o

# --------------------------
# libraries
# --------------------------
LIBS = $(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GLIB_LIBS) $(NCURSES_LIBS) $(ZMQ_LIBS) -lm

# --------------------------
# includes 
# --------------------------
INCLUDES = -I.  $(GLIB_CFLAGS) $(EP_INC) $(MSQ_INC) $(EP_INC) $(PSQL_INC) $(NCURSES_CFLAGS) $(ZMQ_CFLAGS)

# --------------------------
# Linker flags 
# --------------------------
LFLAGS = $(LIBS)

# dependencies

cmp_time.h:
	$(__CMP_TIME__H__)

cmp_time.c:
	$(__CMP_TIME__C__)

cmp_time.o: cmp_time.c cmp_time.h

# --------------------------

all: $(EXE_DIR)/$(PRJ_NAME)



rf.o: rf.h rfext.h rfstruct.h rffun.h $(COMMON_DEPS)

rffun.o: rf.h rfext.h rfstruct.h rffun.h $(COMMON_DEPS)

rfvar.o: rf.h rfext.h rfstruct.h rffun.h $(COMMON_DEPS)

rfbox.o: rf.h rfext.h rfstruct.h rffun.h $(COMMON_DEPS)

rfdb.o: rf.h rfext.h rfstruct.h rffun.h $(COMMON_DEPS)

cmp_time.o: cmp_time.c

# --------------------------
# link rules
# --------------------------

$(EXE_DIR)/$(PRJ_NAME):  $(OBJS) $(SRCS:.c=.o)
	@echo '------ Linking ...$@'
	$(CC) $(SRCS:.c=.o) $(OBJS) $(LFLAGS) -o $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'

clean:
	@echo '------ Cleaning ...'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)
	@echo '------ Terminated'



.PHONY: cmp_time.c cmp_time.h
