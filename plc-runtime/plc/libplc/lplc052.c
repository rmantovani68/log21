/* lplc052.c */
#include "lplc.h"

/*
* Funzione "plcStatistics"
* ------------------------
*
*  Questa funzione copia in "curr", "min, e "max" la durata corrente, minima
* massima del ciclo di plc.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStatistics(long *curr,long *min, long *max)
{
char msg[4];
char answ[MAX_REQ];

	msg[0] = (char) R_GET_TCYSTAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*curr = *(long *)(&answ[4]);
	*min = *(long *)(&answ[4 + sizeof(long)]);
	*max = *(long *)(&answ[4 + 2*sizeof(long)]);
	return 0;
}

