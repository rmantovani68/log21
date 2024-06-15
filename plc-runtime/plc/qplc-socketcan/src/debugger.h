/*
* @(#) debugger.h 1.11 Tue Nov  7 12:48:08 MET 2000
*
* Definizioni legate al debugger in tempo reale del Plc.
*
* 04/12/96 GG 1.0 Prima stesura.
* 16/12/96 GG 1.1 Modifichine.
* 17/12/96 GG 1.2 Evitato lo schianto nell'abilitazione di una lista vuota.
* 16/01/97 GG 1.3 Modificato il commento di "brkAdd".
* 20/01/97 GG 1.4 Aggiunto un puntatore all'indietro nella lista delle liste.
* 10/03/97 GG 1.5 Pazzesco ! Incredibile ! Erano scambiate le definizioni
*             di ESI ed EDI !
* 18/06/97 GG 1.6 Sono stati aggiunti nuovi segnali di scambio per
*             riconoscere la ricopertura degli eventi e individuare l'insieme
*             di dati ancora affidabili nel buffer. Sono anche stati aggiunti
*             appositi campi nella struttura di stato per aggiornare in fretta
*             i nuovi segnali.
* 08/07/97 GG 1.7 Altri segnali di scambio per il riconoscimento rapido del
*             passaggio per una certa lista di break.
* 17/11/97 GG 1.8 Non si scrive TOOGGLE ! 
*             E`  stato aggiunto un campo ai descrittori interni id lista
*             per registrare la posizione in cui e` registrato l'insieme di
*             eventi relativi all'ultimo ciclo durante il quale sia stato
*             incontrato ciascun watchpoint.
*             Aggiunta la funzione "brkLastEvent", che fornisce gli indirizzi
*             e le lunghezze dei blocchi che costituiscono l'ultimo evento
*             registrato.
* 03/11/00 GG 1.9 Aggiunte le funzioni "brkHistoryMode", "brkHistoryLock"
*             e "brkHistoryUnlock", ad uso del debugger remoto. Servono
*             a gestire in modo piu` efficiente il buffer di debug, e
*             risolvere il problema del codice percorso raramente anche
*             col debugger remoto.
* 04/11/00 GG 1.10 Eliminato ul parameto "cycle" della "brkHistory",
*             perche` e` inutile.
* 07/11/00 GG 1.11 Aggiunte le funzioni "debErrorLog" e "debErrorList",
*             che servono ad immagazzinare gli ultimi errori prodotti
*             (vedi "plcError"), e a ottenere il buffer degli errori.
*             Queste funzioni usano tre nuovi segnali di scambio per gestire
*             il solito buffer circolare... Si tratta di "$DEB_ERRORS"
*             (e` il buffer degli errori), "$DEB_FIRSTERRP" (primo errore)
*             e "$DEB_ERRLENGTH" (numero di byte significativi nel buffer).
*/

#ifndef _DEBUGGER_H_

/* Struttura di stato della CPU, come prodotta dalla funzione 
 "brk_hook". */

typedef struct _cpu_sts_t {
	unsigned long flags;
	unsigned long regs[8];
	unsigned char *pc;
} cpu_sts_t;

/* Indici dei registri nell'array "reg" della struttura di stato. */

enum {
/* L'ordine e` quello imposto dall'istruzione
 di macchina "PUSHAL", utilizzata per salvare lo stato della CPU. */
_R_EDI, _R_ESI, _R_EBP, _R_ESP, _R_EBX, _R_EDX, _R_ECX, _R_EAX,
};

/* Codici di registro prodotti dal compilatore IL, e tabella di conversione
 da utilizzare per convertirli nella rappresentazione interna. */

typedef enum _deb_reg_t {
	R_NULL,
	R_EAX, R_EBX, R_ECX, R_EDX, R_EDI, R_ESI, R_EBP, R_ESP
} deb_reg_t;

#define DEB_TRANSREG { \
	0, \
	_R_EAX, _R_EBX, _R_ECX, _R_EDX, _R_EDI, _R_ESI, _R_EBP, _R_ESP \
}

/* Struttura descrivente un indirizzamento in stile Intel386. */

typedef struct _i386_operand_t {
	char r1; /* 0 : ignorare. */
	char r2; /* 0 : ignorare. */
	char scale; /* Se == 0, accesso diretto ad r1 ed r2 */
	char size; /* Se == 0, non e` richiesta alcuna registrazione. */
	unsigned long off; /* Offset, eventualmente nullo. */

	char ar;  /* Altro registro da osservare. */
	          /* Non fa parte dell'indirizzo. */
	          /* 0 : ignorare. */
} i386_operand_t;

/* Lunghezza dell'istruzione "call debugger_hook". */

#define WATCHPOINT_LENGTH 5

/* Flag di azione/condizione. */

/* Condizioni. */
#define WATCH_C_FALSE 0x00 /* Condizione sempre falsa */
#define WATCH_C_TRUE  0x01 /* Condizione sempre vera */
#define WATCH_C_AND   0x02 /* Condizione vera se (FLAGS & opc) == opc */
#define WATCH_C_OR    0x03 /* Condizione vera se (FLAGS & opc) != 0 */
#define WATCH_C_NOR   0x04 /* Condizione vera se (FLAGS & opc) == 0 */
#define WATCH_C_TGT   0x05 /* Condizione vera se ctot > opc */
#define WATCH_C_TLE   0x06 /* Condizione vera se ctot <= opc */
#define WATCH_C_TEQ   0x07 /* Condizione vera se ctot == opc */
#define WATCH_C_CGT   0x08 /* Condizione vera se ccurr > opc */
#define WATCH_C_CLE   0x09 /* Condizione vera se ccurr <= opc */
#define WATCH_C_CEQ   0x0a /* Condizione vera se ccurr == opc */
#define WATCH_C_RZ    0x0b /* Condizione vera se REG(opc) == 0 */
#define WATCH_C_NRZ   0x0c /* Condizione vera se REG(opc) != 0 */
#define WATCH_C_MZ    0x0d /* Condizione vera se MEM(opc) == 0 */
#define WATCH_C_NMZ   0x0e /* Condizione vera se MEM(opc) != 0 */
/* Azioni (sono OR-ABILI). Queste definizioni si applicano anche al
 campo "condact". */
#define WATCH_A_NOP   0x00 /* Nessuna azione. */
#define WATCH_A_SET   0x01 /* FLAGS |= opa1 */
#define WATCH_A_RESET 0x02 /* FLAGS &= ~opa2 */
#define WATCH_A_TOGGLE 0x04 /* FLAGS ^= opa2 */
#define WATCH_A_INC   0x08 /* ++COUNT se primo elemento della lista. */
#define WATCH_A_CYC   0x10 /* Registra il numero di ciclo corrente. */
#define WATCH_A_LOG   0x20 /* Registra l'evento nel buffer di debug. */
#define WATCH_A_LOGFL 0x40 /* Registra anche il valore di FLAGS se primo el. */
#define WATCH_A_BRK   0x80 /* Ripristina il watchpoint indicato da "apar". */

/* Struttura descrivente le caratteristiche di un watchpoint (pubblica). */

typedef struct _watch_t {
/* Posizione in memoria. */
	unsigned char * addr;
/* Descrizione dell'operando da seguire (azione WATCH_A_LOG). */
	i386_operand_t operand;
} watch_t;

/* Numero massimo di liste di watch. */
#define MAX_WATCHPOINT 20
/* Numero totale di punti di watch disponibili. */
#define MAX_WATCHNODE  300

/* Struttura descrivente un elemento della catena di watchpoint. */

typedef struct _watch_node_t {
/* Informazioni pubbliche */
	watch_t prop;
/* Copia del tratto di codice al quale e` stata sostituita la chiamata
 a "debugger_hook". */
	char backup_code[WATCHPOINT_LENGTH];
/* Puntatore al prossimo elemento della catena. */
	struct _watch_node_t *next;
} watch_node_t;

/* Struttura descrivente il comportamento di una lista di watch. */

typedef struct _watch_descr_t {
/* Azioni */
	unsigned short act;
/* Maschera delle azioni eseguite solo se la condizione e` vera */
	unsigned short condact;
/* Condizioni */
	unsigned short cond;
/* Eventaule parametro associato all'azione. */
	unsigned short apar;
/* Operandi. */
	unsigned long opc;      /* Operando utilizzato nelle condizioni */
	unsigned long opa1;     /* Operando di "SET". */ 
	unsigned long opa2;     /* Operando di "RESET". */
} watch_descr_t;

/* Struttura di stato di una catena di watchpoint. */

#define WS_USED 0x01     /* Entry utilizzata. */
#define WS_ENABLED 0x02  /* Breakpoint abilitato. */

typedef struct _watch_sts_t {
/* Puntatore al prossimo elemento della lista dei watch. */
	struct _watch_sts_t *next;
/* Puntatore al precedente elemento della lista dei watch. */
	struct _watch_sts_t *pred;
/* Identificativo del blocco. */
	int id;
/* Flag di abilitazione. */
	int flags;
/* Descrizione fornita dall'esterno. */
	watch_descr_t descr;
/* Puntatore al primo elemento della catena dei watchpoint del gruppo. */
	watch_node_t *chain;
/* Puntatore all'ultimo elemento della catena dei watchpoint del gruppo. */
	watch_node_t *chain_tail;
/* Puntatore all'elemento corrente della catena. */
	watch_node_t *curr;
/* Contatori di passaggio : */
	unsigned long ctot;	/* dalla nascita del watchpoint. */
	unsigned long ccurr;	/* dall'inizio del ciclo. */
/* Ultimo ciclo nel quale il programma e` passato dal watchpoint. */
	unsigned long cycle;
/* Posizione dell'ultimo record generato dal watchpoint. */
	unsigned long record;
/* Posizione dell'insieme di eventi relativi all'ultimo ciclo durante
 il quale vi sia stato un passagio per il watchpoint. */
	unsigned long event_record;
} watch_sts_t;

/*
* Nomi dei segnali di scambio riservati al debugger.
*/

#define DEBSHV_CYCLE  "$DEB_CYCLE"    /* Ciclo corrente (long) */
#define DEBSHV_BUFFER "$DEB_BUFFER"   /* Area contenente gli eventi. */
#define DEBSHV_BSIZE  "$DEB_BSIZE"    /* Dimensione dell'area degli eventi. */
#define DEBSHV_EVENTS "$DEB_EVENTS"   /* Puntatori agli insiemi di eventi */
                                      /* dello stesso ciclo (long). Vale */
                                      /* -1 se non punta a niente di valido. */
#define DEBSHV_NEVT   "$DEB_NEVT"     /* Numero di elementi di $DEB_EVENTS. */
#define DEBSHV_LASTEV "$DEB_LASTEV"   /* Ultimo elemento di $DEB_EVENTS */
                                      /* aggiornato dal PLC. */
#define DEBSHV_FIRSTEVP "$DEB_FIRSTEVP" /* Posizione del primo evento */
                                      /* completo, cioe` non ricoperto. */
#define DEBSHV_LASTEVP "$DEB_LASTEVP" /* Posizione dell'ultimo evento */
                                      /* completo, cioe` non ricoperto. */
#define DEBSHV_EVCOUNT "$DEB_EVCOUNT" /* Contatore di eventi ricoperti. E` */
                                      /* azzerabile da programma esterno. */
#define DEBSHV_NREGEV "$DEB_NREGEV"   /* Numero di eventi ancora integri */
                                      /* (cioe` non ricoperti) nel buffer. */
#define DEBSHV_MAXBRK "$DEB_MAXBRK"   /* Numero di liste di break definibili.*/
#define DEBSHV_BRKCYC "$DEB_BRKCYC"   /* Tabella contenete l'ultimo ciclo */
                                      /* durante il quale si e` passati per */
                                      /* ciascuna lista. */

#define MAX_LOGGED_EVENTS 200         /* Dimensione di $DEB_EVENTS (default). */
#define EVENT_BUFFER_SIZE 30000       /* Dimensione di $DEB_BUFFER (default). */

/* Log degli errori. */
#define DEBSHV_ERRORS "$DEB_ERRORS"   /* Buffer degli errori. */
#define DEBSHV_FIRSTERRP "$DEB_FIRSTERRP" /* Primo errore nel buffer. */
#define DEBSHV_ERRLENGTH "$DEB_ERRLENGTH" /* Lunghezza dei dati nel buffer. */
#define DEBSHV_ERRBUFFSZ "$DEB_ERRBUFFSZ" /* Lunghezza del buffer. */

#define ERROR_BUFFER_SIZE 2048     /* Dimensione di $DEB_ERRORS (default). */

/*
* Struttura descrivente un evento di debug (passaggio per un watchpoint).
*/

typedef struct _brk_event {
/* Dimensione dell'intera struttura. Posso pretendere che si salvino
 comunque meno di 64k di dati in un colpo solo ? */
	unsigned short be_size;
/* Indice della lista di watch, come ottenuto da "brkCreate". */
	char be_id;
/* ZERO o piu` byte di dati. Sono i valori delle variabili o dei registri. */
	char be_data[1]; /* Trucco ! E` un campo di dimensione variabile */
} brk_event;

/*
* Struttura descrivente un insieme di eventi di debug.
* E` ad oggetti di questo tipo che puntano gli elementi di $DEB_EVENTS.
*/

typedef struct _brk_event_list {
/* Dimensione dell'intero blocco di dati. */
	unsigned long bl_size;
/* Ciclo di Plc durante il quale sono stati registrati gli eventi. */
	unsigned long bl_cycle;
/* Lista degli eventi. Ce ne sara` sempre almeno uno. */
	brk_event bl_event[1]; /* Il solito trucco. */
} brk_event_list;

/*
* Struttura descrivente un errore registrato nel buffer apposito.
* Ha solo uno scopo ducumentativo, non essendo utilizzata (ne` utilizzabile,
* a causa dei soliti problemi di allineamento degli elementi).
*/

#if 0
typedef struct _deb_error_t {
/* Dimensione dell'intera struttura (max. 255 byte + \0 finale). */
	unsigned char er_size;
/* Codice di errore. */
	int er_code;
/* Stringa associata - termina con '\0'. */
	char er_text[1 /* o piu`... */];
} deb_error_t;
#endif

/*
* Funzione "brkInit"
* ------------------
*
*  Inizializza il sistema dei breakpoint. Oltre alle solite cose,
* registra gli indirizzi dati, e li considera
* come destinazione di puntatori a funzioni che l'esecutore chiama
* ad inizio e fine ciclo. Quando si abilita almeno un watchpoint,
* le variabili puntate sono aggiornate, ed il Plc eseguira` le funzioni
* di ipostazione e cancellazione dei breakpoint nel codice.
*  Quando nessun watchpoint e` piu` attivo, le variabili sono riportate
* al valore originale, che sara` tipicamente l'indirizzo di una funzione
* vuota.
*/

void brkInit(void(**p_begin_cycle)(void), void(**p_end_cycle)(void));

/*
* Funzione "brkReinit"
* --------------------
*
*  Inizializza (a caldo) il sistema dei breakpoint.
*/

void brkReinit(void);

/*
* Funzione "brkCreate"
* --------------------
*
* Definisce un breakpoint (alias "lista di watch").
* Restituisce l'ID della lista, o -1 in caso di errori.
*/

int brkCreate(watch_descr_t * wd);

/*
* Funzione "brkEnable"
* --------------------
*
* Abilita un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkEnable(int wlid);

/*
* Funzione "brkDisable"
* ---------------------
*
* Disabilita un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkDisable(int wlid);

/*
* Funzione "brkDestroy"
* ---------------------
*
* Distrugge un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkDestroy(int wlid);

/*
* Funzione "brkAdd"
* -----------------
*
* Aggiunge un watchpoint alla "lista di watch" data.
* Restituisce -1 in caso di errori, altrimenti da` la posizione del
* nuovo elemento aggiunto.
*/

int brkAdd(int wlid, watch_t * w);

/*
* Funzione "brkCore"
* ------------------
*
* Questa funzione realizza il break vero e proprio.
*/

void brkCore(cpu_sts_t *cpu);

/*
* Funzione "brkRegisterProc"
* ---------------------------
*
*  Questa funzione registra gli indirizzi dati, e li considera
* come destinazione di puntatori a funzioni che l'esecutore chiama
* ad inizio e fine ciclo. Quando si abilita almeno un watchpoint,
* le variabili puntate sono aggiornate, ed il Plc eseguira` le funzioni
* di ipostazione e cancellazione dei breakpoint nel codice.
*  Quando nessun watchpoint e` piu` attivo, le variabili sono riportate
* al valore originale, che sara` tipicamente l'indirizzo di una funzione
* vuota.
*/

void brkRegisterProc(void(**p_begin_cycle)(void), void(**p_end_cycle)(void));

/*
* Funzione "brkSet"
* -----------------
*
* Inserisce il primo punto di break  di ogni lista nell'indirizzo
* che gli compete.
*/

void brkSet(void);

/*
* Funzione "brkClear"
* -------------------
*
* Ripristina il codice originario in tutti i punti di break.
*/

void brkClear(void);

/*
* Funzione "brkBegin"
* ------------------
*
*  Questa funzione e` chiamata ad ogni inizio ciclo se c'e` almeno un
* breakpoint attivo.
*/

void brkBegin(void);

/*
* Funzione "brkEnd"
* -----------------
*
*  Questa funzione e` chiamata ad ogni fine ciclo se c'e` almeno un
* breakpoint attivo.
*/

void brkEnd(void);

/*
* Funzione "brkLastEvent"
* -----------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'ultimo blocco di eventi registrato.
* Riporta anche l'indice di ciclo del blocco di eventi.
*/

int brkLastEvent(unsigned char **b1, unsigned long *n1,
                 unsigned char **b2, unsigned long *n2,
                 unsigned long *cycle);

/*
* Funzione "brkHistoryMode"
* -------------------------
*
* Questa funzione attiva la modalita` di debug tradizionale
* (mode == 0) o compressa (mode == 1). Nella modalita` tradizionale,
* ogni evento dei debug e` registrato incondizionatamente nel buffer
* di debug. Nella modalita` compressa, e` registrato un solo evento
* per ogni lista di break, e solo la chiamata della "brkHistoryLock"
* permette di registrare ulteriori eventi.
*/

void brkHistoryMode(int mode);

/*
* Funzione "brkHistoryLock"
* -------------------------
*
* questa funzione blocca l'evoluzione dell'history. in modo 0 (modo
* ordinario), il debugger cessa di registrare gli eventi.
* in modo 1, il debugger porta avanti la posizione di lettura, e
* marca "non incontrati" tutti i watchpoint.
*/

void brkHistoryLock(void);

/*
* Funzione "brkHistoryUnlock"
* ---------------------------
*
* Questa funzione sblocca l'evoluzione dell'history. Ha effeto
* solo in modo 0.
*/

void brkHistoryUnlock(void);

/*
* Funzione "brkHistory"
* ---------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'intera storia degli eventi registrati.
*/

int brkHistory(unsigned char **b1, unsigned long *n1,
               unsigned char **b2, unsigned long *n2);

/*
* Funzione "debErrorInit"
* -----------------------
*
* Questa funzione inizializza il sistema di log degli errori.
*/

void debErrorInit(void);

/*
* Funzione "debErrorLog"
* ----------------------
*
* Questa funzione copia il messaggiodi errore di codeice "code" e
* messaggio aggiuntivo "text" nel buffer degli errori.
*/

void debErrorLog(int code, char *text);

/*
* Funzione "debErrorList"
* -----------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'intera storia degli errori registrati.
*/

int debErrorList(unsigned char **b1, unsigned long *n1,
                 unsigned char **b2, unsigned long *n2);

/*
* Funzione "debErrorFindInit"
* ---------------------------
*
* Questa funzione inizializza la variabile puntata da "p" in modo che
* possa essere utilizzata per la lettura sequenziale degli errori
* registrati nel buffer.
*/

void debErrorFindInit(unsigned long *p);

/*
* Funzione "debErrorFindNext"
* ---------------------------
*
* Questa funzione legge il prossimo errore immagazzinato nel buffer,
* e aggiorna la variabile puntata da "p" per la successiva ricerca.
* il codice dell'errore e` scaricato in "code", il testo e` copiato in
* "text". La funzione vale 1 se tutto e` andato bene, 0 se non ci sono
* piu` errori da leggere.
*/

int debErrorFindNext(unsigned long *p, int *code, char *text);

#define _DEBUGGER_H_

#endif /* _DEBUGGER_H_ */

