
/*
* ioscni_priv.h 1.2 Tue Feb 13 15:36:39 CET 2001
*
* 28/06/00 GG 1.0 Prima stesura.
* 30/06/00 GG 1.1 Aggiunto un campo privato contenente la lunghezza
*             totale dell'area mappabile.
* 12/02/01 GG 1.2 Aggiunto "unregister_chrdev" se non ci sono schede.
* 02/04/02 GG+RM 2.0 Modifiche varie per Kernel 2.4.x
*/

#ifndef _IOSCNI_PRIV_H

#define _IOSCNI_PRIV_H

#include "ios.h"

#define VER_IOSCNI "2.0"

#define IOSCNI_MAJOR 0
#define IOSCNI_NAME "ioscni"

#define IOSCNI_OFF_CPU1 0x0000
#define IOSCNI_OFF_CPU2 0x1000
#define IOSCNI_SIZE_CPU 0x1000
#define IOSCNI_OFF_NVRAM 0x2000
#define IOSCNI_SIZE_NVRAM 0x2000

struct ios_info {
	unsigned int ioaddr;
	unsigned long memaddr;
};

/* Struttura di stato di un'istanza del dispositivo. */

struct ios_board {

/******************/
/* Parte pubblica */
/******************/

  	struct iosboard b;

/*****************/
/* Parte privata */
/*****************/

	unsigned long tot_size;
	unsigned long virtaddr;
};


#define MAX_IOSCNI_BOARD 4

struct ios_statics {
	int nb;
	struct ios_board board[MAX_IOSCNI_BOARD];
};


#endif /* _IOSCNI_PRIV_H */

