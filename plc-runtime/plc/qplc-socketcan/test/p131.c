
/*
* @(#) p131.c 1.0 Tue Mar 12 16:20:33 MET 2002
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 12/03/02 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>
#undef _POSIX_SOURCE
#include <math.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test P131 1.0";

static long encoder = 0;

int UserStart(int * code)
{
int i;
char dev[40];

	i = 0;
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.5.p131.%d",i);
	if (! ioAttach(dev,&encoder,32,DRIVER_MODE_INPUT)) {
			/* Errore... */
		plcError(5,dev);
	}
	return 1;
}

void UserStop(void)
{
}

int UserMain(int start)
{
unsigned long t;
static unsigned long timer;
char tmp;
int i;


	t = plcTime();
	if (start) {
		timer = t;
	}

	if (t - timer > 1000) {
		plcMessage(1,"Encoder = %d",encoder);
		timer = t;
	}
}

