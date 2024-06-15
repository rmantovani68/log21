/* lplc018.c */
#include "lplc.h"


/*
* Funzione plcGo()
* ----------------
*
* Tenta di far partire il Plc. Vale 1 se il Plc e` partito, 0 altrimenti.
*/

int plcGo(void)
{
	if (! pStatus)
		return 0;
	if (SYST -> start == START_OK)
		return 1;
	if (SYST -> start != START_INIT)
		return 0;
	SYST -> start = START_APL;
	for (;;) {
		if (SYST -> start == START_INIT)
			return 0;
		if (SYST -> start == START_OK)
			return 1;
		usleep(LOOP_SLEEP);
	}
}

