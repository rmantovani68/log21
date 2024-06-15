/;
/; @(#) lista.s 1.6 Mon Feb  5 13:01:00 MET 2001
/;
/; Lista delle funzioni esterne che il linker "deve" caricare.
/:
/; 23/05/97 GG 1.0 Prima stesura.
/; 07/07/97 GG 1.1 Aggiunti "sdmIsConfLoaded", "write_mdata" (commentato)
/;             e "GetMaccInfo".
/; 24/07/97 GG 1.2 Aggiunto "GetStatoDm".
/; 10/06/98 GG 1.3 Aggiunte "SetAxesData" e "SetAxesDataNi".
/; 01/06/99 GG 1.4 Aggiunto "GetTestData".
/; 28/07/00 GG 1.5 Aggiustamenti per compilazione su Linux. Esistono
/;             ora due versioni di questo file: lista-Linux.s e lista-LynxOS.s
/; 05/02/01 GG 1.6 Aggiunta "write_mdata".
/;
	.text
lista_esterne:
	.long	read_mdata
	.long	write_mdata
	.long	ax_mirror
	.long	sdmIsConfLoaded
	.long	GetMaccInfo
	.long	GetStatoDm
	.long	GetAxesData
	.long	GetAxesDataNi
	.long	SetAxesData
	.long	SetAxesDataNi
	.long	GetAxesNumByName
	.long	GetGenericData
	.long	GetOrigData
	.long	GetOrigItem
	.long	GetOrigName
	.long	GetConfActive
	.long	GetConfData
	.long	GetMandAttr
	.long	GetMandData
	.long	GetCmbUtItem
	.long	GetGenData
	.long	GetAxesItem
	.long	GetPuntData
	.long	GetCmbUtData
	.long	GetTestData
	.long	SetPuntData
	.long	SetGenericData
	.long	fflush
	.long	first_input
	.long	first_output
	.long	invXerror
	.long	memset
	.long	num_output
	.long	pKeyTab
	.long	pchMem
	.long	qualeloop
	.long	regA
	.long	sprintf
	.long	strcmp
	.long	strcpy
	.long	tim_att
	.long	tim_old
	.long	printf
	.long	fprintf
	.long	strcat
	.long	memcpy
	.long	sys_err
	.long	errno
	.long	perror
	.long	sendto
	.long	recvfrom
	.long	send
	.long	recv
	.long	socket
	.long	bind
	.long	connect
	.long	inet_addr
	.long	gethostbyname
	.long	init_client
	.long	mkshm
	.long	shmmap
	.long	close_socket
	.long	shmunmap
	.long	teccy_socket
	.long	mksem
	.long	init_col
	.long	unlink
	.long	release_shared
	.long	read_parm
	.long	write_parm
	.long	sempost
