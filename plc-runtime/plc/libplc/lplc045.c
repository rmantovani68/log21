/* lplc045.c */
#include "lplc.h"

/*
* Funzione "plcTraceSetBuf"
* ------------------------
*
*  Questa funzione imposta la dimensione del buffer di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcTraceSetBuf(int n)
{
char msg[1 + sizeof(int)];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_SETBUF;
	*(int *)(&msg[1]) = n;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

