/* lplc038.c */
#include "lplc.h"

/*
* Funzione plcWait()
* ------------------
*
*  Questa funzione sinronizza il processo chiamante con il kernel del plc.
*  La sincronizzazione si realizza con un polling di uno dei puntatori alle
* aree statiche globali di Isagraf.
*
*  La funzione vale 0 in caso di successo, -1 in caso di errore
* (plc non inizializzato).
*/

/* Periodo di polling dello start del Plc. */

#define PLC_WAIT_TIMEOUT 1

int plcWait(void)
{
	if (! pStatus)
		return -1;

	while (! plcReady()) {
		sleep(PLC_POLL_TIME);
	}

	return 0;
}

