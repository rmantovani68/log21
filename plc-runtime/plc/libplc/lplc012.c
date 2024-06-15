/* lplc012.c */
#include "lplc.h"

/*
* Funzione "dbQPLC"
* -----------------
*
*  Questa funzione dice se c'e` qplc al posto di isaker.
*/

int dbQPLC(void)
{
	return (SYMS && SYMS[1] == 'Q');
}

