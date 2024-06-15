
/*
* @(#) plcrem.h 1.2 Fri Jul 28 18:16:48 MET DST 2000
*
*  Modulo server per controllo remoto di QPLC.
*
* 17/11/97 GG 1.0 Prima stesura.
* 02/12/97 GG 1.1 Aggiunta la definizione dell'evento associato al trigger,
*             e le grandezze legate al trigger (funzioni, azioni...).
* 28/07/00 GG 1.2 Aggiunti eventi asincroni per cancellazione errori e
*             invio e cancellazione messaggi.
*/

#ifndef _PLCREM_H_

/* Nome del servizio TCP utilizzato da qplc. */
#define PLCREM_SERVICE_NAME "qplc"
/* Numero del servizio utilizzato se non e` definito il precedente. */
#define PLCREM_SERVICE_NUMBER 8039

/* Codici di errore. */
enum {
/* Operazione completata. */
DSV_E_OK = 0,
/* Comando sconosciuto. */
DSV_E_UNKNOWN,
/* Comando troppo lungo. */
DSV_E_LONGLINE,
/* Numero di argomenti errato. */
DSV_E_NARGS,
/* Parametro non valido. */
DSV_E_BADARG,
/* Stato PLC incompatibile con l'operazione. */
DSV_E_BADSTS,
};

/* Eventi asincroni. */
enum {
/* Emissione di un errore da Plc. */
DSV_EVENT_ERROR = 0,
/* Entrata in HALT. */
DSV_EVENT_HALT,
/* Entrata in GO. */
DSV_EVENT_GO,
/* Entrata in FAULT. */
DSV_EVENT_FAULT,
/* Buffer di trace pronto. */
DSV_EVENT_TRACE,
/* Condizione di trigger verificata. */
DSV_EVENT_TRIGGER, /* = 5 */
/* Cancellazione di un errore. */
DSV_EVENT_DELERR,
/* Emissione/Update/Cancellazione di un messaggio. */
DSV_EVENT_MESSAGE,
};

/* Funzioni di trigger. */
enum {
/* Condizione sempre falsa. */
DSV_TRIGGER_NULL = 0,
/* Fronte di salita di una variabile booleana. */
DSV_TRIGGER_RTRIG,
/* Fronte di discesa di una variabile booleana. */
DSV_TRIGGER_FTRIG,
/* Cambiamento di una variabile di 8, 16 e 32 bit. */
DSV_TRIGGER_CHG8,
DSV_TRIGGER_CHG16,
DSV_TRIGGER_CHG32, /* = 5 */
/* Raggiungimento di un valore prefissato ad 8, 16 e 32 bit. */
DSV_TRIGGER_EQ8,
DSV_TRIGGER_EQ16,
DSV_TRIGGER_EQ32,
};

/* Flag descriventi le azioni associate ai trigger. */

#define DSV_TRIGGER_ACT_TRACEON 0x01
#define DSV_TRIGGER_ACT_TRACEOFF 0x02
#define DSV_TRIGGER_ACT_NOTIFY 0x04

#ifndef _REMOTE_H_

/*
* Maschere per l'interpretazione del campo "flags" delle caratteristiche
* di un segnale di scambio.
*/

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

#endif

#define _PLCREM_H_
#endif

