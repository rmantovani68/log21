/* lplc058.c */
#include "lplc.h"

/*
* Funzione "plcGetExeName"
* -----------------------
*
*  Questa funzione copia in "dest" il path dell'eseguibile PLC.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcGetExeName(char *dest)
{
char msg[MAX_REQ];
char answ[MAX_REQ+1];

	msg[0] = (char) R_GET_EXENAME;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	strcpy(dest,&answ[3]);
	return 0;
}

