/*
* @(#) modules.h 1.5 Fri Jul 28 17:00:24 MET DST 2000
*
* Creazione dei moduli caricati dinamicamente.
*
* 20/05/96 GG 1.0 Prima stesura
* 30/08/96 GG 1.1 Aggiunto il metodo "esci" al modulo "hatsh".
* 29/11/96 GG 1.2 Aggiunti campi per la versione e la data a livello IL.
* 19/12/96 GG 1.3 Modifichine per le versioni (ancora !).
* 09/01/97 GG 1.4 Aggiunto il simbolo opzionale "PlcInit" nel modulo 480.
*             E` una funzione lanciata ad ogni start, utilizzta tipicamente
*             per la costruzione dinamica della tabella MSD in stile 480.
* 28/07/00 GG 1.5 Aggiustamenti per compilazione su Linux.
*/

#ifndef _MOD_USER_H_

#define _MOD_USER_H_

#ifdef Linux
#ifndef SAX_VERS
#define SAX_VERS
typedef void * Sh_Address;
#endif
#endif

#define OLD_MOD_STR_HATSH "HATSH_CODE"
#define OLD_MOD_STR_USER "U_CODE"
#define OLD_MOD_STR_480 "C_CODE"

/*
* Tipi descriventi le operazioni (e anche le variabili) importate dai
* moduli caricati dinamicamente.
*/

typedef struct {
	unsigned int (*cninit)(void);
	void (*hat)(void);
	void (*shoes)(int);
	int (*enable)(void);
	Sh_Address **cpax;
	int *ok;
	char *version;
	void (*exit)(void);
} mod_hatsh_t;

typedef struct {
	int (*start)(int *);
	int (*main)(int);
	void (*stop)(void);
	char *version;
	char **ver_il;
	unsigned long *timestamp;
} mod_user_t;

typedef struct {
	void (*main)(void);
	unsigned int *msdin;
	unsigned int *msdout;
	void (*init)(void);
	char *version;
} mod_plc480_t;

/*
* Creazione dei moduli.
*/

void create_modules(void);

/*
* Caricamento dinamico dei moduli "veccio stile".
*/

void load_modules(void);

#endif

