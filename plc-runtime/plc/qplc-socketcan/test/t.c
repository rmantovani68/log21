
/*
* @(#) hwtest.c 1.0 Fri Jul 18 17:53:24 MET DST 1997
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 18/07/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test 1.0 Fri Jul  7 21:15:14 MET DST 2000";


int UserStart(int * code)
{
	return 1;
}

void UserStop(void)
{
}

int UserMain(int start)
{
unsigned long t;
static unsigned long timer;
char msg[80];
char tmp;
static char tasto;

	t = plcTime();
	if (start) {
		timer = t;
	}

	if (t - timer > 3000) {
		plcMessage(100,"Tempo!");
		timer = t;
	}

	plcGetBVect(plc_TASTI,0,&tmp);
	if (tmp != tasto) {
		plcMessage(100,"Tasto!");
	}
}

