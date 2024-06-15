/* lplc059.c */
#include "lplc.h"

/*
* Funzione "plcMultiMemCheck"
* ------------------------------
*
*  Questa funzione riporta in "dest" gli "n" flag di abilitazione
* per altrettante coppie indirizzo-lunghezza in "src". L'insieme di
* aree cosi` descritte sara` poi letto dalle successive "plcMultiMemCopy".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcMultiMemCheck(char *dest,struct plc_mcopy_t *src,int n)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	if (n <= 0)
		return 0;
	if (n*sizeof(src[0])-2*sizeof(long) > MAX_REQ) {
		n = (MAX_REQ-2*sizeof(long))/sizeof(src[0]);
	}
	msg[0] = (char) R_KERMEM_MCHK;
	memcpy(&msg[4],(char *)(& n),sizeof(int));
	memcpy(&msg[4 + sizeof(int)],(char *)src,n*sizeof(src[0]));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

