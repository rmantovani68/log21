
LOCAL_LIB=../
LOCAL_INCL=../ # Inclusioni specifiche del programma Plc
DEVCNI=/usr/local/cni
SHELL=/bin/bash
CC=gcc
LD=gcc
LD_R=ld -r
CFLAGS=-m 32 -g \
       -mposix -D_POSIX_SOURCE -mthreads -DLynx \
       -I$(LOCAL_INCL) -I$(DEVCNI)/include
LDFLAGS=-mposix -D_POSIX_SOURCE -mthreads \
        -L$(LOCAL_LIB) -L$(DEVCNI)/lib \
        -Xlinker -Ttext -Xlinker 0x10000
LIBS=-lqplc -lserax -l_serverdm -lsock -ldevcni -lxerr -lbsd

OBJS=dm_sample.o

.c.o:
	$(CC) -c $(CFLAGS) $<

user.oo:	hwtest.o
	/bin/ld -r hwtest.o -o user.oo

piani-user.oo:	piani.o
	/bin/ld -r piani.o -o piani-user.oo

vale.oo:	vale.o
	/bin/ld -r vale.o -o vale.oo

all:: qplc prog.oo

dm_sample.o:	dm_sample.c

prog.o: $(OBJS)
	$(LD_R) $(OBJS) -o prog.o

#
# Generazione dell'esecutore monolitico, buono per debug.
#

qplc: prog.o
	$(LD) $(LDFLAGS) prog.o $(LIBS) -o qplc

#
# Generazione del modulo caricabile dinamicamente, consigliabile
# per la distribuzione.
#

prog.oo: prog.o
	/bin/ld -r prog.o -o prog.oo

