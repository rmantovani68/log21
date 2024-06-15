# Makefile per librerie delle code messaggi di PMX

# Flags di compilazione (Debug)
#CFLAGS = -Wall -g -fwritable-strings -DTRACE -DNO_CHSIZE -I.

# Flags di compilazione (versione)
CFLAGS = -Wall -g -DTRACE -DNO_CHSIZE -I.

CC = gcc

# modifico il metodo di default per la creazione della libreria
# (elimino la cancellazione del codice oggetto)
.c.a:
	$(CC) $(CFLAGS) -c -o $% $<
	$(AR) $(ARFLAGS) $@ $%

# Parametri generali
PMX_MSQLIB = ../libPmxMsq.a
PMX_MSQLIBOBJS =	$(PMX_MSQLIB)(xcodemsg.o)\
					$(PMX_MSQLIB)(keybd_in.o)\
					$(PMX_MSQLIB)(msq_lib.o)\
					$(PMX_MSQLIB)(mscr_out.o)\
					$(PMX_MSQLIB)(nccom.o)\
					$(PMX_MSQLIB)(params.o)\
					$(PMX_MSQLIB)(trace.o)\
					$(PMX_MSQLIB)(shared.o)\
					$(PMX_MSQLIB)(timeout.o)\
					$(PMX_MSQLIB)(mission.o)\
					$(PMX_MSQLIB)(cni_dos.o)

# Regole di compilazione della libreria per code messaggi
all:	$(PMX_MSQLIB)

library:	$(PMX_MSQLIB)

$(PMX_MSQLIB):	$(PMX_MSQLIBOBJS)

# Dipendenze dei files oggetto
xcodemsg.o:	xcodemsg.h msq_lib.h
keybd_in.o:	keybd_in.h msq_lib.h
msq_lib.o:	msq_lib.h
mscr_out.o:	colours.h mscr_out.h msq_lib.h
nccom.o:	nccom.h trace.h
params.o:	params.h
trace.o:	trace.h
shared.o:	shared.h msq_lib.h
timeout.o:	timeout.h
mission.o:	mission.h
cni_dos.o:	cni_dos.h
