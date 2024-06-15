/* lplc015.c */
#include "lplc.h"

/* Flag di variabile in RAM non volatile. Bisogna usare "_plclib_nvram",
 anziche` "pchMem", come indirizzo di base. */

#define SHV_MODE_RETAIN 0x80

/*
* Funzione "dbGetQplcVarDescr"
* ----------------------------
*
*  Questa funzione trova l'indirizzo ed il descrittore di una variabile
* di QPLC dato il nome. L'indirizzo del descrittore e` copiato nel
* puntato del parametro "qsym".
*  Se il nome non e` presente, o gli indici contenuti nel noe
* sono fuori dai liiti previsti per la variabile, riporta NULL.
*/

#if _NO_PROTO
char * dbGetQplcVarDescr(name,qsym) char *name; struct qplc_shv_t ** qsym;
#else
char * dbGetQplcVarDescr(char *name, struct qplc_shv_t ** qsym)
#endif
{
int row,col;
char *p;

	*qsym = dbFindQplcVar(name,&row,&col);

	if (! *qsym || (*qsym) -> dim1 <= row || (*qsym) -> dim2 <= col)
		return (char *) 0;

	if ((*qsym) -> mode & SHV_MODE_RETAIN) {
		if (_plclib_nvram)
			p = _plclib_nvram;
		else
			return (char *) 0;
	}
	else {
		p = pchMem;
	}
	return p + pKeyTab[(*qsym) -> key + (*qsym) -> dim2*row + col].off;
}

