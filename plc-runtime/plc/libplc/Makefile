
#
# Costruzione della libreria di colloquio con Isagraf.
#

SHELL=/bin/bash
DEFS=defs
CC=gcc
#CFLAGS=-g -I$(DEFS) -I../src -I/usr/local/include
CFLAGS=-g -DLynx -DLinux -I$(DEFS) -Wall -I. -I/usr/local/include 

TARGET=libplc.a

NOMETEMP=lplc
NOMELIB=libplc

SOURCES=libplc.c
HEADERS=plclink.h plclink_p.h

all: $(NOMELIB).a

$(NOMELIB).a: $(NOMETEMP).h $(HEADERS) hsort.o version.o
	$(CC) $(CFLAGS) -c $(NOMETEMP)*.c
	rm -f $(NOMELIB).a
	ar qc $(NOMELIB).a $(NOMETEMP)*.o hsort.o version.o
	ranlib $(NOMELIB).a
#	rm -f $(NOMETEMP)*.c
	rm -f $(NOMETEMP)*.o

hsort.o:	hsort.c hsort.h

spezza $(NOMETEMP).h: $(NOMELIB).c
	awk  -v "NOMETEMP=$(NOMETEMP)" '\
BEGIN { n=0; file=NOMETEMP ".h"; nomeinclude=file;\
		print "/* " file " */" > file; }\
$$0 == "/*LIB*/" {\
		++n;\
		close(file);\
		file = sprintf("%s%03d.c",NOMETEMP,n);\
		print "/* " file " */" > file;\
		print "#include \"" nomeinclude "\"" >> file;\
		next;\
	}\
{ print $$0 >> file; }' $(NOMELIB).c

quick:
	rm -f $(NOMELIB).a
	ar qc $(NOMELIB).a $(NOMETEMP)*.o version.o
	ranlib $(NOMELIB).a
	rm -f $(NOMETEMP)*

version.o:	version.c

version.c:	$(SOURCES) $(HEADERS)
	@if [ -r version.c ]; then \
		VERSION=`awk '/^$$/ {next;} {split($$6,a,".");print a[1] "." (a[2]+1);}' version.c`;\
	else \
		VERSION=1.0; \
	fi; \
	echo 'Numero di versione proposto : '"$$VERSION"' - ENTER=OK, oppure scrivine uno tu'; \
	read a; if [ "$$a" ]; then VERSION="$$a"; fi; \
	echo 'static char id[] = "libplc.a '"$$VERSION"' '"`date`"'";' > version.c

