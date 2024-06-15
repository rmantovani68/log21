
/*
* @(#) mid.c Thu Mar  2 12:51:48 MET 2000
*
*  Estrazione di sottostringa.
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

struct mid_t {
	char *str __attribute__((packed));
	long l __attribute__((packed));
	long p __attribute__((packed));
} _f_MID = {
	"",
	0,
	1,
};

#define MAX_MID_RESULT_LEN 511

long MID(void)
{
static char _MID_result[MAX_MID_RESULT_LEN+1];
char *s;
int len,pos,l;

	s = _f_MID.str;
	if (_f_MID.l < 0)
		len = 0;
	else
		len = _f_MID.l;
	if (l > MAX_MID_RESULT_LEN)
		l = MAX_MID_RESULT_LEN;
	if (_f_MID.p < 1)
		pos = 1;
	else
		pos = _f_MID.p;
	for (l = 1; *s; ++s, ++l) {
		if (l == pos)
			break;
	}
	for (l = 0; *s; ++s, ++l) {
		if (l == len)
			break;
		_MID_result[l] = *s;
	}
	_MID_result[l] = '\0';

	return (long) _MID_result;
}

