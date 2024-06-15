
/*
* @(#) md_set_axis.c 1.0 Wed Jun 10 12:28:52 CEST 1998
*
*  Blocco funzionale per la lettura delle tabelle assi di XCN.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 10/06/98 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_set_axis_t {
	float f_in __attribute__((packed));
	long axis __attribute__((packed));
	char *d_name __attribute__((packed));
} _f_MD_SET_AXIS = {
	0.0,
	0,
	"",
};

long MD_SET_AXIS(void)
{
union {
	char ch[256];	/* Per evitare in ogni caso sforamenti, */
	float fl;	/* si sta dalla parte del sicuro. */
	long l;
} dest;
int rv;

/* Lettura, per conoscere il tipo di dato ed evitare schianti. */
	rv = GetAxesDataNi(MData, (int)_f_MD_SET_AXIS.axis,
	                   _f_MD_SET_AXIS.d_name,&dest);
	if (rv < 0 || rv == ID_STRING)
		return 0;
	dest.fl = _f_MD_SET_AXIS.f_in;
	if (SetAxesDataNi(MData, (int)_f_MD_SET_AXIS.axis,
	                   _f_MD_SET_AXIS.d_name,&dest) < 0)
		return 0;
	return dest.l;
}

