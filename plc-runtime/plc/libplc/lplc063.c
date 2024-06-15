/* lplc063.c */
#include "lplc.h"

/*
* Funzione "plcAddBreak"
* ----------------------
*
*  Questa funzione aggiunge un punto di break la lista di watch generata con
* "plcAddBreak". Vuole l'ID della lista di watch ed un  puntatore ad
* una struttura descrivente le azioni da compiere 
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcAddBreak(int wid,watch_t *brk)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_ADD_BREAK;
	*(int *)(&msg[1]) = wid;
	memcpy(&msg[1+sizeof(int)],(char *)brk,sizeof(*brk));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

