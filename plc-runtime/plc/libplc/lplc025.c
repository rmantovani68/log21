/* lplc025.c */
#include "lplc.h"

void plcSetFScal(int var,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*(float *)(pchMem + off) = val;
}

