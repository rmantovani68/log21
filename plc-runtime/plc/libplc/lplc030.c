/* lplc030.c */
#include "lplc.h"


/* Nome del file di configurazione. */

char chConfigFile[] = DEFAULT_CONFIGFILE;
char * szConfigFile = chConfigFile;

struct trtel_t * pKeyTab;

/* Indice del blocco di shared memory utilizzato dall'applicazione
 e identificativo associato. */
int iMemKey = DEFAULT_MEMKEY;
int iMemId = -1;

/* Indice del semaforo per la shared memory utilizzato dall'applicazione
 e identificativo associato. */
int iSemKey = DEFAULT_SEMKEY;
int iSemId = -1;

/* Permessi di accesso per le risorse IPC. */
int iSemPerms = DEFAULT_SEMPERMS;
int iMemPerms = DEFAULT_MEMPERMS;

/* Indirizzo del direttorio degli spazi occupati e dei blocchi liberi. */
struct spc_t *spDir = (struct spc_t *)0;
struct spc_t *spFree = (struct spc_t *)0;

/* Indirizzo della tabella di stato del Plc
 (ad uso delle applicazioni esterne). */
long *pStatus = (long *)0;

/* Dimensione ed indirizzo dell'area condivisa. */
long lMemSize = DEFAULT_MEMSIZE;
char *pchMem = (char *)0;
/* Indirizzo dell'area condivisa nello spazio di isaker. */
char *pchMMem = (char *)0;
/* Indirizzo dell'area di RAM non volatile del PLC. */
char *_plclib_nvram = (char *)0;

/* Puntatori alle variabili ed alle strutture di stato di Isagraf. */
str_ker *KER;
str_system *SYST;
char *SYMS;
unsigned char *ANSW;
long *QplcHash;
char *BF_BOO;
long *BF_ANA;
long *BF_TMR;

/* Lunghezza della tabella di corrispondenza. */

int iLenTransTab = MAX_TRTAB_ENTRY;

/* Indirizzo e lunghezza dei simboli di Isagraf. */

char *pchIsaSym = (char *) 0;
long lIsaSymLen = 0;

/* Tabella di hash per l'accesso rapido ai simboli di Isagraf. */

struct plchash_t *pHashTab;

/* Puntatore alla symbol table (allocata dinamicamente). */

struct symtab_t *pSymTab = (struct symtab_t *) 0;

/* Lunghezza della symbol table. */

int iSymTabLen = 0;

/* Indice della prima posizione libera nella symbol table. */

int iSymTabFree = 0;

