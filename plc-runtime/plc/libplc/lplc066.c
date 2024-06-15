/* lplc066.c */
#include "lplc.h"

/*
* Funzione "plcResetDebugEventList"
* ---------------------------------
*
*  Questa funzione reinizializza le funzioni di lettura dell'area di debug.
*  Vale 0 se tutto e` andato bene, -1 in caso di errori.
*/

struct _plc_get_event_t plc_get_event_status = { 0, 0, 0, /*...*/ };

int plcResetDebugEventList(void)
{
struct plcvar_t t;
struct _plc_get_event_t s;

	plc_get_event_status.buffer = NULL;

	if (! dbGetVar(DEBSHV_BSIZE,&t))
		return -1;
	if (! (s.size = *(unsigned long *) t.pval) )
		return -1;

	if (! dbGetVar(DEBSHV_NEVT,&t))
		s.nevt = 0;
	else
		s.nevt = *(unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_EVENTS,&t))
		s.events = (unsigned long *) 0;
	else
		s.events = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_LASTEV,&t))
		s.lastev = (unsigned long *) 0;
	else
		s.lastev = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_LASTEVP,&t))
		s.lastevp = (unsigned long *) 0;
	else
		s.lastevp = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_FIRSTEVP,&t))
		s.firstevp = (unsigned long *) 0;
	else
		s.firstevp = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_EVCOUNT,&t))
		s.evcount = (unsigned long *) 0;
	else
		s.evcount = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_NREGEV,&t))
		s.nregev = (unsigned long *) 0;
	else
		s.nregev = (unsigned long *) t.pval;

	if (!(s.nevt && s.events && s.lastev)) {
		s.nevt = 0;
		s.events = (unsigned long *)0;
		s.lastev = (unsigned long *)0;
	}

	if (!(s.lastevp && s.firstevp && s.evcount && s.nregev)) {
		s.lastevp = (unsigned long *)0;
		s.firstevp = (unsigned long *)0;
		s.evcount = (unsigned long *)0;
		s.nregev = (unsigned long *)0;
	}

	if (!s.nevt && !s.lastevp)  
		return -1;

	if (! dbGetVar(DEBSHV_BUFFER,&t))
		return -1;
	s.deb_buffer = (unsigned char *) t.pval;

	if (plc_get_event_status.buffer) {
		free(plc_get_event_status.buffer);
		plc_get_event_status.buffer = NULL;
	}
	s.buffer = (unsigned char *) malloc(s.size);
	if (! s.buffer)
		return -1;

	if (plc_get_event_status.lists) {
		free((void *)plc_get_event_status.lists);
		plc_get_event_status.lists = (brk_event_list **)NULL;
	}
/* Calcolo del massimo numero di liste di eventi teoricamente presenti nel
 buffer. E` la dimensione del buffer diviso per la dimensione del piu`
 piccolo descrittore di lista. Il "- sizeof(brk_event)" tiene conto
 della possibilita` (del tutto ipotetica) che si presentino liste di
 zero eventi (la descrizione del tipo "liste di eventi" in C standard non
 permette di indicare un vettore di dimensione 0, percio`...). La stima che
 si ottiene e` un po' pessimistica, ma previene sfighe. */
	s.n_lists = s.size / (sizeof(brk_event_list) - sizeof(brk_event));
	s.lists = (brk_event_list **) malloc(s.n_lists
	                                   * sizeof(brk_event_list *));
	if (! s.lists) {
		free(plc_get_event_status.buffer);
		plc_get_event_status.buffer = NULL;
		return -1;
	}

	s.cycle_ok = 0;
	s.cycle = 0;
	s.event_end = 0;
	s.read_pos = 0;
	s.history.bh_first_cycle = 0;
	s.history.bh_last_cycle = 0;
	s.history.bh_list_count = 0;

	plc_get_event_status = s;

	return 0;
}

