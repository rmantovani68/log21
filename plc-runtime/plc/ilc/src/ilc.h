/*
* ilc.h 1.5 Wed Jan 24 20:32:05 MET 2001
* @(#) Rif. Guerrini
*
* 03/07/96 1.0 Prima stesura.
* 16/07/96 1.1 Aggiunta la definizione del tipo "file_action_t".
* 28/11/96 GG 1.2 Aggiunto il flag "gdb".
* 03/12/96 GG 1.3 Aggiunto il flag "gil". "gdb" diventa "ggdb".
* 20/01/97 GG 1.4 Aggiunte le opzioni "blinfo" ed "elinfo", per emettere
*             selettivamente informazioni di inizio e fine istruzione.
*             Corretta la dimensione del flag "bound_check", (era char,
*             doveva essere int).
* 24/01/01 GG 1.5 Aggiunto il flag "optsr", per generare codice ottimizzato
*             (ma non debuggabile) sulle istruzioni SET e RES.
*/

#ifndef _ILC_H_

/* Formati del file d'uscita. */

enum { OF_NULL, OF_C, OF_ASM, OF_HEADER, OF_OBJ, OF_EXE };

/* Struttura di stato globale del programma. */

typedef struct {
	char exe_path[MAXPATHLEN + 1];
	char *exe_base;
	char *asmfile;
	char *deffile;
	char *objfile;
	char *exefile;
	char *cfile;
	char *version;
	char *ext[OF_EXE+2];
	int debug;
	int warning;
	char err_sigla[10];
	char stand_alone;
	char quiet;
	char verify;
	char gencode;
	char il_lang;
	char renflag;
	char objformat;
	char verbose;
	char genexe;
	char standard;
	char gil;
	char ggdb;
	char optsr;
	char ignore_err;
	int bound_check;
	int begin_lineinfo;
	int end_lineinfo;
	filescope_t * scope_list;
	filescope_t * scope_list_tail;
} ilc_statics;

extern ilc_statics main_status;

/*
* Struttura descrivente le azioni da compiere su di un file
* specificato nella linea di comando.
*/

typedef struct _file_action_t {
	char source[MAXPATHLEN + 1];
	char object[MAXPATHLEN + 1];
	char il_lang;
	char gencode;
	char objformat;
	struct _file_action_t * next;
} file_action_t;

#define _ILC_H_

#endif

