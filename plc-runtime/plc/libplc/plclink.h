/*
* @(#) plclink.h 2.4 Wed Jul  9 13:18:21 MET DST 1997
*
* Primitive di accesso ai simbili pubblici del Plc Isagraf.
*
* --/--/-- 1.0. Prima stasura.
* 20/06/95 1.1. Aggiunta la funzione "plcLdStatus".
* 14/07/95 1.2. Corretto un errore che poteva provocare il crash nella funzione
*          "dbFindIsaVar".
* 13/09/95 1.3. (Versione per Solaris) Aggiunta la funzione "plcDbChanged".
* 18/09/95 1.4. Unificato il sorgente per Solaris e LynxOS.
* 04/12/95 1.5. Aggiunte funzioni per lettura byte arbitrari e statistica.
* 30/01/96 1.6. Aggiunta la funzione "plcModProp", per ottenere i dati
*          rilevanti su di un modulo caricato dinamicamente.
* 31/01/96 1.7. Aggiunti comandi di controllo della statistica.
* 02/02/96 1.8. Aggiunte le funzioni di collegamento non bloccante al Plc :
*          "plcTryAttach(0)", "plcReady()". Sono da utilizzare nell'ordine
*          dato. Il successo della seconda autorizza il chiamante ad utilizzare
*          la "plcLinkStatics()".
* 13/02/96 1.9. La funzionalita` delle primitive aggiunte nella versione
*          precedente e` stata raccolta nella nuova funzione "plcTryOpen()".
*          Sono poi state aggiunte funzioni per la lettura multipla di aree
*          e la validazione di indirizzi.
* 21/03/96 1.10. Corretta la funzione "plcStClear"...che non andava.
* 10/06/96 1.11. Aggiunta la funzione "plcTraceAddr", che permette di
*          specificare un indirizzo qualunque per il trace.
* 22/06/96 1.12. Aggiunta la funzione "plcSafeCopyVar", per la copia di
*          variabili in maniera sicura, cioe` senza schianti, qualsiasi cosa
*          succeda...o quasi.
*          Aggiunta la funzione "dbQPLC", che dice se si sta usando qplc.
*          Aggiunta la funzione "dbFindQplcVar" che strova una variabile
*          condivisa di QPLC scomponendo un nome in stile isagraf in
*          parte base ed indici.
*          Aggiunta la funzione "dbGetQplcVarDescr", che trova indirizzo
*          e descrittore di una variabile di QPLC.
* 25/06/96 1.13. Corrette le immancabili sfighe aggiunte.
* 28/11/96 GG 1.14 Aggiunta la gestione delle variabili in RAM non volatile.
*             Tale gestione e` limitata alle funzioni che utilizzano la
*             primitive "dbGetQplcVarDescr", e cioe` "dbGetVar" e
*             "dbGetVarAddr". Non funzionano invece le varie "plcGetVar..."
*             "plcSerVar...", perche` la modifica richiesta comporterebbe
*             un rallentamento sensibile, senza che vi sia, al momento,
*             un reale bisogno della nuova prestazione. Bisogna inoltre
*             ricordare che queste funzioni sono in via di obsolescenza,
*             anche se il loro completo abbandono richiedera` molto tempo.
*             La gestione della RAM non volatile non e` a totale carico della
*             libreria, perche` la modialita` di aggancio al dispositivo
*             puo` variare parecchio da un'applicazione all'altra. Si e`
*             percio` deciso che il puntatore alla zona di RAM non volatile
*             del PLC (cioe` l'area nominata "NVRAM_LBL_PLC480" in "nvram.h"),
*             deve essere ottenuto dall'applicazione stessa, e comunicato
*             alla libreria con la nuova funzione "dbRegisterNvram". Se
*             questo non viene fatto, l'indirizzo associato a variabili
*             in RAM non volatile varra` NULL.
* 11/12/96 GG 2.0 Modificata la dimensione del campo "size" nella struttura
*             "plcvar_t", per poter rappresentare oggetti di grandi dimensioni.
*             A causa di questa modifica, SI E` PERSA LA COMPATIBILITA` con
*             il passato. I programmi che volessero utilizzare la nuova
*             versione della libreria DEVONO essere ricompilati interamente,
*             non si possono cioe` mescolare oggetti compilati con la versione
*             precedente di questo file con oggetti compilati con questa.
* 16/12/96 GG 2.1 Aggiunte funzioni per il debug in tempo reale di QPLC :
*             plcCreateWatchList, plcDeleteWatchList, plcAddBreak,
*             plcEnableWatchList, plcDisableWatchList.
* 17/12/96 GG 2.2 Aggiunta la funzione (qplc >= 1.3.3.8) plcClearAllWatches.
* 03/07/97 GG 2.3 Aggiunte nuove definizione per prestazioni aggiuntive di
*             debug: tipo "brk_history", funzioni "plcLoadDebugHistory",
*             "plcIndexDebugEventList" e "plcDebugMemCopy", nuovi segnali di
*             scambio.
* 09/07/97 GG 2.4 Aggiunto un parametro a "plcLoadDebugHistory".
*/

#ifndef _PLCLINK_H

/* Definizioni ereditate da "plcdb.h" (kernel plc). */

#ifndef _PLCDB_H

#define MAXTRACESIZE 400	/* Dimensione di un buffer di trace. */
#define TRACEBUFSIZE MAXTRACESIZE	/* ...alias... */

/* Tipo utilizzato per rappresentare un campione nel buffer di trace. */
typedef unsigned long TRACERECORD;

#endif

/* Errori prodotti da alcune funzioni della libreria. */

#define PLCERR_REMERR   1 /* Errore restituito dal kernel plc. */
#define PLCERR_TIMEOUT  2 /* Timeout nella comunicazione col PLC. */
#define PLCERR_TOOVAR   3 /* Troppe variabili tracciate. */
#define PLCERR_BADVAR   4 /* Variabile non booleana. */
#define PLCERR_NOVAR    5 /* Variabile non trovata. */
#define PLCERR_NOTREADY 6 /* Dato non pronto. */

/* Tipi di dati rappresentati da una variabile Isagraf. */

#define ISAVAR_T_UNKN 0
#define ISAVAR_T_BOOL 1
#define ISAVAR_T_ANA_I 2
#define ISAVAR_T_ANA_F 3
#define ISAVAR_T_TIMER 4
#define ISAVAR_T_MSG 5

/* Classi delle variabili Isagraf. */

#define ISAVAR_A_UNKN 0
#define ISAVAR_A_INPUT 1
#define ISAVAR_A_OUTPUT 2
#define ISAVAR_A_INTERNAL 3

/* Macro per la traduzione dei puntatori dallo spazio isaker a
 quello dell'applicazione. */

/* Da isaker ad applicazione. */
#define MTRANS(x)  (pchMem + ((char *)(x) - pchMMem))
/* Da applicazione ad isaker. */
#define MRTRANS(x) (pchMMem + ((char *)(x) - pchMem))

/* Periodo di polling della funzione plcWait (secondi). */
#define PLC_POLL_TIME 1

/* Elemento della symbol table. */

#ifndef _PLCDB_H

#pragma pack(1)

struct symtab_t {

/* Posizione del nome (nello spazio virtuale del plc). */

	char *name;

/* Puntatore al valore (nello spazio virtuale del plc). */

	char *pval;

/* Dimensione della variabile. */

	char len;
};

#pragma pack()

#endif

/* Struttura descrivente una variabile Isagraf nello spazio virtuale
 del processo utente. */

struct plcvar_t {

/* Posizione del nome (nello spazio virtuale del processo utente). */

	char *name;

/* Puntatore al valore (nello spazio virtuale del processo utente). */

	char *pval;

/* Indice del modulo di appartenenza, o 0 se e solo se la variabile
 e` globale. */

	unsigned int scope;

/* Formato di visualizzazione. Per le variabili booleane, i puntatori
 alle stringhe da visualizzare per rappresentare 0 ed 1, per le variabili
 analogiche il puntatore ad "I" o "F" e quello alla "unit string",
 per i timer un puntatore fisso ad "I" ed uno a "", per i messaggiun
 puntatore ad "S" ed uno a "". */

	char *format[2];

/* Dimensione della variabile. */

	unsigned int size;

/* Tipo di dato rappresentato. */

	char type;

/* Attributo della variabile. */

	char attr;
};

/* 
* Definizione della struttura utilizzata dalla funzione "plcModProp"
* per cominucare i dati al chiamante :
*
*  - quattro byte per l'indirizzo di base ("text"),
*  - quattro byte per l'indirizzo della sezione "data",
*  - quattro byte per l'indirizzo della sezione "bss",
*  - quattro byte per l'indirizzo della sezione "common",
*  - quattro byte per la dimensione ("size"),
*  - duecento byte per il nome del file ("file").
*/

struct modprop_t {
	long text;
	long data;
	long bss;
	long common;
	long size;
	char file[200];
};

/*
* Definizione di un elemento della tabella utilizzata dalla plcMultiMemCheck.
*/

struct plc_mcopy_t {
	char *addr;	/* Indirizzo di un'area. */
	int len;	/* Lunghezza. */
};

/* Definizione degli elemeti della symbol table di QPLC */

#define MAX_SHV_NAME 19

struct qplc_shv_t {

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

};

/****************************************************/
/* Grandezze legate al debug in tempo reale (Qplc). */
/****************************************************/

/* Indici dei registri nell'array "reg" della struttura di stato. */

enum { _R_EDI, _R_ESI, _R_EBP, _R_ESP, _R_EBX, _R_EDX, _R_ECX, _R_EAX };

/* Codici di registro prodotti dal compilatore IL. */

typedef enum _deb_reg_t {
	R_NULL,
	R_EAX, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP, R_ESP
} deb_reg_t;

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
#define WATCH_A_TOOGGLE 0x04 /* FLAGS &= ~opa2 */
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


#define MAX_LOGGED_EVENTS 200         /* Dimensione di $DEB_EVENTS (default). */
#define EVENT_BUFFER_SIZE 30000       /* Dimensione di $DEB_BUFFER (default). */

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
* Questa struttura descrive l'insieme di liste registrate dal plc.
* La funzione "plcLoadDebugHistory" restituisce un puntatore ad una
* struttura di questo tipo.
*/

typedef struct _brk_history {
/* Ciclo di Plc corrispondente al primo degli eventi registrati. */
	unsigned long bh_first_cycle;
/* Ciclo di Plc corrispondente all'ultimo degli eventi registrati. */
	unsigned long bh_last_cycle;
/* Numero di liste di eventi registrate. */
	int bh_list_count;
} brk_history;

/**************/
/* Variabili. */
/**************/

extern char *pchMem;
extern char *pchMMem;
extern char *BF_BOO;
extern long *BF_ANA;
extern long *BF_TMR;

/*
* Funzione plcClose()
* -------------------
*
* Questa funzione scollega dal PLC il processo chiamante.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcClose(void);

/*
* Funzione plcTryAttach(flag)
* -----------------------
*
* Questa funzione collega lo spazio condiviso del PLC al processo chiamante.
* Se "flag" vale 1, il processo chiamante e` bloccato finche` il Plc non ha
* allocato tutte le risorse di sistema necessarie.
* Se "flag" vale 0 ed il Plc non e` ancora partito, la funzione esce
* immediatamente con errore.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcTryAttach(int flag);

/*
* Funzione plcAttach()
* --------------------
*
* Questa funzione collega lo spazio condiviso del PLC al processo chiamante.
* Il processo chiamante e` bloccato finche` il processo plc non ha allocato
* tutte le risorse di sistema necessarie.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcAttach(void);

/*
* Funzione plcLinkStatics()
* -------------------------
*
*  Questa funzione inizializza i puntatori alle aree di plc allocate
* una volta per tutte, quindi con posizione indipendente dal programma
* di plc caricato (o, eventualmente, non caricato !).
*  La chiamata di questa funzione e` sufficiente se si desidera accedere
* alle variabili plc solo tramite il meccanismo dei segnali di scambio.
*  Se invece si vuole accedere a tutti i simboli di Isagraf, e` necessario
* attendere la partenza dell'applicazione plc e chiamare la "plcLinkDymanics".
*
*  La funzione vale 0 in caso di successo, -1 in caso di errore
* (plc non inizializzato).
*/

int plcLinkStatics(void);

/*
* Funzione plcDbChanged()
* -----------------------
*
*  Questa funzione vale 1 se il database e` cambiato rispetto
* all'ultima operazione di link, 0 altrimenti. Se il database
* e` cambiato, essa aggiorna automaticamente la variabile interna
* che registra l'ultima versione. Una volta ottenuta una segnalazione,
* quindi, il risultato di chiamate successive sara` di nuovo 0 !.
*/

int plcDbChanged(void);

/*
* Funzione plcLinkDynamics()
* --------------------------
*
*  Questa funzione inizializza i puntatori alle aree di plc allocate
* dinamicamente ad ogni caricamento di applicazione.
*  Vale 0 in caso di successo, -1 in caso di errore (plc non in start).
*/

int plcLinkDynamics(void);

/*
* Funzione plcReady()
* -------------------
*
*  Questa funzione vale 1 se il Plc e` stabilizzato, cioe` ha terminato la 
* fase di startup, 0 altrimenti.
*/

int plcReady(void);

/*
* Funzione plcWait()
* ------------------
*
*  Questa funzione sinronizza il processo chiamante con il kernel del plc.
*  La sincronizzazione si realizza con un polling di uno dei puntatori alle
* aree statiche globali di Isagraf.
*
*  La funzione vale 0 in caso di successo, -1 in caso di errore
* (plc non inizializzato).
*/

int plcWait(void);

/*
* Funzioni "plcPSem" e "plcVSem"
* ------------------------------
*
* Uso interno.
*/

void plcPSem(int n);

void plcVSem(int n);

/*
* Funzione plcOpen()
* ------------------
*
*  Questa funzione collega lo spazio condiviso del PLC al processo chiamante,
* e inizializza i puntatori alle aree statiche contenenti lo stato del Plc
* e la tabella di accesso ai segnali di scambio. Non inizializza invece i
* puntatori alle aree allocate dinamicamente, poiche` dipendono dal particolare
* programma di plc eventualmente caricato. Le inizializzazioni eseguite da
* questa funzione sono comunque sufficienti a garantire il funzionamento delle
* primitive di accesso ai segnali di scambio.
*
*  Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcOpen(void);


/*
* Funzione plcStatus()
* --------------------
*
* Vale 0 se il Plc e` in HALT, 1 se in GO.
*/

int plcStatus(void);

/*
* Funzione plcGo()
* ----------------
*
* Tenta di far partire il Plc. Vale 1 se il Plc e` partito, 0 altrimenti.
*/

int plcGo(void);

/*
* Funzione plcHalt()
* ------------------
*
* Forza l'Halt del Plc. Vale 1 se l'halt ha avuto successo, 0 altrimenti.
*/

int plcHalt(void);

/*
* Funzione plcSetBScal(), plcSetLScal(), plcSetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBScal(int var,char val);

void plcSetLScal(int var,long val);

void plcSetFScal(int var,float val);

/*
* Funzione plcGetBScal(), plcGetLScal(), plcGetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var + dim*ind"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBScal(int var,char * dest);

void plcGetLScal(int var,long * dest);

void plcGetFScal(int var,float * dest);

/*
* Funzione plcSetBVect(), plcSetLVect(), plcSetFVect()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBVect(int var,int col,char val);

void plcSetLVect(int var,int col,long val);

void plcSetFVect(int var,int col,float val);

/*
* Funzione plcGetBVect(), plcGetLVect(), plcGetFVect()
* -------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBVect(int var,int col,char * dest);

void plcGetLVect(int var, int col,long * dest);

void plcGetFVect(int var,int col,float * dest);

/*
* Funzione plcSetBArr(), plcSetLArr(), plcSetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col +  dim*row" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBArr(int var,int row,int col,char val);

void plcSetLArr(int var,int row,int col,long val);

void plcSetFArr(int var,int row,int col,float val);

/*
* Funzione plcGetBArr(), plcGetLArr(), plcGetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBArr(int var,int row,int col,char * dest);

void plcGetLArr(int var,int row, int col,long * dest);

void plcGetFArr(int var,int row,int col,float * dest);

/*
* Funzione "plcSafeCopyVar"
* -------------------------
*
*  Questa funzione trasferisce "size" byte dal segnale di scambio
* di chiave base "var", con indici di riga e di colonna "row" e "col",
* nel buffer "dest". Sono eseguiti controlli di congruenza dei dati
* della funzione per assicurare che si acceda solo ad indirizzi di
* memoria validi. Se tali controlli rilevano errori, la funzione
* evita di leggere dall'area dei segnali di scambio, azzera la
* destinazione ed esce con valore di ritorno 0. Se non sono rilevati errori,
* la funzione copia i byte richiesti in "dest", e termina con valore di
* ritorno 1.
*  Se "dest" e` il puntatore NULL, la funzione si limita a convalidare
* i dati e riportare il risultato del controllo nel valore di ritorno.
*
* NOTA 1 : Questa funzione non e` in grado di rilevare tutti i possibili
* accessi illegali. In particolare, non e` possibile verificare se un
* indice di riga superi la dimensione del vettore "var", ne` e` possibile
* controllare che la dimensione della destinazione sia compatibile con quella
* della variabile da leggere. Per l'indice di riga, e` possibile solo
* trovare un limite superiore teorico, corrispondente alla dimensione della
* tabella delle variabili. Per quanto riguarda "size", la funzine ammette
* un massimo di 4 byte per la destinazione. Non esistono infatti variabili
* di dimensione maggiore. Cio` che la funzione comunque assicura e` che,
* dati certi indici, non si rischi di accedere ad un indirizzo di memoria
* non valido, cioe` non facente parte dello spazio di indirizzamento del
* processo. Parte del controllo sulla sensatezza logica dell'operazione
* resta a carico del chiamante.
*
* NOTA 2 : L'uso incondizionato di due indici ha senso per tutti i segnali
* di scambio, anche se scalari o vettori monodimensionali. I primi infatti
* possono essere considerati matrici 1x1 (gli indici saranno quindi 0,0),
* i secondi sono matrici Nx1 (il secondo indice sara` quindi 0).
*/

int plcSafeCopyVar(int var,int row,int col,char * dest,int size);

/*
* Funzione "dbQPLC"
* -----------------
*
*  Questa funzione dice se c'e` qplc al posto di isaker.
*/

int dbQPLC(void);

/*
* Funzione "dbFindIsaVar"
* -----------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table.
*  Se il nome non e` presente, riporta NULL.
*/

#if _NO_PROTO
struct symtab_t * dbFindIsaVar();
#else
struct symtab_t * dbFindIsaVar(char *name);
#endif

/*
* Funzione "dbFindQplcVar"
* ------------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table (QPLC).
*  Se il nome non e` presente, riporta NULL.
*  In "row" e "col" sono riportati gli eventuali indici di riga e
* colonna ricavati dal nome secondo le regole di isaker.
*/

struct qplc_shv_t * dbFindQplcVar(char *name, int *row, int *col);


/*
* Funzione "dbRegisterNvram"
* --------------------------
*
*  Questa funzione registra come puntatore alla base della RAM non volatile
* del PLC l'indirizzo dato. Tale indirizzo deve essere ottenuto utilizzando
* la funzione "AttachNvram" (in "libdevcni.a") con "NVRAM_LBL_PLC480" come
* primo parametro (le definizioni utili si trovano in "nvram.h"). Dopo
* la chiamata di questa funzione, le "dbGet..." potranno dare risultati
* corretti anche se applicate a variabili in RAM non volatile.
*/

#if _NO_PROTO
void dbRegisterNvram();
#else
void dbRegisterNvram(void * nvram);
#endif

/*
* Funzione "dbGetQplcVarDescr"
* ----------------------------
*
*  Questa funzione trova l'indirizzo ed il descrittore di una variabile
* di QPLC dato il nome. L'indirizzo del descrittore e` copiato nel
* puntato del parametro "qsym".
*  Se il nome non e` presente, o gli indici contenuti nel noe
* sono fuori dai liiti previsti per la variabile, riporta NULL.
*/

#if _NO_PROTO
char * dbGetQplcVarDescr();
#else
char * dbGetQplcVarDescr(char *name, struct qplc_shv_t ** qsym);
#endif

/*
* Funzione "dbGetVarAddr"
* -----------------------
*
*  Questa funzione trova l'indirizzo di una variabile di Isagraf dato
* il nome.
*  Se il nome non e` presente, riporta NULL.
*/

#if _NO_PROTO
char * dbGetVarAddr();
#else
char * dbGetVarAddr(char *name);
#endif

/*
* Funzione "dbGetVar"
* -------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table, e fornisce
* nella struttura puntata da "res" le caratteristiche della variabile.
*  Se "res" e` NULL, la funzione alloca dinamicamente la struttura, che
* dovra` essere liberata dall'utente dopo l'utilizzo con una "free()".
*  Se il nome non e` presente, riporta NULL.
*/

#if _NO_PROTO
struct plcvar_t * dbGetVar();
#else
struct plcvar_t * dbGetVar(char *name, struct plcvar_t *res);
#endif

/*
* Funzione "plcTell"
* ------------------
*
* Questa funzione realizza il colloquio coll'esecutore PLC Isagraf.
* Utilizza lo stesso protocollo del programma IsaTst.
* Invia il messaggio "msg" (MAX_QUESTION byte alpiu`) ed attende la risposta
* in "answ" per "time_out" millisecondi.
* Il valore di ritorno e` il numero di byte contenuti nella risposta,
* oppure 0 in caso di timeout.
*
* NOTE : Il messaggio e` formato da un byte di comando (i valori possibili
*  si trovano in "grsy0tst.h") seguito da eventuali parametri.
*   La risposta e` costituita da almeno tre byte :
*    0 : il codice di comando impostato nel messaggio trasmesso,
*    1,2 : un codice di errore, memorizzato come short.
*    byte successivi : eventuali dati aggiuntivi.
*/

int plcTell (unsigned char * msg, unsigned char * answ, long timeout);

/*
* Funzione "plcTraceAddr"
* -----------------------
*
*  Questa funzione aggiunge la variabile di indirizzo "addr" all'insieme
* di trace.
*  Vale 1 se tutto e` andato bene, altrimenti 0, ed in "*err" e` scritto
* uno dei seguenti valori :
*
* PLCERR_TOOVAR  : troppe variabili tracciate,
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*
* NOTA 1 : L'indirizzo e` da intendersi relativo allo spazio virtuale del PLC.
* NOTA 2 : Nelle versioni precedenti alla 3.2 di isaker, la validita`
*          dell'indirizzo non e` controllata dall'esecutore, col rischio
*          di provocare il crash dell'esecutore.
*/

int plcTraceAddr(unsigned char * addr);

/*
* Funzione "plcTraceVar"
* ----------------------
*
*  Questa funzione aggiunge la variabile "name" all'insieme di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TOOVAR  : troppe variabili tracciate,
* PLCERR_BADVAR  : variabile non booleana,
* PLCERR_NOVAR   : variabile non trovata,
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceVar(char * name);

/*
* Funzione "plcTraceStart"
* -----------------------
*
*  Questa funzione comunica al plc di iniziare il trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceStart(void);

/*
* Funzione "plcTraceStop"
* -----------------------
*
*  Questa funzione comunica al plc di terminare il trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceStop(void);

/*
* Funzione "plcTraceClear"
* -----------------------
*
*  Questa funzione cancella la lista di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceClear(void);

/*
* Funzione "plcTraceSetBuf"
* ------------------------
*
*  Questa funzione imposta la dimensione del buffer di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcTraceSetBuf(int n);

/*
* Funzione "plcTraceGetBuf"
* ------------------------
*
*  Questa funzione riporta in "*n" la dimensione del buffer di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceGetBuf(int *n);

/*
* Funzione "plcTraceDownLoad"
* ---------------------------
*
*  Questa funzione copia l'ultimo buffer di trace generato, oppure
* esce con errore.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_NOTREADY : non e` ancora pronto il buffer da copiare.
* PLCERR_TIMEOUT  : timeout nella comunicazione col PLC.
* PLCERR_REMERR   : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceDownLoad(TRACERECORD *p);

/*
* Funzione "plcLoadProg"
* ----------------------
*
*  Questa funzione carica dinamicamente un programma di plc dal file
* oggetto (LynxOS a.out) "name".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcLoadProg(char *name);

/*
* Funzione "plcUnloadProg"
* ------------------------
*
*  Questa funzione rimouve l'ultimo programma di plc caricato dinamicamente
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcUnloadProg(void);

/*
* Funzione "plcLdStatus"
* ----------------------
*
*  Questa funzione riporta in "*n" lo stato dell'ultimo caricamento dinamico.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC (NORMALE !).
* PLCERR_REMERR  : errore restituito dal kernel plc.
*
* NOTA : DATA LA LENTEZZA DELL'OPERAZIONE DI CARICAMNETO, L'ERRORE DI TIMEOUT
* MENTRE UN CARICAMENTO E` IN CORSO E` DA CONSIDERARE FISIOLOGICO, E NON DEVE
* PREOCCUPARE.
*/

int plcLdStatus(int *n);

/*
* Funzione "plcMemCopy"
* ---------------------
*
*  Questa funzione copia in "dest" gli "n" byte all'indirizzo "src"
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*
* NOTA : IL PLC NON CONTROLLA LA SENSATEZZA DELLA RICHIESTA. INDIRIZZI
* O DIMENSIONI ERRATE POSSONO PROVOCARE IL CRASH DEL PLC !!!
*/

int plcMemCopy(char *dest,char *src,int n);

/*
* Funzione "plcStatistics"
* ------------------------
*
*  Questa funzione copia in "curr", "min, e "max" la durata corrente, minima
* massima del ciclo di plc.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStatistics(long *curr,long *min, long *max);

/*
* Funzione "plcModProp"
* ---------------------
*
*  Questa funzione copia in "dest" l'immagine della struttura "modprop_t"
* inerente il modulo di nome "name".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcModProp(struct modprop_t *dest,char *name);

/*
* Funzione "plcStStatus"
* ----------------------
*
*  Questa funzione riporta in "*n" lo stato del flag di abilitazione della 
* statistica.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcStStatus(int *n);

/*
* Funzione "plcStEnable"
* ---------------------
*
*  Questa funzione porta il flag di abilitazione della statistica al valore
* indicato in 
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStEnable(int flag);

/*
* Funzione "plcStClear"
* ---------------------
*
*  Questa funzione azzera la statistica.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcStClear(void);

/*
* Funzione "plcGetExeName"
* -----------------------
*
*  Questa funzione copia in "dest" il path dell'eseguibile PLC.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcGetExeName(char *dest);

/*
* Funzione "plcMultiMemCheck"
* ------------------------------
*
*  Questa funzione riporta in "dest" gli "n" flag di abilitazione
* per altrettante coppie indirizzo-lunghezza in "src". L'insieme di
* aree cosi` descritte sara` poi letto dalle successive "plcMultiMemCopy".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcMultiMemCheck(char *dest,struct plc_mcopy_t *src,int n);

/*
* Funzione "plcMultiMemCopy"
* ------------------------------
*
*  Questa funzione riporta in "dest" "n" dei byte impostati con la
* "plcMultiMemCheck".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcMultiMemCopy(char *dest, int n);

/*
* Funzione plcTryOpen()
* ---------------------
*
*  Questa funzione collega lo spazio condiviso del PLC al processo chiamante,
* e inizializza i puntatori alle aree statiche contenenti lo stato del Plc
* e la tabella di accesso ai segnali di scambio. Non inizializza invece i
* puntatori alle aree allocate dinamicamente, poiche` dipendono dal particolare
* programma di plc eventualmente caricato. Le inizializzazioni eseguite da
* questa funzione sono comunque sufficienti a garantire il funzionamento delle
* primitive di accesso ai segnali di scambio.
* NOTA : A differenza della "plcOpen", questa funzione non e` bloccante.
*  Puo` essere chiamata ripetutamente, ed il suo valore di ritorno
* indichera` se il collegamento con il PLC e` stato completato o no.
*
*  Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcTryOpen(void);

/*
* Funzione "plcCreateWatchList"
* ------------------------------
*
*  Questa funzione crea una nuova lista di watch, e riporta in "dest"
* l'ID ottenuto. La lista di watch avra` le caratteristiche date nella
* struttura puntatat da "wprop".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcCreateWatchList(watch_descr_t *wprop,int *dest);

/*
* Funzione "plcDeleteWatchList"
* ------------------------------
*
*  Questa funzione cancella la lista di watch generata con
* "plcCreateWatchList".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcDeleteWatchList(int wid);

/*
* Funzione "plcAddBreak"
* ----------------------
*
*  Questa funzione aggiunge un punto di break la lista di watch generata con
* "plcAddBreak". Vuole l'ID della lista di watch ed un  puntatore ad
* una struttura descrivente le azioni da compiere 
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcAddBreak(int wid,watch_t *brk);

/*
* Funzione "plcEnableWatchList"
* ------------------------------
*
*  Questa funzione abilita la lista di watch generata con
* "plcCreateWatchList".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcEnableWatchList(int wid);

/*
* Funzione "plcDisableWatchList"
* ------------------------------
*
*  Questa funzione disabilita la lista di watch generata con
* "plcCreateWatchList".
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcDisableWatchList(int wid);

/*
* Funzione "plcResetDebugEventList"
* ---------------------------------
*
*  Questa funzione reinizializza le funzioni di lettura dell'area di debug.
*  Vale 0 se tutto e` andato bene, -1 in caso di errori.
*/

int plcResetDebugEventList(void);

/*
* Funzione "plcLastDebugEventList"
* --------------------------------
*
*  Questa funzione carica la lista di eventi di debug prodotta
* nell'ultimo ciclo di Plc, e la copia in un'area allocata internamente
* dalla libreria. Le successive "plcGetDebugEvent" restituiranno in sequenza
* gli eventi contenuti nella lista.
*/

brk_event_list * plcLastDebugEventList(void);

/*
* Funzione "plcGetDebugEvent"
* ---------------------------
*
*  Questa funzione legge il prossimo evento di debug dal buffer, e lo
* copia in un'area allocata internamente dalla libreria.
*  Restituisce il puntatore all'evento letto, oppure NULL.
*/

brk_event * plcGetDebugEvent(void);

/*
* Funzione "plcClearAllWatches"
* -----------------------------
*
*  Questa funzione reinizializza il sistema dei breakpoint.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel PLC.
*/

int plcClearAllWatches(void);

/*
* Questa funzione e` usata internamente per copiare dati dall buffer circolare
* del debugger.
*/

void plcDebugMemCopy(char *dest, unsigned long src, unsigned long n);

/*
* Funzione "plcIndexDebugEventList"
* ---------------------------------
*
*  Questa funzione porta la posizione di lettura degli eventi all'inizio
* della lista di indice dato. L'indice rappresenta la posizione della lista
* nell'insieme di liste caricate nel buffer locale della libreria con la
* funzione "plcLoadDebugHistory".
*  Restituisce il puntatore al descrittore della lista di eventi selezionata,
* oppure NULL in caso di errori.
*/

brk_event_list * plcIndexDebugEventList(int index);

/*
* Funzione "plcLoadDebugHistory"
* ------------------------------
*
*  Questa funzione carica la parte consistente (cioe` non ancora ricoperta)
* del buffer di debug, e la copia in un'area allocata internamente dalla
* libreria. La posizione di lettura degli eventi e` portata all'inizio
* dell'ultima lista caricata.
*  Restituisce il puntatore ad una struttura statica interna che descrive
* l'insieme degli eventi caricati (primo ed ultimo ciclo di plc coinvolti,
* numero di liste presenti), oppure NULL in caso di errori.
*  Se il parametro "p_el" e` diverso da NULL, nella cella puntata sara`
* scaricato il puntatore al descrittore dell'ultima lista trovata, oppure
* NULL se la lista caricata e` uguale alla precedente. Questo valore e` in
* effetti lo stesso che si otterrebbe dalla funzione "plcLAstDebugEventList",
* della quale questa e` di fatto un'estensione.
*/

brk_history * plcLoadDebugHistory(brk_event_list ** p_el);

#define _PLCLINK_H

#endif /* _PLCLINK_H */

