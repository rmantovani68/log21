/* lplc009.c */
#include "lplc.h"

/*
* Funzione "plcVSem(int n)"
*
* Libera il semafono n.
*/

void plcVSem(int n)
{
struct sembuf op;

	op.sem_num = n;
	op.sem_op = 1;
	op.sem_flg = 0;
	if (semop(iSemId, &op, 1) < 0) {
		perror("semop()");
	}
}

