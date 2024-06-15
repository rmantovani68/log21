/* lplc022.c */
#include "lplc.h"

/*
* Funzione plcOpen()
* ------------------
*
*  Questa funzione collega lo spazio condiviso del PLC al processo chiamante,
* e inizializza i puntatori alle aree statiche contenenti lo stato del Plc
* e la tabella di accesso ai segnali di scambio. Non inizializza invece i
* puntatori alle aree allocate dinamicamente, poiche` dipendono dal particolare
* programma di plc eventualmente caricato. Le inizializzazioni eseguite da
* questa funzione sono comunque sufficienti a garantire il funzionamento delle
* primitive di accesso ai segnali di scambio.
*
*  Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcOpen(void)
{

	/* Collega il processo allo spazio condiviso del PLC. */

	if (plcAttach() < 0)
		return -1;

	/* Attende che il plc sia pronto ed
	 inizializza i puntatori alle aree statiche di interesse. */

	if (plcWait() < 0)
		return -1;

	return plcLinkStatics();
}

