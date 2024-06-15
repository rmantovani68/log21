
/*
* @(#) main.c 1.6 Wed Jan 24 20:35:53 MET 2001
* @(#) Rif. Guerrini
*
* 03/07/96 1.0 Prima stesura (in continua evoluzione).
* 15/10/96 GG 1.1 Modificato il default per l'opzione "-bound-chk".
* 28/11/96 GG 1.2 Aggiunto il flag "gdb".
* 03/12/96 GG 1.3 Aggiunto il flag "gil". "gdb" diventa "ggdb".
* 20/01/97 GG 1.4 Aggiunte le opzioni "blinfo" ed "elinfo", per emettere
*             selettivamente informazioni di inizio e fine istruzione.
* 22/09/97 GG 1.5 Un piccolo aggiustamento per compilazione su NT.
* 24/01/01 GG 1.6 Modificata la generazione di codice di SET e RES,
*             in modo che l'indirizzo dell'operando sia calcolato sempre,
*             cioe` anche quando l'operando non deve essere alterato.
*             La vecchia modalita` di funzionamento e` ancora ottenlibile
*             con la nuova opzione da linea di comando "-optsr".
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "errs.h"
#include "pass2.h"

extern char Versione[];

/*
* Struttura di stato del programma (stato generale : file eseguibile,
* opzioni di lancio...).
*/

ilc_statics main_status;

/*
* Struttura descrivente un'opzione della linea di comando.
*
* Pero ogni opzione riconosciuta, sono dati
* - la stringa corrispondente,
* - la stringa da visualizzare nell'help,
* - il puntatore alla funzione di gestione,
* - due parametri generici per la funzione.
*/

struct option_t {
	const char * string;
	const char * help;
	int (*handler)(char **argv, void * param1, void *param2);
	void * param1;
	void * param2;
};

/* Gestori per opzioni. */
/* Gestore per opzioni a flag (attivatori). Il parametro e`
 il puntatore al flag (char) da porre ad 1. */
static int opt_setflag(char **argv, void *param1, void *param2);
static int opt_resflag(char **argv, void *param1, void *param2);
/* Gestore di opzioni a stringa. Il parametro e` un puntatore
 a carattere, destinazione della stringa. */
static int opt_setstring(char **argv, void *param1, void *param2);
/* Gestore di opzioni a intero. */
static int opt_setint(char **argv, void *param1, void *param2);

static const struct option_t options[] = 
{
{ "-bound-chk","n  0 : Disabilita il controllo sugli indici dei vettori.\n"
   "               1 : Controllo rapido, perde l'indirizzo di errore.\n"
   "               2 : Controllo lento, conserva l'indirizzo di errore (default)",
  opt_setint,
  (void *)(&main_status.bound_check),
  (void *) 0,
},
{ "-blinfo","{0,1} Emette informazioni di inizio istruzione (default=0)",
  opt_setint,
  (void *)(&main_status.begin_lineinfo),
  (void *) 0,
},
{ "-code","        I file che seguono produrranno codice (default)",
  opt_setflag,
  (void *)(&main_status.gencode),
  (void *) 0,
},
{ "-debug","n      Imposta il livello di debug",
  opt_setint,
  (void *)(&main_status.debug),
  (void *) 0,
},
{ "-elinfo","{0,1} Emette informazioni di fine istruzione (default=1)",
  opt_setint,
  (void *)(&main_status.end_lineinfo),
  (void *) 0,
},
{ "-extended","    Abilita le estensioni CNi (default)",
  opt_resflag,
  (void *)(&main_status.standard),
  (void *) 0,
},
{ "-g","           Genera informazioni per il debugger IL",
  opt_setflag,
  (void *)(&main_status.gil),
  (void *) 0,
},
{ "-ggdb","        Genera informazioni di debug comprensibili da GDB",
  opt_setflag,
  (void *)(&main_status.ggdb),
  (void *) 0,
},
{ "-header","      I file che seguono non produrranno codice",
  opt_resflag,
  (void *)(&main_status.gencode),
  (void *) 0,
},
{ "-il","          Applica la sintassi IL per i file successivi (default)",
  opt_setflag,
  (void *)(&main_status.il_lang),
  (void *) 0,
},
{ "-ignore-err","  Mantiene il livello di errore a 0 indipendentemente dal\n"
   "               successo della compilazione",
  opt_setflag,
  (void *)(&main_status.ignore_err),
  (void *) 0,
},
{ "-oc","file      Produce il sorgente C specificato",
  opt_setstring,
  (void *)(&main_status.cfile),
  (void *)(&main_status.renflag),
},
{ "-oh","file      Produce il file di prototipi specificato",
  opt_setstring,
  (void *)(&main_status.deffile),
  (void *)(&main_status.renflag),
},
{ "-os","file      Produce il file assembler specificato",
  opt_setstring,
  (void *)(&main_status.asmfile),
  (void *)(&main_status.renflag),
},
{ "-oo","file      Produce il file oggetto specificato",
  opt_setstring,
  (void *)(&main_status.objfile),
  (void *)(&main_status.renflag),
},
{ "-optsr","       Genera codice ottimizzato per le istruzioni SET e RES",
  opt_setflag,
  (void *)(&main_status.optsr),
  (void *) 0,
},
{ "-o","file       Produce il file eseguibile specificato",
  opt_setstring,
  (void *)(&main_status.exefile),
  (void *)(&main_status.genexe),
},
{ "-quiet","       Emette solo gli errori in forma concisa (default)",
  opt_resflag,
  (void *)(&main_status.verbose),
  (void *) 0,
},
{ "-st","          Applica la sintassi ST ai i file successivi (sperimentale)",
  opt_resflag,
  (void *)(&main_status.il_lang),
  (void *) 0,
},
{ "-standard","    Disabilita le estensioni CNi",
  opt_setflag,
  (void *)(&main_status.standard),
  (void *) 0,
},
{ "-v","           Visualizza la versione ed esce",
  opt_setflag,
  (void *)(&main_status.verify),
  (void *) 0,
},
{ "-verbose","     Visualizza la linea che ha prodotto l'errore",
  opt_setflag,
  (void *)(&main_status.verbose),
  (void *) 0,
},
{ "-w","n          Imposta il livello di warning",
  opt_setint,
  (void *)(&main_status.warning),
  (void *) 0,
},
};

static file_action_t *worklist = (file_action_t *) 0;
static file_action_t *worklist_end = (file_action_t *) 0;

/*
* Formattazione del nome del programma.
*
* Questa funzione ricava il path completo ed il nome base del file
* eseguibile "qplc".
*/

static void format_exe_name(char * argv0)
{
char *p;

#ifdef WIN32
	main_status.exe_base = "ilc win32";
#else

	if (argv0[0] != '/') {
		getwd(main_status.exe_path);
		strcat(main_status.exe_path,"/");
		strcat(main_status.exe_path,argv0);
		main_status.exe_base = strrchr(main_status.exe_path, '/') + 1;
	}
	else {
		strcpy(main_status.exe_path, argv0);
		p = strrchr(argv0, '/');
		p = p ? p + 1 : argv0;
		main_status.exe_base = p;
	}
#endif
}

/*
* Imposta una ragionevole configurazione di default.
* E` bene ricordarsi di aggiungere qui un'inizializzazione
* per ogni nuovo campo definito nella struttura "qplc_statics".
*/

static void load_default_conf(void)
{
	memset((char *)&main_status, 0, sizeof(main_status));

	strcpy(main_status.exe_path,"ilc");
	main_status.exe_base = main_status.exe_path;
	strcpy(main_status.err_sigla,"PLC");
	main_status.stand_alone = 0;
	main_status.verify = 0;
	main_status.standard = 0;
	main_status.gil = 0;
	main_status.ggdb = 0;
	main_status.optsr = 0;
	main_status.begin_lineinfo = 0;
	main_status.end_lineinfo = 1;
	main_status.debug = 0;
	main_status.warning = 1;
	main_status.version = Versione + 5; /* Salta "@(.) " */
	main_status.gencode = 1;
	main_status.il_lang = 1;
	main_status.renflag = 0;
	main_status.genexe = 0;
	main_status.verbose = 0;
	main_status.bound_check = 2;
	main_status.ignore_err = 0;
	main_status.objformat = OF_ASM;
	main_status.cfile = (char *) 0;
	main_status.asmfile = (char *) 0;
	main_status.deffile = (char *) 0;
	main_status.objfile = (char *) 0;
 	main_status.exefile = (char *) 0;
	main_status.ext[OF_NULL] = "";
	main_status.ext[OF_C] = ".c";
	main_status.ext[OF_ASM] = ".s";
	main_status.ext[OF_HEADER] = ".ilh";
	main_status.ext[OF_OBJ] = ".o";
	main_status.ext[OF_EXE] = ".oo";
	main_status.scope_list = (filescope_t *) 0;
	main_status.scope_list_tail = (filescope_t *) 0;
}

/*
* Lettura del file di configurazione.
*/

static void read_config_file(void)
{
/* Non c'e` fretta... */
}

/*
* Visualizza l'help ed esce.
*/

static void usagerr(void)
{
int i;

	fprintf(stderr, "%s : uso :\n%s\n",
		main_status.exe_base,
		main_status.exe_base );
	for (i = 0; i < sizeof(options)/sizeof(options[0]); ++i)
		fprintf(stderr," %s %s.\n",options[i].string,options[i].help);
	fprintf(stderr,"\n");
	exit(1);
}

/*
* Analisi della linea di comando.
* -1 in caso di errore.
*/

/* Gestore per opzioni a flag (attivatori). Il parametro e`
 il puntatore al flag (char) da porre ad 1. */

static int opt_setflag(char **argv, void * param1, void * param2)
{
	if (param1)
		*(char *)param1 = 1;
	if (param2)
		*(char *)param2 = 1;
	return 0;
}
static int opt_resflag(char **argv, void * param1, void * param2)
{
	if (param1)
		*(char *)param1 = 0;
	if (param2)
		*(char *)param2 = 0;
	return 0;
}

/* Gestore di opzioni a stringa. Il parametro e` un puntatore
 a carattere, destinazione della stringa. */

static int opt_setstring(char **argv, void * param1, void * param2)
{
	if (! argv[1])
		return -1;
	if (param1)
		*(char **) param1 = argv[1];
	if (param2)
		*(char *) param2 = 1;
	return 1;
}

/* Gestore di opzioni a interi. Il parametro e` un puntatore
 a intero, destinazione del valore. */

static int opt_setint(char **argv, void * param1, void * param2)
{
int v;
char *p;

	if (! argv[1])
		return -1;
	for (v = 0, p = argv[1]; *p; ++p) {
		if (*p < '0' || *p > '9')
			return -1;
		v = v * 10 + *p - '0';
	}
	*(int *)param1 = v;
	return 1;
}

/* Analizzatore della linea di comando. */

static int scan_command_line(int argc, char **argv)
{
int i,j,rv,ok;
file_action_t *p;
char *q;

	for (i = 1; i < argc; ++i) {
		ok = 0;
		for (j = 0; j < sizeof(options)/sizeof(options[0]); ++j) {
			if (strcmp(options[j].string,argv[i]) == 0) {
				rv = options[j].handler(&argv[i],
				                        options[j].param1,
				                        options[j].param2);
				if (rv >= 0) {
					ok = 1;
					i += rv;
				}
				break;
			}
		}
		if (! ok) {
			if (argv[i][0] == '-')
				return -1;
			p = (file_action_t*)util_malloc(sizeof(file_action_t));
			if (! p)
				return -1;
			strcpy(p -> source,argv[i]);
			strcpy(p -> object,argv[i]);
			q = strrchr(p -> object,'.');
			if (q)
				strcpy(q,
				   main_status.ext[(int)(main_status.objformat)]);
			else
				strcat(p -> object,
				   main_status.ext[(int)(main_status.objformat)]);
			p -> gencode = main_status.gencode;
			p -> objformat = main_status.objformat;
			p -> il_lang = main_status.il_lang;
			p -> next = (file_action_t *) 0;
			if (worklist_end)
				worklist_end -> next = p;
			else
				worklist = p;
			worklist_end = p;
		}
		else if (main_status.renflag) {
			p = worklist_end;
			if (! p)
				return -1;
			main_status.renflag = 0;
			if ( (q = main_status.deffile) ) {
				p -> objformat = OF_HEADER;
			}
			else if ( (q = main_status.asmfile) ) {
				p -> objformat = OF_ASM;
			}
			else if ( (q = main_status.objfile) ) {
				p -> objformat = OF_OBJ;
			}
			strcpy(p -> object, q);
			main_status.deffile
			 = main_status.asmfile
			 = main_status.asmfile
			 = (char *) 0;
		}
	}

	return 0;
}
 

/********/
/* MAIN */
/********/

void main(int argc, char **argv)
{
int rv;
file_action_t *p;

/* Carica la configurazione di default. */
	load_default_conf();
/* Ricava il nome dell'eseguibile. */
	format_exe_name(argv[0]);
/* Legge la configurazione. */
	read_config_file();
/* Analizza la linea di comando. Se errore, mostra l'help ed esce. */
	if (scan_command_line(argc,argv) < 0) {
		usagerr();
	}
/* Esce se si voleva solo leggere la versione. */
	if (main_status.verify) {
		printf("%s version %s\n",
		       main_status.exe_base, main_status.version);
		exit(0);
	}
	if (! worklist) {
		usagerr();
	}

/* ... e adesso, si fa sul serio. */

/* Passo 1 : Costruzione della symbol table e calcolo dei riferimenti. */

	p1_init();

	rv = 1;

/* Applica il parser ad ogni elemento della lista di lavoro, e
 costruisce una catena di contesti rappresentante la sequenza
 dei sorgenti incontrati. */

	for (p = worklist; p; p = p -> next) {

#ifdef DEBUG
		if (main_status.debug & 0x01) {
			printf("\nSource file = \"%s\"\n",p -> source);
			printf("     object = \"%s\"\n",p -> object);
			printf("    gencode = %d\n",p -> gencode);
			printf("  objformat = %d\n",p -> objformat);
			printf("    il_lang = %d\n\n",p -> il_lang);
		}
#endif

		main_status.objformat = p -> objformat;
		main_status.gencode   = p -> gencode  ;
		main_status.il_lang   = p -> il_lang  ;

		rv = p1_start(p) && rv;

	}

#ifdef DEBUG
	db_dump_context_chain();
#endif

/* Passo 2. Generazione degli oggetti. */

	p2_init();

	for (p = worklist; p && rv; p = p -> next) {

#ifdef DEBUG
		if (main_status.debug & 0x01) {
			printf("\nSource file = \"%s\"\n",p -> source);
			printf("     object = \"%s\"\n",p -> object);
			printf("    gencode = %d\n",p -> gencode);
			printf("  objformat = %d\n",p -> objformat);
			printf("    il_lang = %d\n\n",p -> il_lang);
		}
#endif

		main_status.objformat = p -> objformat;
		main_status.gencode   = p -> gencode  ;
		main_status.il_lang   = p -> il_lang  ;

		rv = p2_start(p) && rv;

	}

	exit(! (rv || main_status.ignore_err));
}

