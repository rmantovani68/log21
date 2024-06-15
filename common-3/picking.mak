# ---------------------------------------------------------
# picking.mak : include file con definizioni generali da
# includere in tutti i Makefile per Easy Picking 3.0
# ---------------------------------------------------------
# Copyright A&L 2000-2021
# ---------------------------------------------------------
# Copyright CNI srl 1995-2000
GTK3_LIBS   = `pkg-config gtk+-3.0 --libs`  `pkg-config gmodule-export-2.0 --libs`
GTK3_CFLAGS = `pkg-config gtk+-3.0 --cflags`  `pkg-config gmodule-export-2.0 --cflags`

GNOME_LIB    = `pkg-config libgnome-2.0 --libs` `pkg-config libgnomeui-2.0 --libs` 
GNOME_CFLAGS = `pkg-config libgnome-2.0 --cflags` `pkg-config libgnomeui-2.0 --cflags` 

GTK_LIB    = `pkg-config gtk+-2.0 --libs` 
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags` 

GLIB_LIBS   = `pkg-config glib-2.0 --libs` 
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags` 

GIMP_LIBS = -lm
GIMP_CFLAGS = -I../common-3/gimpwidgets -DGIMP_WIDGETS_COMPILATION -DGIMP_BASE_COMPILATION

GLADE_LIB    = `pkg-config libglade-2.0 --libs` 
GLADE_CFLAGS = `pkg-config libglade-2.0 --cflags` 

# libreria di gestione Code Messaggi, Comunicazioni Seriali ...
MSQ_LIB = -L../common-3/lib -lPmxMsq
MSQ_INC = -I../common-3/lib

# libreria di gestione Database PostgreSql
PSQL_LIB = `pkg-config libpq --libs`
PSQL_INC = `pkg-config libpq --cflags`

CNI_INCLUDES = -I /usr/local/cni/include
DEVCNI_LIB = -L /usr/local/cni/lib -ldevcni

# Direttorio contenete file generali da includere
GEN_INC = ../common-3

# Direttorio degli eseguibili
EXE_DIR=../exe

# Include files di uso comune a tutti i processi
COMMON_DEPS=$(GEN_INC)/dbfun.h $(GEN_INC)/proc_list.h $(GEN_INC)/picking.h $(GEN_INC)/ca_file.h $(GEN_INC)/sockets.h 


# Macro per generazione di ID RCS
__CMP_TIME__=@echo  '\
char rcsid[]="$$Id: '$(PRJ_NAME)' '`date +"%d/%m/%Y %H:%M:%S"` '$$"; \
char __module_name__[]        = "'$(PRJ_NAME)'"; \
char __module_version__[]     = "'`cat VERSION`'"; \
char __version__[]="'`cat ../doc/BASE_VERSION`'"; \
char __customer__[]="'`cat ../doc/CUSTOMER`'"; \
char __authors__[]="'`cat ../doc/AUTHORS`'"; \
char __configuration_file__[]="'`cat ../doc/CONFIGURATION_FILE`'"; \
char __copyright__[]="'`cat ../doc/COPYRIGHT`'";'> cmp_time.c


CC=gcc
DEFINES= $(CB_DEF) $(USR_DEF)
#CFLAGS= $(DEFDEBUG) -Wall -fwritable-strings $(DEFINES) $(INCLUDES) $(CNI_INCLUDES) $(GLIB_CFLAGS)
CFLAGS= $(DEFDEBUG) $(DEFINES) $(INCLUDES) $(CNI_INCLUDES) $(GLIB_CFLAGS)  $(GIMP_CFLAGS) 

#
# objs comuni a tutte le applicazioni
#

# funzioni per la gestione di linked list
$(GEN_INC)/linklist.o: $(GEN_INC)/linklist.c $(GEN_INC)/linklist.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/linklist.c -o $(GEN_INC)/linklist.o

# funzioni comuni
$(GEN_INC)/ep-common.o: $(GEN_INC)/ep-common.c $(GEN_INC)/ep-common.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/ep-common.c -o $(GEN_INC)/ep-common.o

# funzioni per l'accesso ai database
$(GEN_INC)/ep-db.o: $(GEN_INC)/ep-db.c $(GEN_INC)/ep-db.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/ep-db.c -o $(GEN_INC)/ep-db.o

# funzioni per l'accesso ai database
$(GEN_INC)/dbfun.o: $(GEN_INC)/dbfun.c $(GEN_INC)/dbfun.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/dbfun.c -o $(GEN_INC)/dbfun.o

# lettura e scrittura file di configurazione
$(GEN_INC)/ca_file.o: $(GEN_INC)/ca_file.c $(GEN_INC)/ca_file.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/ca_file.c -o $(GEN_INC)/ca_file.o

# Gestione sockets communication
$(GEN_INC)/sockets.o: $(GEN_INC)/sockets.c $(GEN_INC)/sockets.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/sockets.c -o $(GEN_INC)/sockets.o

# Gestione PLC socket communication
$(GEN_INC)/plcsock.o: $(GEN_INC)/plcsock.c $(GEN_INC)/plcsock.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/plcsock.c -o $(GEN_INC)/plcsock.o

# support for gtk
$(GEN_INC)/support.o: $(GEN_INC)/support.c $(GEN_INC)/support.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/support.c -o $(GEN_INC)/support.o


# gtk-support for gtk
$(GEN_INC)/gtk-support.o: $(GEN_INC)/gtk-support.c $(GEN_INC)/gtk-support.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/gtk-support.c -o $(GEN_INC)/gtk-support.o

# dbfun-gtk 
$(GEN_INC)/dbfun-gtk.o: $(GEN_INC)/dbfun-gtk.c $(GEN_INC)/dbfun-gtk.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/dbfun-gtk.c -o $(GEN_INC)/dbfun-gtk.o

# gimpruler
$(GEN_INC)/gimpwidgets/gimpruler.o: $(GEN_INC)/gimpwidgets/gimpruler.c $(GEN_INC)/gimpwidgets/gimpruler.h
	@echo '------ Compiling $<'
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/gimpwidgets/gimpruler.c -o $(GEN_INC)/gimpwidgets/gimpruler.o

# gimpunit
$(GEN_INC)/gimpwidgets/gimpunit.o: $(GEN_INC)/gimpwidgets/gimpunit.c $(GEN_INC)/gimpwidgets/gimpunit.h
	@echo '------ Compiling $<'
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/gimpwidgets/gimpunit.c -o $(GEN_INC)/gimpwidgets/gimpunit.o

# gimpbase-private
$(GEN_INC)/gimpwidgets/gimpbase-private.o: $(GEN_INC)/gimpwidgets/gimpbase-private.c $(GEN_INC)/gimpwidgets/gimpbase-private.h
	@echo '------ Compiling $<'
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/gimpwidgets/gimpbase-private.c -o $(GEN_INC)/gimpwidgets/gimpbase-private.o

# shmfun 
$(GEN_INC)/shmfun.o: $(GEN_INC)/shmfun.c $(GEN_INC)/shm.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/shmfun.c -o $(GEN_INC)/shmfun.o

# gwyhruler
$(GEN_INC)/gwyhruler.o: $(GEN_INC)/gwyhruler.c $(GEN_INC)/gwyhruler.h
	@echo '------ Compiling $<'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c $(GEN_INC)/gwyhruler.c -o $(GEN_INC)/gwyhruler.o
