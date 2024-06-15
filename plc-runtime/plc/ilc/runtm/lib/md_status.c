
/*
* @(#) md_status.c 1.0 Wed Oct 16 14:12:56 MET 1996
*
*  Blocco funzionale per l'acuisizione dello stato del server dati macchina.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 16/10/96 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#ifdef Lynx
#include <ipc.h>
#include <msg.h>
#include <services.h>
#endif

extern char *MData;

long MD_STATUS(void)
{
	return GetStatoDm(MData);
}

