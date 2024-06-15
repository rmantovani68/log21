/*
* @(#) port.h 2.6 Thu Jul  3 09:51:53 MET DST 1997
* @(#) Rif. Guerrini
*
*  Questo file contiene le definizioni utilizzate dalle interfacce a livello
* applicativo con le primitive di sistema. Tali interfacce sono raccolte in
* "grsy0lyn.c". Altri punti in cui si utilizzano queste definizioni
* sono l'inizio e la fine della funzioni "main" delle due parti del
* pacchetto (file "isaker.c" e "isatst.c"), nei quali le risorse sono,
* se necessario, allocate, inizializzate o rilasciate, e nel file che
* contiene le primitive di accesso ai dispositivi di I/O.
*
* 251194 Versione 1.0a.
* 160195 Versione 1.1. Aggiunte definizioni per la configurabilita` del
*        dispositivo di I/O.
* 080295 Versione 1.2. Aggiunti due nuovo spazi, finche` si e` in tempo.
* 030395 Versione 1.3. Aggiunte dimensioni per tabelle dei segnali speciali.
* 230395 Versione 1.4. Portato a 20 il numero di spazi. Definita una nuova
*        variabile globale di stato.
* 270495 Versione 1.5. Aggiunto il flag legato all'opzione "-u".
* 220595 Versione 1.6. Aggiunta la definizione del nome standard per il modulo
*        "user defined".
* 260695 Versione 1.7. Aggiunto il valore iniziale per l'override.
* 270695 Versione 1.8. Aggiunto un nuovo modulo caricabile dinamicamente.
* 040795 Versione 1.9. Aggiunta la gestione del riaggancio dei moduli.
* 180995 Versione 2.0. Unificati i sorgenti per LynxOS e Solaris.
* 231195 Versione 2.1. Aggiunto il flag di gestione della tastiera speciale.
* 041295 Versione 2.2. Aggiunta la misura sw del tempo di ciclo.
* 181295 Versione 2.3. Aggiunta la riconfigurabilita` della dimansione della
*        tabella dei segnali di scambio e del nome del database.
* 290196 Versione 2.4. Introdotto il modulo "hatsh". Automatizzato il
*        caricamento dinamico dei moduli alla partenza.
* 220296 Versione 2.5. Aggiunto un flag per la corretta abilitazione run-time
*        della statistica.
* 03/07/97 GG 2.6. Modificata la dimensione di default della shared memory.
*/

#ifndef _PORT_H

#include <termios.h>
#include <sys/types.h>

/* Numero massimo di esecutori ammesso. */

#define MAX_SLAVE 2

/* Numero di spazi per esecutore (DA NON MODIFICARE) */

#define  MAX_SPACE 20

/* Dimensione del direttorio degli spazi e dei blocchi liberi. */

#define MAX_TOTSPACE (MAX_SPACE*MAX_SLAVE)

/* Numero di variabili di stato del Plc. */

#define MAX_STATUS 16

/* Periodo di campionamento del PLC (millisecondi). */

#define DEFAULT_CYCLE 20

/* Duarata dell'attesa (ciclica) della preparazione delle risorse di sitema
 nella fase di lancio (secondi). */

#define SMEM_POLL_TIME 3

/* Nome del file di configurazione. */

#define DEFAULT_CONFIGFILE "isarc"

/* Lunghezza massima di un nome di path. */

#define MAXNAME 300

/* Lunghezza massima di una linea nel file di configurazione. */

#define MAX_CNF_LINE (MAXNAME+100)

/* Indici dei semafori utilizzati. */

/* Semaforo di accesso alla shared memory. */
#define SEM_MEM 0
/* Semaforo di accesso esclusivo nelle "sys_lock_sem" e "sys_unlock_sem". */
#define SEM_FLG 1
/* Numero di semafori utilizzati. */
#define MAX_SEM 2

/* Linea seriale di default. */
#define DEFAULT_LINE "/dev/com2"
/* Configurazione di default per la linea seriale. */
#define DEFAULT_BAUD (B19200)
/* Bit per carattere */
#define DEFAULT_CHSIZE (CS8)
/* Parita` */
#define DEFAULT_PARITY 0
/* Stop bit */
#define DEFAULT_STOPBIT 0
/* Controllo di flusso (hardware) */
#define DEFAULT_HWFLOW (CLOCAL)

/* Dimensione delle tabelle delle variabili a rilevamento di fronte. */
#define DEFAULT_MAX_EDGE_BOOL 500
#define DEFAULT_MAX_EDGE_LONG 100

/* Nomi dei moduli di codice riconosciuti. */
#define USER_MODULE_NAME "U_CODE"
#define PLC480_MODULE_NAME "C_CODE"
#define HATSH_MODULE_NAME "HATSH_CODE"
/* Etichette dei moduli di codice per la visualizzazione della versione. */
#define USER_PRVER "usercode"
#define PLC480_PRVER "plc480"
#define HATSH_PRVER "hatsh"

/* Nome del sibmbolo che punta al nome di un modulo
 da caricare dinamicamente. */
#define MODULE_LABEL "module_name"

/* Tipo associato agli elementi del direttorio degli spazi. */

struct spc_t {
	long size;		/* Dimensione dello spazio, 0 se non definito. */
	long offset;	/* Posizione rispetto all'inizio dell'are condivisa. */
};


/* Valori di default delle chiavi della shared memory e del semaforo
 associato. */
#define DEFAULT_MEMKEY 1919
#define DEFAULT_SEMKEY 1919
/* Valore di default della dimensione dell'area condivisa. */
#define DEFAULT_MEMSIZE 0x30000
/* Permessi di accesso alle risorse IPC. */
#define DEFAULT_SEMPERMS 0777
#define DEFAULT_MEMPERMS 0777

/* Dispositivo di I/O di default. */
#define DEFAULT_IODEV "/dev/ios"
/* Nome di default dell'area di shared memory associata al dispositivo. */
#define DEFAULT_IOMEMNAME "iosmem"

/* Default timeout nel retrigger della CP-AX. */
#define DEFAULT_CPAX_TIMEOUT 100

/* Default dei file contenenti il programma in stile 480, il modulo
 di utente (attualmente e` una shell per il programma in stile 480),
 ed il modulo "hat-shoes". */
#define DEFAULT_PROG480 "plc-objs/prog.oo"
#define DEFAULT_USERPROG "plc-objs/user.oo"
#define DEFAULT_HATSH "plc-objs/hatsh.oo"

/* Periodo di campionamento iniziale (devo ancora capire come si imposti). */
extern long lCycle;

/* Flag che indica se il debugger sia autorizzato a cambiare
 il tempo di ciclo. */
extern int iChangeTCycle;

/* Flag che indica l'assenza delle altre parti del CN (smistatore...). */

extern int iStandAlone;

/* Flag che indica che non deve essere lanciato il thread di controllo del
 watchdog quando il PLC e` in HALT. */

extern int iNoWdThread;

/* Flag che indica se si deve gestire la tastiera speciale o no. */

extern int iUseSPCK;

/* Nome del programma. */
extern char * szPathName;	/* Grezzo. */
extern char * szName;		/* Normalizzato. */

/* File descriptor associato alla linea di comunicazione (per ora). */
extern int hLine;

/* Hndle per la comunicazione col server dei dati macchina. */

extern char *MData;

/* Dispositivo di comunicazione. */
extern char chLine[];
extern char *szLine;

/* Configurazione della linea (per i valori, vedere "termio.h"). */
/* Velocita` */
extern int iSpeed;
/* Bit per carattere */
extern int iChSize;
/* Parita` */
extern int iParity;
/* Stop bit */
extern int iStopBit;
/* Controllo di flusso (hardware) */
extern int iFlow;

/* Indice del blocco di shared memory utilizzato dall'applicazione
 e identificativo associato. */
extern int iMemKey;
extern int iMemId;

/* Indice del semaforo per la shared memory utilizzato dall'applicazione
 e identificativo associato. */
extern int iSemKey;
extern int iSemId;

/* Permessi di accesso per le risorse IPC. */
extern int iSemPerms;
extern int iMemPerms;

/* Timeout ammesso per il retrigger della CP-AX da parte del watchdog. */
extern int iCpaxTimeout;

/* Nomi del dispositivo di I/O e dell'area di shared memory associata. */
extern char chIoDev[];
extern char *szIoDev;
extern char chIoMemName[];
extern char *szIoMemName;

/* Dimensione ed indirizzo dell'area condivisa. */
extern long lMemSize;
extern char *pchMem;
/* Indirizzo del direttorio degli spazi occupati e dei blocchi liberi. */
extern struct spc_t *spDir;
extern struct spc_t *spFree;
/* Indirizzo della tabella di stato del Plc
 (ad uso delle applicazioni esterne). */
extern long *pStatus;

/* Indice delle parole di stato nello spazio apposito (pStatus[]). */
/* Start-stop. */
#define PLCSTS_GO 0
/* Indizzo di base della memoria condivisa, nello spazio del PLC. */
#define PLCSTS_MEMBASE 1
/* Aree statiche pronte. Ci si puo` "agganciare" al plc. */
#define PLCSTS_OKLINK 2
/* Numero di versione del database dei simboli in memoria. E`
 incrementato ogni volta che il database e` ricreato. */
#define PLCSTS_DBVER 3
/* Flag di Aggiornamento dello spazio dei simboli (ad uso del tester) */
#define PLCSTS_LDSYM 4

/* Dimensione delle tabelle delle variabili a rilevamento di fronte. */
extern int nMaxEdgeBool;
extern int nMaxEdgeLong;

/* Flag di debug. */
extern int iDebugMode;

/* Valore iniziale per gli override. */
extern int iSoftOvrVal;

/* Flag di connessione automatica dei moduli IOS. */
extern int iIosAutoConnect;

/* Flag che indica che si desidera la misura "hardware" del tempo di ciclo. */

extern int iHardMeasure;

/* Flag che indica che si desidera la misura "software" del tempo di ciclo. */

extern int iSoftMeasure;
/* Flag di abilitazione run-time della statistica (autoresettante). */
extern int iEnableSoftMeasure;

/* Variabili globali contenenti la statistica del tempo di ciclo. */

extern long lCurrUSec;	/* Valore corrente. */
extern long lMinUSec;	/* Valore minimo. */
extern long lMaxUSec;	/* Valore massimo. */

/* Da "plcdb.h" */
/* Nome di default del database di associazione codice-simbolo. */

#define DEFAULT_DBFILENAME "PlcDb"

/* Massimo codice gestito dal sistema di associazioni (lunghezza
 della tabella di associazione). */

#define MAX_TRTAB_ENTRY 5000

/* Lunghezza della tabella di corrispondenza. */

extern int iLenTransTab;

/* Nome del file contenente le associazioni. */

extern char chDbFileName[];
extern char *szDbFileName;

/* Nomi dei file contenenti il programma in stile 480, il modulo
 di utente (attualmente e` una shell per il programma in stile 480),
 ed il modulo "hat-shoes". */

extern char chProg480[];
extern char *szProg480;
extern char chUserProg[];
extern char *szUserProg;
extern char chHatSh[];
extern char *szHatSh;

/* Funzioni di utilita`. */

extern void psem(int n);
extern void vsem(int n);

#define _PORT_H

#endif /* _PORT_H */

