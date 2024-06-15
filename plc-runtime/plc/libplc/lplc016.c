/* lplc016.c */
#include "lplc.h"

/*
* Funzione "dbGetVarAddr"
* -----------------------
*
*  Questa funzione trova l'indirizzo di una variabile di Isagraf dato
* il nome.
*  Se il nome non e` presente, riporta NULL.
*
*  Il programmatore C dovrebbe utilizzare sempre questa funzione per
* conoscere l'indirizzo di una variabile Isagraf.
*/

#if _NO_PROTO
char * dbGetVarAddr(name) char *name;
#else
char * dbGetVarAddr(char *name)
#endif
{
struct symtab_t * sym;
struct qplc_shv_t * qsym;
int row,col;

	if (dbQPLC()) {
		return dbGetQplcVarDescr(name,&qsym);
	}
	else {
		sym = dbFindIsaVar(name);

		if (! sym)
			return (char *) 0;

		return MTRANS(sym -> pval);
	}
}

