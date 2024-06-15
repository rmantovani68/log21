/* lplc026.c */
#include "lplc.h"

/*
* Funzione plcGetBScal(), plcGetLScal(), plcGetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il contenuto della variabile di plc "var + dim*ind"
* (indice interno CNI, vedere "isasym.h") nell'area puntata da "dest".
* Se la variabile non esiste, la destinazione e` lasciata inalterata.
*/

void plcGetBScal(int var,char * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = pchMem[off];
	else
		*dest = 0;
}

