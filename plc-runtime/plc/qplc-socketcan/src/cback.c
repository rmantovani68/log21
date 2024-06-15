/*
* cback.c 1.2 Wed Nov 12 09:03:32 MET 1997
*
*  Questo file contiene primitive che realizzano un maccanismo di
* callback per le componenti di QPLC.
*
* 04/11/97 GG 1.0 Prima stesura.
* 10/11/97 GG 1.1 Aggiunto un controllo sul puntatore alla funzione.
* 12/11/97 GG 1.2 Aggiunta la funzione "cb_setarg".
*/


#include "cback.h"

static cback_t cbtab[MAX_CBACK];

/*
* Chiamata della callback di descrittore "cb".
*/

void cb_call_cb(register cback_t * cb, void *cd)
{
	if (cb -> f)
		(*(cb -> f))(cb -> arg,cd);
}

/*
* Chiamata della callback di indice "fn".
*/

void cb_call(register int fn,void *cd)
{
	cb_call_cb(&cbtab[fn],cd);
}

/*
* Assegnamento di valori al descrittore "cb".
*/

void cb_set_cb(register cback_t *cb, void (*f)(void *,void *), void *arg)
{
	cb -> f = f;
	cb -> arg = arg;
}

/*
* Impostazione della callback di indice "fn". Se "old_cb" non e` NULL,
* copia il precedente valore in *old_cb;
*/

void cb_set(int fn, void (*f)(void *,void *), void *arg, cback_t *old_cb)
{
register cback_t *cb;

	cb = &cbtab[fn];
	if (old_cb)
		*old_cb = *cb;
	cb_set_cb(cb, f, arg);
}

/*
* Impostazione del solo parametro della callback di indice "fn".
* Se "old_arg" non e` NULL, copia il precedente valore in *old_arg;
*/

void cb_setarg(int fn, void *arg, void **old_arg)
{
register cback_t *cb;

	cb = &cbtab[fn];
	if (old_arg)
		*old_arg = cb -> arg;
	cb -> arg = arg;
}



