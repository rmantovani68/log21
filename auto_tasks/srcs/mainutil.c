/*
* Modulo : recutil.c
* --------------------
* Funzioni di varia utilita'
*
*
* Data creazione 17/05/2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <malloc.h>
#include <signal.h>
#include <glib.h>
#include <gio/gio.h>

#include <fcntl.h>
// #include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef TRACE
	#include <trace.h>
#endif
#include <timeout.h>
#include <msq_lib.h>
#include <proc_list.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>


#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"


/*
* Conta le righe di un file ascii
*/
int ContaRighe(char *szFileName)
{
	FILE *fp;
	int nRighe=0;
	char c;

	if((fp=fopen(szFileName,"r"))!=(FILE *)NULL){
		while((c=fgetc(fp))!=EOF){
			if(c=='\n'){
				nRighe++;
			}
		}
		fclose(fp);
	}
	return nRighe;
}





