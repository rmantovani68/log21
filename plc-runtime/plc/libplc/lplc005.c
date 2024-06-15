/* lplc005.c */
#include "lplc.h"

void plcGetLArr(int var,int row, int col,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

