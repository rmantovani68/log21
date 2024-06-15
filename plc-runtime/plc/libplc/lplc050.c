/* lplc050.c */
#include "lplc.h"

/*
* Funzione "plcLdStatus"
* ----------------------
*
*  Questa funzione riporta in "*n" lo stato dell'ultimo caricamento dinamico.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC (NORMALE !).
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcLdStatus(int *n)
{
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato del trace. */

	msg[0] = (char) R_DYNLD_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*n = *(int *)(& answ[4]);

	return 0;
}

