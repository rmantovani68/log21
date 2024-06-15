/* lplc034.c */
#include "lplc.h"

/*
* Funzione plcGetBVect(), plcGetLVect(), plcGetFVect()
* ----------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var+col+dim*row"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBVect(int var,int col,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

