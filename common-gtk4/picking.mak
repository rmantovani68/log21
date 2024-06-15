# ---------------------------------------------------------
# picking.mak : include file con definizioni generali da
# includere in tutti i Makefile per Easy Picking 3.0
# ---------------------------------------------------------
# Copyright A&L 2000-2021
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021
GTK3_LIBS   = `pkg-config gtk+-3.0 --libs`  `pkg-config gmodule-export-2.0 --libs`
GTK3_CFLAGS = `pkg-config gtk+-3.0 --cflags`  `pkg-config gmodule-export-2.0 --cflags`

GNOME_LIB    = `pkg-config libgnome-2.0 --libs` `pkg-config libgnomeui-2.0 --libs` 
GNOME_CFLAGS = `pkg-config libgnome-2.0 --cflags` `pkg-config libgnomeui-2.0 --cflags` 

GTK_LIB    = `pkg-config gtk+-2.0 --libs` 
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags` 

GLIB_LIBS   = `pkg-config glib-2.0 --libs` 
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags` 

PANGO_LIBS   = `pkg-config pango --libs`   `pkg-config pangocairo --libs` 
PANGO_CFLAGS = `pkg-config pango --cflags` `pkg-config pangocairo --cflags` 

GIMP_LIBS = -L ../common/gw -lGW -lm
GIMP_CFLAGS = -DGIMP_WIDGETS_COMPILATION -DGIMP_BASE_COMPILATION  -I../common/gw 

GLADE_LIB    = `pkg-config libglade-2.0 --libs` 
GLADE_CFLAGS = `pkg-config libglade-2.0 --cflags` 

# libreria di gestione Code Messaggi, Comunicazioni Seriali ...
MSQ_LIB = -L../common/pmx -lPmxMsq
MSQ_INC = -I../common/pmx

# libreria Easy Picking
EP_LIB = -L../common/ep -lEP
EP_INC = -I../common/ep

# libreria di gestione Database PostgreSql
PSQL_LIB = `pkg-config libpq --libs`
PSQL_INC = `pkg-config libpq --cflags`

# libreria di gestione code messaggi ZMQ
ZMQ_LIBS    = `pkg-config libzmq --libs`
ZMQ_CFLAGS = `pkg-config libzmq --cflags`
#
# libreria di gestione ncurses per char terminal
NCURSES_LIBS   = `pkg-config panel --libs`   `pkg-config form --libs`    `pkg-config menu --libs`   `pkg-config ncurses --libs`
NCURSES_CFLAGS = `pkg-config panel --cflags` `pkg-config form --cflags`  `pkg-config menu --cflags` `pkg-config ncurses --cflags`

# Direttorio degli eseguibili
EXE_DIR=../exe

__CMP_TIME__C__=@/bin/echo -en '\
char rcsid[]="$$Id: '$(PRJ_NAME)' '`date +"%d/%m/%Y %H:%M:%S"` '$$";\n\
char __application_name__[]="'`cat ../doc/APPLICATION_NAME`'";\n\
char __module_name__[] = "'$(PRJ_NAME)'";\n\
char __module_version__[] = "'`cat VERSION`'";\n\
char __version__[]="'`cat ../doc/BASE_VERSION`'";\n\
char __customer__[]="'`cat ../doc/CUSTOMER`'";\n\
char __authors__[]="'`cat ../doc/AUTHORS`'";\n\
char __configuration_file__[]="'`cat ../doc/CONFIGURATION_FILE`'";\n\
char __copyright__[]="'`cat ../doc/COPYRIGHT`'";\n'> cmp_time.c


__CMP_TIME__H__=@/bin/echo -e  '\
\#pragma once\n\
extern char rcsid[];\n\
extern char __application_name__[];\n\
extern char __module_name__[];\n\
extern char __module_version__[];\n\
extern char __version__[];\n\
extern char __customer__[];\n\
extern char __authors__[];\n\
extern char __configuration_file__[];\n\
extern char __copyright__[];\n'> cmp_time.h


CC=gcc

DEFDEBUG=-g

CFLAGS= \
	-Wall \
	-Wno-unused-variable \
	-Wno-format-overflow \
	-Wno-unused-but-set-variable \
	-Wno-misleading-indentation \
	-Wno-format-security \
	$(DEFDEBUG) \
	$(GLIB_CFLAGS)  \
	$(GIMP_CFLAGS) \
	$(ZMQ_CFLAGS) \
	$(NCURSES_CFLAGS)

#
# Default compile rule
.c.o:
	@echo 'C - $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $<
