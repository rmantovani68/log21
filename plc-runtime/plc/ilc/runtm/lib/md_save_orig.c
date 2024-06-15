
/*
* @(#) md_save_orig.c 1.0 Tue Dec 12 17:16:36 MET 2000
*
*  Funzione per il salvataggio delle tabelle origini.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 12/12/00 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

struct md_save_orig_t {
	long dummy __attribute__((packed));
} _f_MD_SAVE_ORIG = {
	0,
};

long MD_SAVE_ORIG(void)
{

	plcSaveOrig();
	return _f_MD_SAVE_ORIG.dummy;
}


