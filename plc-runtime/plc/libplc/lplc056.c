/* lplc056.c */
#include "lplc.h"

/*
* Funzione "plcStClear"
* ---------------------
*
*  Questa funzione azzera la statistica.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStClear(void)
{
char msg[1 + sizeof(char)];
char answ[4];

	msg[0] = (char) R_CLEAR_STAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

