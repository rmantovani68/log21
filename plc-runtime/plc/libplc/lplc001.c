/* lplc001.c */
#include "lplc.h"

/*
* Funzione plcSetBArr(), plcSetLArr(), plcSetFArr()
* -------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var + col +  dim*row" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBArr(int var,int row,int col,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		pchMem[off] = val;
}

