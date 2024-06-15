/*
* errs.h 1.0 Mon Jul 15 13:13:35 MET DST 1996
*
* Gestione degli errori.
*
* 15/07/96 GG 1.0 Prima stesura.
*/

#ifndef _ERRS_H_

#include "errlist.h"

/*
* Funzione err_error
* -----------------
*
* Parametri :
*
*	n : codice d'errore.
*	p : evantuale messaggio aggiuntivo.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Visualizza l'errore.
*/

void err_error(int n,char *p);

/*
* Funzione err_warning
* --------------------
*
* Parametri :
*
*	n : codice d'errore.
*	p : evantuale messaggio aggiuntivo.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Visualizza l'errore (warning).
*/

void err_warning(int n,char *p);

#define _ERRS_H_

#endif

