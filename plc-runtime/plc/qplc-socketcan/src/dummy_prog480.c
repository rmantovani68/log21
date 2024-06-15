/*
* @(#) dummy_prog480.c 1.2 Fri Feb 28 09:33:10 MET 1997
*
* Pseudo-programma PLC che non fa altro che emettere un errore e forzare un
* HALT.
*
* 28/03/96 GG 1.0 Prima stesura.
* 20/09/96 GG 1.1 Adattamenti minuscoli per qplc.
* 28/02/97 GG 1.2 Aggiunto la funzione vuota "PlcInit" per permettere il
*             link statico di programmi 480.
*/

#include "plcapi.h"

char ver_prog[] = "";

BEGIN_MSD_INPUT
END_MSD

BEGIN_MSD_OUTPUT
END_MSD

void PlcInit(void)
{
}

void PlcMain(void)
{
/* ERRORE. Modulo programma 480 non caricato. */
	plcError(199,"prog.oo");
	plcHalt();
}

