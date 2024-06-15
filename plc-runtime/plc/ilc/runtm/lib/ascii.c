
/*
* @(#) mid.c Thu Mar  2 12:51:48 MET 2000
*
*  Estrazione dell'n-esimo carattere contenuto nella stringa (codice ascii).
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

struct ascii_t {
	long p __attribute__((packed));
	char *str __attribute__((packed));
} _f_ASCII = {
	1,
	"",
};

long ASCII(void)
{
char *s;
int i,pos;

	s = _f_ASCII.str;
	pos = _f_ASCII.p - 1;
	if (pos >= 0) {
		for (i = 0; *s; ++i, ++s) {
			if (i == pos) {
				return (long) (unsigned char)*s;
			}
		}
	}

	return (long) 0;
}

