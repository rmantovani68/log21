/* lplc068.c */
#include "lplc.h"

/*
* Funzione "plcLastDebugEventList"
* --------------------------------
*
*  Questa funzione carica la lista di eventi di debug prodotta
* nell'ultimo ciclo di Plc, e la copia in un'area allocata internamente
* dalla libreria. Le successive "plcGetDebugEvent" restituiranno in sequenza
* gli eventi contenuti nella lista.
*/

brk_event_list * plcLastDebugEventList(void)
{
brk_event_list *el;
unsigned long nb,lastev;

/* Inizializza se necessario, errore se non si riesce. */

	if (! plc_get_event_status.buffer
	 && ! plcResetDebugEventList()
	 && ! plc_get_event_status.buffer)
		return (brk_event_list *) NULL;

	plc_get_event_status.read_pos = 0;

/* Se e` definito il segnale di scambio $DEB_LASTEV, allore ci devono
 anche essere $DEB_NEVT e $DEB_EVENTS. In caso contrario, devono esistere
 almeno $DEB_LASTEVP e $DEB_FIRSTEVP. */

	if (plc_get_event_status.lastevp) {

/* Nuova gestione (qplc > 1.5.1.2). Si usa il segnale di scambio $DEB_LASTEVP*/

		lastev = *plc_get_event_status.lastevp;
		if (lastev == -1)
			return (brk_event_list *) NULL;
	}
	else if (plc_get_event_status.lastev) {

/* Vecchia gestione. */

/* Legge il segnale di scambio che contiene l'indice nell'array degli
 eventi dell'ultima lista di eventi registrata (-1 se non ce ne sono). */

		lastev = *plc_get_event_status.lastev;
		if (lastev == -1)
			return (brk_event_list *) NULL;

/* Legge la posizione della lista di eventi nel buffer dall'array.
 (contrlolla che, per qualche sfiga, non valga -1, anche se non
 dovrebbe mai accadere). */

		lastev = plc_get_event_status.events[lastev];
		if (lastev == -1)
			return (brk_event_list *) NULL;
	}
	else {
		return (brk_event_list *) NULL;
	}

/* Copia gli eventi nel buffer locale. */

	/* Lunghezza del blocco. */
	plcDebugMemCopy((char *)&nb,lastev,sizeof(nb));
	plc_get_event_status.event_end = nb;
	if (nb == 0 || nb > plc_get_event_status.size)
		return (brk_event_list *) NULL;
	/* Dati. */
	plcDebugMemCopy(plc_get_event_status.buffer,lastev,nb);

	el = (brk_event_list *)plc_get_event_status.buffer;

/* Confronta il ciclo caratteristico di Plc della lista di eventi corrente con
 quello della lettura precedente (se ce n'e` stata una). Se sono uguali,
 vuol dire che non ci sono in realta` nuovi eventi. */

	if (plc_get_event_status.cycle_ok
	 && el -> bl_cycle == plc_get_event_status.cycle)
		return (brk_event_list *) NULL;

/* Costruzione dei puntatori alle liste di eventi. */

	plc_get_event_status.lists[0] = el;

/* Aggiorna la struttura descrivente la storia. */

	plc_get_event_status.history.bh_first_cycle
	 = plc_get_event_status.history.bh_last_cycle
	 = plc_get_event_status.lists[0] -> bl_cycle;
	plc_get_event_status.history.bh_list_count = 1;

/* Porta la posizione di lettura degli eventi all'inizio della lista. */

	plcIndexDebugEventList(0);

	return el;
}

