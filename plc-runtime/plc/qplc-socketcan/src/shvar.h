/*
* @(#) shvar.h 3.5 Mon Sep 28 13:55:57 MET 1998
*
*  Primitive di gestione delle variabili condivise, altrimenti
* dette "segnali di scambio".
*
* 05/06/96 GG 1.0 Prima stesura.
* 04/09/96 GG 1.1 Aggiunta la funzione "shvMakeEdge".
* 10/09/96 GG 1.2 Modificate le chiamate alla primitive di creazione
*             delle variabili a rilevamento di fronte. La funzione
*             "shvMakeEdge" e` stata ribattezzata "shvInit", ed e`
*             utilizzata all'inizializzazione del Plc, oppure per reset
*             globali "al volo".
* 26/11/96 GG 2.0 Aggiunta la possibilita` di allocare variabili in
*             RAM non volatile.
* 11/12/96 GG 2.1 I nomi possono ora contenere "$". E` poi stata aggiunto
*             un tipo "EXTENDED", per associare aree di dimensione arbitraria
*             ad un segnale di scambio.
* 12/12/96 GG 2.2 Documentata (ma non mi sogno nemmeno di eliminarla) una
*             limitazione della funzione "shvKeyToAddr".
* 30/05/97 GG 2.3 Cambiato il significato del valore scaricato in "change"
*             da "shvCreate". Ora l'intero si interpreta come una maschera di
*             bit, ciascuno dei quali segnala un conflitto tra proprieta`
*             richesta e proprieta` ottenuta. Vedere la macro "SHV_CHG_...".
* 03/06/97 GG 3.0 Aggiunta una tabella di conversione chiave --> descrittore
*             esteso per i segnali di scambio. Grazie ad essa, la funzione
*             "shvKeyToAddr" non soffre piu` di limitazioni.
*             Aggiunto l'attributo "SHV_MODE_RESET", che caratterizza le
*             variabili che si azzerano ad ogni GO del PLC. Di conseguenza,
*             e` stato aggiunto il flag "SHV_CHG_RESET" (e` attivato solo se
*             si richiede la creazione senza RESET per una variabili che
*             possiede gia` l'attributo).
*             Aggiunta la funzione "shvRestart", che azzera tutte le
*             variabili con  l'attributo "RESET".
* 03/06/97 GG 3.1 Aggiunte le funzioni "shvSet" ed "shvGet", utile per
*             scrivere o leggere in blocco segnali di scambio.
* 21/11/97 GG 3.2 Aggiunta la definizione del flag di "variabile cancellata".
* 24/11/97 GG 3.3 Uno schifoso, rivoltante, abominevole trucco. Si tratta di
*             una pezza "ad hoc" per risolvere "in qualche modo" il problema
*             della non cancellabilita` delle variabili RETAIN. Ora, ad ogni
*             GO tali variabili sono marcate "DELETED". Si provvede qui a
*             ridefinire una variabile che possieda gli attributi RETAIN e
*             DELETED. Se non ci sono cambiamenti nella configurazione della
*             RAM non volatile, la ridefinizione delle variabili RETAIN al GO
*             produce identici assegnamenti di indirizzi ogni volta.
* 20/03/98 GG 3.4 Aggiunta la funzione "shvStat", che fornisce una statistica
*             di occupazione dell'area delle variabili condivise.
* 28/09/98 GG 3.8 Aggiunta la funzione "shvAddrToVar", che trova (se esiste)
*             la variabile corrispondente all'indirizzo dato. NOTA: e` lenta.
*/

#ifndef _SHVAR_H_

#define MAX_SHV_NAME 19

/*
* Flag per shvCreate.
*/

#define SHV_MODE_TEST 0x01 /* Da` errore se non c'e` gia`, (non crea). */
#define SHV_MODE_EXCL 0x02 /* Da` errore se c'e` gia`. */
#define SHV_MODE_CONTIG 0x04 /* Alloca una matrice contigua. */
#define SHV_MODE_EDGE 0x08 /* Il Plc deve essere sensibile ai cambiamenti
                              della variabile */
#define SHV_MODE_TYPE 0x70 /* Tipo della variabile. */
/* Tipi ad un byte */
#define SHV_MODE_TYPE_BOOL 0x00 /* Tipo booleano. */
#define SHV_MODE_TYPE_CHAR 0x10 /* Tipo signed char. */
#define SHV_MODE_TYPE_UCHAR 0x20 /* Tipo unsigned char. */
/* Tipi a quattro byte */
#define SHV_MODE_TYPE_LONG 0x30 /* Tipo long. */
#define SHV_MODE_TYPE_ULONG 0x40 /* Tipo unsigned long. */
#define SHV_MODE_TYPE_FLOAT 0x50 /* Tipo float. */
/* Tipi a otto byte */
#define SHV_MODE_TYPE_DOUBLE 0x60 /* Tipo double. */
/* Estensione : tipi con numero arbitrario di byte. */
#define SHV_MODE_TYPE_EXTENDED 0x70 /* Cosa sono ? */
/* Flag di variabile in RAM non volatile. La sua base non
 sara` "pchMem", ma "NVRAM_pub". */
#define SHV_MODE_RETAIN 0x80
/* Flag di variabile da azzerare al GO. */
#define SHV_MODE_RESET 0x100
/* Flag di variabile rimossa. Solo uso interno. */
#define SHV_MODE_DELETED 0x200

/*
* Definizione del tipo che descrive una variabile condivisa.
* Le informazioni qui contenute completano quelle registrate nella
* tabella condivisa.
*/

typedef struct _shv_t {

/* Nome della variabile. */

	char name[MAX_SHV_NAME+1];

/* Dimensioni della variabile. */

	int dim1;
	int dim2;

/* Dimensione di un elemento. */

	int size;

/* Proprieta` della variabile. */

	int mode;

/* Chiave di accesso alla tabella nell'area condivisa. */

	int key;

/* Offset nella ram condivisa del prossimo elemento. */
/* Per l'elemento 0 della tabella, rappresenta l'inizio della lista
 delle variabili. */

	long next;

/* Offset nella ram condivisa del prossimo elemento della lista di hash. */

	long next_hash;

} shv_t;

/* 
* Definizione dei flag indicanti differenza di proprieta` nella creazione
* di un segnale di scambio. La maschera di bit restituita nel parametro
* "change" di "shvCreate" si interpreta utilizzando queste macro.
*/

#define SHV_CHG_SIZE   0x01 /* Diversa grandezza in byte del tipo. */
#define SHV_CHG_DIM1   0x02 /* Prima dimensione diversa. */
#define SHV_CHG_DIM2   0x04 /* Seconda dimensione diversa. */
#define SHV_CHG_KEY    0x08 /* Chiave diversa da quella richiesta. */
#define SHV_CHG_EDGE   0x10 /* Proprieta` EDGE diversa. */
#define SHV_CHG_RETAIN 0x20 /* Proprieta` RETAIN diversa. */
#define SHV_CHG_CONTIG 0x40 /* Proprieta` CONTIG diversa. */
#define SHV_CHG_TYPE   0x80 /* Tipi non compatibili. */
#define SHV_CHG_ITYPE  0x100 /* Tipi non identici (puo` non essere grave). */
#define SHV_CHG_RESET  0x200 /* Flag di RESET precedentemente impostato. */

/*
* Struttura utilizzata dalla funzione "shvStat", descrivente
* la quota di area condivisa libera.
*/

typedef struct _shvstat_t {
/* Memoria non volatile libera. */
	int free_nvram;
/* Memoria libera per nomi e dati . */
	int free_heap;
/* Numero complessivo di chiavi libere. */
	int free_keys;
/* Dimensione del pi` grande blocco di chiavi libere contigue. */
	int max_free_bkey;
} shvstat_t;

/*
* Funzione "shvStat"
* ------------------
* 
* Questa funzione restituisce la dimensione dei blocchi liberi nell'area
* delle variabili condivise. Il risultato e` scaricato nella struttura
* di tipo "shvstat_t" puntata dall'argomento.
* Risultato: 0 se tutto bene, -1 se errore.
*/

int shvStat(shvstat_t *s);

/*
* Funzione shvSet
* ---------------
*
*  Questa funzione inizializza una variabile a partire da un'immagine
* data.
*/

void shvSet(shv_t * var, char * img);

/*
* Funzione shvGet
* ---------------
*
*  Questa funzione copia in un buffer una variabile.
*/

void shvGet(shv_t * var, char * img);

/*
* Funzione shvInit
* ----------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* reinizializzarle, se richiesto, e, in ogni caso, per registrare
* quelle con l'attributo "EDGE" nella lista delle variabili a
* rilevamento di fronte. Questa
* funzione deve essere chiamata all'inizializzazione del Plc.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int shvInit(void);

/*
* Funzione shvRestart
* -------------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* azzerare tutte quelle con l'attributo "SHV_MODE_RESET".
*  Questa funzione dovrebbe essere chiamata ad ogni GO del PLC.
*/

void shvRestart(void);


/*
* Funzione shvDeleteRetain
* ------------------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* marcare "cancellate" tutte quelle con l'attributo "SHV_MODE_RETAIN".
*  Questa funzione dovrebbe essere chiamata ad ogni GO del PLC.
*/

void shvDeleteRetain(void);

/*
* Funzione "shvDefine"
* --------------------
*
*  Crea la variabile condivisa "name", di "dim1*dim2" elementi.
*  Se "key" e` diversa da -1, tenta di assegnargliela,
* altrimenti la chiave sara` scelta d'ufficio. "mode" decide la
* modalita` di creazione della variabile e la dimensione della cella.
* NOTA : Tutti questi parametri sono campi della struttura "v".
*  L'intero  puntato da "change" all'uscita dice se le caratteristiche
* della variabile sono diverse da quelle che l'utente si aspettava. Si
* interpreta come un campo di bit, ciascuno dei quali indica conflitto in
* una diversa proprieta` della variabile. Si usino le macro "SHV_CHG_..."
* per interpretare il risultato.
*/

int shvDefine(shv_t * v, int * change);

/*
* Funzione "shvCreate"
* --------------------
*
*  Crea la variabile condivisa "name", di "dim1*dim2" elementi.
*  Se "key" e` diversa da -1, tenta di assegnargliela,
* altrimenti la chiave sara` scelta d'ufficio. "mode" decide la
* modalita` di creazione della variabile e la dimensione della cella.
* NOTA : Tutti questi parametri sono campi della struttura "v".
*  Il flag "change" indica all'uscita se le caratteristiche della
* variabile sono diverse da quelle che l'utente si aspettava. Vale 0
* se non ci sono state modifiche pericolose (alcune proprieta`
* possono differire senza pericolo, per esempio il tipo di una
* variabile intera puo` essere definito con segno o senza segno, senza
* per questo pregiudicare la funzionalita`), 1 altrimenti.
*/

int shvCreate(shv_t * v, int * change);

/*
* Funzione "shvKeyToAddr"
* -----------------------
*
*  Questa funzione calcola l'indirizzo della variabile condivisa di
* chiave "key" ed indici "n" ed "m". Se la variabile e` scalare, gli indici
* devono valere 0 e 0. Se la variabile e` un vettore, il secondo indice
* deve valere 0. Se la chiave non coincide con quella della base di
* una variabile multidimensionale, "n" ed "m" devono valere 0.
*  Se la variabile e` un vettore o una matrice, ed e` contigua (flag
* SHV_MODE_CONTIG acceso nel campo della proprieta`) l'indirizzo dell'elemento
* 0,0 e` la base dell'area che le compete, vale cioe` la regola :
*
*     addr(v[i][j]) == addr(v[0][0]) + dim2(v)*i*size(v) + j*size(v)
*
*  Se la variabile non e` contigua, la regola si applica solo alle chiavi :
*
*     key(v[i][j]) == key(v[0][0]) + dim2(v)*i + j
*
*  Quest'ultima regola e` sempre valida.
*
*  La funzione restituisce l'indirizzo richiesto, oppure NULL in caso di
* errori. Si ha errore se la chiave non esiste e se gli indici sono
* maggiori delle dimensioni impostate.
*/

void *shvKeyToAddr(int key, int n, int m);

/*
* Funzione shvAddrToVar
* ---------------------
*
*  Questa funzione esplora la lista delle variabili condivise
* alla ricerca dell'elemento piu` vicino all'indirizzo "addr".
*  Il descrittore della variabile e` riportato in "ret". "key" e` il
* puntatore ad un intero, che all'uscita conterra` la chiave
* associata all'elemento trovato. Serve per elementi vettoriali,
* nei quali la chiave puo` differire da quella dell'elemento [0][0].
* La differenza tra chiave corrente e chiave base permette di ricavare gli
* indici dell'elemento.
*  La funzione vale 0 in caso di successo, -1 in caso di errore.
*/

int shvAddrToVar(void *addr, shv_t *ret, int *key);

#define _SHVAR_H_

#endif


