/*
* txser.c : trasmissione sulla porta seriale
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <pmx_msq.h>
#include <msq_lib.h>

char szRcvBuffer[256];


/* 
* Messaggi in trasmissione
* relativo simulatore 
*/
MSG_SPEC msgSnd[] = {
	{"\x1b\x05", 2, 0, 0, 2, 0, 0, 0, 0},
	{"\x24", 1, 0, 0, 21, 0, 0, 0, 0},
};


/* 
* main() 
*/
void main(int argc, char **argv)
{
	char szSndBuffer[80];
	int nNumCar;
	char szCommDevice[80];
	int nBaudrate;
	int nParity;
	int nDataBits;
	int nStopBits;
	int nPortFD;

	if (argc != 6) {
		printf("Utilizzo : %s Porta Baudrate Parity DataBits StopBits\n",argv[0]);
		return;
	} else {
		strcpy(szCommDevice,argv[1]);
		nBaudrate=atoi(argv[2]);
		nParity=atoi(argv[3]);
		nDataBits=atoi(argv[4]);
		nStopBits=atoi(argv[5]);
	}

	printf("Parametri di comunicazione : %s : %d,%d,%d,%d\n",
			szCommDevice,nBaudrate,nParity,nDataBits,nStopBits);
	/* Apertura porta seriale */
	if (InitSer(szCommDevice,&nPortFD,nBaudrate,nDataBits,nParity,nStopBits,O_RDWR)) {
		printf("Apertura porta '%s' fallita\n", szCommDevice);
	}

	sprintf(szSndBuffer,"%07d %07d AAA\r\n",12345,67890);

	if (SendSerMsg(nPortFD,szSndBuffer,msgSnd+1,&nNumCar)){
		/* Trasmissione fallita */
		printf("Errore in trasmissione\n");
	}
	/*
	printf("Spedito : [%s]\n",szSndBuffer);
	*/
	close(nPortFD);
}
