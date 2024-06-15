
/*
* @(#) md_axis.c 1.0 Tue Mar 25 10:40:56 MET 1997
*
*  Blocco funzionale per la lettura delle tabelle utente assi XCN.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 25/03/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_axis_t {
	float scale __attribute__((packed));
	long axis __attribute__((packed));
	char *d_name __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_AXIS = {
	1.0,
	0,
	"",
	"F",
};

long MD_AXIS(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_AXIS.format[0];

	if (GetAxesDataNi(MData,_f_MD_AXIS.axis,
	                   _f_MD_AXIS.d_name,&dest) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_AXIS.scale);
}

