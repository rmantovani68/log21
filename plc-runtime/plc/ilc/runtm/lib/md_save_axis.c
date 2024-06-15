
/*
* @(#) md_save_axis.c 1.0 Wed Jun 10 12:28:52 CEST 1998
*
*  Funzione per il salvataggio delle tabelle assi.
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

struct md_save_axis_t {
	long axis __attribute__((packed));
} _f_MD_SAVE_AXIS = {
	0,
};

long MD_SAVE_AXIS(void)
{

	plcSaveAx(_f_MD_SAVE_AXIS.axis);
	return _f_MD_SAVE_AXIS.axis;
}


