/*
* @(#) nvram.h 2.0 Fri Nov 22 10:11:36 MET 1996
* @(#) Rif. Giuseppe Guerrini (beppe@tlaloc.cni.it)
*
* Indirizzi e dimensioni delle aree di NVRAM correntemente utilizzate.
*
* Ogni area e` descritta da tre "#define", indicanti l'offeset e la dimensione
* dell'area, e l'indice della scheda IOS sulla quale e` montato il chip :
*
* NVRAM_BRD_... : Indice di scheda.
* NVRAM_OFF_... : Offset dell'area.
* NVRAM_SIZ_... : Dimensione dell'area.
* NVRAM_LBL_... : Etichetta associata all'area.
*
* 27/03/96 GG 1.0, 1.1 : Prima versione.
* 09/10/96 GG 1.2 Aggiunta l'area per Lelli.
* 22/11/96 GG 2.0 Aggiunta la definizione del tipo "nvram_t", utilizzato
*             dalla funzione di libreria "AttachNvram", e etichette
*             per identificare le aree predefinite. Le definizioni
*             date in questo file (NVRAM_...) sono mantenute per compatibilita`
*             in attesa che tutti i programmi si allineino ed utilizzino
*             la funzione "AttachNvram" anziche` l'accesso diretto alla IOS.
*/

#ifndef _NVRAM_H

#define _NVRAM_H

/*
* Le seguenti definizioni sono da considerare "in via di obsolescenza".
* Sono mantenute per permettere la compilazione di programmi "vecchio
* stile", che non utilizzino ancora la funzione "AttachNvram". Con
* L'andare del tempo, le uniche definizioni significative saranno
* le etichette, essendo tutto il resto definito nel file di configurazione
* "nvram.conf", o nella variabile di ambiente "NVRAM_CONF". Si noti
* che se esistono applicazioni che accedono alla Ram non volatile
* con il vecchio metodo, la configurazione deve prevedere aree con
* posizione e dimensione compatibili.
*/

/* Ad uso di "serverdm" (rif. Francesco Maiorana). */

#define NVRAM_BRD_SERVERDM 0
#define NVRAM_OFF_SERVERDM 0
#define NVRAM_SIZ_SERVERDM 0x800
#define NVRAM_LBL_SERVERDM "SDM"

/* Ad uso del processo "ax" (rif. Andrea "Sax" Sarti). */

#define NVRAM_BRD_AX 0
#define NVRAM_OFF_AX 0x800
#define NVRAM_SIZ_AX 0x400
#define NVRAM_LBL_AX "SAX"

/* Ad uso del programma Plc BiEsse (rif. Guerrini, Bronzini, Salvia). */

#define NVRAM_BRD_PLC480 0
#define NVRAM_OFF_PLC480 0x1E00
#define NVRAM_SIZ_PLC480 0x200
#define NVRAM_LBL_PLC480 "480"

/* Ad uso interno del Plc (rif. Guerrini). */

#define NVRAM_BRD_PLC 0
#define NVRAM_OFF_PLC 0x1D00
#define NVRAM_SIZ_PLC 0x100
#define NVRAM_LBL_PLC "PLC"

/* Ad uso di GMM (rif. Lelli). */

#define NVRAM_BRD_GMM 0
#define NVRAM_OFF_GMM 0x1B00
#define NVRAM_SIZ_GMM 0x200
#define NVRAM_LBL_GMM "GMM"

/*
* Definizione della struttura utilizzata dalla funzione "AttachNvram"
* per restituire il risultato.
*/

typedef struct _nvram_t {
/* Indirizzo dell'area. */
	unsigned char *addr;
/* Dimensione. */
	int size;
} nvram_t;

/*
* Funzione "AttachNvram"
* ----------------------
*
*  Questa funzione provvede a collegare il processo all'area di
* ram non volatile indicata dall'etichetta "name". Le caratteristiche
* dell'area sono restituite in "area". Il parametro "base" e` un puntatore
* alla base del dispositivo "NVRAM", che puo` essere ottenuto per esempio
* dalla funzione "iosOpen". Se vale NULL (e` il caso piu` comune), la
* funzione stessa provvede ad apire e mappare il dispositivo. Il parametro
* "ios" contiene l'indice della IOS alla quale "base" fa riferimento, ed
* e` ignorato se "base" vale NULL. In caso contrario, la funzione controlla
* se l'indice coincide con quello previsto dalla configurazione. Se questo
* avviene, sono calcolati i campi di "area" e la funzione termina con successo,
* altrimenti si ottiene un errore. In ogni caso, se "base" non e` NULL,
* non si avranno aperture di dispositivi IOS.
*  Chiamate successive di "AttachNvram" per ottenere aree risiedenti nello
* stesso dispositivo non provocano nuove aperture di IOS, perche` la
* funzione registra gli indirizzi di base dei chip utilizzati di volta in
* volta, e li ricicla.
*
*  La funzione vale 0 in caso di successo, oppure -1 in caso di errore.
*  Data la delicatezza delle azioni compiute, questa funzione emette
* sempre messaggi su "stdout" ed "stderr" in caso di situazioni anomale.
*/

int AttachNvram(char *name, nvram_t *area, void *base, int ios);

#endif

