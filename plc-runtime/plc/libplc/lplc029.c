/* lplc029.c */
#include "lplc.h"

/*
* Funzione plcStatus()
* --------------------
*
* Vale 0 se il Plc e` in HALT, 1 se in GO.
*/

int plcStatus(void)
{
	if (pStatus && pStatus[PLCSTS_GO])
		return 1;
	else
		return 0;
}

