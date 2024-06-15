/* lplc051.c */
#include "lplc.h"

/*
* Funzione "plcMemCopy"
* ---------------------
*
*  Questa funzione copia in "dest" gli "n" byte all'indirizzo "src"
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*
* NOTA : IL PLC NON CONTROLLA LA SENSATEZZA DELLA RICHIESTA. INDIRIZZI
* O DIMENSIONI ERRATE POSSONO PROVOCARE IL CRASH DEL PLC !!!
*/

int plcMemCopy(char *dest,char *src,int n)
{
char msg[1 + sizeof(int) + sizeof(char *)];
char answ[MAX_REQ];

	msg[0] = (char) R_KERMEM_COPY;
	memcpy(&msg[1],(char *)(& src),sizeof(char *));
	memcpy(&msg[1 + sizeof(char *)],(char *)(& n),sizeof(int));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

