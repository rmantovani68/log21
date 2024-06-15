
/*
* @(#) md_conf.c 1.0 Wed Nov 26 16:31:36 MET 1997
*
*  Blocco funzionale per la lettura dei dati di configurazione del mandrino.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 26/11/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_conf_t {
	float scale __attribute__((packed));
	long cen __attribute__((packed));
	char *mand __attribute__((packed));
	long nattr __attribute__((packed));
	char *d_name __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_CONF = {
	1.0,
	0,
	"",
	0,
	"",
	"F",
};

long MD_CONF(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_CONF.format[0];

	if (GetConfData(MData,
	                 _f_MD_CONF.cen,
	                 _f_MD_CONF.mand,
	                 _f_MD_CONF.nattr,
	                 _f_MD_CONF.d_name,
	                 &dest,
	                 ID_SDM_RUNTIME) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_CONF.scale);
}

