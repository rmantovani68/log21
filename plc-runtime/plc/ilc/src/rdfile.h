/*
* rdfile.h 1.1 Fri Oct 11 17:48:44 MET 1996
*
*  Questo file contiene variabili e funzioni di accesso ai file.
*
* 10/07/96 1.0 Prima stesura.
* 11/10/96 GG 1.1 Aggiunta la gestione delle stringhe.
*/

#ifndef _RDFILE_H_

/* Tipo associato allo stato del lettore. */

typedef long RDSTS, PTR_ LPRDSTS;

/* Struttura associata allo stato del lettore e dello scanner (manipolata
 dalle primitive di richiesta stato e salvataggio dello scanner). */

typedef struct rssts_t {

	/* Stato dello scanner. */
	SCNSTS s;

	/* Stato del lettore. */
	RDSTS r;

} RSSTS, PTR_ LPRSSTS;

/* Struttura di stato del gestore di file. */

typedef struct {
/* Buffer di lettura per lo scanner. */
	char line[MAXLINE+1];

/* Nome del file sorgente. */
	char *file_name;

/* Stream di lettura. */
	FILE * file_handle;

/* Posizione dell'ultima linea letta. */
	RDSTS last_line_pos;

/* Flag di EOF, ad uso della "readline". Serve solo se l'input e`
 un terminale. */
	int read_eof;

/* Area di lettura di una parola chiave o un identificatore
 (ad uso del parser). */
	char identifier[MAXIDLEN+1];

/* Area di lettura di una stringa numerica, eventualmente float
 (ad uso del parser). */
	char number[MAXNUMLEN*3+5];

/* Area di lettura di una costante stringa. */
	char string[MAXSTRINGLEN+1];

/* Area contenente la lunghezza della costante stringa. */
	int lenstring;

/* Flag di errore. */
	int error;
} rdf_t;

extern rdf_t rdf;

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

void rdf_init(void);

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

int rdf_open(char *name);

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

void rdf_close(void);

#define _RDFILE_H_

#endif

