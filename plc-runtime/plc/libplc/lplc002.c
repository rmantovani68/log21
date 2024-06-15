/* lplc002.c */
#include "lplc.h"

void plcSetLArr(int var,int row,int col,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*(long *)(pchMem + off) = val;
}

