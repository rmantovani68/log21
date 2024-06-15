
/*
* @(#) md_user.c 1.0 Tue Oct 15 14:35:23 MET 1996
*
*  Blocco funzionale per la lettura delle tabelle utente di XCN.
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

struct md_user_t {
	float scale __attribute__((packed));
	char *tab_name __attribute__((packed));
	char *d_name __attribute__((packed));
	long index __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_USER = {
	1.0,
	"",
	"",
	1,
	"F",
};

long MD_USER(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_USER.format[0];

	if (GetGenericData(MData,_f_MD_USER.tab_name,(int)_f_MD_USER.index,
	                   _f_MD_USER.d_name,&dest) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_USER.scale);
}

