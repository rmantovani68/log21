
/*
* @(#) md_toolchg.c 1.0 Mon Apr  7 16:32:24 MET DST 1997
*
*  Funzione per la lettura delle tabelle dell'attrezzaggio del cambio
* utensile.
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

struct md_toolchg_t {
	float scale __attribute__((packed));
	long cen __attribute__((packed));
	long ncu __attribute__((packed));
	long type __attribute__((packed));
	char *d_name __attribute__((packed));
	char *format __attribute__((packed));
} _f_MD_TOOLCHG = {
	1.0,
	0,
	0,
	0,
	"",
	"F",
};

long MD_TOOLCHG(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
register char c = _f_MD_TOOLCHG.format[0];

	if (GetCmbUtData(MData,
	                 _f_MD_TOOLCHG.cen,
	                 _f_MD_TOOLCHG.ncu,
	                 _f_MD_TOOLCHG.type,	
	                 _f_MD_TOOLCHG.d_name,
	                 &dest,
	                 ID_SDM_RUNTIME) < 0)
		return 0;
	else if (c == 'F' || c == 'f')
		return dest.l;
	else if (c == 'I' || c == 'i')
		return (long)(dest.fl);
	else
		return (long)(dest.fl * _f_MD_TOOLCHG.scale);
}

