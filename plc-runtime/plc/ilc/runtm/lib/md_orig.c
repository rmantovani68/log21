
/*
* @(#) md_orig.c 1.0 Fri Jan 22 11:33:53 MET 1999
*
*  Funzione per la lettura delle tabelle delle origini.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 07/04/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_orig_t {
	float scale __attribute__((packed));
	char *o_name __attribute__((packed));
	char *d_name __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_ORIG = {
	1.0,
	"",
	"",
	"F",
};

long MD_ORIG(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_ORIG.format[0];

	if (GetOrigData(MData,
	                 _f_MD_ORIG.o_name,
	                 _f_MD_ORIG.d_name,
	                 &dest) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_ORIG.scale);
}

