# ---------------------------------------------------------
# Makefile for Mondadori Picking - Stampa Bolle 
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

PRJ_NAME = printlabel

DEBUG = 1
# --------------------------
# Program debug flags
# --------------------------

ifeq ($(DEBUG),1)
	# debug version
	DEFDEBUG = -g -DDEBUG -DTRACE
else
	# release version
	DEFDEBUG = -O 
endif

# include file contenente le definizioni generiche
include ../common/picking.mak


# object files componenti il progetto
OBJS =  cmp_time.c

# sorgenti componenti il progetto
SRCS = printlabel.c printfun.c printutil.c printvar.c printrac.c printoldrac.c printxab.c  printxab_A4.c printpacklist.c printdoccons.c printdatiordine.c printdist.c stampa.c 


# libraries
LIBS =$(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GLIB_LIBS) $(ZMQ_LIBS)

# includes 
INCLUDES = -I.  $(GLIB_CFLAGS) $(EP_INC) $(MSQ_INC) $(EP_INC) $(PSQL_INC)

# Linker flags 
LFLAGS = $(LIBS)

# main target
all: $(EXE_DIR)/$(PRJ_NAME) 





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



printlabel.o: printlabel.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printvar.o: printvar.c printlabel.h printext.h printstruct.h printfun.h stampa.h $(COMMON_DEPS)

printfun.o: printfun.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printutil.o: printutil.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printxab.o: printxab.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printxab_A4.o: printxab_A4.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printpacklist.o: printpacklist.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printdoccons.o: printdoccons.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printdist.o: printdist.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printdatiordine.o: printdatiordine.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printrac.o: printrac.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

printoldrac.o: printoldrac.c printlabel.h printext.h printstruct.h printfun.h $(COMMON_DEPS)

stampa.o: stampa.c stampa.h $(COMMON_DEPS)


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
	@echo '------ Cleaning'
	@rm -f $(SRCS:.c=.o) $(EXE_DIR)/$(PRJ_NAME)


