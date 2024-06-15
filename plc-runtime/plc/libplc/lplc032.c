/* lplc032.c */
#include "lplc.h"

void plcSetLVect(int var,int col,long val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*(long *)(pchMem + off) = val;
}

