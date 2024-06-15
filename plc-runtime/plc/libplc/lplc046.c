/* lplc046.c */
#include "lplc.h"

/*
* Funzione "plcTraceGetBuf"
* ------------------------
*
*  Questa funzione riporta in "*n" la dimensione del buffer di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcTraceGetBuf(int *n)
{
struct tracests_t status;
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato del trace. */

	msg[0] = (char) R_TRACE_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	status = *(struct tracests_t *)(& answ[4]);

	*n = status.dimbuff;

	return 0;
}

