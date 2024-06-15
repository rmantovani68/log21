/*
* @(#) drivers.h 3.4 Fri May 11 15:28:16 CEST 2001
*
*  Gestione dei dispositivi di QPLC.
*  I dispositivi sono rappresentati da una stringa cosi` composta :
*
*     <nome del dispositivo>.<istanza>.<stringa>.[<stringa>...]
*
*  Il nome del dispositivo identifica il paricolare driver
* che si intende attivare, "istanza" e` un numero che identifica l'istanza
* del dispositivo, cio` che segue ha un significato
* che dipende esclusivamente dal dispositivo.
*
*  Tutti i driver riconoscono i seguenti servizi :
*
*  install  : chiamata al lancio di QPLC, esegue l'allocazione delle
*             aree necessarie al driver.
*  restart  : chiamata ad ogni GO per inizializzare il dispositivo.
*  check    : chiamata ad ogni ciclo di PLC per controllare lo stato del
*             dispositivo.
*  trigger  : Chiamata ad ogni ciclo per operazioni di "mantenimento"
*             del dispositivo (tipicamente, rinfresco di watchdog).
*             La funzione e` chiamata anche quando si e` in HALT, a meno
*             che non si sia verificato un errore fatale.
*  parse    : controlla la correttezza del nome di variabile dato (parte
*             dipendente dal dispositivo), ed eventualmente prepara il
*             terreno per la "attach".
*  attach   : associa una variabile di input o di output
*             alla "linea" descritta dall'ultima "parse".
*  detach   : ritira tutte le associazioni realizzate da "attach".
*  up       : chiamata all'entrata in GO, dopo la "restart" e le "attach".
*  down     : chiamata tipicamente all'entrata in HALT, per portare in uno
*             stato sicuro gli I/O.
*  close    : chiamata nella fase finale dell'entrata in HALT dal supervisore,
*             per eseguire azioni legate al sistema e potenzialmente bloccanti.
*  fatal    : chiamata in caso di emergenza per portare in uno stato
*             sicuro i dispositivi. Il suo effetto puo` essere irreversibile.
*  read     : chiamata all'inizio di ogni ciclo per acquisire lo stato
*             degli input.
*  write    : chiamata alla fine del ciclo per aggiornare gli output.
*  list     : chiamata dalla ioList per ottenere la descrizione dei
*             fratelli e dei figli di un nodo. 
*  show     : chiamata dalla ioShow per ottenere in modo non invasivo,
*             se possibile, lo stato di un segnale descritto nella forma
*             data da "ioList" o "ioParse".
*  extension: chiamata durente il ciclo dalla funzione "ioExtendedOp" per
*             eseguire operazioni che non rientrano nel modello ordinario
*             del PLC. Vale 1 in caso di successo, 0 in caso di errore.
*             Ha come perametri il driver, il valore registrato con
*             "ioRegisterExtension", e due parametri dati dal chiamante
*             (un codice di comando ed un dato arbitrario).
*
*  Ogni driver e` descritto da un handle, che contiene, tra le altre
* cose, i metodi associati.
*
*  Per comodita` di scrittura dei driver, si riporta qui lo
* scheletro di un generico driver, che puo` essere preso come base
* per lo sviluppo. Si sostituisca "xxx" con il nome del dispositivo.
static int xxx_install(driver_t * d, int conf)
{
}
static void xxx_restart(driver_t * d)
{
}
static int xxx_parse(driver_t * d, int i_off, devnode_t * l)
{
}
static int xxx_list(driver_t * d, devnode_t * l)
{
}
static int xxx_attach(driver_t * d, devnode_t * l, void *var)
{
}
static int xxx_detach(driver_t * d)
{
}
static int xxx_check(driver_t * d)
{
}
static void xxx_trigger(driver_t * d)
{
}
static void xxx_up(driver_t * d)
{
}
static void xxx_down(driver_t * d)
{
}
static void xxx_close(driver_t * d)
{
}
static void xxx_fatal(driver_t * d)
{
}
static void xxx_read(driver_t * d)
{
}
static void xxx_write(driver_t * d)
{
}
static int xxx_show(driver_t * d, devnode_t * l, void * dest)
{
}
static int xxx_extension(driver_t * d, void * sys_p, int cmd, void * user_p)
{
}

driver_op_t driver_xxx = {
	"xxx",
	xxx_install,
	xxx_restart,
	xxx_parse,
	xxx_list,
	xxx_attach,
	xxx_detach,
	xxx_check,
	xxx_trigger,
	xxx_up,
	xxx_down,
	xxx_close,
	xxx_fatal,
	xxx_read,
	xxx_write,
	xxx_show,
	xxx_extension
};

* NOTA : Tutti i metodi, eccetto "install", possono essere lasciati
* indefiniti, indicando NULL nel campo corrispondente della struttura.
*
* 19/09/96 GG 1.2 Aggiunto un meccanismo di visualizzazione degli errori
*                 rilevati dalle "check".
* 15/04/97 GG 1.3 Aggiunta la gestione delle variabili di configurazione
*                 provenienti dalla linea di comando (funzioni ioStartResource
*                 ed ioNextResource).
* 16/07/97 GG 2.0 Aggiunti i metodo "up" e "close" dei driver, e le funzioni
*                 "ioUp" ed "ioClose" per chiamarli.
*                 Aggiunto il metodo "extension", e la funzione "ioExtendedOp"
*                 per chiamarlo. Questo metodo implica la registrazione
*                 dell'estensione in un'apposita tabella con la funzione
*                 "ioRegisterExtension".
*                 Aggiunto un numero d'ordine unico per le istanze dei driver,
*                 analogo al "major number" dei sistemi UNIX.
* 11/03/98 GG 2.1 Aggiunto il metodo "detach", che ritira tutte le
*                 associazioni realizzate con "attach". Aggiunta ovviamente
*                 anche la funzione "ioDetach".
* 24/03/98 GG 2.2 Aggiunto il metodo "list", in vista dell'aggiunta del
*                 servizio di lista dei dispositivi collegati.
*                 Aggiunta la funzione "ioList", interfaccia verso i
*                 metodi "list" dei driver.
*                 Aggiunto il parametro "conf" al metodo "install" e
*                 alla funzione "ioInstall".
* 01/04/98 GG 3.0 Sconvolto tutto. I metodi "attach", "parse" e "list"
*                 ora usano gli stessi tipi. Pericolo di sfighe.
* 03/04/98 GG 3.1 Aggiunto il metodo "show", e le funzioni "ioShow"
*                 e "ioParse".
* 24/09/98 GG 3.2 Aggiunta la definizione di una struttura adatta a
*                 contenere le associazioni di I/O in forma leggibile.
*                 Sara` utilizzata dal debugger.
*                 Aggiunte la funzioni "ioGetBindings", che fornisce
*                 il puntatore alla lista delle associazioni in chiaro,
*                 e "ioFindBinding", che convalida un puntatore a un
*                 elemento della lista.
* 01/10/98 GG 3.3 Resa pubblica la funzione "ioCanonify".
* 11/05/01 GG 3.4 Aggiunta la funzione "ioClearErrors", che cancella
*                 gli errori di I/O memorizzati. Deve essere chiamata
*                 all'entrata in GO e all'entrata in HALT.
*/

#ifndef _QPLC_DRIVERS_H_

/*
* Flag di modo di accesso (valori utilizzabili per il parametro "mode"
* del metodo "parse" e del campo "flags" del metodo "list".
*/

#define DRIVER_MODE_INPUT 0x01
#define DRIVER_MODE_OUTPUT 0x02
#define DRIVER_MODE_LIST 0x04 /* Nodo esplorabile (solo per "list"). */

/*
* Tipo descrivente i metodi previsti da un driver.
*/

#define MAX_DRIVER_NAME 10

struct _driver_t;
struct _devnode_t;

typedef struct _driver_op {

/* Nome del driver. */

	char name[MAX_DRIVER_NAME+1];

/* Installatore, chiamato una volta per tutte al lancio di QPLC.
  Il parametro e` il descrittore di driver appena allocato.
  Se la primitiva fallisce, il descrittore e` liberato. */

	int (*install)(struct _driver_t * self, int conf);

/* Inizializzatore "a caldo", chiamato ad ogni entrata in GO. */

	void (*restart)(struct _driver_t * driver);

/* Traduttore dei parametri di una variabile : verifica l'esistenza
 della variabile indicata dal parametro "name" e ne restituisce la
 descrizione interna nei campi appositi della struttura "*l", che puo`
 essere utilizzata in seguito da "attach" per realizzare l'associazione
 con una variavile.
 Il parametro "i_off" serve quando si desidera interpretare
 il campo "istanza" come dato aggiuntivo per il driver specifico. Vale
 normalmente 0, salvo quando si tenta la "ioAttach" con un numero di istanza
 inesistente, nel qual caso il parametro conterra` la differenza tra il
 numero di istanza richiesto ed il piu` alto esistente. */

	int (*parse)(struct _driver_t * driver,int i_off,struct _devnode_t *l);

/* Produzione delle informazioni caratteristiche di un elemento di I/O
 controllato dal driver. La struttura data come secondo argomento fornisce
 informazioni sul dispositivo "precedente" o "genitore" (queste parole
 hanno un significato generale conformemente all'architettura gerarchica
 dei driver e della descrizione degli indirizzi, e un significato
 specifico del particolare dispositivo, che e` responsabile della
 gestione di parte della gerarchia). */

	int (*list)(struct _driver_t * driver, struct _devnode_t * l);

/* Definitore di variabile : assegna la variabile di indirizzo "var"
 al segnale identificato dalla struttura "*l" inizializzata da "parse"
 o da "list".
  L'area "var" deve  avere la dimensione in bit indicata dal valore di
 ritorno della "parse". */

	int (*attach)(struct _driver_t * driver,struct _devnode_t *l,void *v);

/* Funzione per la rimozione delle associazioni di I/O. Non altera
 lo stato del dispositivo. */

	void (*detach)(struct _driver_t * driver);

/* Funzione di test, chiamata ad ogni ciclo dall'esecutore. Se fallisce,
 l'esecutore entra in HALT, ed e` chiamata la "disable". */

	int (*check)(struct _driver_t * driver);

/* Funzione di "mantenimento" del dispositivo. E` chiamata ad ogni ciclo,
 anche in HALT, per eseguire le operazioni periodiche richieste dal
 dispositivo. La sua chiamata e` sospesa quando si verifica un errore
 fatale. */

	void (*trigger)(struct _driver_t * driver);

/* Funzione per il completamento della procedura di inizializzazione del
 dispositivo, chiamata all'entrata in GO dop "down" e "attach". */

	void (*up)(struct _driver_t * driver);

/* Funzione di "azzeramento" del dispositivo, chiamata all'entrata in
 HALT. */

	void (*down)(struct _driver_t * driver);

/* Funzione di liberazione delle risorse occupate dal dispositivo,
 chiamata dal thread supervisore nella fase finale dell'entrata in HALT. */

	void (*close)(struct _driver_t * driver);

/* Funzione chiamata in caso di errore o emergenza. */

	void (*fatal)(struct _driver_t * driver);

/* Lettura degli imput, chiamata dall'esecutore all'inizio di ogni ciclo. */

	void (*read)(struct _driver_t * driver);

/* Scrittura degli output, chiamata dall'esecutore alla fine di ogni ciclo. */

	void (*write)(struct _driver_t * driver);

/* Lettura non invasiva dello stato di un segnale di I/O. Il valore del
 segnale e` copiato in "v", che deve essere di dimensioni adeguate, e
 la descrizione del segnale deve essere ottenuta con "ioParse" o "ioList". */

	int (*show)(struct _driver_t * driver, struct _devnode_t * l, void *v);

/* Esecuzione di operazioni particolari. */

	int (*extension)(struct _driver_t * driver, void *s_p,
	                 int cmd, void *u_p);
} driver_op_t;

/*
* Completamento del tipo "driver_t".
*/

/*
* Tipo "driver handle", prodotto dal metodo "install" del driver.
*/

typedef struct _driver_t {

/* Metodi associati al driver. */

	struct _driver_op op;

/* Istanza del dispositivo. */

	int instance;

/* Indice globale (analogo al major number dei sistemi UNIX). */

	int index;

/* Indicatore di errore sul dispositivo. */

	int error;

/* Puntatore al successivo driver. */

	struct _driver_t * next;

/* Puntatore al successivo driver nella catena di input. */

	struct _driver_t * next_in;

/* Puntatore al successivo driver nella catena di output. */

	struct _driver_t * next_out;

/* Puntatore al successivo driver nella catena di check. */

	struct _driver_t * next_check;

/* Puntatore al successivo driver nella catena di trigger. */

	struct _driver_t * next_trigger;

/* Puntatore alla posizione corrente di lettura nella lista delle risorse. */

	char * resource_pointer;

/* Puntatore alla parte dipendente dal dispositivo. */

	void * device;

} driver_t;

/*
*  Struttura base, contenente i puntatori alle liste di input, output,
* trigger, check, e quella di tutti i driver installati.
*/

typedef struct {

/* Puntatore alla lista dei driver installati. */

	driver_t * driver_list;

/* Puntatore alla lista dei driver interessati dalla lettura degli input. */

	driver_t * in_list;

/* Puntatore alla lista dei driver interessati dalla lettura degli output. */

	driver_t * out_list;

/* Puntatore alla lista dei driver che possiedono il metodo "check". */

	driver_t * check_list;

/* Puntatore alla lista dei driver che possiedono il metodo "trigger". */

	driver_t * trigger_list;

/* Numero di istanziazioni eseguite. */

	int ndrvrs;

/* Puntatore alla tabella dei driver estesi e sua dimensione. */

	struct _extended_driver_t * extensions;
	int n_ext;

} driver_control_t;

/* Struttura descrivente un'estensione delle operazioni. Questa struttura
 descrive una particolare linea di un'istanza di driver esteso. Contiene
 informazioni necessarie alla funzione "ioExtendedOp" per reperire il
 metodo ed i parametri assiciati alla linea. L'indirizzo di questa
 struttura e` copiato nella variabile di input associata alla linea. */

typedef struct _extended_driver_t {
/* Puntatore all'istanza del driver. */
	driver_t * driver;
/* Parametro descivente la linea. */
	void * param;
} extended_driver_t;

/* Struttura descrivente un nodo dell'albero dei dispositivi. Ad uso
 della "ioList". */

#define MAX_DEVLIST_SPEC 8
#define MAX_DEVLIST_NAME 48
#define MAX_DEVLIST_COMMENT 32

typedef struct _devnode_param_t {
/* Puntatore all'istanza del driver. */
	driver_t * driver;
/* Dati interni specifici del dispositivo. */
	unsigned char spec[MAX_DEVLIST_SPEC];
} devnode_param_t;

typedef struct _devnode_t {
/* Descrittore di accesso al sottoalbero che fa capo al nodo corrente
 (albero dei nodi "figlio"). */
	devnode_param_t tree;
/* Descrittore di accesso al successivo nodo allo stesso
 livello del corrente (nodo "fratello"). */
	devnode_param_t next;
/* Indice di profondita`. */
	int ideep;
/* Proprieta` del nodo (vedi "DRIVER_MODE_..."). */
	int flags;
/* Dimensione in bit del segnale associato. */
	int nbit;
/* Stringa prodotta da "ioList". */
	char name[MAX_DEVLIST_NAME];
/* Puntatore corrente in "name" (punta alla parte "device specific"). Puo`
 essere alterato dai driver. */
	char *pname;
/* Informazioni aggiuntive per l'utente. */
	char comment[MAX_DEVLIST_COMMENT];
} devnode_t;

/* Struttura descrivente un elemento della lista di associazione di I/O
 in forma leggibile (ad uso del debugger). */

typedef struct _iobind_t {
/* Proprieta` del nodo (vedi "DRIVER_MODE_..."). */
	int flags;
/* Dimensione in bit del segnale associato. */
	int nbit;
/* Stringa prodotta da "ioList". */
	char name[MAX_DEVLIST_NAME];
/* Indirizzo della variabile associata al segnale. */
	void *addr;
/* Puntatore all'elemento successivo della lista. */
	struct _iobind_t *next;
} iobind_t;

/*******************************************/
/* COMANDI PER LA PRIMITIVA "ioExtendedOp" */
/*******************************************/

/* Primitive per dispositivi di tipo "stream" (seriali, socket, pipe...)
 e strutture associate. */

#define DREXTOP_STREAM_OP_SEND 0x01 /* Trasmissione. */
#define DREXTOP_STREAM_OP_RECV 0x02 /* Ricezione. */
typedef struct _stream_rxtx_t {
	int msg_size; /* Numero di byte da trasmettere o ricevere. */
	int msg_curr_pos; /* Numero di byte trasmessi o ricevuti finora. */
	                  /* Deve essere azzerato la prima volta. */
	int status; /* Vari flag di stato. */
	char *message; /* Puntatore al buffer. */
} stream_rxtx_t;
/* Significato del campo "status". */
#define DREXTOP_STREAM_F_DONE 0x0001 /* Operazione completata. */
#define DREXTOP_STREAM_F_ERRS 0x0002 /* Errore generico. */

/************/
/* FUNZIONI */
/************/

/*
* Funzione "ioRead"
* -----------------
*
*  Questa funzione e` chiamata all'inizio di ogni ciclo di PLC
* per leggere gli input.
*/

void ioRead(void);

/*
* Funzione "ioWrite"
* -----------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC
* per aggiornare gli output.
*/

void ioWrite(void);

/*
* Funzione "ioCheck"
* -----------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC, dopo
* l'aggiornamento degli output, per controllare lo stato dei dispositivi.
*  Vale 0 in caso di errore, altrimenti il valore e` diverso da zero.
*/

int ioCheck(void);

/*
* Funzione "ioRestart"
* --------------------
*
*  Questa funzione e` chiamata ogni volta che il PLC entra in GO.
*/

void ioRestart(void);

/*
* Funzione "ioTrigger"
* --------------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC, dopo
* l'aggiornamento degli output, per il refresh dei dispositivi che lo
* richiedano. La funzione e` chiamata anche quando il PLC e` in HALT.
*/

void ioTrigger(void);

/*
* Funzione "ioDetach"
* -------------------
*
*  Questa funzione e` chiamata ogni volta che sia necessario ridefinire
* le associazionei di I/O.
*/

void ioDetach(void);

/*
* Funzione "ioUp"
* ---------------
*
*  Questa funzione e` chiamata immediatamente prima dall'entrata in RUN
* del PLC, dopo che sono state realizzate tutte le associazioni di I/O.
*/

void ioUp(void);

/*
* Funzione "ioDown"
* -----------------
*
*  Questa funzione e` chiamata all'uscita dalla modalita` RUN del PLC,
* per azzerare gli output in modo sicuro.
*/

void ioDown(void);

/*
* Funzione "ioClose"
* ------------------
*
*  Questa funzione e` chiamata all'uscita dalla modalita` RUN del PLC,
* per eseguire le operazioni "lente" che possono coinvolgere risorse di
* sistema, attese indefinite... A differenza della "ioDown", e` eseguita
* dal thread supervisore, percio` non risente delle limitazioni di
* tempo tipiche dell'esecutore.
*/

void ioClose(void);

/*
* Funzione "ioFatal"
* ------------------
*
*  Questa funzione e` chiamata in caso di emergenza, errori fatali
* ed altro. Il suo effetto puo` essere irreversibile.
*/

void ioFatal(void);

/*
* Funzione "ioInstall"
* --------------------
*
*  Questa funzione e` chiamata al lancio di QPLC.
*  Installa un'istanza del driver indicato dalla struttura indicata
* come parametro, e passa al driver il parametro aggiuntivo "conf".
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioInstall(driver_op_t * self, int conf);

/*
* Funzione "ioGetInstance"
* ------------------------
*
*  Questa funzione fornisce l'istanza "n" del driver di nome "name",
* oppure NULL se la combinazione non esiste.
*/

driver_t * ioGetInstance(char * name, int n);


/*
* Funzione "ioGetLastInstance"
* ----------------------------
*
*  Questa funzione fornisce l'ultima istanza del driver di nome "name",
* oppure NULL se il driver non esiste.
*/

driver_t * ioGetLastInstance(char * name);

/*
* Funzione "ioGetNum"
* -------------------
*
*  Questa funzione estrae dalla stringa data il primo numero, e calcola
* la posizione di lettura successiva. La stringa deve avere la forma
* <sequenza_di_cifre_decimali>['.'<altro>]. La posizione di lettura
* e` rappresentata da una variabile, che all'ingresso punta alla stringa,
* ed all'uscita punta oltre il "." (o sul '\0' di fine stringa). La funzione
* si aspetta l'indirizzo di questa variabile.
*  Un risultato uguale a -1 e` da interpretare come errore. In quel caso,
* la variabile non viene alterata.
*/

int ioGetNum(char ** p);

/*
* Funzione "ioParse"
*
*  Questa funzione traduce la stringa di descrizione di un segnale di
* I/O nella sua descrizione interna.
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioParse(char * name, int mode, devnode_t *l);

/*
* Funzione "ioCoreAttach"
* -----------------------
*
*  Questa funzione collega una variabile ad un segnale di I/O descritto
* dalla struttura "*l" (ottenuta con "ioList" o "ioParse").
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioCoreAttach(devnode_t *l, void * addr);

/*
* Funzione "ioAttach"
* --------------------
*
*  Questa funzione e` chiamata tipicamente ad ogni rilancio del PLC.
*  Serve a collegare una variabile di indirizzo e dimensione dati ad un
* segnale di input o di output ("mode" decide). La variabile e` descritta
* da una stringa nella forma "nome.istanza.altro...", dove "nome" e` il nome
* del driver, "istanza" e` il numero di istanza dello stesso, "altro" e` una
* sequenza di caratteri qualsiasi, la cui interpretazione e` a carico del
* driver.
*  "size" e` la dimensione della variabile associata IN BIT.
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioAttach(char * name, void * addr, int size, int mode);

/*
* Funzione "ioList"
* -----------------
*
* Questa funzione fornisce il nodo "successivo" a quello indicato
* dalla struttura puntata dal parametro "l". Se il campo "ideep"
* vale 0, si intente esplorare la radice dell'albero dei dispositivi.
* Il campo "explore" indica la profondita` con cui si intende esplorare
* la gerarchia. La funzione, ricorrendo eventualmente ai metodi "list"
* specifici dei driver, fornisce il "successivo" elemento al livello
* "explore" di quello descritto dai restanti campi. Il valore dei campi
* si intende prodotto da una precedente chiamata della stessa funzione
* allo stesso livello o a quello immediatamente superiore. La funzione
* non controlla, se non superficialmente, la sensatezza dei dati
* della struttura. Se comunque viene rilevata un'incongruenza, essa vale
* -1, altrimenti vale 0. La fine dell'esplorazione di un livello e`
* segnalata dalla stringa vuota nel campo "name".
*/

int ioList(devnode_t *l);

/*
* Funzione "ioShow"
*
*  Questa funzione riporta nel buffer puntato da "dest" lo stato
* del segnale descritto da "l".
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioShow(devnode_t *l, void *dest);

/*
* Funzione "ioClearErrors"
* ------------------------
*
*  Questa funzione cancella gli errori registrati nella lista degli
* errori di I/O rilevato dai driver. Deve essere chiamata all'entrata
* in GO.
*/

void ioClearErrors(void);

/*
* Funzione "ioAddError"
* ---------------------
*
*  Questa funzione registra i dati relativi ad un problema hardware
* rilevato dal driver "driver". Se l'errore non e` ancora stato segnalato,
* e` aggiunto alla lista, e provochera` una visualizzazione.
*/

void ioAddError(driver_t *driver, int code, long data);

/*
* Funzione "ioStartResource"
* --------------------------
*
*  Questa funzione inizia una sequenza di letture di variabili di
* configurazione inerenti al dispositivo "d".
*/

void ioStartResource(driver_t *d);

/*
* Funzione "ioNextResource"
* -------------------------
*
*  Questa funzione fornisce il puntatore alla prima variabile di
* configurazione inerente al dispositivo "d".
*/

char * ioNextResource(driver_t *d);

/*
* Funzione "ioRegisterExtension"
* ------------------------------
*
* Questa funzione fornisce un codice da utilizzare per la chiamata
* di specifici servizi non standard messi a disposizione da certi driver.
* Il codice ottenuto dovra` essere utilizzato per accedere al servizio
* tramite la funzione "ioExtendedOp".
* Questa funzione cerca di produrre un codice tale da essere difficilmente
* confuso, in modo che sia poco probabile l'accesso accidentale a servizi
* per errori di programmazione.
* Il secondo parametro sara` passato al matodo "extension" del driver
* al momento della chiamata.
* Vale 0 in caso di errore.
*/

long ioRegisterExtension(driver_t *d, void *s_p);

/*
* Funzione "ioExtendedOp"
* -----------------------
*
* Questa funzione invoca il metodo "extension" del driver e con i parametri
* indicati dall'indice di estensione dato. L'indice deve essere stato
* ottenuto con "ioRegisterExtension". Il secondo parametro sara` passato al
* metodo come terzo parametro, il terzo come quarto.
* La funzione vale 1 se tutto e` andato bene, 0 in caso di errori.
*/

int ioExtendedOp(long ex, int cmd, void * u_p);

/*
* Funzione "ioGetBindings"
* ------------------------
*
*  Questa funzione fornisce il puntatore alla lista delle associazioni
* in chiaro.
*/

iobind_t * ioGetBindings(void);

/*
* Funzione "ioFindBinding"
* ------------------------
*
*  Questa funzione fornisce il puntatore all'elemento di chiave "key"
* della lista di associazione, oppure NULL se la chiave non e` presente.
* Attualmente la chiave non e` altro che l'indirizzo stesso.
*/

iobind_t * ioFindBinding(unsigned long key);

/*
* Funzione "ioCanonify"
* ---------------------
*
*  Normalizzazione di un nome di I/O: "src" e` convertito in minuscolo e sono
* eliminati gli zeri superflui dei campi numerici. Il risultato e`
* scaricato in "dest", di dimensioene "len" (compreso NUL finale).
*  La funzione elimina anche l'eventuale "_" iniziale dal nome, e restituisce
* 1 se l'eliminazione e` avvenuta, altrimenti 0.
*/

int ioCanonify(char *dest, int len, char *src);

#define _QPLC_DRIVERS_H_

#endif

