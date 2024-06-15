#-----------------------------------------------------------------------------
# Makefile per MODULO WISH con comandi per debug edit_plc
#-----------------------------------------------------------------------------
#	Descrizione macro principali:
#
#	APPL		Nome dell' Applicazione
#	OBJS		Elenco degli oggetti
#	SRCS		Elenco dei sorgenti
#
#-----------------------------------------------------------------------------
#	Definizione macro:

SHELL=		/bin/bash

DEFINES=	-DDEBUG_PLC -DLinux
INCLUDES=	-I/usr/local/include/ -I/usr/local/cni/include/ -I/usr/include/tcl8.0 -I/usr/include/tk8.0 
LIBRARY= 	-L/usr/local/cni/lib/ -L/usr/X11R6/lib


APPL=		wishDeb
OBJS=		tkMain.o ctclcni.o 
SRCS=		tkMain.c ctclcni.c 
CC=		gcc

#LNORMAL=	-fwritable-strings -ltixsam4.1.8.0 -ltk8.0 -ltcl8.0 -lplc -lpcftp -lX11_s -lnetinet 
LNORMAL=	-fwritable-strings -ltixsam4.1.8.0 -ltkTable -ltk8.0 -ltcl8.0 -lplc -lX11 -lpcftp
#LNORMAL=	-fwritable-strings -ltk8.0 -ltcl8.0 -lplc -lX11_s -lm -lnetinet 

#FNORMAL=	-g -Xlinker -Ttext -Xlinker 0x10000 $(LIBRARY)
FNORMAL=	-g $(LIBRARY)
CNORMAL=	-g -Wall $(DEFINES) $(INCLUDES)

FNODEBUG=	-Xlinker -Ttext -Xlinker 0x10000 $(LIBRARY)
CNODEBUG=	-Wall $(DEFINES) $(INCLUDES)

LDFLAGS=	$(FNORMAL)
CFLAGS=		$(CNORMAL)
LIBS=		$(LNORMAL)
#-----------------------------------------------------------------------------
#       Definizione entry Make

$(APPL):  $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(APPL)
	@echo Fine Link.

.c.o:
	$(CC) -c $(CFLAGS) $*.c
	@echo $*.c compiled.


#-----------------------------------------------------------------------------
#       Funzioni ad uso generale

all:
	rm -f *.o core ; make

purge:
	rm -f *.o ../*.o wishDeb
	@echo Fine Purge.

release:
	make \
	LDFLAGS="$(FNODEBUG)" \
	CFLAGS="$(CNODEBUG)" \
	LIBS="$(LNORMAL)" \
	$(APPL)
	@echo Fine Release.
