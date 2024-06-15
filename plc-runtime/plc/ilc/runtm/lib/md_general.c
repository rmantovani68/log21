
/*
* @(#) md_general.c 1.0 Tue Oct 15 14:35:23 MET 1996
*
*  Blocco funzionale per la lettura dei dati generali di XNC.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_general_t {
	float scale __attribute__((packed));
	char *name __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_GENERAL = {
	1.0,
	"",
	"F",
};

long MD_GENERAL(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_GENERAL.format[0];

	if (GetGenData(MData,_f_MD_GENERAL.name, &dest) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_GENERAL.scale);
}

