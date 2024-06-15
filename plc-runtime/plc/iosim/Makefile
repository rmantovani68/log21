# Makefile per la generazione dell'editor binario

COMPDIR=	/home/iosim
LAE_DIR=	/home/iosim/lib

INCLUDES=       -I/usr/local/include -I/usr/local/cni/include/
LIBRARY=        -L/usr/local/lib/ -L/usr/local/cni/lib/

APPL=           Iosim
OBJS = 		Iosim.o 
OBJS1 =         /comtcl/ctclcni.o
SRCS =  	Iosim.c 
CC=             gcc

LNORMAL=       	-fwritable-strings -lTktable -ltk8.0 -ltcl8.0 -ltixsam4.1.8.0\
		-lplc -l_serverdm -lxerr -lX11_s -lm -lnetinet

FNORMAL=        -Xlinker -Ttext -Xlinker 0x10000 $(LIBRARY)
CNORMAL=        -DTKTABLE -Wall $(DEFINES) $(INCLUDES) -DTCL_STALONE\
		-DTCL_ACTIVE

LDFLAGS=        $(FNORMAL)
CFLAGS=         $(CNORMAL)
LIBS=           $(LNORMAL)


FILES=  \
	$(LAE_DIR)/FileSel.tcl \
	$(LAE_DIR)/Azioni.tcl \
	$(LAE_DIR)/windows.tcl \
	$(LAE_DIR)/Inizia.tcl \
	$(LAE_DIR)/FileSel.tcl \
	$(LAE_DIR)/files.tcl \
	$(LAE_DIR)/CallBacks.tcl \
	$(LAE_DIR)/Plc.tcl \
	$(LAE_DIR)/util.tcl \
	$(LAE_DIR)/PopUp.tcl 

all: gen $(APPL)	

gen:
	/comtcl/tcl2ccni -o $(COMPDIR)/Iosim.c $(FILES) $(COMPDIR)/iosim -tix
#
#
#------------  COMPILAZIONE ----------------------------------------
$(APPL): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(OBJS1) $(LIBS) -o $(APPL)
	@echo Fine Link.

.c.o:
	$(CC) -c $(CFLAGS) $*.c
	@echo $*.c compiled.

#-------------------------------------------------------------------
#-------------- FINE COMPILAZIONE ----------------------------------

depend:
	/comtcl/tcl2c -o $(COMPDIR)/Iosim.c $(FILES) $(COMPDIR)/iosim -tix
	makedepend $(INCLUDES) $(SRCS)
	@echo Fine Depend.

editver: 
	gnutar zcvPf /home/LaeEdt1.0.0.1.tgz $(COMPDIR)/LaeEdt \
		$(COMPDIR)/default $(COMPDIR)/lingue \
		$(COMPDIR)/dati/ $(COMPDIR)/image 

# DO NOT DELETE THIS LINE -- make depend depends on it.

LaeEdt.o: /usr/local/include/tcl.h /usr/include/stdio.h
LaeEdt.o: /usr/include/ansidecl_lynx.h /usr/include/ipc.h
LaeEdt.o: /usr/include/sys/types.h /usr/include/sem.h /usr/include/stat.h
LaeEdt.o: /usr/include/time.h /usr/include/timers.h /usr/include/events.h
LaeEdt.o: /usr/include/signal.h /usr/include/param.h /usr/include/conf.h
