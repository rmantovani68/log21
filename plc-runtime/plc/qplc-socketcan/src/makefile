#
# @(#) Makefile per Lynx di qplc Wed Nov 27 09:26:27 MET 1996
# @(#) Rif. Guerrini
#

#
# Linux
#

QPLC=qplc-socketcan

DEVCNI=/usr/local/cni

CAN4LINUX=../../cni
DEFS=.
LOCAL_LIB=LIB_TEMP
LOCAL_INCL=LIB_TEMP

SHELL=/bin/bash
CC=gcc


#	-Wtraditional \

WARNING_FLAGS = -Wall  -Wno-unused-variable

CFLAGS= \
	$(WARNING_FLAGS)  \
	-g \
	-m32 \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-align \
	-Wwrite-strings \
	-Wconversion \
	-Waggregate-return \
	-Wstrict-prototypes  \
	-Wmissing-prototypes \
	-Wredundant-decls \
	-Wdiscarded-qualifiers \
	-Wnested-externs \
	-DLinux -D_REENTRANT \
	-I$(LOCAL_INCL) -I$(DEFS) -I$(DEVCNI)/include

#	-I$(LOCAL_INCL) -I$(DEFS) -I$(DEVCNI)/include -I/u/prj/can4linux-3.5.4-pc600-r1/  -I/u/prj/ixxat/bci_usb-0.9.1
#	-I$(LOCAL_INCL) -I$(DEFS) -I$(DEVCNI)/include -I/u/prj/can4linux-3.5.4-pc600-r1/  -I/u/prj/ixxat/bci_usb-0.9.1
LD=gcc
LD_R=ld -r
#LDFLAGS=-static -L. -L$(LOCAL_LIB) -L$(DEVCNI)/lib
LDFLAGS= -L. -L$(LOCAL_LIB) -L$(DEVCNI)/lib
        
LDRFLAGS=-L. -L$(DEVCNI)/lib
LIBS=-ldevcni -lpthread -ldl
AR=ar
ARFLAGS=qv
RANLIB=ranlib

VERLIB=$(DEVCNI)/VERSIONE
VERLIB_3=$(DEVCNI_3)/VERSIONE

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(AS) -32 $< -o $@

#DEBUG_SRC=dummy.c
#DEBUG_OBJS=dummy.o
DEBUG_SRC=
DEBUG_OBJS=

#	dr_keyb.c \
#	dr_ax.c \
#	dr_bgage.c \
#	dummy_serverdm.s
#

SOURCES=\
	$(DEBUG_SRC) \
	main.c \
	util.c \
	util2.s \
	modules.c \
	dynaload.c \
	lista-Linux.s \
	dummy_prog480.c \
	dummy_hatsh.c \
	plc480.c \
	drivers.c \
	dr_can.c \
	dr_ios.c \
	dr_ser.c \
	compat.c \
	shvar.c \
	exec.c \
	superv.c \
	ilcrtm.c \
	glue.c \
	sysv.c \
	posix.c \
	debugger.c \
	remote.c \
	cback.c \
	ioconf.c \
	hsort.c \
	symbol_table.c \
	debhook.s

HEADERS=\
	qplc.h \
	util.h \
	modules.h \
	dynaload.h \
	drivers.h \
	dr_can.h \
	dr_ios.h \
	compat.h \
	shvar.h \
	exec.h \
	superv.h \
	glue.h \
	cback.h \
	debugger.h \
	plcrem.h \
	ioconf.h \
	remote.h

OBJS=\
	$(DEBUG_OBJS) \
	main.o \
	util.o \
	util2.o \
	modules.o \
	dynaload.o \
	lista-Linux.o \
	drivers.o \
	dr_can.o \
	compat.o \
	dr_ios.o \
	dr_ser.o \
	shvar.o \
	exec.o \
	superv.o \
	ilcrtm.o \
	glue.o \
	sysv.o \
	posix.o \
	debugger.o \
	remote.o \
	cback.o \
	ioconf.o \
	hsort.o \
	debhook.o


OBJS2= \
	symbol_table.o \
	dummy_prog480.o \
	dummy_hatsh.o \
	plc480.o \
	ver_core.o

#OBJS3= \
	dummy_prog480.o

all::	qplc libqplc.a

clean:
	rm -f *.o

symbol_table.c:	qplc.o
	nm -v qplc.o | grep -v get_pc_thunk | awk ' \
BEGIN { \
	print "/* Generato automaticamente. */"; \
} \
function add_sym(s) \
{ \
	if (s != "exit" && s != "abort" && s != "qplc_symbol_table" && s != "qplc_symbol_table_size") { \
		print "extern int " s ";"; \
		symtab[s] = 1; \
	} \
} \
{ \
	if ($$2 == "T" || $$2 == "D" || $$2 == "A" || $$2 == "C" || $$2 == "B") { \
		add_sym($$3); \
	} \
	else if ($$1 == "U") { \
		add_sym($$2); \
	} \
} \
END { \
	print "\nconst struct {"; \
	print "\tconst int *fn;"; \
	print "\tconst char *name;"; \
	print "} qplc_symbol_table[] = {"; \
	for (i in symtab) \
		print "{ &" i ", \"" i "\" },"; \
	print "};"; \
	print "\nconst int qplc_symbol_table_size"; \
	print " = sizeof(qplc_symbol_table)/sizeof(qplc_symbol_table[0]);\n"; \
}' > symbol_table.c

release::
	rm -f qplc libqplc.a *.o
	$(MAKE) RELEASE=1

qplc:	$(SOURCES:.c=.o) ver.o
	@$(LD) -m32 $(LDFLAGS) $(SOURCES:.c=.o) ver.o $(OBJS3)  $(LIBS) -o qplc
	@if [ '$(RELEASE)' = 1 ]; then strip -g -x qplc; fi
	@size qplc


MY_OBJS=/u/beppe/ilc/src/test/user.o
my_qplc:	$(MY_OBJS)
	$(LD) $(LDFLAGS) $(MY_OBJS) -lqplc $(LIBS) -o my_qplc

qplc.o:	$(OBJS)
	$(LD_R) -melf_i386 $(LDRFLAGS) $(OBJS) -o qplc.o
	size qplc.o

libqplc.a:	qplc.o $(OBJS2)
	@if [ '$(RELEASE)' = 1 ]; then strip -g -x qplc.o $(OBJS2); fi
	@rm -f libqplc.a
	@$(AR) $(ARFLAGS) libqplc.a qplc.o $(OBJS2)
	@$(RANLIB) libqplc.a

main.o:	main.c $(HEADERS)
util.o:	util.c util.h $(HEADERS)
util2.o:	util2.s
modules.o:	modules.c $(HEADERS)
dynaload.o:	dynaload.c $(HEADERS)
dummy_prog480.o:	dummy_prog480.c plcapi.h
dummy_hatsh.o:	dummy_hatsh.c
plc480.o:	plc480.c $(HEADERS) plcapi.h
debug.o:	debug.c $(HEADERS)
compat.o:	compat.c $(HEADERS)
drivers.o:	drivers.c $(HEADERS)
dr_ios.o:	dr_ios.c $(HEADERS)
dr_can.o:	dr_can.c $(HEADERS)
dr_ax.o:	dr_ax.c $(HEADERS)
dr_keyb.o:	dr_keyb.c $(HEADERS)
dr_ser.o:	dr_ser.c $(HEADERS)
dr_bgage.o:	dr_bgage.c $(HEADERS)
shvar.o:	shvar.c $(HEADERS)
exec.o:	exec.c $(HEADERS)
superv.o:	superv.c $(HEADERS)
ilcrtm.o:	ilcrtm.c ilcrtm.h $(HEADERS) plcapi.h
sysv.o:	sysv.c glue.h
posix.o:	posix.c glue.h
glue.o:	glue.c glue.h
debugger.o:	debugger.c $(HEADERS)
debhook.o:	debhook.s
lista-Linux.o:	lista-Linux.s
dummy_serverdm.o:	dummy_serverdm.s
remote.o:	remote.c $(HEADERS)
ioconf.o:	ioconf.c $(HEADERS) hsort.h
hsort.o:	hsort.c hsort.h

asend.o:	asend.c asend.h

plc480mix.o:	plc480mix.c $(HEADERS) plcapi.h

ver.o:	ver.c
ver.c:	VERSIONE
	versione=`cat VERSIONE`; \
	echo -en 'char Versione[] = "@(#) '"$$versione" > ver.c; \
	echo -en '\\t\\t[' >> ver.c; \
	echo -en "`date +"%d/%m/%Y %H:%M:%S"`" >> ver.c; \
	echo -en ']\\tlib: ??? ";\n' >> ver.c

ver_core.o:	ver_core.c
ver_core.c:	VERSIONE
	@ \
	versione=`cat VERSIONE`; \
	echo -en 'char Versione[] = "@(#) '"$$versione" > ver_core.c; \
	echo -en '\\t\\t[' >> ver_core.c; \
	echo -en "`date +"%d/%m/%Y %H:%M:%S"`" >> ver_core.c; \
	echo -en ']\\tlib:?.?.?   ";\n' >> ver_core.c; \

VERSIONE:	$(SOURCES) $(HEADERS) makefile 
	@if [ -r VERSIONE ]; then \
		versione=`awk '{ split($$0,a,"."); print a[1] "." a[2] "." a[3] "." (a[4]+1); }' VERSIONE` ;\
	else \
		versione=1.0.0.0; \
	fi; \
	echo 'Numero di versione QPLC : '"$$versione"; 

tags:	$(SOURCES)
	ctags $(SOURCES)

archive:
	arch=qplc-`cat VERSIONE`; \
	cd ../../ \
	&& tar zcvf $$arch.tgz $(QPLC) \
	&& rm -f $(QPLC)/src/*.o $(QPLC)/src/*.a $(QPLC)/src/ver.c \
	         $(QPLC)/src/ver_core.c $(QPLC)/src/qplc \
	         $(QPLC)/src/plc-objs/* \
	&& tar zcvf $$arch-src.tgz $(QPLC)
	
