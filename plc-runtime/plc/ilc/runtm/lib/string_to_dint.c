
/*
* @(#) string_to_dint.c 1.0 Thu Sep 18 18:35:07 MET DST 1997
*
*  Funzione per la conversione da stringa a intero con segno.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 18/09/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

struct string_to_dint_t {
	char *str __attribute__((packed));
} _f_STRING_TO_DINT = {
	"0",
};

long STRING_TO_DINT(void)
{
	return atol(_f_STRING_TO_DINT.str);
}

