/*
* @(#) msd.h 1.2 Mon Dec  2 10:13:20 MET 1996
*
* Macro utilizzate per generare le informazioni MSD di un programma IL.
* Le informazioni qui contenute interessano anche il caricatore dinamico di
* programmi di "qplc". Pertanto, questo file influenza sia il compilatore IL,
* sia l'esecutore.
*
* ??/??/?? GG 1.0 Prima stesura.
* 26/11/96 GG 1.1 Aggiunta la definizione del fleg di dati estesi.
* 02/12/96 GG 1.2 Aggiunto il flag di inizializzazione forzata di un segnale
*             di scambio.
*/

#ifndef _MSD_H_

/*
* Parolina magica da far trovare al caricatore dinamico.
*/

#define ILC_MAGIC "IL_CODE"

/*
* Descrizione di un elemento di configurazione (MSD) :
*
* Primo (ed eventualmente unico) byte : codice di inizializzazione.
* Byte successivi (numero dipendente dal codice) : dati di inizializzazione.
*
* Codifica del primo byte :
*/

/* Tipo di associazione di I/O. */
#define ILC_MSD_BIND_NONE 0x00
#define ILC_MSD_BIND_IN 0x01
#define ILC_MSD_BIND_OUT 0x02
#define ILC_MSD_BIND_MEM 0x03
#define ILC_MSD_BIND_M 0x03
/* Dimensione della variabile in bit. */
#define ILC_MSD_SIZE_1 0x00
#define ILC_MSD_SIZE_8 0x04
#define ILC_MSD_SIZE_16 0x08
#define ILC_MSD_SIZE_32 0x0C
#define ILC_MSD_SIZE_64 0x10
#define ILC_MSD_SIZE_M 0x1C
/* Proprieta` specifiche delle variabili "MEM" (variabili condivise). */
#define ILC_MSD_MEMVAR_FIXED 0x20 /* Il campo "target" contiene una chiave. */
#define ILC_MSD_MEMVAR_FLOAT 0x40 /* Variabile "float" o "double" */
#define ILC_MSD_EXTENSION 0x80 /* Ci sono informazioni aggiuntive */
                               /* nell'entry successiva. */
/* Byte di estensione delle proprieta` (nel campo dati dell'entry successiva
 ad una con la proprieta` ILC_MSD_EXTENSION). */
#define ILC_MSD_EXT_RETAIN 0x01 /* Variabile in RAM non volatile. */
#define ILC_MSD_EXT_INIT 0x02 /* Variabile da inizializzare. */

/*
* Valore da assegnare al campo "flags" di un descrittore MSD
* per indicare "fine lista". E` un codice che non ha senso.
*/

#define ILC_END_LIST (ILC_MSD_SIZE_1 | ILC_MSD_MEMVAR_FLOAT | ILC_MSD_BIND_IN)
/* Valore long che contiene lo stesso codice. Fa comodo. */
#define ILC_END ILC_END_LIST | (ILC_END_LIST << 8) | (ILC_END_LIST << 16) | (ILC_END_LIST << 24)

#define _MSD_H_

#endif

