/* lplc070.c */
#include "lplc.h"

/*
* Funzione "plcIndexDebugEventList"
* ---------------------------------
*
*  Questa funzione porta la posizione di lettura degli eventi all'inizio
* della lista di indice dato. L'indice rappresenta la posizione della lista
* nell'insieme di liste caricate nel buffer locale della libreria con la
* funzione "plcLoadDebugHistory".
*  Restituisce il puntatore al descrittore della lista di eventi selezionata,
* oppure NULL in caso di errori.
*/

brk_event_list * plcIndexDebugEventList(int index)
{
brk_event_list *el;

	if (index < 0 || index >= plc_get_event_status.history.bh_list_count)
		return (brk_event_list *) NULL;

	el = plc_get_event_status.lists[index];

/* Se si seleziona l'ultimo evento della lista, aggiorna le variabili
 utilizzate per l'individuazione di nuovi eventi da parte delle
 funzioni "plcLastDebugEventList" e "plcLoadDebugHistory". */

	if (index == plc_get_event_status.history.bh_list_count - 1) {
		plc_get_event_status.cycle_ok = 1;
		plc_get_event_status.cycle = el -> bl_cycle;
	}

/* Porta la posizione di lettura degli eventi all'inizio della lista. */

	plc_get_event_status.read_pos = (char *)&(el -> bl_event[0])
	                              - (char *)plc_get_event_status.buffer;

/* Calcola la fine dell'evento. */

	plc_get_event_status.event_end
	 = el -> bl_size + ((char *)el - (char *)plc_get_event_status.buffer);

	return el;
}

