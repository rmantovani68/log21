/* lplc055.c */
#include "lplc.h"

/*
* Funzione "plcStEnable"
* ---------------------
*
*  Questa funzione porta il flag di abilitazione della statistica al valore
* indicato in 
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStEnable(int flag)
{
char msg[1 + sizeof(char)];
char answ[4];

	msg[0] = (char) R_START_STAT;
	msg[1] = (char) flag;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

