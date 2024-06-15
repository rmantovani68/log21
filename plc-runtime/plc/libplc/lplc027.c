/* lplc027.c */
#include "lplc.h"

void plcGetLScal(int var,long * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = *(long *)(pchMem + off);
	else
		*dest = 0;
}

