/*
* cback.h 1.3 Fri Jul 28 18:56:26 MET DST 2000
*
*  Questo file contiene primitive che realizzano un maccanismo di
* callback per le componenti di QPLC.
*
* 04/11/97 GG 1.0 Prima stesura.
* 10/11/97 GG 1.1 Aggiunta la definizione della callback degli errori.
* 12/11/97 GG 1.2 Aggiunta la funzione "cb_setarg".
* 28/07/00 GG 1.3 Aggiunti eventi asincroni per cancellazione errori
*             e gestione messaggi.
*/


#ifndef _CBACK_H

/*
* Tipo descrittore di callback.
*/

typedef struct {
	void (*f)(void *, void *);
	void *arg;
} cback_t;

/* Lista dei punti di callback possibili. */

enum {
/* Chiamata dal supervisore alla ricezione di un comando SV_COMM_CALLBACK. */
	QPLC_CB_SUPERV,
/* Callback attivata in caso di errori da programma. */
	QPLC_CB_PLCERR,
/* Chiamata dall'esecutore alla ricezione del comando SV_COMM_CBEXEC. */
	QPLC_CB_EXECUTOR,
/* Chiamata dall'esecutore al momento di registrare il trace. */
	QPLC_CB_TRACE,
/* Chiamata dal supervisore su richiesta di notifica di evento asincrono
 per i debugger da parte dell'esecutore. */
	QPLC_CB_EVNOTIFY,
/* Callback attivata in caso di  cancellazione di errori da programma. */
	QPLC_CB_PLCDELERR,
/* Callback attivata in caso di emissione o cancellazione di
 messaggi da programma. */
	QPLC_CB_PLCMSG,

/* L'ultimo codice noto. */
	MAX_CBACK,
};

/* Prototipi. */

/*
* Chiamata della callback di descrittore "cb".
*/

void cb_call_cb(register cback_t * cb, void *cd);

/*
* Chiamata della callback di indice "fn".
*/

void cb_call(register int fn, void *cd);

/*
* Assegnamento di valori al descrittore "cb".
*/

void cb_set_cb(register cback_t *cb, void (*f)(void *,void *), void *arg);

/*
* Impostazione della callback di indice "fn". Se "old_cb" non e` NULL,
* copia il precedente valore in *old_cb;
*/

void cb_set(int fn, void (*f)(void *,void *), void *arg, cback_t *old_cb);

/*
* Impostazione del solo parametro della callback di indice "fn".
* Se "old_arg" non e` NULL, copia il precedente valore in *old_arg;
*/

void cb_setarg(int fn, void *arg, void **old_arg);

#define _CBACK_H

#endif

