/* lplc040.c */
#include "lplc.h"

/*
* Funzione "plcTraceAddr"
* -----------------------
*
*  Questa funzione aggiunge la variabile di indirizzo "addr" all'insieme
* di trace.
*  Vale 1 se tutto e` andato bene, altrimenti 0, ed in "*err" e` scritto
* uno dei seguenti valori :
*
* PLCERR_TOOVAR  : troppe variabili tracciate,
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*
* NOTA 1 : L'indirizzo e` da intendersi relativo allo spazio virtuale del PLC.
* NOTA 2 : Nelle versioni precedenti alla 3.2 di isaker, la validita`
*          dell'indirizzo non e` controllata dall'esecutore, col rischio
*          di provocare il crash dell'esecutore.
*/

int plcTraceAddr(unsigned char * addr)
{
char msg[1 + sizeof(char *)];
char answ[3];

/* Comunica l'indirizzo della variabile (tradotto per il processo plc). */

	msg[0] = (char) R_TRACE_ADD;
	*(char **)(& msg[1]) = (char *)(addr);
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

