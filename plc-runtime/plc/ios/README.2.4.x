Tue Apr  2 17:16:18 CEST 2002

 Installazione del driver IOSCNI  per Kernel 2.4.x

0) Diventare "root"

1) Eseguire le segeanti operazioni:

	cd ios/kernel-2.4.x/drivers/char
	make all
	make install

per caricare il modulo ioscni.o occorre lanciare lo script di shell
	ioscni_load 
presente nel direttorio
	ios/kernel-2.4.x/drivers/char


