/* lplc006.c */
#include "lplc.h"

void plcGetFArr(int var,int row,int col,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

