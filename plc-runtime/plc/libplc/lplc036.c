/* lplc036.c */
#include "lplc.h"

void plcGetFVect(int var,int col,float * dest)
{
register int off;

	if (pKeyTab && pStatus[PLCSTS_GO] && (off = pKeyTab[var+col].off))
		*dest = *(float *)(pchMem + off);
	else
		*dest = 0;
}

