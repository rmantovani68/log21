/* lplc019.c */
#include "lplc.h"

/*
* Funzione plcHalt()
* ------------------
*
* Forza l'Halt del Plc. Vale 1 se l'halt ha avuto successo, 0 altrimenti.
*/

int plcHalt(void)
{
	if (! pStatus)
		return 0;
	if (SYST -> start != START_OK)
		return 1;
	SYST -> start = START_STOP;
	while (SYST -> start != START_INIT)
		usleep(LOOP_SLEEP);
	return 1;
}

