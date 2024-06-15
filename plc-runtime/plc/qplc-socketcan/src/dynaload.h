
/*
* @(#) dynaload.h 2.2 Fri Jul 28 17:00:24 MET DST 2000
* @(#) rif. Guerrini
*
*  Questo modulo contiene le primitive per il caricamento dinamico
* di oggetti da parte del Plc.
*
* 14/04/95 GG : Versione 1.0. E` gestito il solo formato "a.out".
* 26/04/95 GG : Versione 1.1. Si e` separato il nome del modulo da
*               quello dei simboli.
* 19/06/95 GG : Versione 1.2. Aggiunto il campo di proprieta` dei punti
*               di aggancio.
* 18/09/95 GG : Versione 1.3. Condizionato il tutto alla definizione di "Lynx",
*               per unificare senza fatica le compilazioni su LynxOS e Solaris.
* 30/01/96 GG : Versione 1.4. Aggiunti campi al descrittore di modulo,
*               per tenere traccia dei file caricati.
* 02/02/96 GG : Versione 1.5. Aggiunti campi per gli indirizzi delle
*               sezioni "data", "bss" e "common" al descrittore di modulo.
* 17/05/96 GG : Versione 2.0. Trasformate in tipi le strutture caratteristiche.
*               Rimaneggiamenti in vista per passaggio al PLC nuovo.
* 22/10/96 GG 2.1 Aggiunto un puntatore all'immagine dei valori iniziali
*             della sezione dati, e relativa dimensione.
* 28/07/00 GG 2.2 Aggiustamenti per compilazione su Linux.
*/

#if defined(Lynx) || defined(Linux)

#ifdef Linux
#define MAXPATHLEN FILENAME_MAX
#endif

#ifndef _DYNALOAD_H

/* Massima lunghezza di un nome di modulo. */

#define MAX_MODNAME 16

/* Struttura descrivente un "punto d'aggancio". Un modulo e`
 definito dall'insieme non vuoto dei punti d'aggancio che lo
 caratterizzano. Un punto d'aggancio e` una funzione contenuta
 nel modulo e utilizzata da qualche altra parte del sistema. */

typedef struct {
	char * name;		/* Nome del simbolo. */
	void * target;		/* Indirizzo della variabile che contiene il puntatore alla funzione. */
	void * defaultfn;	/* Valore di default per la variabile. */
	int flags;			/* Proprieta` del simbolo (vedi DY_PROP_...). */
} hook_t;

/* Proprieta` di un punto di aggancio. */

/* Se il simbolo non e` definito, non emette un errore. */
#define DY_PROP_OPTIONAL	0x01
/* Se il simbolo e` opzionale e non definito, usa il valore di default (1)
 o quello preesistente (0). */
#define DY_PROP_USE_DEFAULT		0x02

/* Struttura descrivente un modulo. */
 
typedef struct module_t {
	char name[MAX_MODNAME];             /* Nome del modulo. */
	char * base;                        /* Puntatore all'area in cui risiede il modulo. NULL se il modulo non e` caricato. */
	char * text;                        /* Puntatore all'area in cui risiede il codice. NULL se il modulo non e` caricato. */
	char * data;                        /* Puntatore alla sezione "data". */
	char * bss;                         /* Puntatore alla sezione "bss". */
	char * common;                      /* Puntatore alla sezione "common". */
	char * init_data;		            /* Puntatore all'area di backup dei valori iniziali dell'area dati. */
	int data_size;			            /* Dimensione dell'area "init_data". */
	int zero_size;			            /* Dimensione dell'area dati non inizializzata. */
	hook_t * hooks;			            /* Puntatore alla tabella delle funzioni di modulo. L'ultimo elemento deve avere NULL nel campo nome. */
	int nhooks;				            /* Numero di punti d'aggancio. */
	struct module_t * next;	            /* Puntatore al prossimo modulo. */
	char file[MAXPATHLEN+1];            /* Nome del file correntemente caricato ("" se non c'e`). */
	char default_file[MAXPATHLEN+1];	/* Nome del file nella configurazione di default ("" se non c'e`). */
	int loaded;				            /* Flag di "modulo caricato". */
	int size;				            /* Dimensione del modulo. */
	char * default_text;	            /* Puntatore all'area in cui risiede il modulo di default. NULL se il modulo non e` caricato. */
	char * default_data;	            /* Puntatore alla sezione "data". */
	char * default_bss;		            /* Puntatore alla sezione "bss". */
	char * default_common;	            /* Puntatore alla sezione "common". */
	int default_size;		            /* Dimensione del codice di default. */
} module_t;

/* 
* Definizione della struttura utilizzata dalla funzione "dyModProp"
* per comunicare i dati al chiamante :
*
*  - quattro byte per l'indirizzo di base ("text"),
*  - quattro byte per l'indirizzo della sezione "data",
*  - quattro byte per l'indirizzo della sezione "bss",
*  - quattro byte per l'indirizzo della sezione "common",
*  - quattro byte per la dimensione ("size"),
*  - MAXPATHLEN byte per il nome del file ("file").
*/

typedef struct {
	long text;
	long data;
	long bss;
	long common;
	long size;
	char file[MAXPATHLEN + 1];
} modprop_t;

/* Lista dei moduli caricati. Ogni oggetto
 caricato dinamicamente deve contenere tutti i simboli
 indicati in un elemento di questa tabella. Se questo non avviene,
 il modulo non e` puo` essere accettato.
  Un modulo si identifica tramite il primo dei simboli elencati
 nel suo descrittore. */

extern module_t * Modules;

/*
* Funzione dyLoad
* ---------------
*
*  Questa funzione carica per l'esecuzione "objname", file oggetto in formato
* "Lynx a.out (object)", ne corregge le rilocazioni, risolve i riferimenti
* esterni utilizzando le informazioni registrate nella symbol table
* del file "exename" (che deve essere il nome dell'eseguibile da cui e`
* nato il processo chiamante, ricavato tipicamente da argv[0]).
*  Il modulo deve contenere tutti i simboli definiti nel descrittore
* di modulo corrispondente.
*  Il nome del modulo e` la stringa puntata dal valore del simbolo
* indicato dal parametro "label". Se non esiste alcun modulo con
* quel nome, la funzione cerca il modulo anonimo "" e lo rinomina.
*  La funzione riporta il descrittore del modulo, oppure NULL in caso di
* errore.
*/

module_t * dyLoad(char * objname, char * exename, char * label);


/*
* Funzione dyInitData
* -------------------
*
*  Questa funzione ricarica il valore iniziale della parte dati di un modulo
* precedentemente caricato.
*/

void dyInitData(module_t * mod);

/*
* Funzione dyDefineModule
* -----------------------
*
*  Registra un modulo, cioe` aggiunge una nuova definizione alla
* lista dei moduli caricabili. "hooks" e` un vettore di "n"
* punti di aggancio. "name" e` il nome del modulo.
*  Come valore di ritorno da` il puntatore al descrittore di modulo,
* oppure NULL in caso di errore.
*/

module_t * dyDefineModule(char *name, hook_t *hooks,int nhooks);

/*
* Funzione dyAllocHooks
* ---------------------
*
*  Questa funzione alloca una copia dell'insieme di agganci "hooks" di
* dimensione "nhooks". Anche i nomi sono riallocati e copiati. E` utile
* ogni volta che l'originale fa parte di un'area che potrebbe essere
* riciclata, come lo stack o l'area dati di un modulo caricato dinamicamente.
*  Il valore della funzione e` un puntatore alla copia, oppure NULL se
* non c'e` piu` memoria.
*/

hook_t * dyAllocHooks(hook_t * hooks, int nhooks);

/*
* Funzione dyInitDefault
* ----------------------
*
*  Questa funzione inizializza il campo "defaultfn" di ogni aggancio del
* modulo "mod", copiandone il valore dalla variabile associata.
*/

void dyInitDefault(module_t * mod);

/*
* Funzione dyMakeDefault
* ----------------------
*
*  Questa funzione fa si` che il modulo "mod", correntemente caricato,
* figuri "non caricato", e le funzioni contenute diventino il default.
*  L'effetto di questa funzione e` di rendere "permanente" un codice
* caricato dinamicamente.
*  L'area allocata non puo` piu` essere rilasciata, a meno che non si siano
* presi opportuni provvedimenti.
*/

void dyMakeDefault(module_t * mod);

/*
* Funzione dyFindModule
* ---------------------
*
*  Questa funzione trova il modulo di nome "name".
*/

module_t * dyFindModule(char * name);

/*
* Funzione dyUnload
* -----------------
*
*  Questa funzione scarica un modulo, libera l'area associata e riporta
* tutte le variabili associate al valore di default.
*/

void dyUnload(module_t * mod);

/*
* Funzione dyRename
* -----------------
*
*  Questa funzione rinomina un modulo. Il vecchio nome resta in memoria,
* e` responsabilita` dell'utente liberarlo, se necessario.
*  La funzione restituisce il puntatore al vecchio nome. 
*/

void dyRename(module_t * mod, char * name);

/*
* Funzione dyStatus
* -----------------
*
*  Questa funzione copia in "*sts"lo stato del caricatore dinamico.
*  Restituisce il numero di byte copiati.
*/

int dyStatus(int * sts);

/*
* Funzione dyModProp
* ------------------
*
*  Questa funzione copia in "*sts" le proprieta` principali del modulo "prop" :
*  - quattro byte per l'indirizzo di base ("text"),
*  - quattro byte per la dimensione ("size"),
*  - duecento byte per il nome del file ("file").
*  Restituisce il numero di byte copiati.
*/

int dyModProp(module_t * mod, modprop_t * sts);

#define _DYNALOAD_H
#endif /* _DYNALOAD_H */

#endif /* Lynx */

