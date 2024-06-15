/* lplc023.c */
#include "lplc.h"

/*
* Funzione plcTryOpen()
* ---------------------
*
*  Questa funzione collega lo spazio condiviso del PLC al processo chiamante,
* e inizializza i puntatori alle aree statiche contenenti lo stato del Plc
* e la tabella di accesso ai segnali di scambio. Non inizializza invece i
* puntatori alle aree allocate dinamicamente, poiche` dipendono dal particolare
* programma di plc eventualmente caricato. Le inizializzazioni eseguite da
* questa funzione sono comunque sufficienti a garantire il funzionamento delle
* primitive di accesso ai segnali di scambio.
* NOTA : A differenza della "plcOpen", questa funzione non e` bloccante.
*  Puo` essere chiamata ripetutamente, ed il suo valore di ritorno
* indichera` se il collegamento con il PLC e` stato completato o no.
*
*  Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcTryOpen(void)
{
int rv;
static int fase_plc = 0;

	rv = -1;

	switch (fase_plc) {

	/* Fase 0 : tutto e` ancora da fare. */

	case 0:
		/* Ci vuole il parametro 0 (PERCHE` SI`) */
		if (plcTryAttach(0) != 0)
			break;

		/* Qualcosa c'e`... Si cambia fase. */

		fase_plc = 1;

	/* ... E AVANTI ! ... */

	/* Fase 1 : le risorse globali esistono, ma bisogna vedere se
	  il Plc ci ha scritto gia` qualcosa di sensato. */

	case 1:
		if (! plcReady())
			break;

		/* E` partito il Plc ! */

		fase_plc = 2;

		plcLinkStatics();

	/* ... E AVANTI ! ... */

	default:
		rv = 0;
		break;
	}

	return rv;
}

