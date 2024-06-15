/* lplc020.c */
#include "lplc.h"

/*
* Funzione plcLinkDynamics()
* --------------------------
*
*  Questa funzione inizializza i puntatori alle aree di plc allocate
* dinamicamente ad ogni caricamento di applicazione.
*  Vale 0 in caso di successo, -1 in caso di errore (plc non in start).
*/

int plcLinkDynamics(void)
{
char *addr;

	if (! pStatus || ! pStatus[PLCSTS_GO])
		return -1;

	/* Trova la struttura di stato del kernel Plc. */

	addr = spDir[SPC_KER].size ? pchMem + spDir[SPC_KER].offset : (char *)0;
	KER = (str_ker *) addr;

	/* Trova le variabili. */

	if (KER) {
		BF_BOO = (char *)MTRANS(KER -> bf_boo);
		BF_ANA = (long *)MTRANS(KER -> bf_ana);
		BF_TMR = (long *)MTRANS(KER -> bf_tmr);
	}
	else
		return -1;

	return 0;
}

