/* lplc004.c */
#include "lplc.h"

/*
* Funzione plcGetBArr(), plcGetLArr(), plcGetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBArr(int var,int row,int col,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

