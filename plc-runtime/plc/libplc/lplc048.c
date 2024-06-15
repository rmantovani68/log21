/* lplc048.c */
#include "lplc.h"

/*
* Funzione "plcLoadProg"
* ----------------------
*
*  Questa funzione carica dinamicamente un programma di plc dal file
* oggetto (LynxOS a.out) "name".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*
* NOTA : DATA LA LENTEZZA DELL'OPERAZIONE, L'ERRORE DI TIMEOUT E` DA
* CONSIDERARE FISIOLOGICO, E NON DEVE PREOCCUPARE.
*/

int plcLoadProg(char *name)
{
char msg[256];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_DYNLD_LOAD;
	strncpy(msg + 1,name,sizeof(msg) - 3);
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

