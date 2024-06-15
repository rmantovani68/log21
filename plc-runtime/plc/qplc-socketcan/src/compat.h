/*
* @(#) compat.h 1.5 Tue Nov 11 13:55:37 MET 1997
*
*  Questo file contiene definizioni riguardanti la parte di
* compatibilita` col vecchio plc "isaker".
*
* 07/06/96 GG 1.0 Prima stesura.
* 22/07/96 GG 1.1 Corretto un disallineamento nella definizione della
*             struttura "ik_system_t".
* 03/09/96 GG 1.2 Aggiunto un meccanismo per la creazione selettiva di
*             segnali di scambio standard.
* 10/09/96 GG 1.3 Modificata la sintassi delle funzioni "compat...edge".
*             Aggiunta la funzione "compat_register_edge".
* 03/06/97 GG 1.4 Aggiunta la funzione "compat_restart_edges", per annullare
*             eventuali fronti pendenti al GO.
* 11/11/97 GG 1.5 Rese globali le funzioni legate al trace (non ancora!).
*/

#ifndef _COMPAT_H_

/*
* Maschere di creazione dei segnali di scambio standard. Sono creati
* solo quelli che hanno una maschera compatibile con quella corrente,
* impostabile da linea di comando.
*/

#define PKG_MIN 0x01	/* Segnali indispensabili (sono sempre definiti). */
#define PKG_XNC 0x02	/* Segnali per comunicazione con XNC. */
#define PKG_XIO 0x04	/* Segnali di XNC associati per default a certi I/O. */
#define PKG_ALL 0xFF	/* Tutti i segnali presenti. */

/*
* Definizione degli indici di partizione utili (piu` o meno)
* previsti per compatibilita` con IsaGraf.
*/

/* Spazio utilizzato per il modulo di comunicazione "old style". */
#define _IK_SPC_OSYS 0
/* Spazio per i simboli IsaGraf. Sara` riempito con una stringa particolare,
 che permettera` di distinguere isaker da qplc. */
#define _IK_SPC_OSYM 3
/* Spazio utilizzato per il colloquio "vecchio stile" col debugger. */
#define _IK_SPC_OTST 5
/* Spazio della chiavi, utilizzato per contenere la tabella di accesso
 per chiave alle variabili condivise, alias "segnali di scambio". Il
 suo contenuto e la sua organizzazione non cambiano rispetto ad isaker. */
#define _IK_SPC_SHVK 10
/* Symbol table "vecchio stile". Vuoto. */
#define _IK_SPC_OSYT 11
/* Hash table "vecchio stile". Allocata, ma con tutti gli elementi a 0. */
#define _IK_SPC_OHTB 12
/* Hash table "nuova", per la ricerca veloce dei nomi delle variabili
 condivise. */
#define _IK_SPC_SHVH 17
/* Heap condiviso. Contiene la lista dei simboli condivisi, i loro descrittori,
 e le aree associate (eccetto nei casi di variabili sparse). La sua dimensione
 cresce man mano che si creano nuove variabili condivise. */
#define _IK_SPC_SHVN 18
/* Spazio libero. E` utilizzato per "marcare" la fine dell'heap condiviso. */
#define _IK_SPC_FREE 19
#define _IK_MAX_SPC 20


/* Definizioni mantenute per compatibilita` con IsaGraf. */

/* Dimensione del buffer di richiesta "vecchio stile". */
#define MAX_OLD_COMM_REQ 1024
/* Dimensione massima di un messaggio del client. */
#define MAX_OLD_COMM_QUESTION 256

	#pragma pack(1)
typedef struct 
{
	unsigned char start;	/* exec. status. */
	unsigned char filler1[3];
	long filler2[5];
	char question_ready;    /* question to kernel */
	char client;            /* client semaphore */
	unsigned char *question_ker;     /* question to server */
	unsigned char *answer_ker;       /* response to kernel */
	short answer_size;       /* response size */ 
	char filler3[32];
	char buffer[MAX_OLD_COMM_QUESTION * 2];        /* question buffer */
} ik_system_t;
	#pragma pack()

/*
* Numero massimo di variabili protette da azzeramenti indebiti.
*/

#define MAX_COMPAT_SHADOW 10

/*
* Tipo associato alla definizione di variabili a rilevamento di fronte.
* Descrive un blocco omogeneo di variabili a rilevamento di fronte.
*/

typedef struct {
/* Numero di variabili. */
	int n;
/* Dimensione delle variabili (1 o 4 : char o long). */
	int size;
/* Puntatore alla lista degli indirizzi. */
	char ** varlist;
/* Puntatore alla lista delle immagini. */
	char * imglist;
} compat_edge_t;

/*
* Funzioni "compat_init_1" e "compat_init_2"
* ------------------------------------------
*
* Queste funzioni inizializzano parte del sottosistema di compatibilita`
* con isaker.
*/

void compat_init_1(void);
void compat_init_2(void);


/*
* Funzione "compat_start"
* -----------------------
*
*  Questa funzione esegue ad ogni "START" le azioni richieste per
* compatibilita` con isaker.
*/

void compat_start(void);

/*
* Funzione "compat_add_edge"
* --------------------------
*
* Riserva "n" celle per variabili a rilevamento di fronte di dimensione "size".
* "n" e "size" sono campi della struttura "*edge", pre-caricata.
* Restituisce il puntatore alla struttura stessa, oppure
* NULL se non c'e` piu` posto. In caso di successo, la struttura contiene
* campi che puntano alla lista degli indirizzi delle "n" variabili e alla
* lista delle immagini delle stesse. Si usi la funzione "compat_register_edge"
* per riempire queste liste.
*/

compat_edge_t * compat_add_edge(compat_edge_t * edge);

/*
* Funzione "compat_remove_edge"
* -----------------------------
*
*  Questa funzione ritira le ultime "n" celle a
* rilevamento di fronte della dimensione "size". "n" e"size" sono
* campi della struttura "*edge".
*/

void compat_remove_edge(compat_edge_t * edge);

/*
* Funzione "compat_register_edge"
* -------------------------------
*
*  Questa funzione registra nella lista di variabili a rilevamento di fronte
* "edge" l'a variabile di indirizzo "addr" nella posizione "n".
*/

void compat_register_edge(compat_edge_t * edge, int n, char *addr);

/*
* Funzione "compat_restart_edges"
* -------------------------------
*
*  Questa funzione inizializza le immagini dei fronti per evitare che
* ad una sequenza HALT-GO siano rilevati erroneamente eventi.
*/

void compat_restart_edges(void);

/*
* Funzione "compat_init_spaces"
* -----------------------------
*
*  Creazione dell partizioni della shared memory secondo uno schema
* compatibile con IsaGraf.
*/

void compat_init_spaces(void);

/*
* Funzione "compat_begin_cycle"
* -----------------------------
*
*  Questa funzione deve essere chiamata all'inizio del ciclo ordinario
* del PLC, dopo "hat".
*  Attualmente, registra lo stato di alcuni segnali di scambio (se esistono)
* associati a tasti "importanti", il valore finale dei quali dovra`
* essere l'OR logico tra quello iniziale e quello impostato dal programma.
*/

void compat_begin_cycle(void);

/*
* Funzione "compat_trace"
* -----------------------
*
*  Questa funzione memorizza lo stato delle variabili di trace selezionate
* in un apposito buffer interno, ed eventualmente comunica la fine dell'ope-
* razione al debugger (trace vecchio stile).
*/

void compat_trace(void);


/*
* Funzione "compat_comm"
* ----------------------
*
*  Questa funzione realizza parte del colloquio con i debugger
* "isaker-compatibili". "isaker" gestiva completamente i comandi e
* le risposte all'interno del ciclo di esecuzione. Ora non e` piu` cosi` :
* i comandi piu` pesanti sono realizzati dal thread supervisore, mentre
* all'esecutore resta il compito di riconoscere i comandi piu` semplici
* e di arbitrare il colloquio.
*/

void compat_comm(void);

/*
* Funzioni "compat_chk...", "compat_go", "compat_halt"
* -------------------------------------------------------
*
*  Queste funzioni controllano lo stato "vecchio stile", impostato dai
* debugger "isaker-compatibili".
*/

void compat_chkgo(void);

void compat_chkhalt(void);

void compat_go(void);

void compat_halt(void);

/*
* Funzione compat_check_io()
* --------------------------
*
*  Questa funzione controlla che la configurazione registrata in NVRAM
* sia compatibile con la situazione reale dei moduli.
*/

int compat_check_io(void);

/*
* Funzione compat_save_io()
* -------------------------
*
*  Questa funzione salva che la configurazione in NVRAM.
*/

void compat_save_io(void);

/*
* Funzione "sys_err"
* ------------------
*
* Mantenuta per compatibilita` con "isaker". Obsoleta.
*/

void sys_err(int n, char *s);

#define _COMPAT_H_

#endif

