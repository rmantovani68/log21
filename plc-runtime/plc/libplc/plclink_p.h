/*
* @(#) plclink_p.h 2.2 Thu Jul  3 11:54:16 MET DST 1997
*
* Definizioni non di interesse pubblico per la libreria di colloquio
* col plc.
*
* --/--/-- 1.0. Prima stesura.
* 15/05/95 1.1. Versione stabilizzta.
* 25/06/96 1.2. Aggiunte modifiche per connessione con QPLC.
* 28/11/96 GG 1.3 Aggiunto il puntatore alla RAM non volatile.
* 16/12/96 GG 2.0 Definizioni per il debug in tempo reale.
* 17/12/96 GG 2.1 Aggiunto un comando per cancellare tutti i watchpoint.
* 03/07/97 GG 2.2 Aggiunte nuove definizioni per nuove prestazioni di
*             debug.
*/

/* Comandi per il debug in tempo reale. */

#define R_CREATE_WATCH 182	/* Crea una lista di watch (ottiene l'ID). */
#define R_DELETE_WATCH 183	/* Cancella una lista di watch dato l'ID. */
#define R_ADD_BREAK 184		/* Aggiunge un punto di break al watch. */
#define R_ENABLE_WATCH 185	/* Abilita la lista di watch dato l'ID. */
#define R_DISABLE_WATCH 186	/* Disabilita la lista di watch dato l'ID. */
#define R_CLEARALL_WATCH 187	/* Reinizializza il sistema di watch. */

/* Struttura interna contenente lo stato del lettore di eventi di debug. */

struct _plc_get_event_t {
	unsigned char *deb_buffer;
	unsigned char *buffer;
	unsigned long size;
	unsigned long nevt;
	unsigned long * events;
	unsigned long * lastev;

	unsigned long * lastevp;
	unsigned long * firstevp;
	unsigned long * evcount;
	unsigned long * nregev;

	unsigned long cycle;
	unsigned long event_end;
	unsigned long read_pos;
	int cycle_ok;

	int n_lists;
	brk_event_list **lists;
	brk_history history;
};

extern struct _plc_get_event_t plc_get_event_status;

/* Macro di utilita`. */

#define MATCH(s1,s2) (strncmp(s1,s2,strlen(s2)) == 0)

#define PLC_WAIT_TIMEOUT 1

/* Nome del file di configurazione. */

extern char chConfigFile[];
extern char * szConfigFile;

extern struct trtel_t * pKeyTab;

/* Indice del blocco di shared memory utilizzato dall'applicazione
 e identificativo associato. */
extern int iMemKey;
extern int iMemId ;

/* Indice del semaforo per la shared memory utilizzato dall'applicazione
 e identificativo associato. */
extern int iSemKey;
extern int iSemId;

/* Permessi di accesso per le risorse IPC. */
extern int iSemPerms;
extern int iMemPerms;

/* Indirizzo del direttorio degli spazi occupati e dei blocchi liberi. */
extern struct spc_t *spDir;
extern struct spc_t *spFree;

/* Indirizzo della tabella di stato del Plc
 (ad uso delle applicazioni esterne). */
extern long *pStatus;

/* Dimensione ed indirizzo dell'area condivisa. */
extern long lMemSize;
extern char *pchMem;
/* Indirizzo dell'area condivisa nello spazio di isaker. */
extern char *pchMMem;
/* Indirizzo dell'area di RAM non volatile del PLC. */
extern char *_plclib_nvram;

/* Puntatori alle variabili ed alle strutture di stato di Isagraf. */
extern str_ker *KER;
extern str_system *SYST;
extern char *SYMS;
extern unsigned char *ANSW;
extern long *QplcHash;
extern char *BF_BOO;
extern long *BF_ANA;
extern long *BF_TMR;

/* Lunghezza della tabella di corrispondenza. */

extern int iLenTransTab;

/* Indirizzo e lunghezza dei simboli di Isagraf. */

extern char *pchIsaSym;
extern long lIsaSymLen;

/* Tabella di hash per l'accesso rapido ai simboli di Isagraf. */

extern struct plchash_t *pHashTab;

/* Puntatore alla symbol table (allocata dinamicamente). */

extern struct symtab_t *pSymTab;

/* Lunghezza della symbol table. */

extern int iSymTabLen;

/* Indice della prima posizione libera nella symbol table. */

extern int iSymTabFree;

