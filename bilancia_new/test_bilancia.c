/*
* test_bilancia.c : test bilancia
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


#define TIMEOUT							4
#define TIMEOUT_WITHIN			0
#define TIMEOUT_BEFORE			4
#define FETCH_DATA_OFFSET		4

/* 
* Messaggi in ricezione
*/
MSG_SPEC msgRcv[] = {
	{"\x24", 1, 0, 0, 22, 1, 0x0a0d, 0, 0},
};

/* 
* Messaggi in trasmissione
*/
MSG_SPEC msgSnd[] = {
	{"\x1b\x05", 2, 0, 0, 2, 0, 0, 0, 0},
};

/* 
* main() 
*/
void main(int argc, char **argv)
{
	char szRcvBuffer[256];
	int nNumCar;
	char szCommDevice[80];
	int nBaudrate;
	int nParity;
	int nDataBits;
	int nStopBits;
	int nPortFD;
	int	nMsgLung;
	int	nRitorno;
	int	nMsgList[MAX_NUM_MESSAGES];
	int nNumMsg;

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

	if(SendSerMsg(nPortFD,NULL,msgSnd,&nNumCar)){
		printf("Trasmissione Fallita\n");
	}

	while(1){

		nMsgList[0] = 0;
		nNumMsg = 1;
		nRitorno = RecSerMsg(nPortFD, szRcvBuffer, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
		printf("Arrivato : [%s] [%d]\n",szRcvBuffer,nRitorno);
	}

}
