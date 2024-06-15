/* lplc047.c */
#include "lplc.h"

/*
* Funzione "plcTraceDownLoad"
* ---------------------------
*
*  Questa funzione copia l'ultimo buffer di trace generato, oppure
* esce con errore.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_NOTREADY : non e` ancora pronto il buffer da copiare.
* PLCERR_TIMEOUT  : timeout nella comunicazione col PLC.
* PLCERR_REMERR   : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceDownLoad(TRACERECORD *p)
{
struct tracests_t status;
char msg[1 + 2*sizeof(int)];
char answ[MAX_REQ];
int i,n,pos,bl,nr;
TRACERECORD *q;

/* Richiede lo stato del trace. */

	msg[0] = (char) R_TRACE_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	status = *(struct tracests_t *)(& answ[4]);

/* Se il buffer non e` ancora pronto, esce con un codice di errore apposito. */

	if (! status.completed)
		return PLCERR_NOTREADY;

/* Calcola il numero massimo di record ottenibili con una singola
 interrogazione del plc. */

	bl = (MAX_REQ - 4) / sizeof(TRACERECORD);

/* Copia il buffer a passi di alpiu` "bl" elementi. */

	for (n = status.dimbuff, pos = status.lastbuff; n; n -= nr) {

	/* Costruisce il comando. */

		nr = n > bl ? bl : n;
		msg[0] = (char) R_TRACE_LOAD;
		*(int *)(& msg[1]) = pos;
		*(int *)(& msg[1 + sizeof(int)]) = nr;
		if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
			return PLCERR_TIMEOUT;
		if (*(short *)(& answ[1]) != CR_OK)
			return PLCERR_REMERR;

	/* Copia il blocco nella destinazione finale. */

		for (i = 0, q = (TRACERECORD *)(& answ[4]); i < nr; ++i)
			*(p++) = *(q++);

	/* Manda avanti la posizione di lettura. */

		pos += nr;
		if (pos >= status.dimtot)
			pos = 0;
	}

/* Fine. Segnala al plc che il download e` stato completato. */

	msg[0] = (char) R_TRACE_SIGNAL;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

