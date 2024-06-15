/* lplc044.c */
#include "lplc.h"

/*
* Funzione "plcTraceStop"
* -----------------------
*
*  Questa funzione comunica al plc di terminare il trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceStop(void)
{
char msg[1];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_STOP;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

