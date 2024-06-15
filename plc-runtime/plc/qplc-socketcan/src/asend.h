/*
* @(#) asend.h 1.0 Mon Aug 26 12:06:54 MET DST 1996
*
*  Questo file contiene le primitive di colloquio asincrono con
* lo smistatore. L'invio del messaggio e` affidato ad un thread separato,
* controllato da un semaforo.
*/


#ifndef _ASEND_H_

#define MAX_ASEND_QUEUE 16

/* Tipi di messaggio. */

#define ASEND_GMM 1	/* Per Lelli. */
#define ASEND_AX 2	/* Per Sarti. */

/*
* Funzione "asend_enqueue"
* ------------------------
*
*  Questa funzione accoda un messaggio (16 byte) di un dato tipo. Il
* tipo individua la componente del CN a cui e` destinato.
*/

void asend_enqueue(int type, long *data);

/*
* Funzione "asend_start"
* -----------------------
*
*  Questa funzione lancia il thread del colloquio con il CN.
*/

void asend_start(void);

#define _ASEND_H_

#endif

