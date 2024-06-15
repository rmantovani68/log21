/* lplc060.c */
#include "lplc.h"

/*
* Funzione "plcMultiMemCopy"
* ------------------------------
*
*  Questa funzione riporta in "dest" "n" dei byte impostati con la
* "plcMultiMemCheck".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcMultiMemCopy(char *dest,int n)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_KERMEM_MCOPY;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

