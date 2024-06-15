/* lplc037.c */
#include "lplc.h"

/*
* Funzione plcReady()
* -------------------
*
*  Questa funzione vale 1 se il Plc e` stabilizzato, cioe` ha terminato la 
* fase di startup, 0 altrimenti.
*/

int plcReady(void)
{
	if (! pStatus)
		return 0;

	return pStatus[PLCSTS_OKLINK];
}

