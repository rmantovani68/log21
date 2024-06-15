
/*
* @(#) md_save_user.c 1.0 Thu Feb 18 18:12:53 MET 1999
*
*  Funzione per il salvataggio delle tabelle assi.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 18/02/99 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_save_user_t {
	char *tab_name __attribute__((packed));
} _f_MD_SAVE_USER = {
	"",
};

long MD_SAVE_USER(void)
{

	plcSaveGenericTable(_f_MD_SAVE_USER.tab_name);
	return 0;
}


