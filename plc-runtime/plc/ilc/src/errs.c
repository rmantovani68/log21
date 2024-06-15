/*
* errs.c 1.0 Mon Jul 15 13:13:35 MET DST 1996
*
* Gestione degli errori.
*
* 15/07/96 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "conf.h"
#include "sysdep.h"
#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "util.h"
#include "ilc.h"
#include "pass1.h"
#include "cexpr.h"
#include "debug.h"
#include "rdfile.h"
#include "errs.h"

#include "errlist.c"

/*
* Funzione err_error_and_warning
* ------------------------------
*
* Parametri :
*
*	n : codice d'errore.
*	p : evantuale messaggio aggiuntivo.
*	s : "error" o "warning".
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Visualizza l'errore.
*/

static void err_error_and_warning(int n,char *p, char *s)
{
	fprintf(stderr,"%s:%d %s %d : %s",
	        rdf.file_name,
	        pScCtl -> n_line,
	        s,
	        n,
	        err_list[n]);
	if (p)
		fprintf(stderr," %s",p);
	fprintf(stderr,"\n");
	if (main_status.verbose) {
		fprintf(stderr,"--> \"");
		for (p = pScCtl->line; *p && *p != '\n'; ++p)
			fprintf(stderr,"%c", (*p < ' ') ? ' ' : *p);
		fprintf(stderr,"\"\n....");
		for (p = pScCtl->line; p < pScCtl->curr; ++p)
			fprintf(stderr,".");
		fprintf(stderr,"^^^\n");
	}
}

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

void err_error(int n,char *p)
{
	err_error_and_warning(n,p,"error");
	rdf.error = 1;
}

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

void err_warning(int n,char *p)
{
	if (! main_status.warning)
		return;
	err_error_and_warning(n,p,"warning");
}

