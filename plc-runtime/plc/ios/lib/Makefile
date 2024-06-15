
SHELL=/bin/bash
CC=gcc
DEVCNI=/usr/local/cni
INCLCNI=$(DEVCNI)/include
LIBCNI=$(DEVCNI)/lib
CFLAGS=-m32 -g -I. -I$(INCLCNI)
#CFLAGS=-I. -I$(INCLCNI)
SYSHDIR=$(INCLCNI)
SYSLIB=$(LIBCNI)/libdevcni.a

NOMETEMP=devcni
NOMELIB=lib$(NOMETEMP)

SOURCES=libdevcni.c
HEADERS=hwcni.h


#OBJS=mmap.o
OBJS=

clean: 
	rm -f $(NOMELIB).o
	rm -f $(NOMELIB).a

all: libdevcni.a $(SYSHEADER) $(SYSLIB) $(SYSHDIR)

$(SYSHDIR):	$(HEADERS)
	@for i in $(HEADERS); do diff $(SYSHDIR)/$$i $$i > /dev/null || \
		echo -e '\n$(SYSHDIR)/'$$i' deve essere aggiornato !\n'; done

$(SYSLIB):	libdevcni.a
	@cmp $(SYSLIB) libdevcni.a > /dev/null || \
		echo -e '\n$(SYSLIB) deve essere aggiornato !\n'

$(NOMELIB).a: $(NOMELIB).c $(HEADERS) version.o
	$(CC) $(CFLAGS) -c $(NOMELIB).c
	rm -f $(NOMELIB).a
	ar qc $(NOMELIB).a $(NOMELIB).o version.o
	ranlib $(NOMELIB).a

version.o:	version.c

version.c:	$(SOURCES) $(HEADERS)
	@if [ -r version.c ]; then \
		VERSION=`awk '/^$$/ {next;} {split($$6,a,".");print a[1] "." (a[2]+1);}' version.c`;\
	else \
		VERSION=1.0; \
	fi; \
	echo 'Numero di versione proposto : '"$$VERSION"' - ENTER=OK, oppure scrivine uno tu'; \
	read a; if [ "$$a" ]; then VERSION="$$a"; fi; \
	echo 'static char id[] = "libdevcni.a '"$$VERSION"' '"`date`"'";' > version.c

