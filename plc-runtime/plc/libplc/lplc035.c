/* lplc035.c */
#include "lplc.h"

void plcGetLVect(int var, int col,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

