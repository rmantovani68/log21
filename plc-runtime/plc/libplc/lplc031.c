/* lplc031.c */
#include "lplc.h"

/*
* Funzione plcSetBVect(), plcSetLVect(), plcSetFVect()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBVect(int var,int col,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		pchMem[off] = val;
}

