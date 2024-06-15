
/*
* @(#) md_set_user.c 1.0 Tue Mar 31 15:29:07 MET DST 1998
*
*  Blocco funzionale per la lettura delle tabelle utente di XCN.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 31/03/98 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_set_user_t {
	float f_in __attribute__((packed));
	char *tab_name __attribute__((packed));
	char *d_name __attribute__((packed));
	long index __attribute__((packed));
} _f_MD_SET_USER = {
	0.0,
	"",
	"",
	1,
};

long MD_SET_USER(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
int rv;

/* Lettura, per conoscere il tipo di dato ed evitare schianti. */
	rv = GetGenericData(MData,_f_MD_SET_USER.tab_name,
	                    (int)_f_MD_SET_USER.index,
	                   _f_MD_SET_USER.d_name,&dest);
	if (rv < 0 || rv == ID_STRING)
		return 0;
	dest.fl = _f_MD_SET_USER.f_in;
	if (SetGenericData(MData,_f_MD_SET_USER.tab_name,
		           (int)_f_MD_SET_USER.index,
	                   _f_MD_SET_USER.d_name,&dest) < 0)
		return 0;
	return dest.l;
}

