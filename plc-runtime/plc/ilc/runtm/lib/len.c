
/*
* @(#) len.c Thu Mar  2 12:51:48 MET 2000
*
*  Lunghezza di una stringa
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 02/03/00 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <string.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

struct len_t {
	char *str __attribute__((packed));
} _f_LEN = {
	"",
};

long LEN(void)
{
	return (long) strlen(_f_LEN.str);
}

