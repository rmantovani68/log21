/* lplc061.c */
#include "lplc.h"

/*
* Funzione "plcCreateWatchList"
* ------------------------------
*
*  Questa funzione crea una nuova lista di watch, e riporta in "dest"
* l'ID ottenuto. La lista di watch avra` le caratteristiche date nella
* struttura puntatat da "wprop".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcCreateWatchList(watch_descr_t *wprop,int *dest)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_CREATE_WATCH;
	memcpy(&msg[1],(char *)wprop,sizeof(*wprop));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*dest = *(int *)(& answ[4]);
	return 0;
}

