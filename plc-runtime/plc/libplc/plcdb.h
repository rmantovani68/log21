
/*
* @(#) plcdb.h 1.3 Mon Dec 18 11:19:25 MET 1995
*
* In questo modulo si costruisce la tabella di associazione tra
* variabili di Isagraf e simboli CNI.
*
* 020295 GG Versione 1.0.
* 020395 GG Versione 1.1. Aggiunto il campo "dim" nella tabella delle chiavi.
* 150595 GG Versione 1.2. Aggiunto le funzioni di trace.
* 181295 GG Versione 1.3. Spostate alcune definizioni in "port.h" per
*           facilitarne la riconfigurabilita`.
*/

#ifndef _PLCDB_H

#define _PLCDB_H

/* Lunghezza massima di una riga nel file di associazione. */

#define MAX_TRTAB_LINELEN 200

/* Numero di chiavi di hash per l'accesso rapido ai simboli Isagraf. */

#define MAX_HASH_KEY 127

/* Codici di errore specifici di questo modulo. */

#define DB_ERR_NOSPC 100
#define DB_ERR_NOFILE 101
#define DB_ERR_SYNTAX 102
#define DB_ERR_RANGE 103
#define DB_ERR_DDEF 104
#define DB_ERR_UNDEF 105
#define DB_ERR_BADISA 106
#define DB_ERR_NOSYM 107
#define DB_ERR_NOMEM 108
#define DB_ERR_TOOEV 109

/* Struttura degli elementi della tabella di associazione. */

#pragma pack(1)	/* Quanto costa la memoria ! */

struct trtel_t {

/* Offset della variabile rispetto all'inizio del blocco di shared memory
 del PLC. E` espresso in byte per ragioni di efficienza. Se l'elemento
 non e` ancora assegnato, questo campo vale 0. */

	long off;

/* Dimensione della variabile. */

	char len;

/* Dimensione (numero di colonne) della matrice di cui e` la base. */

	unsigned short dim;

};

/* Elemento della symbol table. */

struct symtab_t {

/* Posizione del nome nello spazio Isagraf. */

	char *name;

/* Puntatore al valore. */

	char *pval;

/* Dimensione della variabile. */

	char len;
};


/* Elemento della tabella di hash dei simboli di Isagraf. */

struct plchash_t {

/* Puntatore all'insieme di simboli raccolti sotto la stessa chiave. */

	struct symtab_t *set;

/* Dimensione del vettore delle posizioni. */

	int len;

};

#pragma pack()

/* Indirizzo e lunghezza dei simboli di Isagraf. */

extern char *pchIsaSym;
extern long lIsaSymLen;

/* Tabella di hash per l'accesso rapido ai simboli di Isagraf. */

#ifdef LOCAL_SYMTAB
extern struct plchash_t pHashTab[MAX_HASH_KEY+1];
#else
extern struct plchash_t *pHashTab;
#endif

/* Puntatore alla symbol table (allocata dinamicamente). */

extern struct symtab_t *pSymTab;

/* Lunghezza della symbol table. */

extern int iSymTabLen;

/* Indice della prima posizione libera nella symbol table. */

extern int iSymTabFree;

/* Puntatore alla tabella delle chiavi. */

extern struct trtel_t *pKeyTab;

/* Gestione delle variabili con rilevamento di fronte. */

/* Booleane */
extern char **pEdgeBool;	/* Tabella degli indirizzi delle variabili. */
extern char * pImageBool;	/* Tabella degli stati precedenti. */
extern int nEdgeBool;		/* Numero di variabili da tracciare. */
/* Altre (su piattaforma Intel, sono tutte di 4 byte). */
extern long **pEdgeLong;	/* Tabella degli indirizzi delle variabili. */
extern long * pImageLong;	/* Tabella degli stati precedenti. */
extern int nEdgeLong;		/* Numero di variabili da tracciare. */

/* Variabili e definizioni associate al trace. */

/* Tipo rappresentante un campione nel buffer di trace. */
typedef unsigned long TRACERECORD;

/* Numero massimo di variabili tracciabili. */
#define MAXTRACEVAR (sizeof(TRACERECORD) * 8)

/* Dimensione del buffer di trace. */
/* NOTA : ESAGERARE NON SERVE ! */
#define MAXTRACESIZE 800 /* 400+400 campioni. */

/* Stato del trace. */
struct tracests_t {
	/* Flag di trace attivo. */
	int active;
	/* Numero di variabili sorvegliate. */
	int nvar;
	/* Posizione di scrittura nel buffer. */
	int position;
	/* Posizione dell'ultimo buffer interamente riempito. */
	int lastbuff;
	/* Flag indicante che e` stata completata la scrittura di uno dei buffer. */
	int completed;
	/* Dimensione di un (sotto)buffer. */
	int dimbuff;
	/* Dimensione complessiva del buffer. */
	int dimtot;
	/* Posizione del buffer corrente. */
	int currbuff;
	/* Numero di record immagazzinati nel (sotto)buffer corrente. */
	int relpos;
};


/* Tabella dei puntatori alle variabili sorvegliate. */
extern char * pTraceTab[MAXTRACEVAR];
/* Buffer di trace. */
extern TRACERECORD TraceBuffer[MAXTRACESIZE];
/* Struttura di stato del trace. */
extern struct tracests_t trcStatus;

/*
* MACRO PER L'ACCESSO VELOCE ALLA TABELLA DELLE CHIAVI
* ----------------------------------------------------
*/
/*
* Queste macro si sostituiscono alle omonime funzioni della libreria
* di accesso al Plc da programma esterno.
* L'effetto e` imprevedibile se la variabile Plc referenziata non esiste.
*
* La definizione di queste macro non avviene se e` definito "_USE_PLC_FUNC".
*/

#ifndef _USE_PLC_FUNC

#define plcSetBScal(id,val) *(char *)(pchMem + pKeyTab[id].off) = (val) ? 1 : 0
#define plcSetLScal(id,val) *(long *)(pchMem + pKeyTab[id].off) = (val)
#define plcSetFScal(id,val) *(float *)(pchMem + pKeyTab[id].off) = (val)
#define plcGetBScal(id,dest) \
	*(char *)(dest) = *(char *)(pchMem + pKeyTab[id].off)
#define plcGetLScal(id,dest) \
	*(long *)(dest) = *(long *)(pchMem + pKeyTab[id].off)
#define plcGetFScal(id,dest) \
	*(float *)(dest) = *(float *)(pchMem + pKeyTab[id].off)

#define plcSetBVect(id,col,val) \
	*(char *)(pchMem + pKeyTab[(id)+(col)].off) = (val) ? 1 : 0
#define plcSetLVect(id,col,val) \
	*(long *)(pchMem + pKeyTab[(id)+(col)].off) = (val)
#define plcSetFVect(id,col,val) \
	*(float *)(pchMem + pKeyTab[(id)+(col)].off) = (val)
#define plcGetBVect(id,col,dest) \
	*(char *)(dest) = *(char *)(pchMem + pKeyTab[(id)+(col)].off)
#define plcGetLVect(id,col,dest) \
	*(long *)(dest) = *(long *)(pchMem + pKeyTab[(id)+(col)].off)
#define plcGetFVect(id,col,dest) \
	*(float *)(dest) = *(float *)(pchMem + pKeyTab[(id)+(col)].off)

#define plcSetBArr(id,row,col,val) \
	*(char *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off) = (val) ? 1 : 0
#define plcSetLArr(id,row,col,val) \
	*(long *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off) = (val)
#define plcSetFArr(id,row,col,val) \
	*(float *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off) = (val)
#define plcGetBArr(id,row,col,dest) \
	*(char *)(dest) = \
	*(char *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off)
#define plcGetLArr(id,row,col,dest) \
	*(long *)(dest) = \
	*(long *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off)
#define plcGetFArr(id,row,col,dest) \
	*(float *)(dest) = \
	*(float *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off)

#endif /* _USE_PLC_FUNC */

/*
* FUNZIONI DI ACCESSO ALLA TABELLA DEI SIMBOLI
* --------------------------------------------
*/

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
* Funzione "dbGetVarAddr"
* -----------------------
*
*  Questa funzione trova l'indirizzo di una variabile di Isagraf dato
* il nome.
*  Se il nome non e` presente, riporta NULL.
*
*  Il programmatore C dovrebbe utilizzare sempre questa funzione per
* conoscere l'indirizzo di una variabile Isagraf.
*/

#if _NO_PROTO
char * dbGetVarAddr();
#else
char * dbGetVarAddr(char *name);
#endif

/*
* FUNZIONI DI CREAZIONE DELLE STRUTTURE ASSOCIATE AI SIMBOLI
* ----------------------------------------------------------
*/

/*
* Funzione "dbCreateSymTab"
* -------------------------
*
*  Questa funzione costruisce una tabella per l'accesso rapido
* ai simboli di Isagraf. Ogni elemento della tabella e` un puntatore
* ad un vettore di posizioni nello spazio dei simboli. Nella fase di
* preparazione della tabella, la stessa area e` utilizzata per contenere
* il numero di elementi associati ad ogni chiave.
*  I vettori di posizione sono allocati dinamicamente ad ogni creazione.
*
*  Questa funzione dovrebbe essere chiamata ogni volta che sono ricaricati
* i simboli da Isagraf.
*/

#if _NO_PROTO
int dbCreateSymTab();
#else
int dbCreateSymTab(void);
#endif

/*
* Funzione "dbAllocStatics"
* -------------------------
*
*  Questa funzione alloca tutte le aree statiche, cioe` indipendenti
* dal particolare programma di Plc, relative alla gestione dei simboli
* e dei segnali di scambio. Per la migliore utilizzazione dell'area di
* memoria condivisa, tale allocazione dovrebbe avvenire alla partenza del
* processo, prima di qualsiasi allocazione dinamica di spazi di dimensione
* variabile. Questo assicura, tra l'altro, che la posizione della tabella
* non vari al variare dell'applicazione plc caricata di volta in volta.
*
*  La funzione vale 0 in caso di successo, -1 in caso di errore.
*/

#if _NO_PROTO
int dbAllocStatics();
#else
int dbAllocStatics(void);
#endif

/*
* Funzione "dbCreateKeyTab"
* -------------------------
*
*  Creazione della tabella di corrispondenza tra i simboli Isagraf e
* le costanti CNI.
*  La funzione utilizza lo spazio SPC_TBL e lo riempie con la tabella
* di corrispondenza, basandosi sul database dei simboli e sul file
* delle corrispondenze.
*
*  Questa funzione dovrebbe essere chiamata ogni volta che sono ricaricati
* i simboli dell'applicazione Isagraf, previo lancio della "dbCreateSymTab".
*/

#if _NO_PROTO
int dbCreateKeyTab();
#else
int dbCreateKeyTab(void);
#endif

/*
* Funzione "dbTrace"
* ------------------
*
*  Questa funzione memorizza lo stato delle variabili di trace selezionate
* in un apposito buffer interno, ed eventualmente comunica la fine dell'ope-
* razione al debugger.
*/

#if _NO_PROTO
void dbTrace();
#else
void dbTrace(void);
#endif

/*
* Funzione "dbTraceSignal"
* ------------------------
*
*  Questa funzione spegne il flag di "caricamento dcompletato", che la
* "dbTrace" accende quando e` riempito uno dei due buffer di trace.
*  Dovrebbe essere chiamata dal client alla fine del download di un buffer,
* prima di mettersi in attesa di un nuovo caricamento.
*/

#if _NO_PROTO
void dbTraceSignal();
#else
void dbTraceSignal(void);
#endif

/*
* Funzione "dbTraceStart"
* -----------------------
*
*  Questa funzione predispone il plc ad iniziare il trace.
*  Lo stato del trace viene resettato.
*/

#if _NO_PROTO
void dbTraceStart();
#else
void dbTraceStart(void);
#endif

/*
* Funzione "dbTraceStop"
* ----------------------
*
*  Questa funzione interrompe il trace del plc.
*  Lo stato del trace (buffer, in particolare) non e` alterato
* da quest'operazione.
*/

#if _NO_PROTO
void dbTraceStop();
#else
void dbTraceStop(void);
#endif

/*
* Funzione "dbTraceStatus"
* ------------------------
*
*  Questa funzione copia lo stato del trace all'indirizzo "p".
*  Restituisce il numero di byte copiati.
*/

#if _NO_PROTO
int dbTraceStatus();
#else
int dbTraceStatus(char * p);
#endif

/*
* Funzione "dbTraceAddVar"
* ------------------------
*
*  Questa funzione aggiunge l'indirizzo "p" alla lista di variabili
* da tracciare.
*  La funzione vale 1 se tutto e` andato bene, 0 altrimenti.
*/

#if _NO_PROTO
int dbTraceAddVar();
#else
int dbTraceAddVar(char *p);
#endif

/*
* Funzione "dbTraceClearList"
* ---------------------------
*
*  Questa funzione vuota la lista di trace.
*/

#if _NO_PROTO
void dbTraceClearList();
#else
void dbTraceClearList(void);
#endif

/*
* Funzione "dbTraceCopy"
* ----------------------
*
*  Questa funzione copia n record dalla posizione di trace "pos"
* all'indirizzo "p".
*  Restituisce la dimensione dell'area copiata in byte.
*/

#if _NO_PROTO
int dbTraceCopy();
#else
int dbTraceCopy(TRACERECORD * p,int pos,int n);
#endif

/*
* Funzione "dbTraceSetBuf"
* ------------------------
*
*  Questa funzione imposta la dimensione dei buffer.
*  Vale 1 se tutto va bene, 0 in caso di errore.
*/

#if _NO_PROTO
int dbTraceSetBuf();
#else
int dbTraceSetBuf(int n);
#endif

#endif /* _PLCDB_H */

