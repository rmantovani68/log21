/*
* @(#) superv.c 1.24 Thu Feb  8 13:42:06 MET 2001
*
*  Questo file contiene il supervisore del PLC. Il supervisore e`
* un thread a priorita` ordinaria che controlla l'esecutore e
* gestisce il colloquio col debugger.
*
* 13/06/96 GG 1.0 Prima stesura (e parecchie modifiche al volo, non
*             documentate).
* 22/10/96 GG 1.1 Aggiunto un controllo sul successo della funzione
*             "UserStart".
* 29/11/96 GG 1.2 Piccola modifica per la visualizzazione della versione
*             alternativa dopo il caricamento.
* 11/12/96 GG 1.3 Aggiunta la cancellazione di tutti i breakpoint al
*             caricamento ed allo scaricamento.
* 03/06/96 GG 1.4 Nulla, in realta`.
* 11/06/97 GG 1.5 Aggiunto il parametro "update" alla funzione
*             "superv_message", per poter sfruttare le nuove prestazioni
*             di quote introdotte con la versione 2.6.0.15 . Di conseguenza,
*             e` stato aggiunto il comando "SV_COMM_MESSAGE_UPD" ad uso
*             dell'esecutore.
* 16/07/97 GG 1.6 Aggiunta la chiamata di "ioUp" alla fine della procedura
*             di entrata il GO. Aggiunto il comando "SV_COMM_FINISH_HALT",
*             per l'esecuzione di azioni lente (ma che terminano sicuramente)
*             da parte del supervisore al termine dell'entrata in HALT.
* 05/11/97 GG 1.7 Aggiunto il comando SV_COMM_CALLBACK, che serve ad attivare
*             la callback QPLC_CB_SUPERV.
* 07/11/97 GG 1.8 Resa pubblica la funzione "superv_exec".
* 10/11/97 GG 1.9 Corretto un errore nella visualizzazione delle versioni.
* 12/11/97 GG 1.10 Aggiunti nuovi comandi ad uso del debugger remoto.
*             Comprendono la gestione della statistica, delle liste di
*             break, e la notifica di eventi asincroni ai debugger remoti.
* 11/03/98 GG 1.11 Aggiunta la chiamata di "ioDetach" alla conclusione
*             dell'entrata in HALT. Aggiunto il comando "SV_COMM_FORCEIO"
*             per la manipolazione di segnali di I/O da debugger (lavori
*             in corso).
* 13/03/98 GG 1.12 Completati i lavori sul comando "FORCEIO".
* 16/03/98 GG 1.13 Una modifica buona in debug: se e` impostata l'opzione
*             "-stdout", gli errori ed i messaggi sono diretti anche
*             verso stdout/stderr.
* 02/04/98 GG 1.14 Fatto sparire un warning.
* 10/06/98 GG 1.15 Aggiunto il comando "SV_COMM_SAVEAX" per il salvataggio
*             dei dati di un asse dato l'indice assoluto dell'asse.
* 04/09/98 GG 1.16 Utilizzata la nuova funzione "util_startprio" al posto
*             "getprio(0)".
* 18/02/98 GG 1.17 Aggiunto il comando "SV_COMM_SAVEGENTAB" per il salvataggio
*             dei dati di una tabella utente noto il nome.
* 28/07/00 GG 1.18 Aggiustamenti per compilazione su Linux.
* 28/07/00 GG 1.19 Aggiunti eventi asincroni per cancellazione errori
*             e gestione messaggi.
* 03/11/00 GG 1.20 Aggiunti quattro comandi per la gestione avanzata
*             del buffer di debug (vedi "debugger.*" e "remote.*").
* 07/11/00 GG 1.21 Aggiunto il log degli errori (nuova funzione "debErrorLog",
*             vedere "debugger.c" per i dettagli).
* 07/12/00 GG 1.22 Aggiunta nella routine di start del PLC
*             la chiamata di una funzione per ricalcolare
*             i dati utilizzati internamente dalle funzioni di validazione
*             degli indirizzi. Si e` resa necessaria su Linux, dove
*             il calcolo al volo e` lento, ed e` necessaria percio`
*             una "cache". Su LynxOS la funzione non fa nulla.
* 12/12/00 GG 1.23 Aggiunta la funzione "superv_saveorig", che salva
*             le tabelle origini, e il comando "SV_COMM_SAVEORIG", per
*             permettere la chiamata della funzione da esecutore.
* 08/02/01 GG 1.24 Aggiunta la funzione "superv_savetab", che salva
*             qualsiasi tabella, e il comando "SV_COMM_SAVETAB", per
*             permettere la chiamata della funzione da esecutore.
*             Eliminato codice inutile per Linux.
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#ifdef Linux
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#else
#include <types.h>
#include <ipc.h>
#include <msg.h>

/* Specifici CNi */
#include <services.h>
#endif

#include "qplc.h"
#include "dynaload.h"
#include "superv.h"
#include "debugger.h"
#include "cback.h"
#include "ioconf.h"

#include "util.h"

/*
* Struttura di stato del supervisore.
*/

sv_status_t superv_status;

/*
* Messa in START del Plc.
*/

static int superv_plc_start(void)
{
int code,rv;

/* Rilegge il database dei segnali di I/O. In caso di errori, non parte. */
	if (! cioLoad(0))
		return 0;
/* Reinizializza le associazioni di I/O. */
	ioRestart();
/* Esegue la parte "vecchio stile" dell'azione di START. */
	compat_start();
/* Esegue la procedura di start del modulo "user defined"
 (vecchio stile). */
	/* Reinizializza l'area dati del modulo. */
	rv = 0;
	dyInitData(main_status.mod_user);
	if (main_status.mod_user_op.start(&code)) {
		if (code) {
		/* AIUTO ! Non so che farmene ! */
		}
	/* Completa l'inizializzazione degli I/O prima del GO. */
		ioUp();
	/* Prepara il risultato "Successo" */
		rv = 1;
	}
/* Chiama la funzione per il ricalcolo dei dati utilizzati per la
 validazione dello spazio di indirizzamento. */
	util_check_addr_refresh();
	return rv;
}

/*
* Invio di un comando all'esecutore.
*/

void superv_to_exec(sv_msg_t *msg)
{
	exec_status.msg = msg;
	csem_wait(superv_status.sv_sync_sem, (struct timeval *) 0);
}

/*
* Funzione "superv_error"
* -----------------------
*
*  Questa funzione visualizza un errore di codice "code" e testo "text".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_error(long code, char * text)
{
sv_err_t err;

	err.code = code;
	err.text = text;
	cb_call(QPLC_CB_PLCERR,(void *)&err);

	debErrorLog(code,text);

	if (main_status.stand_alone || main_status.nstdout) {
		fprintf(stderr,"%s : ERROR %ld : %s\n",
			main_status.err_sigla,code,text);
	}
	if (! main_status.stand_alone) {
#ifdef Linux
#else
		invXerror(main_status.err_sigla, code,
		          text[0] ? "%n" : NULL, text);
#endif
	}
}

/*
* Funzione "superv_delerr"
* ------------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_delerr(long code)
{
int ec;

	ec = code;
	cb_call(QPLC_CB_PLCDELERR,(void *)&ec);

	if (main_status.stand_alone || main_status.nstdout) {
		fprintf(stderr,"%s : ERROR %ld DELETED\n",
		        main_status.err_sigla,code);
	}
	if (! main_status.stand_alone) {
#ifdef Linux
#else
		invXerror(main_status.err_sigla, code, "%k");
#endif
	}
}

/*
* Funzione "superv_warning"
* -------------------------
* 
*  Questa funzione visualizza un "warning" su stderr.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_warning(long code, char * text)
{
	fprintf(stderr,"%s : WARNING %ld : %s\n",
		main_status.err_sigla,code,text);
}

/* Funzioncina di comodo per visualizzare messaggi facilmente filtrabili. */
static void _superv_print_message(long code, char * text, char * what)
{
	printf("%s : MESSAGE %ld %s", main_status.err_sigla,code,what);
	if (text)
		printf(" %s",text);
	printf("\n");
}

/*
* Funzione "superv_message"
* -------------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_message(long code, char * text, int update)
{
sv_cbmsg_t msg;

	msg.code = code;
	msg.action = update ? SV_MSG_ACTION_UPDATE : SV_MSG_ACTION_ADD;
	msg.text = text;
	cb_call(QPLC_CB_PLCMSG,(void *)&msg);

	if (main_status.stand_alone || main_status.nstdout) {
		_superv_print_message(code, text, update ? "UPDATED" : ":");
	}
	if (! main_status.stand_alone) {
#ifdef Linux
#else
		invMessaggio(main_status.err_sigla, code,
		             text,
		             update ? XERTIP_MSG_SCRIVI_UP
		                    : XERTIP_MSG_SCRIVI);
#endif
	}
}

/*
* Funzione "superv_delmess"
* ------------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_delmess(long code)
{
sv_cbmsg_t msg;

	msg.code = code;
	msg.action = SV_MSG_ACTION_DELETE;
	msg.text = NULL;
	cb_call(QPLC_CB_PLCMSG,(void *)&msg);

	if (main_status.stand_alone || main_status.nstdout) {
		_superv_print_message(code, NULL, "DELETED");
	}
	if (! main_status.stand_alone) {
#ifdef Linux
#else
		invMessaggio(main_status.err_sigla, code,
		             NULL, XERTIP_MSG_CANC);
#endif
	}
}

/*
* Funzione "superv_saveax"
* ------------------------
*
*  Salva i dati asse dato l'indice assoluto "ax".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_saveax(long ax)
{
#ifdef Linux
#else
int cn;

	if (! main_status.m_data)
		return;
	if (GetAxesNumByNumInt(main_status.m_data,&cn,(int)ax) < 0)
		return;
	SetStatoDm(main_status.m_data);
	SetStartSave(main_status.m_data);
	SaveAxesTable(main_status.m_data,cn);
	SetEndSave(main_status.m_data);
#endif
}

/*
* Funzione "superv_savegentab"
* ----------------------------
*
*  Salva la tabella utente dato il nome.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_savegentab(char *tab)
{
#ifdef Linux
#else
	if (! main_status.m_data)
		return;
	SetStatoDm(main_status.m_data);
	SetStartSave(main_status.m_data);
	SaveGenericTable(main_status.m_data,tab);
	SetEndSave(main_status.m_data);
#endif
}

/*
* Funzione "superv_saveorig"
* --------------------------
*
*  Salva i dati origine.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_saveorig(void)
{
#ifdef Linux
#else
	if (! main_status.m_data)
		return;
	SetStatoDm(main_status.m_data);
	SetStartSave(main_status.m_data);
	SaveOrigTable(main_status.m_data);
	SetEndSave(main_status.m_data);
#endif
}

/*
* Funzione "superv_savetab"
* -------------------------
*
*  Salva la tabella qualsiasi dato il nome ed il centro.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_savetab(char *tabname, int cen)
{
#ifdef Linux
#else
	if (! main_status.m_data)
		return;
	SetStatoDm(main_status.m_data);
	SetStartSave(main_status.m_data);
	fwrite_mdata(main_status.m_data,tabname,cen);
	SetEndSave(main_status.m_data);
#endif
}

/*
* Caricamento dinamico del file "data".
*/

static void superv_dyload(char * path)
{
module_t * mod;
char *ver1[3];
char *ver2[3];
char **pver1[3];
char ***pver2[3];
char *name[3];
char index[3];
int i;
char **dummy = (char **) 0;

	index[0] = OLD_MOD_IDX_HATSH;
	index[1] = OLD_MOD_IDX_USER;
	index[2] = OLD_MOD_IDX_480;

	name[0] = OLD_MOD_STR_HATSH;
	name[1] = OLD_MOD_STR_USER;
	name[2] = OLD_MOD_STR_480;

	ver1[0] = main_status.mod_hatsh_op.version;
	ver1[1] = main_status.mod_user_op.version;
	ver1[2] = main_status.mod_plc480_op.version;

	pver2[0] = &dummy;
	pver2[1] = &main_status.mod_user_op.ver_il;
	pver2[2] = &dummy;
	pver1[0] = *pver2[0];
	pver1[1] = *pver2[1];
	pver1[2] = *pver2[2];

	mod = dyLoad(path, main_status.exe_path, MODULE_LABEL);
	if (! mod)
		return;
	brkReinit();

	ver2[0] = main_status.mod_hatsh_op.version;
	ver2[1] = main_status.mod_user_op.version;
	ver2[2] = main_status.mod_plc480_op.version;

	superv_status.last_module = mod;

	for (i = 0; i < 3; ++i) {
		if (strcmp(mod -> name, name[i]) == 0) {
			util_show_diff_version((pver1[i] && *pver1[i]) ?
					             *pver1[i] : ver1[i],
			                       (*pver2[i] && **pver2[i]) ?
					             **pver2[i] : ver2[i],
			                       index[i]);
			break;
		}
	}
}

/*
* Interprete dei comandi del supervisore. Puo` essere eseguita solo
* dal thread supervisore.
*/

void superv_exec(sv_msg_t *msg)
{
	switch (msg -> cmd) {
	case SV_COMM_GO:
		if (! superv_plc_start())
			msg -> cmd = SV_COMM_HALT;
		superv_to_exec(msg);
		break;
	case SV_COMM_HALT:
		superv_to_exec(msg);
		break;
	case SV_COMM_FINISH_HALT:
		ioClose();
		ioDetach();
		superv_to_exec(msg);
		break;
	case SV_COMM_FAULT:
		superv_to_exec(msg);
		break;
	case SV_COMM_EXECLIST:
		{
		void * p = *(void **)(msg -> data);
		sv_msg_t *q;

			exec_startmsg(p);

			while ( (q = exec_getmsg()) )
				superv_exec(q);
		}
		break;
	case SV_COMM_ERROR:
		superv_error(((sv_comm_error_t *)(msg -> data)) -> code,
		             ((sv_comm_error_t *)(msg -> data)) -> message);
		break;
	case SV_COMM_DELERR:
		superv_delerr(*(long *)(msg -> data));
		break;
	case SV_COMM_WARNING:
		superv_warning(((sv_comm_error_t *)(msg -> data)) -> code,
		               ((sv_comm_error_t *)(msg -> data)) -> message);
		break;
	case SV_COMM_MESSAGE:
		superv_message(((sv_comm_error_t *)(msg -> data)) -> code,
		               ((sv_comm_error_t *)(msg -> data)) -> message,0);
		break;
	case SV_COMM_MESSAGE_UPD:
		superv_message(((sv_comm_error_t *)(msg -> data)) -> code,
		               ((sv_comm_error_t *)(msg -> data)) -> message,1);
		break;
	case SV_COMM_DELMESS:
		superv_delmess(*(long *)(msg -> data));
		break;
	case SV_COMM_DYLOAD:
		superv_dyload(msg -> data);
		break;
	case SV_COMM_DYUNLOAD:
	/* Scaricamento dell'ultimo modulo caricato dinamicamente. */
		dyUnload(superv_status.last_module);
		brkReinit();
		break;
	case SV_COMM_OLDCOMM:
	/* L'esecutore ha richiesto l'aiuto del supervisore per
	 rispondere ad una richiesta impegnativa da parte di un
	 debugger vecchio stile. */
		compat_comm();
		break;
	case SV_COMM_CALLBACK:
	/* Attivazione della callback. */
		cb_call(QPLC_CB_SUPERV,(void *)msg);
		break;
	/* Callback per l'esecutore, definita altrove. */
	case SV_COMM_CBEXEC:
	/* Gestione della statistica. */
	case SV_COMM_ENMEAS:
	case SV_COMM_DISMEAS:
	case SV_COMM_CLRMEAS:
	case SV_COMM_MEAS:
	/* Gestione del debug in linea. */
	case SV_COMM_WCREATE:
	case SV_COMM_WDELETE:
	case SV_COMM_WADD:
	case SV_COMM_WENABLE:
	case SV_COMM_WDISABLE:
	case SV_COMM_WRESET:
	case SV_COMM_WLONGHISTORY:
	case SV_COMM_WSHORTHISTORY:
	case SV_COMM_WLOCKHISTORY:
	case SV_COMM_WUNLOCKHISTORY:
		superv_to_exec(msg);
		break;
	case SV_COMM_EVNOTIFY:
		cb_setarg(QPLC_CB_EVNOTIFY,
		          (void *)(*(long *)(msg -> data)),
		          (void **)NULL);
		cb_call(QPLC_CB_EVNOTIFY,(void *)(msg -> data + sizeof(long)));
		break;
	case SV_COMM_FORCEIO:
		{
		sv_comm_forceio_t *p = (sv_comm_forceio_t*)(msg -> data);
		devnode_t l;

		if (!plcIsHalt()) {
			p -> rv = -1;
		}
		else if (!ioParse(p -> path,
		                  (p -> mode ? DRIVER_MODE_OUTPUT
		                             : DRIVER_MODE_INPUT),
		                  &l)
		 || !ioCoreAttach(&l,(void *)&p -> val)) {
			p -> rv = 0;
		}
		else {
			p -> bit = l.nbit;
			p -> sem = superv_status.sv_lockexec_sem;
			superv_to_exec(msg);
			csem_wait(superv_status.sv_lockexec_sem,
			          (struct timeval *) 0);
			ioDetach();
		}
		}
		break;
	case SV_COMM_SAVEAX:
		superv_saveax(*(long *)(msg -> data));
		break;
	case SV_COMM_SAVEGENTAB:
		superv_savegentab((char *)(msg -> data));
		break;
	case SV_COMM_SAVEORIG:
		superv_saveorig();
		break;
	case SV_COMM_SAVETAB:
		{
		sv_savetab_t *p = (sv_savetab_t*)(msg -> data);

		superv_savetab(p -> tabname, p -> cen);
		}
		break;
	default:
		break;
	}
}

/*
* Questo e` il "cuore" del supervisore. Controlla i canali di
* comunicazione col debugger e l'attivita` del supervisore.
*/

static void * superv_main(void * arg)
{
	for (;;) {

	/* Attesa sul semaforo. */

		csem_wait(superv_status.sv_act_sem, (struct timeval *) 0);

	/* Elaborazione del comando. */

		superv_exec(superv_status.msg);

	/* Comunicazione dell'avvenuta esecuzione del comando. */

		if (superv_status.sync) {
			superv_status.sync = 0;
		}
		else {
			csem_signal(superv_status.sv_answ_sem);
		}
	}
}

/*
* Funzione "superv_start"
* -----------------------
*
*  Questa funzione lancia il thread supervisore.
*/

void superv_start(void)
{
pthread_mutexattr_t attr;

/* Inizializzazione della struttura di stato del supervisore. */

/* Creazione del semaforo di attivazione del supervisore. */

	superv_status.sv_act_sem = csem_create_val(0);

/* Creazione del semaforo di risposta del supervisore. */

	superv_status.sv_answ_sem = csem_create_val(0);

/* Creazione del semaforo di sincronizzazione coll'esecutore. */

	superv_status.sv_sync_sem = csem_create_val(0);

/* Creazione del semaforo di sincronizzazione coll'esecutore per
 operazioni che richiedono piu` di un ciclo. */

	superv_status.sv_lockexec_sem = csem_create_val(0);

/* Creazione del semaforo di accesso ai messaggi. */

#ifdef Linux
	pthread_mutexattr_init(&attr);
#else
	pthread_mutexattr_create(&attr);
#endif
	pthread_mutex_init(&superv_status.m_mutex,&attr);
#ifdef Linux
	pthread_mutexattr_destroy(&attr);
#else
	pthread_mutexattr_delete(&attr);
#endif

	superv_status.sync = 0;
	superv_status.msg = (sv_msg_t *) 0;

	superv_status.last_module = (module_t *) 0;

/* Imposta lo stato iniziale di HALT o di START a seconda della opzioni. */

/* NOTA : La chiamata di "plcHalt" e "plcStart" qui ha senso :
 il thread che sta eseguendo questa parte di codice e` lo stesso
 che eseguira` il ciclo principale del Plc. */

	if (main_status.halt) {
		plcHalt();
	}
	else {
		if (superv_plc_start())
			plcStart();
		else
			plcHalt();
	}

/* Lancio del supervisore. */

	superv_status.tid = util_start_thread(superv_main, util_startprio());
}

/*
* Funzione "superv_talk" e "superv_talk_nonblock"
* -----------------------------------------------
*
*  Questa funzione trasmette al supervisore il messaggio "msg". All'uscita,
* l'area puntata da "msg" conterra` la risposta, se il comando la prevede.
*  Tale area deve percio` essere statica, o comunque disponibile e non
* alterata per tutta la durata del colloquio tra supervisore e chiamante.
*
*  L'esecutore del programma PLC deve sempre utilizzare la versione "nonblock"
* della funzione, che, anziche` essere "void", e` "int", e vale
*  SUPERV_PHASE_DONE : l'operazione e` stata completata,
*  SUPERV_PHASE_BUSY : risorsa bloccata, altro colloquio in corso;
*  SUPERV_PHASE_WORKING : lock impostato, attesa di risposta.
*  E` inoltre presente un altro parametro, "phase", che deve contenere
* il valore di ritorno della funzione alla chiamata precedente. La variabile
* che il task utilizzera` per memorizzarlo e` di fatto una variabile di
* stato che rappresenta la fase di avanzamento dell'elaborazione di un
* comando da parte del supervisore. Il parametro "phase" deve valere 0
* quando un task chiama per la prima volta questa funzione.
*/

void superv_talk(sv_msg_t *msg)
{
/* Blocco della risorsa. */

	pthread_mutex_lock(&superv_status.m_mutex);

/* Impostazione del tipo di sincronizzazione : semaforo. */

	superv_status.sync = 0;

/* Copia del messaggio. */

	superv_status.msg = msg;

/* Attivazione del supervisore. */

	csem_signal(superv_status.sv_act_sem);

/* Attesa del completamento dell'operazione da parte del supervisore. */

	csem_wait(superv_status.sv_answ_sem, (struct timeval *) 0);

/* Operazione completata. Sblocco della risorsa. */

	pthread_mutex_unlock(&superv_status.m_mutex);
}

int superv_talk_nonblock(sv_msg_t *msg, int phase)
{
	if (phase != SUPERV_PHASE_WORKING) {

	/* Fase iniziale. Tenta di bloccare la risorsa, esce subito
	 se la risorsa e` occupata. */

		if (pthread_mutex_trylock(&superv_status.m_mutex))
			return SUPERV_PHASE_BUSY;

	/* Imposta la modalita` di sincronizzazione : polling. */

		superv_status.sync = 1;

	/* Copia il comando. */

		superv_status.msg = msg;

	/* Attivazione del supervisore. */

		csem_signal(superv_status.sv_act_sem);

	/* Uscita, con segnalazione del valore di stato "lavori in corso". */

		return SUPERV_PHASE_WORKING;
	}
	else {

	/* Rientro dopo la segnalazione di "lavori in corso". Si tratta
	 ora di attendere che il supervisore completi il comando
	 e spenga il flag "sync". */

		if (superv_status.sync) {
		/* Elaborazione del comando non ancora completata.
		 Si ritenti. */
			return SUPERV_PHASE_WORKING;
		}

	/* Comando completato. */

	/* Sblocco della risorsa. */

		pthread_mutex_unlock(&superv_status.m_mutex);

	/* Fine. */

		return SUPERV_PHASE_DONE;
	}
}

