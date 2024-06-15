/*
* @(#) libplc.c 2.6 Wed Jan  7 11:15:33 MET 1998
*
* Primitive di colloquio col PLC Isagraf.
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
* 31/07/96 1.14. Corretto un buco nella ricerca dei nomi in stile QPLC :
*          si esplorava erroneamente la lista globale anziche` la lista
*          di hash. Inoltre e` stato corretta l'inversione delle stringhe
*          "TRUE" e "FALSE" nelle proprieta` dei nomi di qplc.
* 28/11/96 GG 1.15 Aggiunta la gestione delle variabili in RAM non volatile.
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
* 09/12/96 GG 1.16 Corretta un'imperfezione in plcModProp. La funzione tentava
*             di copiare alcuni byte dopo la fine del nome del modulo, con
*             schianti se il nome del modulo si trova nei pressi di indirizzi
*             inaccessibili.
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
* 17/12/96 GG 2.2 Aggiunte le funzioni (qplc >= 1.3.3.8) plcClearAllWatches,
*             plcResetDebugEvents, plcLastDebugEventList, plcGetDebugEvent.
* 18/12/96 GG 2.3 Corretto uno schianto in plcLastDebugEventList.
* 03/07/97 GG 2.4 Aumentata la dimensione di default della shared memory,
*             in vista dell'abbandono definitivo del file "isarc".
*             Aggiunte le funzioni "plcLoadDebugHistory" e
*             "plcIndexDebugEventList", che servono a caricare ed esplorare
*             l'intero insieme degli eventi immagazzinati da qplc.
*             "plcDebugMemCopy" e` stata resa pubblica.
* 09/07/97 GG 2.5 Aggiunto un parametro a "plcLoadDebugHistory".
* 07/01/98 GG 2.6 Eliminato il controllo sul tipo per le variabili da
*             tracciare.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <grsy0def.h>
#include <grsy0ker.h>
#include <grsy0tst.h>

#include <hsort.h>
#include <port.h>

#define _USE_PLC_FUNC 1
#include <plcdb.h>
#include "plclink.h"
#include "plclink_p.h"

/*
#define strcmp _dbST_strcmp
#define NEW_strcmp
*/

#define PLC_TIMEOUT 500	/* Il kernel plc ha 500 ms di tempo per rispondere. */

/*LIB*/

/*
* Funzione plcSetBArr(), plcSetLArr(), plcSetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col +  dim*row" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBArr(int var,int row,int col,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		pchMem[off] = val;
}

/*LIB*/

void plcSetLArr(int var,int row,int col,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*(long *)(pchMem + off) = val;
}

/*LIB*/

void plcSetFArr(int var,int row,int col,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*(float *)(pchMem + off) = val;
}

/*LIB*/

/*
* Funzione plcGetBArr(), plcGetLArr(), plcGetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBArr(int var,int row,int col,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

/*LIB*/

void plcGetLArr(int var,int row, int col,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

void plcGetFArr(int var,int row,int col,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

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
* un massimo di 8 byte per la destinazione. Non esistono infatti variabili
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

int plcSafeCopyVar(int var,int row,int col,char * dest,int size)
{
register int off,lim;

	lim =  spDir[SPC_TBL].size / sizeof(*pKeyTab);

	if (
	/* Se e` definita la tabella */
	       pKeyTab
	/* e il PLC e` in GO */
	    && pStatus[PLCSTS_GO]
	/* e la chiave ha un valore sensato */
	    && var >= 0
	    && var < lim
	/* e la dimensione ha un valore ammissibile */
	    && size > 0
	    && size <= sizeof(double)
	/* e l'indice di riga ha un valore credibile */
	    && row >= 0
	    && row < lim
	/* e l'indice di colonna e` dentro i limiti */
	    && col >= 0
	    && col < pKeyTab[var].dim
	/* e la chiave finale e` minore del massimo assoluto */
	    && (var += col + pKeyTab[var].dim * row) < lim
	/* e la variabile esiste */
	    && (off = pKeyTab[var].off)) {

	/* si puo` procedere con la copia, */

		if (dest)
			memcpy(dest, pchMem + off, size);
		return 1;
	}
	else {

	/* altrimenti, niente. */

		if (dest)
			memset(dest,0,size);
		return 0;
	}
}

/*LIB*/

/*
* Funzione "plcPSem(int n)"
*
* Occupa il semafono n.
*/

void plcPSem(int n)
{
struct sembuf op;

	op.sem_num = n;
	op.sem_op = -1;
	op.sem_flg = 0;
	if (semop(iSemId, &op, 1) < 0) {
		perror("semop()");
	}
}

/*LIB*/

/*
* Funzione "plcVSem(int n)"
*
* Libera il semafono n.
*/

void plcVSem(int n)
{
struct sembuf op;

	op.sem_num = n;
	op.sem_op = 1;
	op.sem_flg = 0;
	if (semop(iSemId, &op, 1) < 0) {
		perror("semop()");
	}
}

/*LIB*/

/*
* Funzione ReadIsaConf
* --------------------
*
* Legge il file di configurazione "name", se esiste.
*
*  Vale 1 in caso di successo, 0 se il file non esiste.
*
*  Il file puo` contenere commenti nella forma tipica di Unix (# comento).
*
* NOTA : Questa versione ridotta della funzione. Non e` eseguito alcun
* controllo sui dati, e sono semplicemente ignorate le opzioni sconosciute.
*/

static int ReadIsaConf(char *name)
{
FILE * stream;
char line[MAX_CNF_LINE+2],filler,c;
long l;
int i,ln,n;

/* Tenta di aprire il file. */

	stream = fopen(name,"r");

/* Se il file esiste, tenta di leggere i parametri. */

	if (stream) {
		ln = 0;
		for (;;) {
			if (! fgets(line,sizeof(line)-1,stream))
				break;
			++ln;
			if (sscanf(line," SEMMASK = %o",& iSemPerms) == 1) {
				continue;
			}
			if (sscanf(line," MEMMASK = %o",& iMemPerms) == 1) {
				continue;
			}
			if (sscanf(line," MEMSIZE = 0x%lx",& lMemSize) == 1 ||
			    sscanf(line," MEMSIZE = %ld",& lMemSize) == 1) {
				continue;
			}
			if (sscanf(line," SEMKEY = %ld",& l) == 1) {
				iSemKey = (int) l;
				continue;
			}
			if (sscanf(line," MEMKEY = %ld",& l) == 1) {
				iMemKey = (int) l;
				continue;
			}
		}
		fclose(stream);
		return 1;
	}
	return 0;
}

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

int plcTryAttach(int flag)
{
int rv;

	/* Lettura della configurazione. */

	rv = ReadIsaConf(szConfigFile);
	if (rv < 0) {
		fprintf(stderr,
			"plcInit: syntax error in configuration file \"%s\" at line %d\n",
			szConfigFile, -rv);
		return -1;
	}

/* Attende la creazione del semaforo. */

	do {
		iSemId = semget(iSemKey, MAX_SEM, iSemPerms);
		if (iSemId < 0)  {
			if (errno != ENOENT) {
				perror("semget()");
				return -1;
			}
			else if (! flag)
				return -1;
			sleep(SMEM_POLL_TIME);
		}
	} while (iSemId < 0);

/* Attende la creazione dell'area condivisa. */
	
	do {
		iMemId = shmget(iMemKey, lMemSize, iMemPerms);
		if (iMemId < 0)  {
			if (errno != ENOENT) {
				perror("shmget()");
				return -1;
			}
			else if (! flag)
				return -1;
			sleep(SMEM_POLL_TIME);
		}
	} while (iMemId < 0);

/* Acquisizione dell'area condivisa. */

/* Qui la semaforizzazione serve solo per assicurare la sincronizzazione
 con il creatore della risorsa. */

	plcPSem(SEM_MEM);

	pchMem = (char *) shmat(iMemId, (char *)0, 0);
	if (pchMem == (char *) -1) {
		perror("shmat()");
		plcVSem(SEM_MEM);
		return -1;
	}

	/* Calcola l'indirizzo del direttorio degli spazi e
	 dei blocchi liberi. */

	spDir = (struct spc_t *) pchMem;
	spFree = spDir + MAX_TOTSPACE;
	pStatus = (long *)(spFree + MAX_TOTSPACE);

	plcVSem(SEM_MEM);

	return 0;
}


/*
* Funzione plcAttach()
* --------------------
*
* Questa funzione collega lo spazio condiviso del PLC al processo chiamante.
* Il processo chiamante e` bloccato finche` il processo plc non ha allocato
* tutte le risorse di sistema necessarie.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcAttach(void)
{
	return plcTryAttach(1);
}

/*LIB*/

/*
* Funzione plcClose()
* -------------------
*
* Questa funzione scollega dal PLC il processo chiamante.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcClose(void)
{

/* Libera le risorse di sistema occupate. */

	if (pchMem && shmdt(pchMem) < 0) {
		perror("shmdt()");
		return -1;
	}

	return 0;
}

/*LIB*/

/*
* Funzione "dbQPLC"
* -----------------
*
*  Questa funzione dice se c'e` qplc al posto di isaker.
*/

int dbQPLC(void)
{
	return (SYMS && SYMS[1] == 'Q');
}

/*LIB*/

/*
* Funzione "_dbST_Hash"
* ---------------------
*
*  Calcola una ragionevole funzione di hash per il nome "name".
*/

#if _NO_PROTO
static int _dbST_Hash(name) char *name;
#else
static int _dbST_Hash(char *name)
#endif
{
register char c,*p;
register unsigned long l;

	p = name;
	l = 0;
#ifdef NEW_strcmp
	while ((c = *p++) && (c != ','))
#else
	while ((c = *p++))
#endif
	{
		l += c * (l % 37 + 1); /* Si faccia avanti chi ha idee migliori. */
	}
	return (int)(l % (MAX_HASH_KEY + 1));
}

/*
* Funzione "_dbST_strcmp"
* -----------------------
*
*  Questa funzione realizza l'equivalente della "strcmp", ma considera
* come caratteri di fine stringa anche ','.
*/
 
#ifdef NEW_strcmp

#if _NO_PROTO
static int _dbST_strcmp(x,y) char *x; char *y;
#else
static int _dbST_strcmp(char *x, char *y)
#endif
{
register char cx,cy;
register int d;
 
        for (;;) {
                cx = *x;
                if (cx == ',')
                        cx = 0;
                cy = *y;
                if (cy == ',')
                        cy = 0;
				d = cx - cy;
                if (d)
                        return d;
                else if (! cx)
                        return 0;
                ++x;
                ++y;
        }
}
 
#endif


/*
* Funzione "_dbST_compare"
* ------------------------
*
*  Questa funzione confronta gli elementi "x" e "y" della symbol table,
* basandosi sul seguente criterio :
*
* 	se hash(x) > hash(y) -->  x > y
* 	se hash(x) < hash(y) -->  x < y
* 	se hash(x) == hash(y) -->  la parola passa alla "strcmp".
*
*/

#if _NO_PROTO
static int _dbST_compare(x,y) void *x; void *y;
#else
static int _dbST_compare(void *x, void *y)
#endif
{
char *s1,*s2;
int h1,h2;

/* s1, s2 : puntatori all'inizio del nome. */

	s1 = MTRANS(((struct symtab_t *)x) -> name);
	s2 = MTRANS(((struct symtab_t *)y) -> name);

/* Calcola le funzioni di hash per i due nomi. */

	h1 = _dbST_Hash(s1);
	h2 = _dbST_Hash(s2);

/* Confronto. */

	if (h1 == h2)
		return strcmp(s1,s2);
	else
		return h1 - h2;
}

/*
* Funzione "_dbST_srccomp"
* -----------------------
*
*  Questa funzione confronta alfabeticamente il nome di due
* entry della symbol table.
*/

#if _NO_PROTO
static int _dbST_srccomp(x,y) void *x; void *y;
#else
static int _dbST_srccomp(void *x, void *y)
#endif
{
	return strcmp(MTRANS(((struct symtab_t *)x) -> name),
					MTRANS(((struct symtab_t *)y) -> name));
}

/*
* Funzione "dbFindIsaVar"
* -----------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table (ISAGRAF).
*  Se il nome non e` presente, riporta NULL.
*/

#if _NO_PROTO
struct symtab_t * dbFindIsaVar(name) char *name;
#else
struct symtab_t * dbFindIsaVar(char *name)
#endif
{
int hkey;
struct symtab_t t;

/* Calcola l'indice di hash del nome. */

	hkey = _dbST_Hash(name);

	if (! pHashTab[hkey].len)
		return (struct symtab_t *) 0;

/* Cerca il simbolo nell'insieme di chiave "hkey". */

	t.name = MRTRANS(name);

	return (struct symtab_t *) bsearch(
	    (char *)&t,
	    (char *)MTRANS(pHashTab[hkey].set),
	    pHashTab[hkey].len,
	    sizeof(t),
	    _dbST_srccomp);
}

/*
* Funzione "dbFindQplcVar"
* ------------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table (QPLC).
*  Se il nome non e` presente, riporta NULL.
*  In "row" e "col" sono riportati gli eventuali indici di riga e
* colonna ricavati dal nome secondo le regole di isaker.
*/

struct qplc_shv_t * dbFindQplcVar(char *name, int *row, int *col)
{
int hkey;
struct qplc_shv_t *p;
long offset;
char s[MAX_SHV_NAME*3];
char *q;
int i1,i2,f;

/* Trova le componenti. */

	strncpy(s,name,sizeof(s)-1);
	s[sizeof(s)-1] = '\0';
	i1 = i2 = 0;
	q = s;
	while ( (q = strchr(q,'_')) ) {
		++q;
		if (*q >= '0' && *q <= '9') {
			*(q - 1) = '\0';
			i1 = atoi(q);
			if ( (q = strchr(q,'_')) ) {
				i2 = atoi(++q);
			}
			break;
		}
	}

/* Calcola l'indice di hash del nome. */

	hkey = _dbST_Hash(s);

/* Ricerca del nome nella lista di hash. */

	for (offset = QplcHash[hkey]; offset; offset = p -> next_hash) {
		p = (struct qplc_shv_t *) (pchMem + offset);
		f = strcmp(p -> name, s);
		if (f >= 0) {
			if (f == 0) {
				*row = i1;
				*col = i2;
				return p;
			}
			break;
		}
	}

	return (struct qplc_shv_t *) 0;
}

/*LIB*/

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
void dbRegisterNvram(nvram) void * nvram;
#else
void dbRegisterNvram(void * nvram)
#endif
{
	_plclib_nvram = nvram;
}

/*LIB*/

/* Flag di variabile in RAM non volatile. Bisogna usare "_plclib_nvram",
 anziche` "pchMem", come indirizzo di base. */

#define SHV_MODE_RETAIN 0x80

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
char * dbGetQplcVarDescr(name,qsym) char *name; struct qplc_shv_t ** qsym;
#else
char * dbGetQplcVarDescr(char *name, struct qplc_shv_t ** qsym)
#endif
{
int row,col;
char *p;

	*qsym = dbFindQplcVar(name,&row,&col);

	if (! *qsym || (*qsym) -> dim1 <= row || (*qsym) -> dim2 <= col)
		return (char *) 0;

	if ((*qsym) -> mode & SHV_MODE_RETAIN) {
		if (_plclib_nvram)
			p = _plclib_nvram;
		else
			return (char *) 0;
	}
	else {
		p = pchMem;
	}
	return p + pKeyTab[(*qsym) -> key + (*qsym) -> dim2*row + col].off;
}

/*LIB*/

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
char * dbGetVarAddr(name) char *name;
#else
char * dbGetVarAddr(char *name)
#endif
{
struct symtab_t * sym;
struct qplc_shv_t * qsym;
int row,col;

	if (dbQPLC()) {
		return dbGetQplcVarDescr(name,&qsym);
	}
	else {
		sym = dbFindIsaVar(name);

		if (! sym)
			return (char *) 0;

		return MTRANS(sym -> pval);
	}
}

/*LIB*/


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

/* Tipi di QPLC */
#define SHV_MODE_TYPE 0x70 /* Maschera. */
#define SHV_MODE_TYPE_BOOL 0x00
#define SHV_MODE_TYPE_CHAR 0x10
#define SHV_MODE_TYPE_UCHAR 0x20
#define SHV_MODE_TYPE_LONG 0x30
#define SHV_MODE_TYPE_ULONG 0x40
#define SHV_MODE_TYPE_FLOAT 0x50
#define SHV_MODE_TYPE_DOUBLE 0x60

#if _NO_PROTO
struct plcvar_t * dbGetVar(name,res) char *name; struct plcvar_t *res;
#else
struct plcvar_t * dbGetVar(char *name, struct plcvar_t *res)
#endif
{
struct symtab_t *p;
char *line,*q;
int scope,len;
struct qplc_shv_t * qsym;
int row,col;

	if (dbQPLC()) {

	/* Cerca la variabile. */

		q = dbGetQplcVarDescr(name,&qsym);

		if (! q)
			return (struct plcvar_t *) 0;

		if (! res) {
			res = (struct plcvar_t *) malloc(sizeof(*res));
			if (! res)
				return (struct plcvar_t *) 0;
		}

		res -> name = qsym -> name;
		res -> pval = q;
		res -> size = qsym -> size;
		res -> type = ISAVAR_T_UNKN;
		res -> format[0] = "";
		res -> format[1] = "";
		res -> attr = ISAVAR_A_INTERNAL;
		res -> scope = 0;

		switch (qsym -> mode & SHV_MODE_TYPE) {
		case SHV_MODE_TYPE_BOOL:
			res -> type = ISAVAR_T_BOOL;
			res -> format[0] = "FALSE";
			res -> format[1] = "TRUE";
			break;
		case SHV_MODE_TYPE_CHAR:
		case SHV_MODE_TYPE_UCHAR:
		case SHV_MODE_TYPE_LONG:
		case SHV_MODE_TYPE_ULONG:
			res -> type = ISAVAR_T_ANA_I;
			res -> format[0] = "I";
			break;
		case SHV_MODE_TYPE_FLOAT:
		case SHV_MODE_TYPE_DOUBLE:
			res -> type = ISAVAR_T_ANA_F;
			res -> format[0] = "F";
			break;
		default:
			break;
		}
	}
	else {

	/* Cerca la variabile. */

		p = dbFindIsaVar(name);

		if (! p)
			return (struct plcvar_t *) 0;

		if (! res) {
			res = (struct plcvar_t *) malloc(sizeof(*res));
			if (! res)
				return (struct plcvar_t *) 0;
		}

	/* Traduce i puntatori secondo lo spazio virtuale del processo. */

		res -> name = line = MTRANS(p -> name);
		res -> pval = MTRANS(p -> pval);
		res -> size = p -> len;
		res -> type = ISAVAR_T_UNKN;
		res -> attr = ISAVAR_A_UNKN;
		res -> format[0] = "";
		res -> format[1] = "";
		res -> scope = 0;

	/* Estrae il tipo di variabile dallo spazio dei simboli. */

		switch (*(line - 5)) {
		case '1': res -> type = ISAVAR_T_BOOL; break;
		case '2': res -> type = ISAVAR_T_ANA_I; break;
		case '3': res -> type = ISAVAR_T_TIMER; break;
		case '4': res -> type = ISAVAR_T_MSG; break;
		default:  res -> type = ISAVAR_T_UNKN; return res;
		}

	/* Cerca la fine del nome. */

#ifdef NEW_strcmp
		while (*++line && *line != ',') {
		}
#else
		while (*++line) {
		}
#endif

	/* Estrae l'attributo. */

		if (res -> type != ISAVAR_T_TIMER) {
			line += 2;
			switch (*line) {
			case 'I': res -> attr = ISAVAR_A_INPUT; break;
			case 'O': res -> attr = ISAVAR_A_OUTPUT; break;
			case 'X': res -> attr = ISAVAR_A_INTERNAL; break;
			default:  res -> attr = ISAVAR_A_UNKN; return res;
			}

		/* Passa al campo successivo. */

			++line;
		}
		else {

		/* I timer non hanno attributo. */

			res -> attr = ISAVAR_A_INTERNAL;
		}

	/* Estrae lo "scope". */

		line += 2;

		scope = 0;

		while (isxdigit(*line)) {
			scope *= 16;
			scope += *line <= '9' ? *line - '0'
			                      : toupper(*line) - 'A' + 10;
			++line;
		}

		res -> scope = scope;

	/* Estrae (o costruisce) il formato. */

		switch (res -> type) {
		case ISAVAR_T_BOOL:
		case ISAVAR_T_ANA_I:

		/* "TRUE" o formato numerico. */

			q = ++line;
			while (*line != ',' && *line != '\0') {
				if (*line == '\n')
					return res;
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[0] = q;

		/* "FALSE" o unit string. */

			q = ++line;
			while (*line != '\r'&&*line != '\n'&&*line != '\0') {
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[1] = q;

		/* Se la variabile e` analogica e il formato e` "F", allora si
		 tratta di un "float". */

			if (res -> type == ISAVAR_T_ANA_I
			    && res -> format[0][0] == 'F')
				res -> type = ISAVAR_T_ANA_F;

			break;

		case ISAVAR_T_TIMER:
			res -> format[0] = "I";
			res -> format[1] = "";
			break;

		case ISAVAR_T_MSG:
			res -> format[0] = "S";

		/* I messaggi contengono la lunghezza al posto del formato. */

			q = ++line;
			while (*line!='\r' && *line!='\n' && *line!='\0') {
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[1] = q;

			break;
		default:
			break;
		}
	}

	return res;

}

/*LIB*/


/*
* Funzione plcGo()
* ----------------
*
* Tenta di far partire il Plc. Vale 1 se il Plc e` partito, 0 altrimenti.
*/

int plcGo(void)
{
	if (! pStatus)
		return 0;
	if (SYST -> start == START_OK)
		return 1;
	if (SYST -> start != START_INIT)
		return 0;
	SYST -> start = START_APL;
	for (;;) {
		if (SYST -> start == START_INIT)
			return 0;
		if (SYST -> start == START_OK)
			return 1;
		usleep(LOOP_SLEEP);
	}
}

/*LIB*/

/*
* Funzione plcHalt()
* ------------------
*
* Forza l'Halt del Plc. Vale 1 se l'halt ha avuto successo, 0 altrimenti.
*/

int plcHalt(void)
{
	if (! pStatus)
		return 0;
	if (SYST -> start != START_OK)
		return 1;
	SYST -> start = START_STOP;
	while (SYST -> start != START_INIT)
		usleep(LOOP_SLEEP);
	return 1;
}

/*LIB*/

/*
* Funzione plcLinkDynamics()
* --------------------------
*
*  Questa funzione inizializza i puntatori alle aree di plc allocate
* dinamicamente ad ogni caricamento di applicazione.
*  Vale 0 in caso di successo, -1 in caso di errore (plc non in start).
*/

int plcLinkDynamics(void)
{
char *addr;

	if (! pStatus || ! pStatus[PLCSTS_GO])
		return -1;

	/* Trova la struttura di stato del kernel Plc. */

	addr = spDir[SPC_KER].size ? pchMem + spDir[SPC_KER].offset : (char *)0;
	KER = (str_ker *) addr;

	/* Trova le variabili. */

	if (KER) {
		BF_BOO = (char *)MTRANS(KER -> bf_boo);
		BF_ANA = (long *)MTRANS(KER -> bf_ana);
		BF_TMR = (long *)MTRANS(KER -> bf_tmr);
	}
	else
		return -1;

	return 0;
}

/*LIB*/

/* Variabile statica inizializzata dalla "plcLinkStatics" e controllata
 dalla "plcDbChanged" per individuare i cambiamenti del database
 dei simboli. */

static long last_db_vers = -1;

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

int plcLinkStatics(void)
{
char *addr;

	if (! pStatus)
		return -1;

	/* Trova la struttura di controllo del Plc. */

	addr = spDir[SPC_SYS].size ? pchMem + spDir[SPC_SYS].offset : (char *)0;
	SYST = (str_system *) addr;

	/* Trova il buffer dei simboli (ISA13...). */

	addr = spDir[SPC_SYM].size ? pchMem + spDir[SPC_SYM].offset : (char *)0;
	SYMS = (char *) addr;

	/* Trova il buffer di colloquio col il debugger. */

	addr = spDir[SPC_TST].size ? pchMem + spDir[SPC_TST].offset : (char *)0;
	ANSW = (unsigned char *) addr;

	/* Trova la tabella dei segnali di scambio. */

	addr = spDir[SPC_TBL].size ? pchMem + spDir[SPC_TBL].offset : (char *)0;
	pKeyTab = (struct trtel_t *) addr;

	/* Trova la tabella di hash dei simboli. */

	addr = spDir[SPC_HTB].size ? pchMem + spDir[SPC_HTB].offset : (char *)0;
	pHashTab = (struct plchash_t *) addr;

	/* Trova la tabella di hash dei simboli (stile QPLC). */

	addr = spDir[17].size ? pchMem + spDir[17].offset : (char *)0;
	QplcHash = (long *) addr;

	/* Trova l'indirizzo virtuale dell'area condivisa nello spazio
	 di indirizzamento del Plc. */

	pchMMem = (char *)pStatus[PLCSTS_MEMBASE];

	/* Inizializza ad un valore "impossibile" il numero di versione
	 del database PLC. */

	last_db_vers = -1;

	return 0;
}

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

int plcDbChanged(void)
{
long cv;

	cv = pStatus[PLCSTS_DBVER];
	if (cv == last_db_vers)
		return 0;
	else {
		last_db_vers = cv;
		return 1;
	}
}

/*LIB*/

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

int plcOpen(void)
{

	/* Collega il processo allo spazio condiviso del PLC. */

	if (plcAttach() < 0)
		return -1;

	/* Attende che il plc sia pronto ed
	 inizializza i puntatori alle aree statiche di interesse. */

	if (plcWait() < 0)
		return -1;

	return plcLinkStatics();
}

/*LIB*/

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

int plcTryOpen(void)
{
int rv;
static int fase_plc = 0;

	rv = -1;

	switch (fase_plc) {

	/* Fase 0 : tutto e` ancora da fare. */

	case 0:
		/* Ci vuole il parametro 0 (PERCHE` SI`) */
		if (plcTryAttach(0) != 0)
			break;

		/* Qualcosa c'e`... Si cambia fase. */

		fase_plc = 1;

	/* ... E AVANTI ! ... */

	/* Fase 1 : le risorse globali esistono, ma bisogna vedere se
	  il Plc ci ha scritto gia` qualcosa di sensato. */

	case 1:
		if (! plcReady())
			break;

		/* E` partito il Plc ! */

		fase_plc = 2;

		plcLinkStatics();

	/* ... E AVANTI ! ... */

	default:
		rv = 0;
		break;
	}

	return rv;
}

/*LIB*/


/*
* Funzione plcSetBScal(), plcSetLScal(), plcSetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBScal(int var,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		pchMem[off] = val;
}

void plcSetLScal(int var,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*(long *)(pchMem + off) = val;
}

/*LIB*/

void plcSetFScal(int var,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*(float *)(pchMem + off) = val;
}

/*LIB*/

/*
* Funzione plcGetBScal(), plcGetLScal(), plcGetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var + dim*ind"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBScal(int var,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

/*LIB*/

void plcGetLScal(int var,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

void plcGetFScal(int var,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

/*
* Funzione plcStatus()
* --------------------
*
* Vale 0 se il Plc e` in HALT, 1 se in GO.
*/

int plcStatus(void)
{
	if (pStatus && pStatus[PLCSTS_GO])
		return 1;
	else
		return 0;
}

/*LIB*/


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

/*LIB*/

/*
* Funzione plcSetBVect(), plcSetLVect(), plcSetFVect()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBVect(int var,int col,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		pchMem[off] = val;
}

/*LIB*/

void plcSetLVect(int var,int col,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*(long *)(pchMem + off) = val;
}

/*LIB*/

void plcSetFVect(int var,int col,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*(float *)(pchMem + off) = val;
}

/*LIB*/

/*
* Funzione plcGetBVect(), plcGetLVect(), plcGetFVect()
* ----------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBVect(int var,int col,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

/*LIB*/

void plcGetLVect(int var, int col,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

void plcGetFVect(int var,int col,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

/*LIB*/

/*
* Funzione plcReady()
* -------------------
*
*  Questa funzione vale 1 se il Plc e` stabilizzato, cioe` ha terminato la 
* fase di startup, 0 altrimenti.
*/

int plcReady(void)
{
	if (! pStatus)
		return 0;

	return pStatus[PLCSTS_OKLINK];
}

/*LIB*/

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

/* Periodo di polling dello start del Plc. */

#define PLC_WAIT_TIMEOUT 1

int plcWait(void)
{
	if (! pStatus)
		return -1;

	while (! plcReady()) {
		sleep(PLC_POLL_TIME);
	}

	return 0;
}

/*LIB*/

#define COEFF   20                /* 20 loops of user time out/20 */

static int plcLock(unsigned char * flag)
{
  register int rv;

  /* Si tratta di semafori finti. E per simularli ci vogliono DUE chiamate
 alle primitive di semaforo vere. O tempora !... */

  plcPSem (SEM_FLG);
  rv = *flag;
  if (!rv)
    *flag = 1;
  plcVSem (SEM_FLG);
  return rv;
}

static void plcUnlock(unsigned char * flag)
{
  plcPSem (SEM_FLG);
  *flag = 0;
  plcVSem (SEM_FLG);
}

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

int plcTell (unsigned char * msg, unsigned char * answ, long timeout)
{
int i,rv;

/* Il periodo di timeout e` scomposto in COEFF passi. */

  timeout = (timeout / COEFF) * 1000; /* Converte in microsecondi per usleep. */

/* Attende di avere accesso alla risorsa. */

  for (i = COEFF + 1; plcLock(&(SYST->client)) && --i; )
      usleep (timeout);
  if (! i)
    return 0;	/* ...sfiga... */

/* Copia il messaggio nell'apposita area del kernel plc. */

  SYST -> question_ker = (unsigned char *) MRTRANS(&(SYST -> buffer[0]));
  SYST -> answer_ker = (unsigned char *) MRTRANS(ANSW);
  for (i = 0; i < MAX_QUESTION; ++i)
    SYST -> buffer[i] = msg[i];
  SYST -> question_ready = 1;

/* Attende la risposta. */

  for (i = COEFF + 1; SYST -> question_ready && --i; )
      usleep(timeout);

/* C'e` una condizione critica ineliminabile : il Plc potrebbe eseguire il
 comando prima che il flag venga azzerato per timeout. La situzione si
 risolverebbe se fossero utilizzati due flag distinti. Ma non e` cosi`... */

  if (i) {

/* Copia la risposta nella destinazione. */

    rv = SYST -> answer_size;
    for (i = 0; i < rv; ++i)
      answ[i] = ANSW[i];
  }
  else {
    SYST -> question_ready = 0;
    rv = 0;	/* ...sfiga... */
  }

/* Libera la risorsa. */

  plcUnlock(&(SYST -> client));

  return rv;
}

/*LIB*/

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

int plcTraceAddr(unsigned char * addr)
{
char msg[1 + sizeof(char *)];
char answ[3];

/* Comunica l'indirizzo della variabile (tradotto per il processo plc). */

	msg[0] = (char) R_TRACE_ADD;
	*(char **)(& msg[1]) = (char *)(addr);
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

/*
* Funzione "plcTraceVar"
* ----------------------
*
*  Questa funzione aggiunge la variabile "name" all'insieme di trace.
*  Vale 1 se tutto e` andato bene, altrimenti 0, ed in "*err" e` scritto
* uno dei seguenti valori :
*
* PLCERR_TOOVAR  : troppe variabili tracciate,
* PLCERR_BADVAR  : variabile non booleana,
* PLCERR_NOVAR   : variabile non trovata,
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc (non dovrebbe accadere).
*/

int plcTraceVar(char * name)
{
struct plcvar_t var;
char msg[1 + sizeof(char *)];
char answ[3];

/* Cerca la variabile. Errore se non c'e`. */

	if (! dbGetVar(name,&var))
		return PLCERR_NOVAR;

#if 0
/* Errore se la variabile non e` booleana. */
	if (var.type != ISAVAR_T_BOOL)
		return PLCERR_BADVAR;
#endif

/* Comunica l'indirizzo della variabile (tradotto per il processo plc). */

	msg[0] = (char) R_TRACE_ADD;
	*(char **)(& msg[1]) = (char *)(MRTRANS(var.pval));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcTraceClear(void)
{
char msg[1];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_CLEAR;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcTraceStart(void)
{
char msg[1];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_START;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcTraceStop(void)
{
char msg[1];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_STOP;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcTraceSetBuf(int n)
{
char msg[1 + sizeof(int)];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_TRACE_SETBUF;
	*(int *)(&msg[1]) = n;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

/*
* Funzione "plcTraceGetBuf"
* ------------------------
*
*  Questa funzione riporta in "*n" la dimensione del buffer di trace.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcTraceGetBuf(int *n)
{
struct tracests_t status;
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato del trace. */

	msg[0] = (char) R_TRACE_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	status = *(struct tracests_t *)(& answ[4]);

	*n = status.dimbuff;

	return 0;
}

/*LIB*/

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

int plcTraceDownLoad(TRACERECORD *p)
{
struct tracests_t status;
char msg[1 + 2*sizeof(int)];
char answ[MAX_REQ];
int i,n,pos,bl,nr;
TRACERECORD *q;

/* Richiede lo stato del trace. */

	msg[0] = (char) R_TRACE_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	status = *(struct tracests_t *)(& answ[4]);

/* Se il buffer non e` ancora pronto, esce con un codice di errore apposito. */

	if (! status.completed)
		return PLCERR_NOTREADY;

/* Calcola il numero massimo di record ottenibili con una singola
 interrogazione del plc. */

	bl = (MAX_REQ - 4) / sizeof(TRACERECORD);

/* Copia il buffer a passi di alpiu` "bl" elementi. */

	for (n = status.dimbuff, pos = status.lastbuff; n; n -= nr) {

	/* Costruisce il comando. */

		nr = n > bl ? bl : n;
		msg[0] = (char) R_TRACE_LOAD;
		*(int *)(& msg[1]) = pos;
		*(int *)(& msg[1 + sizeof(int)]) = nr;
		if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
			return PLCERR_TIMEOUT;
		if (*(short *)(& answ[1]) != CR_OK)
			return PLCERR_REMERR;

	/* Copia il blocco nella destinazione finale. */

		for (i = 0, q = (TRACERECORD *)(& answ[4]); i < nr; ++i)
			*(p++) = *(q++);

	/* Manda avanti la posizione di lettura. */

		pos += nr;
		if (pos >= status.dimtot)
			pos = 0;
	}

/* Fine. Segnala al plc che il download e` stato completato. */

	msg[0] = (char) R_TRACE_SIGNAL;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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
*
* NOTA : DATA LA LENTEZZA DELL'OPERAZIONE, L'ERRORE DI TIMEOUT E` DA
* CONSIDERARE FISIOLOGICO, E NON DEVE PREOCCUPARE.
*/

int plcLoadProg(char *name)
{
char msg[256];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_DYNLD_LOAD;
	strncpy(msg + 1,name,sizeof(msg) - 3);
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

/*
* Funzione "plcUnloadProg"
* ----------------------
*
*  Questa funzione rimuove l'ultimo programma di plc caricato dinamicamente.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC.
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcUnloadProg(void)
{
char msg[1];
char answ[3];

/* Poco da dire. Inoltra il comando ed attende la risposta. */

	msg[0] = (char) R_DYNLD_UNLOAD;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

/*
* Funzione "plcLdStatus"
* ----------------------
*
*  Questa funzione riporta in "*n" lo stato dell'ultimo caricamento dinamico.
*  Vale 0 se tutto e` andato bene, altrimenti da` uno dei seguenti valori :
*
* PLCERR_TIMEOUT : timeout nella comunicazione col PLC (NORMALE !).
* PLCERR_REMERR  : errore restituito dal kernel plc.
*/

int plcLdStatus(int *n)
{
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato del trace. */

	msg[0] = (char) R_DYNLD_STATUS;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*n = *(int *)(& answ[4]);

	return 0;
}

/*LIB*/

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

int plcMemCopy(char *dest,char *src,int n)
{
char msg[1 + sizeof(int) + sizeof(char *)];
char answ[MAX_REQ];

	msg[0] = (char) R_KERMEM_COPY;
	memcpy(&msg[1],(char *)(& src),sizeof(char *));
	memcpy(&msg[1 + sizeof(char *)],(char *)(& n),sizeof(int));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

/*LIB*/

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

int plcStatistics(long *curr,long *min, long *max)
{
char msg[4];
char answ[MAX_REQ];

	msg[0] = (char) R_GET_TCYSTAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*curr = *(long *)(&answ[4]);
	*min = *(long *)(&answ[4 + sizeof(long)]);
	*max = *(long *)(&answ[4 + 2*sizeof(long)]);
	return 0;
}

/*LIB*/

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

int plcModProp(struct modprop_t *dest,char *name)
{
char msg[MAX_REQ];
char answ[MAX_REQ+1];

	msg[0] = (char) R_DYNLD_PROP;
	strncpy(&msg[1],name,MAX_REQ-1);
	msg[MAX_REQ-1] = '\0';
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,&answ[4],sizeof(struct modprop_t));
	return 0;
}

/*LIB*/

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

int plcStStatus(int *n)
{
char msg[1];
char answ[MAX_REQ];

/* Richiede lo stato della statistica. */

	msg[0] = (char) R_IF_ENSTAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*n = answ[3];

	return 0;
}

/*LIB*/

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

int plcStEnable(int flag)
{
char msg[1 + sizeof(char)];
char answ[4];

	msg[0] = (char) R_START_STAT;
	msg[1] = (char) flag;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcStClear(void)
{
char msg[1 + sizeof(char)];
char answ[4];

	msg[0] = (char) R_CLEAR_STAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcStatistics(long *curr,long *min, long *max)
{
char msg[4];
char answ[MAX_REQ];

	msg[0] = (char) R_GET_TCYSTAT;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*curr = *(long *)(&answ[4]);
	*min = *(long *)(&answ[4 + sizeof(long)]);
	*max = *(long *)(&answ[4 + 2*sizeof(long)]);
	return 0;
}

/*LIB*/

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

int plcGetExeName(char *dest)
{
char msg[MAX_REQ];
char answ[MAX_REQ+1];

	msg[0] = (char) R_GET_EXENAME;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	strcpy(dest,&answ[3]);
	return 0;
}

/*LIB*/

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

int plcMultiMemCheck(char *dest,struct plc_mcopy_t *src,int n)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	if (n <= 0)
		return 0;
	if (n*sizeof(src[0])-2*sizeof(long) > MAX_REQ) {
		n = (MAX_REQ-2*sizeof(long))/sizeof(src[0]);
	}
	msg[0] = (char) R_KERMEM_MCHK;
	memcpy(&msg[4],(char *)(& n),sizeof(int));
	memcpy(&msg[4 + sizeof(int)],(char *)src,n*sizeof(src[0]));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

/*LIB*/

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

int plcMultiMemCopy(char *dest,int n)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_KERMEM_MCOPY;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	memcpy(dest,answ + 1 + sizeof(short),n);
	return 0;
}

/*LIB*/

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

int plcCreateWatchList(watch_descr_t *wprop,int *dest)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_CREATE_WATCH;
	memcpy(&msg[1],(char *)wprop,sizeof(*wprop));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	*dest = *(int *)(& answ[4]);
	return 0;
}

/*LIB*/

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

int plcDeleteWatchList(int wid)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_DELETE_WATCH;
	*(int *)(&msg[1]) = wid;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcAddBreak(int wid,watch_t *brk)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_ADD_BREAK;
	*(int *)(&msg[1]) = wid;
	memcpy(&msg[1+sizeof(int)],(char *)brk,sizeof(*brk));
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcEnableWatchList(int wid)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_ENABLE_WATCH;
	*(int *)(&msg[1]) = wid;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

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

int plcDisableWatchList(int wid)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_DISABLE_WATCH;
	*(int *)(&msg[1]) = wid;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

/*LIB*/

/*
* Funzione "plcResetDebugEventList"
* ---------------------------------
*
*  Questa funzione reinizializza le funzioni di lettura dell'area di debug.
*  Vale 0 se tutto e` andato bene, -1 in caso di errori.
*/

struct _plc_get_event_t plc_get_event_status = { 0, 0, 0, /*...*/ };

int plcResetDebugEventList(void)
{
struct plcvar_t t;
struct _plc_get_event_t s;

	plc_get_event_status.buffer = NULL;

	if (! dbGetVar(DEBSHV_BSIZE,&t))
		return -1;
	if (! (s.size = *(unsigned long *) t.pval) )
		return -1;

	if (! dbGetVar(DEBSHV_NEVT,&t))
		s.nevt = 0;
	else
		s.nevt = *(unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_EVENTS,&t))
		s.events = (unsigned long *) 0;
	else
		s.events = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_LASTEV,&t))
		s.lastev = (unsigned long *) 0;
	else
		s.lastev = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_LASTEVP,&t))
		s.lastevp = (unsigned long *) 0;
	else
		s.lastevp = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_FIRSTEVP,&t))
		s.firstevp = (unsigned long *) 0;
	else
		s.firstevp = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_EVCOUNT,&t))
		s.evcount = (unsigned long *) 0;
	else
		s.evcount = (unsigned long *) t.pval;

	if (! dbGetVar(DEBSHV_NREGEV,&t))
		s.nregev = (unsigned long *) 0;
	else
		s.nregev = (unsigned long *) t.pval;

	if (!(s.nevt && s.events && s.lastev)) {
		s.nevt = 0;
		s.events = (unsigned long *)0;
		s.lastev = (unsigned long *)0;
	}

	if (!(s.lastevp && s.firstevp && s.evcount && s.nregev)) {
		s.lastevp = (unsigned long *)0;
		s.firstevp = (unsigned long *)0;
		s.evcount = (unsigned long *)0;
		s.nregev = (unsigned long *)0;
	}

	if (!s.nevt && !s.lastevp)  
		return -1;

	if (! dbGetVar(DEBSHV_BUFFER,&t))
		return -1;
	s.deb_buffer = (unsigned char *) t.pval;

	if (plc_get_event_status.buffer) {
		free(plc_get_event_status.buffer);
		plc_get_event_status.buffer = NULL;
	}
	s.buffer = (unsigned char *) malloc(s.size);
	if (! s.buffer)
		return -1;

	if (plc_get_event_status.lists) {
		free((void *)plc_get_event_status.lists);
		plc_get_event_status.lists = (brk_event_list **)NULL;
	}
/* Calcolo del massimo numero di liste di eventi teoricamente presenti nel
 buffer. E` la dimensione del buffer diviso per la dimensione del piu`
 piccolo descrittore di lista. Il "- sizeof(brk_event)" tiene conto
 della possibilita` (del tutto ipotetica) che si presentino liste di
 zero eventi (la descrizione del tipo "liste di eventi" in C standard non
 permette di indicare un vettore di dimensione 0, percio`...). La stima che
 si ottiene e` un po' pessimistica, ma previene sfighe. */
	s.n_lists = s.size / (sizeof(brk_event_list) - sizeof(brk_event));
	s.lists = (brk_event_list **) malloc(s.n_lists
	                                   * sizeof(brk_event_list *));
	if (! s.lists) {
		free(plc_get_event_status.buffer);
		plc_get_event_status.buffer = NULL;
		return -1;
	}

	s.cycle_ok = 0;
	s.cycle = 0;
	s.event_end = 0;
	s.read_pos = 0;
	s.history.bh_first_cycle = 0;
	s.history.bh_last_cycle = 0;
	s.history.bh_list_count = 0;

	plc_get_event_status = s;

	return 0;
}

/*LIB*/

/*
* Questa funzione e` usata internamente per copiare dati dall buffer circolare
* del debugger.
*/

void plcDebugMemCopy(char *dest, unsigned long src, unsigned long n)
{
unsigned long n1,n2;

	if (src + n <= plc_get_event_status.size) {
		n1 = n;
		n2 = 0;
	}
	else {
		n1 = plc_get_event_status.size - src;
		n2 = n - n1;
	}
	if (n1)
		memcpy(dest,plc_get_event_status.deb_buffer + src,(int)n1);
	if (n2)
		memcpy(dest + n1,plc_get_event_status.deb_buffer,(int)n2);
}

/*LIB*/

/*
* Funzione "plcLastDebugEventList"
* --------------------------------
*
*  Questa funzione carica la lista di eventi di debug prodotta
* nell'ultimo ciclo di Plc, e la copia in un'area allocata internamente
* dalla libreria. Le successive "plcGetDebugEvent" restituiranno in sequenza
* gli eventi contenuti nella lista.
*/

brk_event_list * plcLastDebugEventList(void)
{
brk_event_list *el;
unsigned long nb,lastev;

/* Inizializza se necessario, errore se non si riesce. */

	if (! plc_get_event_status.buffer
	 && ! plcResetDebugEventList()
	 && ! plc_get_event_status.buffer)
		return (brk_event_list *) NULL;

	plc_get_event_status.read_pos = 0;

/* Se e` definito il segnale di scambio $DEB_LASTEV, allore ci devono
 anche essere $DEB_NEVT e $DEB_EVENTS. In caso contrario, devono esistere
 almeno $DEB_LASTEVP e $DEB_FIRSTEVP. */

	if (plc_get_event_status.lastevp) {

/* Nuova gestione (qplc > 1.5.1.2). Si usa il segnale di scambio $DEB_LASTEVP*/

		lastev = *plc_get_event_status.lastevp;
		if (lastev == -1)
			return (brk_event_list *) NULL;
	}
	else if (plc_get_event_status.lastev) {

/* Vecchia gestione. */

/* Legge il segnale di scambio che contiene l'indice nell'array degli
 eventi dell'ultima lista di eventi registrata (-1 se non ce ne sono). */

		lastev = *plc_get_event_status.lastev;
		if (lastev == -1)
			return (brk_event_list *) NULL;

/* Legge la posizione della lista di eventi nel buffer dall'array.
 (contrlolla che, per qualche sfiga, non valga -1, anche se non
 dovrebbe mai accadere). */

		lastev = plc_get_event_status.events[lastev];
		if (lastev == -1)
			return (brk_event_list *) NULL;
	}
	else {
		return (brk_event_list *) NULL;
	}

/* Copia gli eventi nel buffer locale. */

	/* Lunghezza del blocco. */
	plcDebugMemCopy((char *)&nb,lastev,sizeof(nb));
	plc_get_event_status.event_end = nb;
	if (nb == 0 || nb > plc_get_event_status.size)
		return (brk_event_list *) NULL;
	/* Dati. */
	plcDebugMemCopy(plc_get_event_status.buffer,lastev,nb);

	el = (brk_event_list *)plc_get_event_status.buffer;

/* Confronta il ciclo caratteristico di Plc della lista di eventi corrente con
 quello della lettura precedente (se ce n'e` stata una). Se sono uguali,
 vuol dire che non ci sono in realta` nuovi eventi. */

	if (plc_get_event_status.cycle_ok
	 && el -> bl_cycle == plc_get_event_status.cycle)
		return (brk_event_list *) NULL;

/* Costruzione dei puntatori alle liste di eventi. */

	plc_get_event_status.lists[0] = el;

/* Aggiorna la struttura descrivente la storia. */

	plc_get_event_status.history.bh_first_cycle
	 = plc_get_event_status.history.bh_last_cycle
	 = plc_get_event_status.lists[0] -> bl_cycle;
	plc_get_event_status.history.bh_list_count = 1;

/* Porta la posizione di lettura degli eventi all'inizio della lista. */

	plcIndexDebugEventList(0);

	return el;
}

/*LIB*/

/*
* Funzione "plcGetDebugEvent"
* ---------------------------
*
*  Questa funzione legge il prossimo evento di debug dal buffer, e lo
* copia in un'area allocata internamente dalla libreria.
*  Restituisce il puntatore all'evento letto, oppure NULL.
*/

brk_event * plcGetDebugEvent(void)
{
brk_event *ev;
unsigned char *p;

	if (! plc_get_event_status.buffer)
		return (brk_event *) NULL;

	if (plc_get_event_status.read_pos >= plc_get_event_status.event_end)
		return (brk_event *) NULL;

	ev = (brk_event *) (plc_get_event_status.buffer
	                    + plc_get_event_status.read_pos);
	plc_get_event_status.read_pos += ev -> be_size;

	return ev;
}

/*LIB*/

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

brk_event_list * plcIndexDebugEventList(int index)
{
brk_event_list *el;

	if (index < 0 || index >= plc_get_event_status.history.bh_list_count)
		return (brk_event_list *) NULL;

	el = plc_get_event_status.lists[index];

/* Se si seleziona l'ultimo evento della lista, aggiorna le variabili
 utilizzate per l'individuazione di nuovi eventi da parte delle
 funzioni "plcLastDebugEventList" e "plcLoadDebugHistory". */

	if (index == plc_get_event_status.history.bh_list_count - 1) {
		plc_get_event_status.cycle_ok = 1;
		plc_get_event_status.cycle = el -> bl_cycle;
	}

/* Porta la posizione di lettura degli eventi all'inizio della lista. */

	plc_get_event_status.read_pos = (char *)&(el -> bl_event[0])
	                              - (char *)plc_get_event_status.buffer;

/* Calcola la fine dell'evento. */

	plc_get_event_status.event_end
	 = el -> bl_size + ((char *)el - (char *)plc_get_event_status.buffer);

	return el;
}

/*LIB*/

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

brk_history * plcLoadDebugHistory(brk_event_list ** p_el)
{
brk_event_list *el;
unsigned long p_firstev,firstev,lastev, pos;
int n;

/* Inizializza se necessario, errore se non si riesce. */

	if (! plc_get_event_status.buffer
	 && ! plcResetDebugEventList()
	 && ! plc_get_event_status.buffer)
		return (brk_history *) NULL;

	plc_get_event_status.read_pos = 0;

/* Devono essere definiti i segnali di scambio $DEB_LASTEVP e $DEB_FIRSTEVP,
 altrimenti non se ne parla nemmeno. */

/* La sequenza di lettura delle informazioni dai segnali di scambio di debug
 e` la seguente:

  1) Lettura della posizione dell'ultima lista di eventi registrata
     ($DEB_LASTEVP).
  2) Lettura della posizione della prima lista di eventi non ricoperta
     ($DEB_FIRSTEVP). Questo dato sara` utilizzato come punto di inizio
     della copia del buffer, in modo che gli eventi siano immagazzinati
     in ordine temporale nel buffer locale.
  3) Lettura dll'intero buffer circolare degli eventi, a partire dalla
     posizione ottenuta al punto (2). In questo modo gli eventi saranno
     riordinati.
  4) Lettura della posizione della prima lista di eventi non ricoperta
     ($DEB_FIRSTEVP). Sara` il valore definitivo, poiche` quello ottenuto al
     punto (2) puo` essere divenuto nel frattempo obsoleto.

  Questa sequenza assicura che i dati che si trovano tra la posizione letta
 al punto 3 e quella letta al punto 1 siano liste di eventi "sane", cioe` non
 ricoperte. Nella peggiore delle ipotesi, la funzione potra` non vedere
 alcuni eventi piu` recenti (che saranno comunque leggibili in una chiamata
 successiva) ed alcuni vecchi eventi non realmente ricoperti (ma si puo`
 supporre che siano gia` stati letti in una chiamata precedente).  */

/* Punto (1): Acquisizione della posizione dell'evento piu` recente. */
/* ----------------------------------------------------------------- */

	if (plc_get_event_status.lastevp) {
		lastev = *plc_get_event_status.lastevp;
		if (lastev == -1)
			return (brk_history *) NULL;
	}
	else {
		return (brk_history *) NULL;
	}

/* Punto (2): Acquisizione posizione provvisoria dell'evento piu` vecchio. */
/* ----------------------------------------------------------------------- */

/* Pre-acquisizione della posizione della lista piu` vecchia. Non e` il valore
 definitivo. Serve solo per avere un punto di partenza per la copia, in modo
 che nella copia locale eventi successivi compaiano in posizioni successive. */

	if (plc_get_event_status.firstevp) {
		p_firstev = *plc_get_event_status.firstevp;
		if (p_firstev == -1)
			p_firstev = lastev;
	}
	else {
		return (brk_history *) NULL; /* Non dovrebbe accadere. */
	}

/* Punto (3): Copia del buffer degli eventi. */
/* ----------------------------------------- */

/* Copia del buffer. Non conviene preoccuparsi di non copiare piu` byte
 del necessario, prima di tutto perche` sarebbe difficile calcolarne la
 quantita` esatta, poi perche`, a regime, il buffer e` sempre ricoperto,
 cioe` pieno a meno della lunghezza media di una lista. */

	plcDebugMemCopy(plc_get_event_status.buffer,
	                p_firstev,
	                plc_get_event_status.size);

/* Punto (4): Acquisizione posizione definitiva dell'evento piu` vecchio. */
/* ---------------------------------------------------------------------- */

	firstev = *plc_get_event_status.firstevp;
	if (firstev == -1)
		firstev = lastev;

/* Normalizzazione degli indici "firstev" e "lastev". */
/* Gli eventuali dati tra "p_firstev" e "firstev" potrebbero essere stati
 ricoperti, quindi vanno ignorati. */

	if (firstev < p_firstev)
		firstev += plc_get_event_status.size;
	firstev -= p_firstev;
	if (lastev < p_firstev)
		lastev += plc_get_event_status.size;
	lastev -= p_firstev;


/* Costruzione dei puntatori alle liste di eventi. */

	for (n = 0, pos = firstev; pos <= lastev; pos += el -> bl_size) {
		el = (brk_event_list *)(plc_get_event_status.buffer + pos);
		plc_get_event_status.lists[n++] = el;
	}

/* Aggiorna la struttura descrivente la storia. */

	plc_get_event_status.history.bh_first_cycle
	 = plc_get_event_status.lists[0] -> bl_cycle;
	plc_get_event_status.history.bh_last_cycle
	 = plc_get_event_status.lists[n - 1] -> bl_cycle;
	plc_get_event_status.history.bh_list_count = n;

/* Confronta il ciclo caratteristico di Plc della lista di eventi corrente con
 quello della lettura precedente (se ce n'e` stata una). Se sono uguali,
 vuol dire che non ci sono in realta` nuovi eventi. */

	if (p_el) {
		if (plc_get_event_status.cycle_ok
		 && el -> bl_cycle == plc_get_event_status.cycle) {
			*p_el = (brk_event_list *) NULL;
		}
		else {
			*p_el = el;
		}
	}

/* Porta la posizione di lettura degli eventi all'inizio dell'ultima lista. */

	plcIndexDebugEventList(n - 1);

	return &plc_get_event_status.history;
}

/*LIB*/

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

int plcClearAllWatches(void)
{
char msg[MAX_REQ];
char answ[MAX_REQ];

	msg[0] = (char) R_CLEARALL_WATCH;
	if (plcTell(msg,answ,PLC_TIMEOUT) == 0)
		return PLCERR_TIMEOUT;
	if (*(short *)(& answ[1]) != CR_OK)
		return PLCERR_REMERR;

	return 0;
}

