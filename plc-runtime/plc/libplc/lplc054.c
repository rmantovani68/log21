/* lplc054.c */
#include "lplc.h"

/*
* Funzione "plcStStatus"
* ----------------------
*
*  Questa funzione riporta in "*n" lo stato del flag di abilitazione della 
* statistica.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcStStatus(int *n)
{
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato della statistica. */

	msg[0] = (char) R_IF_ENSTAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*n = answ[3];

	return 0;
}

