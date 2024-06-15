/* lplc011.c */
#include "lplc.h"

/*
* Funzione plcClose()
* -------------------
*
* Questa funzione scollega dal PLC il processo chiamante.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcClose(void)
{

/* Libera le risorse di sistema occupate. */

	if (pchMem && shmdt(pchMem) < 0) {
		perror("shmdt()");
		return -1;
	}

	return 0;
}

