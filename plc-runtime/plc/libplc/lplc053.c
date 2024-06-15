/* lplc053.c */
#include "lplc.h"

/*
* Funzione "plcModProp"
* ---------------------
*
*  Questa funzione copia in "dest" l'immagine della struttura "modprop_t"
* inerente il modulo di nome "name".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcModProp(struct modprop_t *dest,char *name)
{
char msg[MAX_REQ];
char answ[MAX_REQ+1];

	msg[0] = (char) R_DYNLD_PROP;
	strncpy(&msg[1],name,MAX_REQ-1);
	msg[MAX_REQ-1] = '\0';
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,&answ[4],sizeof(struct modprop_t));
	return 0;
}

