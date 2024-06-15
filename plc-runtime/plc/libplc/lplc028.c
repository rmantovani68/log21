/* lplc028.c */
#include "lplc.h"

void plcGetFScal(int var,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

