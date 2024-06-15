/*
* @(#) ilcrtm.c 2.11 Mon Nov 24 17:48:21 MET 1997
*
* 27/08/96 GG 1.0 Prima versione.
* 06/09/96 GG 1.1 Eliminato il test sui nomi che cominciano per IOS.
*             Sostituite le "printf" con "superv_error" o "plcError",
*             a seconda del caso.
* 16/09/96 GG 1.2 Eliminato un problema nella lettura della dimensione delle
*             variabili di I/O a 16 bit.
* 19/09/96 GG 1.3 Modificati i codici di errore nel caso di fallimento
*             della creazione di un segnale di scambio.
* 20/09/96 GG 1.4 Allineato il codice alle modifice apportate alla
*             libreria degli errori.
* 01/10/96 GG 1.5 Aggiunta la finzione "_il_ex_rtmerr_subscript", che
*             fornisce anche la posizione dell'errore.
* 08/10/96 GG 2.0 Nessuna modifica sostanziale. Il nome non e` piu`
*             "ilcloader.c". Rinominati i simboli globali,
*             l'oggetto e` stato incluso nel kernel di qplc. Cio` rendera`
*             piu` snelli gli oggetti caricati dinamicamente.
* 26/11/96 GG 2.1 Aggiunta la gestione dei dati estesi, che sono entry
*             MSD che continuano la descrizione dell'entry precedente
*             che abbia la proprieta` IL_MSD_EXTENSION.
* 29/11/96 GG 2.2 Aggiunta una (larvale) gestione del "timestamp" per
*             decidere quando e` ora di cancellare la RAM non volatile.
* 02/12/96 GG 2.3 Aggiunto il flag di inizializzazione forzata di un segnale
*             di scambio.
* 15/01/97 GG 2.4 Aggiunta la gestione di moduli IL contenenti piu` di un
*             programma.
* 29/01/97 GG 2.5 Cambiato il codice di errore emesso quando e` individuato
*             un cambio di programma. Passa da 190 a 194.
* 31/01/97 GG 2.6 Corretto un parametro sbagliato in una segnalazione d'errore.
* 30/05/97 GG 2.7 Migliorata la diagnostica nella creazione di segnali di
*             scambio.
* 03/06/97 GG 2.8 Utilizzata la funzione "shvSet" per inizializzare un segnale
*             di scambio, cosi` funzioneranno sia quelli in RAM non volatile
*             che quelli ordinari, che saranno inizializzati ad ogni GO, se
*             richiesto.
* 02/07/97 GG 2.9 Evitata l'entrata il GO se la lettura dell'MSD in stile IL
*             produce errori. Aggiustato qualche commento.
* 19/11/97 GG 2.10 Modificata l'inizializzazione della RAM non volatile per
*             evitare le scritture a 32 bit sul bus ISA, che danno fastidio
*             al floppy.
* 24/11/97 GG 2.11 Modoficata la reinizializzazione delle variabili RETAIN.
*             Ora si utilizza la nuova funzione "shvDeleteRetain", che
*             non risolve granche`, ma almeno "lava i panni sporchi in
*             famiglia".
*/

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <qplc.h>
#include <plcapi.h>
#include "ilcrtm.h"

static jmp_buf rtmerr;

/*
* Funzioni di errore grave in esecuzione.
*/

void _il_ex_rtmerr_subscript(void)
{
	plcRuntimeError(103,util_get_pc(1));
	longjmp(rtmerr,1);
}
void _il_rtmerr_subscript(void)
{
	plcError(103,"?,%d",main_status.curr_prog_id);
	longjmp(rtmerr,1);
}

/*
* Nucleo della procedura di lettura dell'MSD in stile IL. Vale 1 se tutto
* e` andato bene, 0 altrimenti.
*/

static int dump_msd(il_msd_t *msd,int reset)
{
int sz;
int mof,iof,ext,fixed,rv;
int k,chg;
shv_t v;
il_msd_t *msd_n;

	rv = 1;

	for ( ;
	     msd && msd -> flags != ILC_END_LIST;
	     msd = (il_msd_t *)(((char *)&msd_n->data) + msd_n->data_size)) {

	iof = DRIVER_MODE_INPUT;
	mof = SHV_MODE_TYPE_BOOL;
	fixed = 0;

/* Se l'entry e` "estesa", si prende nota della cosa, e si registra
 il puntatore all'estensione in "msd_n". "msd_n" sara` utilizzato anche
 come base per passare alla successiva entry significativa, percio`, nel
 caso non esteso, deve avere lo stesso valore di "msd". */

	ext = 0;
	if (msd -> flags & ILC_MSD_EXTENSION) {
		msd_n = (il_msd_t *)(((char *)&msd->data) + msd->data_size);
		if (msd_n -> data_size > 0)
			ext = 1;
	}
	else {
		msd_n = msd;
		ext = 0;
	}

	switch (msd -> flags & ILC_MSD_SIZE_M) {
	case ILC_MSD_SIZE_1:	sz = 1;
		mof = SHV_MODE_TYPE_BOOL;
		break;
	case ILC_MSD_SIZE_8:	sz = 8;
		mof = SHV_MODE_TYPE_CHAR;
		break;
	case ILC_MSD_SIZE_16:	sz = 16;
		mof = SHV_MODE_TYPE_LONG; /* Trucco. */
		break;
	case ILC_MSD_SIZE_32:	sz = 32;
		if (msd -> flags & ILC_MSD_MEMVAR_FLOAT)
			mof = SHV_MODE_TYPE_FLOAT;
		else
			mof = SHV_MODE_TYPE_LONG;
		break;
	case ILC_MSD_SIZE_64:	sz = 64;
		mof = SHV_MODE_TYPE_DOUBLE;
		break;
	default: sz = 0; break;
	}
	switch (msd -> flags & ILC_MSD_BIND_M) {
	case ILC_MSD_BIND_OUT:
		iof = DRIVER_MODE_OUTPUT;
	case ILC_MSD_BIND_IN:
		if (! ioAttach(msd -> data.io,(void *) msd -> target,sz,iof)) {
		/* ERRORE. I/O non valido. */
			plcError(5,msd -> data.io);
			if (!main_status.go_anyway)
				rv = 0;
		}
		break;
	case ILC_MSD_BIND_MEM:
		strcpy(v.name,msd -> data.shv.name);
		v.mode = mof;
		v.dim1 = msd -> data.shv.dim1;
		v.dim2 = msd -> data.shv.dim2;
		fixed = msd -> flags & ILC_MSD_MEMVAR_FIXED;
		if (fixed) {
			v.key = (int)(msd -> target);
		}
		else {
			v.key = -1;
		}
	/* L'entry estesa puo` contenere la richiesta di sistemare in
	 RAM non volatile la variabile. */
		if (ext && (msd_n -> data.edata[0] & ILC_MSD_EXT_RETAIN)) {
			v.mode |= SHV_MODE_RETAIN;
		}
		k = shvDefine(&v,&chg);
		if (k == -1) {
		/* ERRORE. E` successo qualcosa di grave. */
			plcError(105,v.name);
			rv = 0;
		}
		else if (chg & (SHV_CHG_SIZE | SHV_CHG_DIM1
		              | SHV_CHG_DIM2 | SHV_CHG_KEY
		              | SHV_CHG_RETAIN | SHV_CHG_CONTIG
		              | SHV_CHG_TYPE)) {
		/* ERRORE. L'oggetto non ha esattamente le
		 caratteristiche richieste. */
			plcError(104,v.name);
			rv = 0;
		}
		if (! fixed) {
			*(long *)(msd -> target) = k;
		}
	/* L'entry estesa puo` contenere la richiesta di inizializzare
	 la variabile. Questo deve avvenire per le variabili RETAIN se
         la RAM non volatile non e` conforme al programma, ed in ogni
	 caso per le variabili ordinarie. */
		if (   ext
		    && (msd_n -> data.edata[0] & ILC_MSD_EXT_INIT)
		    && (reset || !(v.mode & SHV_MODE_RETAIN))
		    && k != -1) {
			shvSet(&v,(char *)(&msd_n -> data.edata[1]));
		}
		break;
	default:
		sz /= 8;
		if (sz == 0)
			sz = 1;
		memcpy((char *) (msd -> target), (char *)(&msd -> data), sz);
	}

	}

	return rv;
}

struct il_dprog_t {
	void (*program)(void *statics);
	void *statics;
};

static struct il_dprog_t *il_progs = (struct il_dprog_t *) 0;
static int n_il_progs = 0;

/*
* Distribuzione dei registri e preparazione delle tabelle di corrispondenza.
* Questa funzione deve essere chiamata allo start.
* Vale 1 se tutto e` andato bene, 0 altrimenti (assenza del controllore IOS
* selezionato). "code" contiene l'indice del controllore che non e` stato
* trovato.
*/

static int IL_ReadMsd(il_module_t *mod, int * code)
{
il_config_chain_t *chain;
il_config_t *conf,**pconf;
il_resource_t *res,**pres;
il_program_t *prog,**pprog;
unsigned long *pt;
int reset,rv;
struct il_dprog_t *p;

	rv = 1;
	reset = 0;
	n_il_progs = 0;

/* Controlla la data del modulo "user", per vedere se si dovra` 
 reinizializzare la RAM non volatile. */

	pt = main_status.mod_user_op.timestamp;
	if (pt
	    && main_status.nvram2.addr
	    && *pt != *(unsigned long *) main_status.nvram2.addr ) {
#if 0
		main_status.nvram2_free = main_status.nvram2.addr+sizeof(*pt);
#else
		shvDeleteRetain();
#endif
		util_memcpy((unsigned char *) main_status.nvram2.addr,
		            (unsigned char *) pt, sizeof(*pt));
		util_memset(main_status.nvram2_free, 0,
		       main_status.nvram2.size - sizeof(*pt));
		reset = 1;
		plcError(194,"");
	}

/* Alloca la tabella dei programmi IL. */
	if (!il_progs) {
		il_progs
		 = (struct il_dprog_t *)
		    malloc(sizeof(*il_progs) * main_status.max_il_prog_mod);
		if (! il_progs) {
			plcError(185,"");
			return 0;
		}
	}

	for (chain = mod -> chain; chain; chain = chain -> next) {
		rv = dump_msd(chain -> msd,reset) && rv;
		for (pconf = chain -> conf; *pconf; ++pconf) {
			conf = *pconf;
			for (pres = conf -> resource; *pres; ++pres) {
				res = *pres;
				for (pprog = res -> prog; *pprog; ++pprog) {
					prog = *pprog;
					if (il_progs
					    && n_il_progs
					       < main_status.max_il_prog_mod) {
						p = &il_progs[n_il_progs];
						p -> program = prog -> program;
						p -> statics = prog -> statics;
						++n_il_progs;
					}
					rv = dump_msd(prog -> msd,reset) && rv;
				}
			}
		}
	}
	return rv;
}

int IL_UserStart_fixup(il_module_t *mod, int *code)
{
/* Inizializza le tabelle. */
	return IL_ReadMsd(mod, code);
}
extern int IL_UserStart(il_module_t *mod, int * code);

int IL_UserMain(int start)
{
struct il_dprog_t *p;

	if (start) {
		if (setjmp(rtmerr)) {
		/* Errore grave in esecuzione. */
			plcHalt();
			return 0;
		}
	}

	for (main_status.curr_prog_id = 1, p = il_progs;
	     main_status.curr_prog_id <= n_il_progs;
	     ++main_status.curr_prog_id, ++p)
		util_call_il(p -> program,p -> statics);

	main_status.curr_prog_id = 0;

	return 0;
}

void IL_UserStop(void)
{
}

