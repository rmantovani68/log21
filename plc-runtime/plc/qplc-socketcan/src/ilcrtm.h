/*
* @(#) ilcrtm.h 1.2 Wed Jan 29 15:36:50 MET 1997
*
*  Definizioni per il supporto run-time del compilatore ILC.
*
* 15/07/96 GG 1.0 Prima stesura.
* 26/11/96 GG 1.1 Aggiunta la sottosezione "estensioni" nel campo dati.
* 29/01/97 GG 1.2 Eliminato il riferimento a "/u/beppe..."
*/

#ifndef _ILCRTM_H_

#include "msd.h"

/*
* Struttura descrivente un descrittore MSD.
*
* Il campo "data" si interpreta come segue :
*
* Per associazioni di I/O, contiene la stringa descrivente il segnale,
* per segnali di scambio, contiene quattro byte di indice, quattro byte
* per il puntatore alla variabile chiave associata, ed il nome del
* segnale di scambio.
*/

#pragma pack(1)
typedef struct {
/* Variabile da associare o indirizzo della chiave variabile. */
	void * target;
	unsigned char flags;
	unsigned char data_size;
	union {
		unsigned char edata[1];
		char io[1];
		struct {
			int dim1;
			int dim2;
			char name[1];
		} shv;
		unsigned char cv;
		unsigned short hv;
		unsigned long lv;
		float fv;
		double dv;
	} data; /* E` un trucco ! In realta`, questo campo assomiglia piu` */
	        /* ad una stringa null-terminated (piu` o meno). */
} il_msd_t;
#pragma pack()

/*
* Struttura descrivente l'istanza di un programma.
*/

typedef struct _il_program_t {
/* Puntatore all'area statica specifica dell'istanza del programma. */
	void *statics;
/* Puntatore alla funzione principale del programma. */
	void (*program)(void * statics);
/* Configurazione specifica del programma. Un "NULL" nel campo "target"
 indica la fine della lista. */
	il_msd_t msd[1];
} il_program_t;

/*
* Struttura descrivente una risorsa.
*/

typedef struct _il_resource_t {
/* Nome della risorsa. */
	char *name;
/* Nome della CPU. */
	char *on;
/* Lista delle istanze di programmi. */
	il_program_t **prog;
} il_resource_t;

/*
* Struttura descrivente una configurazione.
*/

typedef struct _il_config_t {
/* Nome della configurazione. */
	char *name;
/* Puntatore alla lista delle risorse. */
	il_resource_t **resource;
} il_config_t;

/*
* Struttura descrivente un elemento della catena delle configurazioni.
*/

typedef struct _il_config_chain_t {
/* Puntatore al prossimo elemento. */
	struct _il_config_chain_t *next;
/* Lista MSD indotta dall'oggetto corrente.  Un "NULL" nel campo "target"
 indica la fine della lista. */
	il_msd_t *msd;
/* Lista delle configurazioni presenti nell'oggetto (null terminated). */
	il_config_t *conf[1];
} il_config_chain_t;

/*
* Struttura principale, l'unica che meriti visibilita` globale.
* E` questo il simbolo che il caricatore dinamico cerca.
*/

typedef struct {
/* Puntatore all'inizio della catena delle configurazioni. */
	il_config_chain_t *chain;
/* "Parolina magica". */
	char magic[1];
} il_module_t;

/* Simbolo globale, definito in "ilc0.s". */

extern il_module_t il_module;

/* Funzioni esportate da "ilcrtm.c". */
void _il_ex_rtmerr_subscript(void);
void _il_rtmerr_subscript(void);
int IL_UserStart(il_module_t *mod, int * code);
int IL_UserMain(int start);
void IL_UserStop(void);

#define _ILCRTM_H_

#endif

