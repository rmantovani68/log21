/*
* @(#) plcapi.h 2.0 Thu Jul 17 12:57:57 MET DST 1997
*
*  Definizioni ad uso del programmatore C, in particolare di chi
* scrive programmi in stile PLC 480.
*  Questo file deriva dai vecchi "isagraf.h" e "cniplc.h". Sono state eliminate
* molte definizioni obsolete, e sono stati aggiunti i prototipi di alcune
* funzioni contenute nel kernel dell'esecutore.
*
* 14/01/97 GG 1.0
* 04/02/97 GG 1.1 Aggiunte definizioni legate al rivelatore di fronti
*             e alla modalita` di lancio (uso Lelli).
* 06/02/97 GG 1.2 Aggiunta la definizione della funzione plcTimeH.
* 19/02/97 GG 1.3 Aggiunta la definizione della variabile "NVRAM_pub",
*             equivalente ad "NVRAM", ma... consigliabile.
* 13/05/97 GG 1.4 Aggiunta la definizione della funzione "ioAttach", sperando
*             che BiEsse si converta...
* 23/05/97 GG 1.5 Aggiunte le funzioni "plcHaltCycle" e "plcRepeatHalt". La
*             prima da` il numero di ciclo di HALT corrente (0 e` il primo),
*             la seconda prenota l'esecuzione di un nuovo ciclo di HALT. La
*             chiamata di queste funzioni ha senso solo nelle procedure di
*             entrata in HALT (es. "UserStop") dei programmi utente. La prima
*             permette di sapere se la procedura e` stata gia` chiamata,
*             la seconda per richiedere la riesecuzione di tutte le procedure
*             di HALT nel ciclo successivo. Tutte le procedure di HALT devono
*             essere scritte in modo da prevedere di essere chiamate piu` di
*             una volta nell'ambito dello STESSA fase di entrata in HALT
*             dell'esecutore.
* 30/05/97 GG 1.6 Aggiunta la definizione di "shvDefine" e delle macro
*             associate. Funzione come "shvCreate", salvo che "*change" e`
*             un campo di flag che indicano quali cambiamenti siano stati
*             rilevati. Usare le macro SHV_CHG_... per interpretare i bit.
* 03/06/97 GG 1.7 Aggiunta la definizione della funzione "shvKeyToAddr".
*             Aggiunte le definizioni per la proprieta` "RESET" dei segnali
*             di scambio.
* 03/06/97 GG 1.8 Aggiunte le funzione "shvSet" ed "shvGet".
* 11/06/97 GG 1.9 Aggiunta la funzione "plcUpdateMessage", che aggiorna
*             un messaggio anziche` accodarlo a quelli gia` presenti
*             (ma ci vuole la versione 2.6.0.15 di quote).
* 17/07/97 GG 2.0 Aggiunti i prototipi di alcune funzioni utili per lo
*             sviluppatore di PLC in C (util_safe_sprintf...).
*             Aggiunte le definizioni per il colloquio seriale.
*/

#ifndef _PLCAPI_H_

#include <stdarg.h>

typedef void * Sh_Address;

/*********************************************************\
* Definizioni legate ai segnali di scambio (da "shvar.h") *
\*********************************************************/

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


/* 
* Definizione dei flag indicanti differenza di proprieta` nella creazione
* di un segnale di scambio. La maschera di bit restituita nel parametro
* "change" di "shvCreate" si interpreta utilizzando queste macro.
*/

#define SHV_CHG_SIZE   0x01 /* Diversa grandezza in bit del tipo. */
#define SHV_CHG_DIM1   0x02 /* Prima dimensione diversa. */
#define SHV_CHG_DIM2   0x04 /* Seconda dimensione diversa. */
#define SHV_CHG_KEY    0x08 /* Chiave diversa da quella richiesta. */
#define SHV_CHG_EDGE   0x10 /* Proprieta` EDGE diversa. */
#define SHV_CHG_RETAIN 0x20 /* Proprieta` RETAIN diversa. */
#define SHV_CHG_CONTIG 0x40 /* Proprieta` CONTIG diversa. */
#define SHV_CHG_TYPE   0x80 /* Tipi non compatibili. */
#define SHV_CHG_ITYPE  0x100 /* Tipi non identici (puo` non essere grave). */
#define SHV_CHG_RESET  0x200 /* Flag di RESET precedentemente impostato. */

#ifndef _SHVAR_H_

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

#endif

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


/*********************************************\
* Definizioni inerenti al caricatore dinamico *
\*********************************************/

/*
* Funzioni di start e stop definite dall'utente.
*/

/* Se "UserStart" vale 0, l'azione di start e` interrotta, e l'esecutore
 emette "code" come messaggio di errore. */

extern int UserStart(int * code);

extern void UserStop(void);

/*
* La routine di aggancio con Isagraf C dovra` avere questo prototipo.
*/

extern int UserMain(int start);

/*
* Definizioni ad uso di "hatsh"
* -----------------------------
*/

extern int iStandAlone;
extern char *pImageBool;
extern char **pEdgeBool;
extern int nEdgeBool;
extern int nMaxEdgeBool;
extern long *pImageLong;
extern long **pEdgeLong;
extern int nEdgeLong;
extern int nMaxEdgeLong;

/*
* DEFINIZIONI ASSOCIATE ALLA LETTURA DEI DATI MACCHINA
* ----------------------------------------------------
*/

/* Handle per la comunicazione col server dei dati macchina. */

extern char * MData;

/* Puntatore all'handle di accesso agli assi. */

extern Sh_Address **pCpaxShm;

/* Macro di comodo per la "read_param", cosi` si evitano scazzi antipatici. */

#define CPAXSHM (*pCpaxShm)

/* Puntatore alla zona di RAM non volatile riservata al PLC.
 Vale NULL se il dispositivo e` assente. */

extern unsigned char * NVRAM; /* Vecchia definizione, obsoleta. */
extern unsigned char * NVRAM_pub; /* Nuova definizione, evita conflitti
				     di nomi IL. */

/*
* DEFINIZIONI UTILI PER LA MANIPOLAZIONE DIRETTA DELLE SCHEDE IOS
* ---------------------------------------------------------------
*/

#define MAXIOSDEV 8

/*
* Vettore contenente i puntatori ai blocchi di memoria
* associati a ciascun dispositivo CNI IOS.
*/

extern unsigned char *CNI_iosbrd[MAXIOSDEV];

/*
* Matrice dei controllori installati. Se un controllore e` assente,
* l'elemento corrispondente vale NULL.
*/

extern unsigned char **CNI_ios[MAXIOSDEV];

/*
* Definizione del puntatore all'area condivisa.
*/

extern char * pchMem;

/*
* PRIMITIVE DI ACCESSO AI SEGNALI DI SCAMBIO
*/

/* Purtroppo questo e` necessario. Eredita` di Isagraf. */
#pragma pack(1)

/* Struttura di un descrittore di segnale di scambio. */

struct trtel_t {
/* Offset della variabile rispetto all'inizio del blocco di shared memory
 del PLC. E` espresso in byte per ragioni di efficienza. Se l'elemento
 non e` ancora assegnato, questo campo vale 0. */
	long off;
/* Campo da non considerare. Puo` contenere il vaolre della variabile. */
	char len;
/* Dimensione (numero di colonne) della matrice di cui e` la base. */
	unsigned short dim;
};

#pragma pack()

/* Puntatore alla tabella di conversione dei segnali di scambio. */

extern struct trtel_t * pKeyTab;

/*
* Queste macro si sostituiscono alle omonime funzioni della libreria
* di accesso al Plc da programma esterno.
* L'effetto e` imprevedibile se la variabile Plc referenziata non esiste.
*/

#define plcSetBScal(id,val) *(char *)(pchMem + pKeyTab[id].off) = (val)
#define plcSetLScal(id,val) *(long *)(pchMem + pKeyTab[id].off) = (val)
#define plcSetFScal(id,val) *(float *)(pchMem + pKeyTab[id].off) = (val)
#define plcGetBScal(id,dest) \
	*(char *)(dest) = *(char *)(pchMem + pKeyTab[id].off)
#define plcGetLScal(id,dest) \
	*(long *)(dest) = *(long *)(pchMem + pKeyTab[id].off)
#define plcGetFScal(id,dest) \
	*(float *)(dest) = *(float *)(pchMem + pKeyTab[id].off)

#define plcSetBVect(id,col,val) \
	*(char *)(pchMem + pKeyTab[(id)+(col)].off) = (val)
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
	*(char *)(pchMem + pKeyTab[(id)+(col)+pKeyTab[id].dim*(row)].off) = (val)
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

/*
* Definizioni associate alle primitive di comunicazione
* -----------------------------------------------------
*/

/* COMANDI PER LA PRIMITIVA "ioExtendedOp" */

/* Primitive per dispositivi di tipo "stream" (seriali, socket, pipe...)
 e strutture associate. */

#ifndef DREXTOP_STREAM_OP_SEND /* Trucco per compilazione interna di qplc */

/* Valori da dare al parametro "cmd" di "ioExtenbdedOp". */
#define DREXTOP_STREAM_OP_SEND 0x01 /* Trasmissione. */
#define DREXTOP_STREAM_OP_RECV 0x02 /* Ricezione. */

/* Struttura da utilizzare con i comandi SEND e RECV. Il terzo parametro
 di "ioExtendedOp" deve essere un puntatore ad un oggetto di questo tipo. */
typedef struct _stream_rxtx_t {
	int msg_size;     /* Numero di byte da trasmettere o ricevere. */
	int msg_curr_pos; /* Numero di byte trasmessi o ricevuti finora. */
			  /* Deve essere azzerato la prima volta. */
	int status;       /* Vari flag di stato. */
	char *message;    /* Puntatore al buffer. */
} stream_rxtx_t;

/* Significato del campo "status" della struttura. */
#define DREXTOP_STREAM_F_DONE 0x0001 /* Operazione completata. */
#define DREXTOP_STREAM_F_ERRS 0x0002 /* Errore generico. */

#endif

/*
* Funzione "ioExtendedOp"
* -----------------------
*
* Questa funzione deve essere utilizzata per accedere a servizi specifici
* di certi dispositivi che non possono per loro natura rientrare nel
* modello classico dei segnali di I/O del PLC (lettura-esecuzione-scrittura).
* Un esempio di dispositivo di questo tipo e` la linea seriale.
* Il parametro "ex" e` un valore a 32 bit che si ottiene leggendo la
* variabile che e` stata associata al dispositivo con "ioAttach".
* Il parametro "cmd" e` un codice che individua lo specifico servizio
* al quale si vuole accedere (per esempio DREXTOP_STREAM_OP_SEND per
* trasemttere un messaggio su di una linea seriale).
* Il parametro "user" e` interpretato diversamente a seconda del servizio.
* Per esempio, nel caso di trasmissioni o ricezioni seriali
* sara` un puntatore ad un oggetto di tipo "stream_rxtx_t";
* La funzione vale 1 se tutto e` andato bene, 0 in caso di errori (gravi,
* cioe` servizio inesistente o codice "ex" sconosciuto).
*/

int ioExtendedOp(long ex, int cmd, void * user);

/*
* Funzione "ioAttach"
* --------------------
*
*  Collega una variabile di indirizzo e dimensione dati ad un
* segnale di input o di output ("mode" decide). La variabile e` descritta
* da una stringa nella forma "nome.istanza.altro...", dove "nome" e` il nome
* del driver, "istanza" e` il numero di istanza dello stesso, "altro" e` una
* sequenza di caratteri qualsiasi, la cui interpretazione e` a carico del
* driver.
*  "size" e` la dimensione della variabile associata IN BIT (e vale 1
* per variabili di un byte interpretate come valori booleani).
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioAttach(char * name, void * addr, int size, int mode);

/* Possibili valori del parametro "mode". */

#define DRIVER_MODE_INPUT 0x01
#define DRIVER_MODE_OUTPUT 0x02

/*
* La routine di aggancio all'inizializzazione del pacchetto di
* compatibilita` col PLC 480 dovra` avere questo prototipo.
*/

extern void PlcInit(void);

/*
* La routine di aggancio con il pacchetto di compatibilita` col PLC 480
* dovra` avere questo prototipo.
*/

extern void PlcMain(void);

/*
* Tabelle di corrispondenza. La loro costruzione e` a carico
* dell'utente. Si vedano le macro definite piu` avanti.
*/

/* Definizione degli input. */
extern unsigned int MSD_input[];
/* Definizione degli output. */
extern unsigned int MSD_output[];

/*
* Macro per la costruzione della tabella di corrispondenza tra I/O
* fisici e registri.
*/

/* Inizio della tabella degli input */
#define BEGIN_MSD_INPUT unsigned int MSD_input[] = {

/* Inizio della tabella degli output */
#define BEGIN_MSD_OUTPUT unsigned int MSD_output[] = {

/* Definizione di un elemento di una tabella :
	IOREG(n. scheda IOSPC, n. controller, n. modulo, n. port sul modulo. */
/* Convenzioni : Indice scheda IOSPC * 0x8000 + indice controller * 0x800 +
		 + 8*indice modulo + posizione della porta. */
#define IOREG_V(x,y,z,t) ((x)*0x8000+(y)*0x800+(z)*8+(t))
#define IOREG(x,y,z,t) IOREG_V(x,y,z,t),

/* Codice di fine tabella (solo per uso interno). */
#define _END_MSD (0xFFFFFFFFu)

/* Fine tabella. */
#define END_MSD _END_MSD };

/*
* Macro per il calcolo dell'indirizzo dello spazio di memoria
* associato ad un I/O fisico, utili qualora si desiderasse accedervi
* direttamente (SCONSIGLIATO).
*/

#define IOSBOARD_ADDR(n) (CNI_iosbrd[(n)])
#define IOSCONTROLLER_ADDR(n,m) (CNI_ios[(n)][m])
#define IOSMODULE_ADDR(n,m,o) (IOSCONTROLLER_ADDR(n,m) + 8*(o))
#define IOSREG_ADDR(n,m,o,p) (IOSMODULE_ADDR(n,m,o) + (p))


/* Variabili definite per compatibilita` col PLC 480. */
extern unsigned char regA[0700][010];
extern unsigned char regB[0200][010];
extern unsigned char regC[0600];
extern unsigned int first_nc_plc;
extern unsigned char num_ss_nc_plc;
extern unsigned int first_plc_nc;
extern unsigned char num_ss_plc_nc;
extern unsigned int first_input;
extern unsigned char num_input;
extern unsigned int first_output;
extern unsigned char num_output;
extern unsigned int first_const;
extern unsigned char num_const;
extern unsigned int tim_att;
extern unsigned int tim_old;
extern unsigned char qualeloop;

/*
* VARIE FUNZIONI DI UTILITA`
*/

/*
* Funzione "plcHalt"
* ------------------
*
*  Questa funzione mette in HALT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzare l'HALT.
*/

void plcHalt(void);

/*
* Funzione "plcFault"
* -------------------
*
*  Questa funzione mette in FAULT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzare il FAULT. L'effetto dell'entrata in FAULT
* e` un halt irreversibile. 
*/

void plcFault(void);

/*
* Funzione "plcError"
* -------------------
*
*  Questa funzione visualizza un errore di codice "code" e testo "format".
*  "format" ha una forma simile a quella di "printf". Si veda la funzione
* "util_safe_vsprintf" per i dettagli.
*/

void plcError(long code, char * format, ...);

/*
* Funzione plcRuntimeError
* ------------------------
*
* Questa funzione emette il codice di errore specificato, e aggiunge come
* commento il valore "pc" del program counter. Tale valore e` confrontato
* con gli indirizzi dei moduli caricati dinamicamente e, se appartiene
* a qualcuno di questi, e` visualizzato come "nome_modulo+offset", per
* facilitare il debug.
*  Si usi "util_get_pc" per ottenere il valore del program counter.
*/

void plcRuntimeError(long code, unsigned long pc);

/*
* Funzione "plcDelerr"
* ---------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare, dipende dalla
* versione del processo "quote" sull'XNC) di codice "code".
*/

void plcDelerr(long code);

/*
* Funzione "plcWarning"
* ---------------------
* 
*  Questa funzione visualizza un "warning" di codice "code" e formato "format".
*/

void plcWarning(long code, char * format, ...);

/*
* Funzione "plcMessage"
* ---------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e formato "format".
*/

void plcMessage(long code, char * format, ...);

/*
* Funzione "plcUpdateMessage"
* ---------------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text",
* sovrascrivendo l'eventuale precedente messaggio con lo stesso codice.
*/

void plcUpdateMessage(long code, char * format, ...);

/*
* Funzione "plcDelmess"
* ---------------------
*
*  Cancella un messaggio (ammesso che la cosa si possa fare) di codice "code".
*/

void plcDelmess(long code);

/*
* Funzione "plcPeriod"
* --------------------
*
*  Questa funzione restituisce il periodo del ciclo di esecuzione
* in millisecondi.
*/

long plcPeriod(void);

/*
* Funzione "plcTime"
* ------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi.
*/

unsigned long plcTime(void);

/*
* Funzione "plcTimeH"
* -------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi (32 bit piu` significativi).
*/

unsigned long plcTimeH(void);

/*
* Funzione "plcNCycle"
* --------------------
*
*  Questa funzione restituisce il numero di ciclo corrente.
*/

unsigned long plcNCycle(void);

/*
* Funzione "util_safe_ltob"
* -------------------------
*
*  Su Lynx non c'e` "ltob". Inoltre, sprintf usa malloc. E se non la usasse,
* non si sarebbe comunque sicuri di prenderci con la lunghezza della stringa.
*/

int util_safe_ltob(char *buff, int bsize, long v, int b);

/*
* Funzione "util_safe_gcvt"
* -------------------------
*
*  Versione di "gcvt" che non esce dai limiti della stringa destinazione.
*/

int util_safe_gcvt(char *buff, int bsize, double v, int b);

/*
* Funzione "util_safe_vsprintf"
* ----------------------------
*
*  Questa funzione realizza una mini-vsprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria. Sono riconosciuti i formati
* s,c,o,x,d,u,f e b. Quest'ultimo visualizza un numero in base 2. Non
* sono riconosciuti i prefissi "h" ed "l", ne` quelli che fissano la larghezza
* minima del campo.
*/

char * util_safe_vsprintf(char *dest, int bsize, char *fmt, va_list args);

/*
* Funzione "util_safe_sprintf"
* ----------------------------
*
*  Questa funzione realizza una mini-sprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria.
*/

char * util_safe_sprintf(char *dest, int bsize, char *fmt, ...);

/*
* Funzione "util_get_pc"
* ----------------------
*
*  Questa funzione restituisce il PC del punto di chiamata del chiamate.
*  "level" indica quanti livelli di chiamata si vogliono ignorare.
*/

unsigned long util_get_pc(int level);

/*
* Funzione "util_call_il"
* ----------------------
*
*  Questa funzione serve a lanciare un programma o un blocco funzionale IL.
*  "prog" e` l'indirizzo del blocco, "statics" e` l'indirizzo della
* struttura contenente le variabili di stato ed i parametri del blocco.
*/

unsigned long util_call_il(void (*prog)(void *statics), void *statics);

/*
* Funzione "plcHaltCycle"
* -----------------------
*
*  Questa funzione restituisce il numero di cicli dell'esecutore dal momento
* dell'entrata in HALT. La sua chiamata ha senso solo all'interno dei metodi
* "down" dei driver e dei metodi "Stop" dei programmi PLC. Il primo ciclo di
* HALT e` lo 0. Lo scopo di questa funzione e` permettere alle procedure di
* HALT di capire se siano state richiamate a seguito della richiesta di
* ripetizione di qualche altra componente.
*/

unsigned long plcHaltCycle(void);

/*
* Funzione "plcRepeatHalt"
* ------------------------
*
*  Questa funzione inoltra la richiesta di riesecuzione delle procedure
* di HALT da parte dell'esecutore al ciclo successivo. E` utile quando,
* in una procedura di HALT, si debbano compiere azioni che richiedano piu`
* di un ciclo di PLC per essere eseguite.
*/

void plcRepeatHalt(void);

#define _PLCAPI_H_

#endif

