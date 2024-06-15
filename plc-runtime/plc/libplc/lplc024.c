/* lplc024.c */
#include "lplc.h"


/*
* Funzione plcSetBScal(), plcSetLScal(), plcSetFScal()
* ----------------------------------------------------
*
* Queste funzioni copiano il valore "val"
* nella variabile di plc "var" (indice interno CNI, vedere
* "isasym.h").
*/

void plcSetBScal(int var,char val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		pchMem[off] = val;
}

void plcSetLScal(int var,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*(long *)(pchMem + off) = val;
}

