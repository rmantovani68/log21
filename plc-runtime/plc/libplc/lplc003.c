/* lplc003.c */
#include "lplc.h"

void plcSetFArr(int var,int row,int col,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] &&
	    (off = pKeyTab[var + col + pKeyTab[var].dim * row].off))
		*(float *)(pchMem + off) = val;
}

