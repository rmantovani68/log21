#
#-----------------------------------------------------------------------------
# Makefile per libpcftp
#-----------------------------------------------------------------------------
#
#	Definizione macro:
#

DIRA=		/home/davide/archive/
DIRL=		/usr/local/cni/lib/
DIRI=		/usr/local/cni/include/
INCLUDES=	
CC=		gcc
CFLAGS=		-g -Wall 
ARFLAG=		rv
NOMELIB=	libpcftp
NOMETEMP=	pcftp

#
#-----------------------------------------------------------------------------
#       Definizione entry Make

$(NOMELIB).a: $(NOMELIB).o $(NOMELIB).h
	gcc -c $(CFLAGS) $(INCLUDES) $(NOMELIB).o
	ar $(ARFLAG) $(NOMELIB).a $(NOMELIB).o


#-----------------------------------------------------------------------------
#       Funzioni ad uso generale

all:
	rm -f *.a ; make

recv_file: recv_file.c libpcftp.a
	gcc $(CFLAGS) recv_file.c libpcftp.a -o recv_file -lbsd

send_file: send_file.c libpcftp.a
	gcc $(CFLAGS) send_file.c libpcftp.a -o send_file -lbsd

purge:
	rm -f $(NOMETEMP)*

deblib:
	$(CC) -c -g $(CFLAGS) $(INCLUDES) $(NOMELIB).c
	rm -f $(NOMELIB).a
	ar $(ARFLAG) $(NOMELIB).a $(NOMELIB).o
	ranlib $(NOMELIB).a
	rm -f $(NOMELIB).o
	@echo Fine deblib.

sun:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(NOMELIB).c
	rm -f $(NOMELIB).a
	ar $(ARFLAG) $(NOMELIB).a $(NOMELIB).o
	rm -f $(NOMELIB).o
	@echo Fine sun.

prova:
	gcc -g -Wall provaftp.c -L./ -lpcftp -lbsd -o provaftp
	gcc -g -Wall recvmarconi.c -L./ -lpcftp -lbsd -o recvmarconi
	@echo Fine prova.
	
provasun:
	gcc -g -Wall provaftp.c -L./ -lpcftp -lsocket -lnsl -o provaftp
	@echo Fine provasun.

install:
	cp $(NOMELIB).a $(DIRL)$(NOMELIB).a
	cp $(NOMELIB).h $(DIRI)$(NOMELIB).h
	@echo Fine install.

volta:
	rcp $(NOMELIB).a volta:/opt/XNC.EVOL$(DIRL)$(NOMELIB).a
	rcp $(NOMELIB).h volta:/opt/XNC.EVOL$(DIRI)$(NOMELIB).h
	@echo Fine volta.

archive:
	@rm -f $(NOMETEMP)*
	@nomea=$(NOMELIB)` grep VERSPCFTP *.h |awk '{print $$3}' |sed 's/"//g' `; \
	echo "Nome file archivio: $$nomea.tgz" ; \
	tar zcvf $$nomea.tgz `ls -l | awk '/^-/ {print $$8 }'` ; \
	if test -f $(DIRA)$$nomea.tgz ; \
	then ( echo -n 'File presente: Vuoi archiviare? [si/*] ' ; read sc ; \
	if test "$$sc" = "si" ; then mv $$nomea.tgz $(DIRA) ; \
	else rm $$nomea.tgz; fi ) ; \
	else ( mv $$nomea.tgz $(DIRA) ) ; fi
	@echo Fine Archive.

?:
	@echo '	all         -->Rigenera la libreria'
	@echo '	purge       -->Cancella i file inutili'
	@echo '	deblib      -->libreria per debug'
	@echo '	sun         -->libreria su solaris'
	@echo '	prova       -->Genera provaftp e recvmarconi di test'
	@echo '	provasun    -->Genera provaftp e recvmarconi di test per solaris'
	@echo '	install     -->Deposita la libreria'
	@echo '	volta       -->Deposita la libreria in volta'
	@echo '	archivia    -->Crea il file di archivio e lo deposita'

#-----------------------------------------------------------------------------
