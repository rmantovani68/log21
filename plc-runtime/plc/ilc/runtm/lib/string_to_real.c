
/*
* @(#) string_to_real.c 1.0 Thu Sep 18 18:35:07 MET DST 1997
*
*  Funzione per la conversione da stringa a valore in virgola mobile.
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

extern double atof(char *);

struct string_to_real_t {
	char *str __attribute__((packed));
} _f_STRING_TO_REAL = {
	"0",
};

long STRING_TO_REAL(void)
{
union {
	float f;
	long l;
} u;
	u.f = (float) atof(_f_STRING_TO_REAL.str);
	return u.l;
}

