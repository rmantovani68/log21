# Makefile per libreria Easy Picking
PSQL_LIBS   = `pkg-config libpq --libs` 
PSQL_CFLAGS = `pkg-config libpq --cflags` 

GTK_LIBS    = `pkg-config gtk+-3.0 --libs`  `pkg-config gmodule-export-2.0 --libs`
GTK_CFLAGS  = `pkg-config gtk+-3.0 --cflags`  `pkg-config gmodule-export-2.0 --cflags`

GTK_LIBS    = `pkg-config gtk4 --libs`
GTK_CFLAGS  = `pkg-config gtk4 --cflags`

GLIB_LIBS   = `pkg-config glib-2.0 --libs` 
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags` 

PMX_INCLUDE_PATH = ../pmx

# Flags di compilazione (debug)
WARNINGS = -Wall \
	-Wno-unused-variable \
	-Wno-format-overflow \
	-Wno-unused-but-set-variable \
	-Wno-misleading-indentation \
	-Wno-format-security \
	-Wno-unused-function \
	-Wno-format-zero-length \
	-Wno-unused-result \
	-Wno-unused-value \
	-Wno-deprecated-declarations

CFLAGS = $(WARNINGS) -g -DTRACE -DNO_CHSIZE -I.

CC = gcc

ARFLAGS = rvU

# modifico il metodo di default per la creazione della libreria
# (elimino la cancellazione del codice oggetto)
.c.a:
	@echo 'C -' $<
	@$(CC) $(GTK_CFLAGS) $(GLIB_CFLAGS) $(PSQL_CFLAGS) $(CFLAGS) -I$(PMX_INCLUDE_PATH) -c -o $% $<
	@$(AR) $(ARFLAGS) $@ $%

%.d: %.c
	@set -e; rm -f $@; \
	@$(CC) -M -MD -MP  $< > $@.$$$$; \
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	@rm -f $@.$$$$

#	$(EPLIB)(plcsock.o)\

# Parametri generali
EPLIB = libEP.a
EPLIBOBJS =	\
	$(EPLIB)(ep-odbc.o)\
	$(EPLIB)(dbfun-gtk.o)\
	$(EPLIB)(ep-resources.o)\
	$(EPLIB)(ep-common.o)\
	$(EPLIB)(ep-db.o)\
	$(EPLIB)(ep_pgsql.o)\
	$(EPLIB)(gtk-support.o)\
	$(EPLIB)(msg-box.o)\
    $(EPLIB)(ep-about.o)\
	$(EPLIB)(linklist.o)\
	$(EPLIB)(shmfun.o)\
	$(EPLIB)(sockets.o)\
	$(EPLIB)(ep-resources.o)\
	$(EPLIB)(dbfun-deltacarichi.o)

# Regole di compilazione della libreria per code messaggi
all:	$(EPLIB)

clean:
	@touch *.[ch]
	@rm -rf $(EPLIB)

library:	$(EPLIB)

$(EPLIB):	$(EPLIBOBJS)

GLIB_COMPILE_RESOURCES = glib-compile-resources

srcdir=.

cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)

cmp_time.c: $(SRCS:.c=.o)
	$(__CMP_TIME__C__)

cmp_time.o: cmp_time.c


#utilities.c: utilities.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/utilities.gresource.xml)
#	@echo 'R - $<'
#	@$(GLIB_COMPILE_RESOURCES) $(srcdir)/utilities.gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-source


ep-resources.h: ep.gresource.xml
	@echo 'R - ' $<
	@$(GLIB_COMPILE_RESOURCES) $< --target=$@ --sourcedir=$(srcdir) --c-name _ep --generate-header --manual-register

ep-resources.c: ep.gresource.xml
	@echo 'R - ' $<
	@$(GLIB_COMPILE_RESOURCES) $< --target=$@ --sourcedir=$(srcdir) --c-name _ep --generate-source --manual-register


ep-resources.o: ep-resources.c

ep-about.o: ep-about.c

dbfun.o: dbfun.c ep-db.h dbfun.h 

dbfun-deltacarichi.o: dbfun-deltacarichi.c dbfun-deltacarichi.c ep-db.h dbfun.h

dbfun-gtk.o: dbfun-gtk.c $(PMX_INCLUDE_PATH)/ascii.h ep-common.h ep-db.h gtk-support.h dbfun-gtk.h

ep-common.o: ep-common.c $(PMX_INCLUDE_PATH)/pmx_msq.h $(PMX_INCLUDE_PATH)/msq_lib.h $(PMX_INCLUDE_PATH)/msq.h $(PMX_INCLUDE_PATH)/keybd_in.h $(PMX_INCLUDE_PATH)/nccom.h $(PMX_INCLUDE_PATH)/cross_in.h $(PMX_INCLUDE_PATH)/crossmat.h $(PMX_INCLUDE_PATH)/mscr_out.h $(PMX_INCLUDE_PATH)/shared.h picking.h linklist.h ep-common.h proc_list.h

ep-db.o: ep-db.c $(PMX_INCLUDE_PATH)/ascii.h $(PMX_INCLUDE_PATH)/pmx_msq.h $(PMX_INCLUDE_PATH)/msq_lib.h $(PMX_INCLUDE_PATH)/msq.h $(PMX_INCLUDE_PATH)/keybd_in.h $(PMX_INCLUDE_PATH)/nccom.h $(PMX_INCLUDE_PATH)/cross_in.h $(PMX_INCLUDE_PATH)/crossmat.h $(PMX_INCLUDE_PATH)/mscr_out.h $(PMX_INCLUDE_PATH)/shared.h proc_list.h picking.h linklist.h ep-common.h ep-db.h ep_pgsql.h ep_pgsql.c

ep-odbc.o: ep-odbc.c ep-odbc.h

ep-resources.c: ep-resources.h

ep-about.c: ep-about.h

gtk-support.h: ep-resources.h
