/* lplc021.c */
#include "lplc.h"

/* Variabile statica inizializzata dalla "plcLinkStatics" e controllata
 dalla "plcDbChanged" per individuare i cambiamenti del database
 dei simboli. */

static long last_db_vers = -1;

/*
* Funzione plcLinkStatics()
* -------------------------
*
*  Questa funzione inizializza i puntatori alle aree di plc allocate
* una volta per tutte, quindi con posizione indipendente dal programma
* di plc caricato (o, eventualmente, non caricato !).
*  La chiamata di questa funzione e` sufficiente se si desidera accedere
* alle variabili plc solo tramite il meccanismo dei segnali di scambio.
*  Se invece si vuole accedere a tutti i simboli di Isagraf, e` necessario
* attendere la partenza dell'applicazione plc e chiamare la "plcLinkDymanics".
*
*  La funzione vale 0 in caso di successo, -1 in caso di errore
* (plc non inizializzato).
*/

int plcLinkStatics(void)
{
char *addr;

	if (! pStatus)
		return -1;

	/* Trova la struttura di controllo del Plc. */

	addr = spDir[SPC_SYS].size ? pchMem + spDir[SPC_SYS].offset : (char *)0;
	SYST = (str_system *) addr;

	/* Trova il buffer dei simboli (ISA13...). */

	addr = spDir[SPC_SYM].size ? pchMem + spDir[SPC_SYM].offset : (char *)0;
	SYMS = (char *) addr;

	/* Trova il buffer di colloquio col il debugger. */

	addr = spDir[SPC_TST].size ? pchMem + spDir[SPC_TST].offset : (char *)0;
	ANSW = (unsigned char *) addr;

	/* Trova la tabella dei segnali di scambio. */

	addr = spDir[SPC_TBL].size ? pchMem + spDir[SPC_TBL].offset : (char *)0;
	pKeyTab = (struct trtel_t *) addr;

	/* Trova la tabella di hash dei simboli. */

	addr = spDir[SPC_HTB].size ? pchMem + spDir[SPC_HTB].offset : (char *)0;
	pHashTab = (struct plchash_t *) addr;

	/* Trova la tabella di hash dei simboli (stile QPLC). */

	addr = spDir[17].size ? pchMem + spDir[17].offset : (char *)0;
	QplcHash = (long *) addr;

	/* Trova l'indirizzo virtuale dell'area condivisa nello spazio
	 di indirizzamento del Plc. */

	pchMMem = (char *)pStatus[PLCSTS_MEMBASE];

	/* Inizializza ad un valore "impossibile" il numero di versione
	 del database PLC. */

	last_db_vers = -1;

	return 0;
}

/*
* Funzione plcDbChanged()
* -----------------------
*
*  Questa funzione vale 1 se il database e` cambiato rispetto
* all'ultima operazione di link, 0 altrimenti. Se il database
* e` cambiato, essa aggiorna automaticamente la variabile interna
* che registra l'ultima versione. Una volta ottenuta una segnalazione,
* quindi, il risultato di chiamate successive sara` di nuovo 0 !.
*/

int plcDbChanged(void)
{
long cv;

	cv = pStatus[PLCSTS_DBVER];
	if (cv == last_db_vers)
		return 0;
	else {
		last_db_vers = cv;
		return 1;
	}
}

