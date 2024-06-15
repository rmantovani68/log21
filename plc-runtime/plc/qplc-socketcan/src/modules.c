/*
* @(#) modules.c 1.7 Fri Jul 28 17:00:24 MET DST 2000
*
* Creazione dei moduli caricabili dinamicamente.
*
* 20/05/96 GG 1.0 Prima stesura
* 30/08/96 GG 1.1 Aggiunto il metodo "esci" al modulo "hatsh".
* 05/09/96 GG 1.2 Risolto un problema nella visualizzazione delle versioni.
* 29/11/96 GG 1.3 Aggiunti campi per la versione e la data a livello IL.
* 08/01/97 GG 1.4 Aggiunto il simbolo "pCpaxShm", per compatibilita` col
*             Plc BiEsse.
* 09/01/97 GG 1.5 Aggiunto il simbolo opzionale "PlcInit" nel modulo 480.
*             E` una funzione lanciata ad ogni start, utilizzta tipicamente
*             per la costruzione dinamica della tabella MSD in stile 480.
*             E` stato aggiunto il flag "DY_PROP_USE_DEFAULT" a parecchi
*             simboli opzionali.
* 28/02/97 GG 1.6 E` stato aggiunto un riferimento alla funzione (tipicamente
*             vuota) "PlcInit", senza il quale non funzionava il programma 480
*             nella versione statica.
* 28/07/00 GG 1.7 Aggiustamenti per compilazione su Linux.
*/

#include <stdio.h>

#ifdef Linux
#include <sys/stat.h>
#endif

#include "qplc.h"

#include "util.h"

#include "dynaload.h"

/* 
* Lista dei punti di aggancio del modulo "hat_sh" (Rif. Lelli)
* (funzioni "CnInit", "hat", "shoes", "hshEnable", variabili "cpaxShm"
* e "iHShOk", tutti obbligatori). 
*/

extern unsigned int CnInit(void);
extern void hat(void);
extern void shoes(int);
extern int hshEnable(void);
extern Sh_Address *cpaxShm;
Sh_Address **pCpaxShm = &cpaxShm;
extern int iHShOk;
extern char ver_hatsh[];
extern void esci(void);

static mod_hatsh_t _mod_hatsh_op = {
	CnInit,
	hat,
	shoes,
	hshEnable,
	&cpaxShm,
	&iHShOk,
	ver_hatsh,
	esci,
};

static hook_t mod_hatsh_hooks[] = {
	{ "CnInit",    &main_status.mod_hatsh_op.cninit,  (void *)CnInit,    0 },
	{ "hat",       &main_status.mod_hatsh_op.hat,     (void *)hat,       0 },
	{ "shoes",     &main_status.mod_hatsh_op.shoes,   (void *)shoes,     0 },
	{ "hshEnable", &main_status.mod_hatsh_op.enable,  (void *)hshEnable, 0 },
	{ "cpaxShm",   &main_status.mod_hatsh_op.cpax,    &cpaxShm,          0 },
/* Un doppione, per compatibilita`. */
	{ "cpaxShm",   &pCpaxShm,                         &cpaxShm,          0 },
	{ "iHShOk",    &main_status.mod_hatsh_op.ok,      &iHShOk,           0 },
	{ "ver_hatsh", &main_status.mod_hatsh_op.version, ver_hatsh,         0 },
	{ "esci",      &main_status.mod_hatsh_op.exit,    esci,              DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT },
};

/* Lista dei punti di aggancio del modulo "user defined" (livello base)
 (funzione "UserMain", obbligatoria, e funzioni "UserStart" ed "UserStop"
 opzionali). */

extern int UserStart(int *);
extern int UserMain(int);
extern void UserStop(void);
extern char ver_user[];

static mod_user_t _mod_user_op = {
	UserStart,
	UserMain,
	UserStop,
	ver_user,
	(char **)0,
	(unsigned long *)0,
};

static hook_t mod_user_hooks[] = {
	{ "UserMain",  &main_status.mod_user_op.main,      (void *)UserMain,   0 },
	{ "UserStart", &main_status.mod_user_op.start,     (void *)UserStart,  DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT },
	{ "UserStop",  &main_status.mod_user_op.stop,      (void *)UserStop,   DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT },
	{ "ver_user",  &main_status.mod_user_op.version,   ver_user,           0 },
	{ "_VERSION",  &main_status.mod_user_op.ver_il,    (void *)0,          DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT },
	{ "_CREATED",  &main_status.mod_user_op.timestamp, (void *)0,          DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT },
};

/* Lista dei punti di aggancio del modulo "user defined" (livello interfaccia
 RT480) (funzione "PlcMain", obbligatoria, tabelle di associazione
 di I/O opzionali, e versione obbligatoria). */

extern char ver_prog[];
extern void PlcMain(void);
extern void PlcInit(void);
extern unsigned int MSD_input[];
extern unsigned int MSD_output[];

static mod_plc480_t _mod_plc480_op = {
	PlcMain,
	MSD_input,
	MSD_output,
	PlcInit,
	ver_prog,
};

static hook_t mod_plc480_hooks[] = {
	{ "PlcMain",    &main_status.mod_plc480_op.main,    (void *)PlcMain, 0 },
	{ "MSD_input",  &main_status.mod_plc480_op.msdin,   MSD_input,       DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT  },
	{ "MSD_output", &main_status.mod_plc480_op.msdout,  MSD_output,      DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT  },
	{ "PlcInit",    &main_status.mod_plc480_op.init,    PlcInit,         DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT  },
	{ "ver_prog",   &main_status.mod_plc480_op.version, ver_prog,        DY_PROP_OPTIONAL | DY_PROP_USE_DEFAULT  },
};

/*
* Creazione dei moduli.
*/

void create_modules(void)
{
	main_status.mod_hatsh_op = _mod_hatsh_op;
	main_status.mod_hatsh = dyDefineModule(OLD_MOD_STR_HATSH, mod_hatsh_hooks, sizeof(mod_hatsh_hooks)/sizeof(mod_hatsh_hooks[0]));
	dyInitDefault(main_status.mod_hatsh);

	main_status.mod_user_op = _mod_user_op;
	main_status.mod_user = dyDefineModule(OLD_MOD_STR_USER, mod_user_hooks, sizeof(mod_user_hooks)/sizeof(mod_user_hooks[0]));
	dyInitDefault(main_status.mod_user);

	main_status.mod_plc480_op = _mod_plc480_op;
	main_status.mod_plc480 = dyDefineModule(OLD_MOD_STR_480, mod_plc480_hooks, sizeof(mod_plc480_hooks)/sizeof(mod_plc480_hooks[0]));
	dyInitDefault(main_status.mod_plc480);
}

/*
* Caricamento dinamico dei moduli "vecchio stile".
*/

struct _ldinit_t {
	char *path;
	char **version;
	char ***pversion2;
	char flag;
	char index;
};
	
void load_modules(void)
{
	struct stat st;
	char *p,**q;
	struct _ldinit_t in[3], ii;
	int i;
	module_t *mod;
	char **dummy = (char **) 0;

	/* 
	* Riempie una tabella con i dati "per modulo" utili. Per poter fare
	* tutto in un ciclo. 
	*/

	in[0].path = main_status.mod_hatsh_path;
	in[1].path = main_status.mod_user_path;
	in[2].path = main_status.mod_prog480_path;

	in[0].flag = main_status.no_mod_hatsh;
	in[1].flag = main_status.no_mod_user;
	in[2].flag = main_status.no_mod_prog480;

	in[0].version = &main_status.mod_hatsh_op.version;
	in[1].version = &main_status.mod_user_op.version;
	in[2].version = &main_status.mod_plc480_op.version;

	in[0].pversion2 = &dummy; /* Truccaccio */
	in[1].pversion2 = &main_status.mod_user_op.ver_il;
	in[2].pversion2 = &dummy;

	in[0].index = OLD_MOD_IDX_HATSH;
	in[1].index = OLD_MOD_IDX_USER;
	in[2].index = OLD_MOD_IDX_480;

	/* Tenta il caricamento dinamico dei moduli noti. */

	for (i = 0; i < sizeof(in)/sizeof(in[0]); ++i) {

		ii = in[i];

		if (stat(ii.path,&st) == 0 && !ii.flag) {
			p = *ii.version;
			q = *ii.pversion2;
			mod = dyLoad(ii.path, main_status.exe_path, MODULE_LABEL);
			util_show_diff_version((q && *q) ? *q : p, (*ii.pversion2 && **ii.pversion2) ? **ii.pversion2 : *ii.version, ii.index);
		}
	}
}

