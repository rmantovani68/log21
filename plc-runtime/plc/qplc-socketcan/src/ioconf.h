
/*
* @(#) ioconf.h 1.1 Mon Nov  9 11:47:49 MET 1998
*
* Definizioni legate alla configurazione degli I/O.
*
* 01/10/98 GG 1.0 Prima stesura.
* 09/11/98 GG 1.1 Aggiunta la distinzione tra segnali proibiti (D) e
*             segnali non citati. Questo permette di modificare il 
*             comportamento di qplc davanti ai segnali non previsti nel
*             file di configurazione.
*             Aggiunta la funzione "cioLoad", che serve a rileggere
*             il file di configurazione.
*/

#ifndef _IOCONF_H_

/* Possibili proprieta` del segnale. */
#define CIO_PROP_REQUIRED  'R'
#define CIO_PROP_OPTIONAL  'O'
#define CIO_PROP_SIMULATED 'S'
#define CIO_PROP_DISABLED  'D'
#define CIO_PROP_UNKNOWN   '?'


/*
* Funzione "cioInit"
* ------------------
*
*  Questa funzione registra il nome del file di configurazione,
* poi lo legge e lo elabora.
*  Vale 1 in caso di successo, 0 in caso di errore.
*/

int cioInit(char *file);

/*
* Funzione "cioLoad"
* ------------------
*
*  Questa funzione legge il file di configurazione e lo elabora.
*  Il parametro "init" vale 1 se ci si trova in una fase di inizializzazione,
* altrimenti (cioe` normalmente) vale 0.
*  Vale 1 in caso di successo, 0 in caso di errore.
*/

int cioLoad(int init);

/*
* Funzione "cioGetProp"
* ---------------------
*
*  Questa funzione restituisce la proprieta` del segnale dato
* ottenendola dal database di configurazione. Restituisce uno dei
* valori di proprieta` "CIO_PROP_...".
*  Se il database non e` disponibile (cioe` non c'era il file
* di configurazione) il segnale e` sempre di tipo "CIO_PROP_REQUIRED".
* NOTA: Il nome del segnale deve essere convertito in forma canonica
* (usare la funzione "ioCanonify").
*/

int cioGetProp(char *name, int size, int flags);

#define _IOCONF_H_
#endif

