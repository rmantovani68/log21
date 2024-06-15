/* lplc071.c */
#include "lplc.h"

/*
* Funzione "plcLoadDebugHistory"
* ------------------------------
*
*  Questa funzione carica la parte consistente (cioe` non ancora ricoperta)
* del buffer di debug, e la copia in un'area allocata internamente dalla
* libreria. La posizione di lettura degli eventi e` portata all'inizio
* dell'ultima lista caricata.
*  Restituisce il puntatore ad una struttura statica interna che descrive
* l'insieme degli eventi caricati (primo ed ultimo ciclo di plc coinvolti,
* numero di liste presenti), oppure NULL in caso di errori.
*  Se il parametro "p_el" e` diverso da NULL, nella cella puntata sara`
* scaricato il puntatore al descrittore dell'ultima lista trovata, oppure
* NULL se la lista caricata e` uguale alla precedente. Questo valore e` in
* effetti lo stesso che si otterrebbe dalla funzione "plcLAstDebugEventList",
* della quale questa e` di fatto un'estensione.
*/

brk_history * plcLoadDebugHistory(brk_event_list ** p_el)
{
brk_event_list *el;
unsigned long p_firstev,firstev,lastev, pos;
int n;

/* Inizializza se necessario, errore se non si riesce. */

	if (! plc_get_event_status.buffer
	 && ! plcResetDebugEventList()
	 && ! plc_get_event_status.buffer)
		return (brk_history *) NULL;

	plc_get_event_status.read_pos = 0;

/* Devono essere definiti i segnali di scambio $DEB_LASTEVP e $DEB_FIRSTEVP,
 altrimenti non se ne parla nemmeno. */

/* La sequenza di lettura delle informazioni dai segnali di scambio di debug
 e` la seguente:

  1) Lettura della posizione dell'ultima lista di eventi registrata
     ($DEB_LASTEVP).
  2) Lettura della posizione della prima lista di eventi non ricoperta
     ($DEB_FIRSTEVP). Questo dato sara` utilizzato come punto di inizio
     della copia del buffer, in modo che gli eventi siano immagazzinati
     in ordine temporale nel buffer locale.
  3) Lettura dll'intero buffer circolare degli eventi, a partire dalla
     posizione ottenuta al punto (2). In questo modo gli eventi saranno
     riordinati.
  4) Lettura della posizione della prima lista di eventi non ricoperta
     ($DEB_FIRSTEVP). Sara` il valore definitivo, poiche` quello ottenuto al
     punto (2) puo` essere divenuto nel frattempo obsoleto.

  Questa sequenza assicura che i dati che si trovano tra la posizione letta
 al punto 3 e quella letta al punto 1 siano liste di eventi "sane", cioe` non
 ricoperte. Nella peggiore delle ipotesi, la funzione potra` non vedere
 alcuni eventi piu` recenti (che saranno comunque leggibili in una chiamata
 successiva) ed alcuni vecchi eventi non realmente ricoperti (ma si puo`
 supporre che siano gia` stati letti in una chiamata precedente).  */

/* Punto (1): Acquisizione della posizione dell'evento piu` recente. */
/* ----------------------------------------------------------------- */

	if (plc_get_event_status.lastevp) {
		lastev = *plc_get_event_status.lastevp;
		if (lastev == -1)
			return (brk_history *) NULL;
	}
	else {
		return (brk_history *) NULL;
	}

/* Punto (2): Acquisizione posizione provvisoria dell'evento piu` vecchio. */
/* ----------------------------------------------------------------------- */

/* Pre-acquisizione della posizione della lista piu` vecchia. Non e` il valore
 definitivo. Serve solo per avere un punto di partenza per la copia, in modo
 che nella copia locale eventi successivi compaiano in posizioni successive. */

	if (plc_get_event_status.firstevp) {
		p_firstev = *plc_get_event_status.firstevp;
		if (p_firstev == -1)
			p_firstev = lastev;
	}
	else {
		return (brk_history *) NULL; /* Non dovrebbe accadere. */
	}

/* Punto (3): Copia del buffer degli eventi. */
/* ----------------------------------------- */

/* Copia del buffer. Non conviene preoccuparsi di non copiare piu` byte
 del necessario, prima di tutto perche` sarebbe difficile calcolarne la
 quantita` esatta, poi perche`, a regime, il buffer e` sempre ricoperto,
 cioe` pieno a meno della lunghezza media di una lista. */

	plcDebugMemCopy(plc_get_event_status.buffer,
	                p_firstev,
	                plc_get_event_status.size);

/* Punto (4): Acquisizione posizione definitiva dell'evento piu` vecchio. */
/* ---------------------------------------------------------------------- */

	firstev = *plc_get_event_status.firstevp;
	if (firstev == -1)
		firstev = lastev;

/* Normalizzazione degli indici "firstev" e "lastev". */
/* Gli eventuali dati tra "p_firstev" e "firstev" potrebbero essere stati
 ricoperti, quindi vanno ignorati. */

	if (firstev < p_firstev)
		firstev += plc_get_event_status.size;
	firstev -= p_firstev;
	if (lastev < p_firstev)
		lastev += plc_get_event_status.size;
	lastev -= p_firstev;


/* Costruzione dei puntatori alle liste di eventi. */

	for (n = 0, pos = firstev; pos <= lastev; pos += el -> bl_size) {
		el = (brk_event_list *)(plc_get_event_status.buffer + pos);
		plc_get_event_status.lists[n++] = el;
	}

/* Aggiorna la struttura descrivente la storia. */

	plc_get_event_status.history.bh_first_cycle
	 = plc_get_event_status.lists[0] -> bl_cycle;
	plc_get_event_status.history.bh_last_cycle
	 = plc_get_event_status.lists[n - 1] -> bl_cycle;
	plc_get_event_status.history.bh_list_count = n;

/* Confronta il ciclo caratteristico di Plc della lista di eventi corrente con
 quello della lettura precedente (se ce n'e` stata una). Se sono uguali,
 vuol dire che non ci sono in realta` nuovi eventi. */

	if (p_el) {
		if (plc_get_event_status.cycle_ok
		 && el -> bl_cycle == plc_get_event_status.cycle) {
			*p_el = (brk_event_list *) NULL;
		}
		else {
			*p_el = el;
		}
	}

/* Porta la posizione di lettura degli eventi all'inizio dell'ultima lista. */

	plcIndexDebugEventList(n - 1);

	return &plc_get_event_status.history;
}

