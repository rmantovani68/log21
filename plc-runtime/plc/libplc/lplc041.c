/* lplc041.c */
#include "lplc.h"

/*
* Funzione "plcTraceVar"
* ----------------------
*
*  Questa funzione aggiunge la variabile "name" all'insieme di trace.
*  Vale 1 se tutto e` andato bene, altrimenti 0, ed in "*err" e` scritto
* uno dei seguenti valori :
*
* PLCERR_TOOVAR  : troppe variabili tracciate,
* PLCERR_BADVAR  : variabile non booleana,
* PLCERR_NOVAR   : variabile non trovata,
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceVar(char * name)
{
struct plcvar_t var;
char msg[1 + sizeof(char *)];
char answ[3];

/* Cerca la variabile. Errore se non c'e`. */

	if (! dbGetVar(name,&var))
		return PLCERR_NOVAR;

#if 0
/* Errore se la variabile non e` booleana. */
	if (var.type != ISAVAR_T_BOOL)
		return PLCERR_BADVAR;
#endif

/* Comunica l'indirizzo della variabile (tradotto per il processo plc). */

	msg[0] = (char) R_TRACE_ADD;
	*(char **)(& msg[1]) = (char *)(MRTRANS(var.pval));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

