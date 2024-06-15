/*
* rdfile.c 1.1 Mon Sep 22 10:06:26 MET DST 1997
*
*  Questo file contiene variabili e funzioni di accesso ai file.
*
* 10/07/96 GG 1.0 Prima stesura.
* 22/09/97 GG 1.1 Un piccolo aggiustamento per compilazione su NT.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdarg.h>
#include <math.h>

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

/* Struttura di stato del "livello alto" del lettore. */

rdf_t rdf;

/*
* -----------------------------------------
* PRIMITIVE DI LETTURA AD USO DELLO SCANNER
* -----------------------------------------
*/

/*
* Funzione rdf_readline
* ---------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione legge una linea dallo stream di ingersso nel buffer
* dello scanner. E` utilizzata da "rdf_read" e da "rdf.seek".
*/

static int rdf_readline(void)
{

/* Legge la linea. Restituisce una linea vuota in caso di errore. */

	if (!rdf.read_eof && fgets(pScCtl -> line, pScCtl -> l_line, rdf.file_handle)) {
		return 1;
	}
	else {
		rdf.read_eof = 1;
		pScCtl -> line[0] = '\0';
		return 0;
	}
}

/*
* Funzione rdf_read
* -----------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione legge una linea dallo stream di ingersso per conto
* dello scanner.
*/

static int rdf_read(void)
{

/* Memorizza la posizione della linea PRIMA di leggerla. */

	rdf.last_line_pos = ftell(rdf.file_handle);

/* Legge la linea. Restituisce una linea vuota in caso di errore. */

	return rdf_readline();
}

/*
* Funzione rdf_tell
* -----------------
*
* Parametri :
*
*	void * where : indirizzo a cui scrivere lo stato del lettore.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione comunica allo scanner lo stato del lettore.
*/

static int rdf_tell(LPVOID where)
{
	*((LPRDSTS) where) = rdf.last_line_pos;
	return 1;
}

/*
* Funzione rdf_seek
* -----------------
*
* Parametri :
*
*	void * where : indirizzo dal quale ricavare lo stato del lettore.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione cambia la posizione di lettura per conto dello scanner.
*/

static int rdf_seek(LPVOID where)
{
RDSTS l;

	l = *((LPRDSTS) where);
	if (fseek(rdf.file_handle, l, SEEK_SET) == 0) {
		rdf.read_eof = 0;
		rdf.last_line_pos = l;
		return rdf_readline();
	}
	else
		return 0;
}

/*
* Funzione rdf_init
* -----------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione inizializza il lettore di file (coinvolge anche il parser
* e lo scanner).
*/

void rdf_init(void)
{

/* Inizializza le primitive di lettura. */

	rdf.read_eof = 0;
	rdf.error = 0;

/* Inizializza lo scanner. */

	pScCtl -> readln = rdf_read;
	pScCtl -> line = rdf.line;
	pScCtl -> l_line = MAXLINE;
	pScCtl -> getrdsts = rdf_tell;
	pScCtl -> setrdsts = rdf_seek;
	pScCtl -> l_rdsts = sizeof(RSSTS);
	pScCtl -> maxidlen = MAXIDLEN;
	pScCtl -> maxnumlen = MAXNUMLEN;
	pScCtl -> maxstringlen = MAXSTRINGLEN;

/* Inizializza il parser. */

	/* Aree di lettura per numeri e nomi. */

	pPaCtl -> symstr = rdf.identifier;
	pPaCtl -> numstr = rdf.number;
	pPaCtl -> stringstr = rdf.string;
	pPaCtl -> lenstr = &rdf.lenstring;
}

/*
* Funzione rdf_open
* -----------------
*
* Parametri :
*
*	file : nome del file.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 in caso di errore.
*
* Descrizione :
*
*  Questa funzione apre un file e lo registra come file di lettura
* corrente per lo scanner.
*/

int rdf_open(char *file)
{
	if (file && strcmp(file,"-")) {
		rdf.file_name = util_strdup(file);
		rdf.file_handle = fopen(rdf.file_name,"r");
	}
	else {
		rdf.file_name = "(stdin)";
		rdf.file_handle = stdin;
	}

	return (rdf.file_handle != (FILE *) 0);
}

/*
* Funzione rdf_close
* ------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Questa funzione chiude il file di lettura corrente.
*/

void rdf_close(void)
{
	if (rdf.file_handle && rdf.file_handle != stdin)
		fclose(rdf.file_handle);
	rdf.file_handle = (FILE *) 0;
}

