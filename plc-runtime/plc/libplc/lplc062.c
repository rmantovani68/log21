/* lplc062.c */
#include "lplc.h"

/*
* Funzione "plcDeleteWatchList"
* ------------------------------
*
*  Questa funzione cancella la lista di watch generata con
* "plcCreateWatchList".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcDeleteWatchList(int wid)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_DELETE_WATCH;
	*(int *)(&msg[1]) = wid;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

