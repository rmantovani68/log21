/* lplc033.c */
#include "lplc.h"

void plcSetFVect(int var,int col,float val)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*(float *)(pchMem + off) = val;
}

