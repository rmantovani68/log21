
/*
* @(#) p137.c 1.0 Wed Mar 13 16:03:18 MET 2002
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 13/03/02 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>
#undef _POSIX_SOURCE
#include <math.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test P137 1.0";

static long motore1 = -1;
static long motore2 = -1;
static long encoder1 = 0;
static long encoder2 = 0;
static long encoder3 = 0;

int UserStart(int * code)
{
int i;
char dev[40];

	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.8.p137");
	if (! ioAttach(dev,&motore1,32,DRIVER_MODE_INPUT)) {
			/* Errore... */
		plcError(5,dev);
	}
	i = 0;
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.8.p137.%d",i);
	if (! ioAttach(dev,&encoder1,32,DRIVER_MODE_INPUT)) {
			/* Errore... */
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.9.p137");
	if (! ioAttach(dev,&motore2,32,DRIVER_MODE_INPUT)) {
			/* Errore... */
		plcError(5,dev);
	}
	i = 0;
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.9.p137.%d",i);
	if (! ioAttach(dev,&encoder2,32,DRIVER_MODE_INPUT)) {
			/* Errore... */
		plcError(5,dev);
	}
	i = 0;
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.7.p131.%d",i);
	if (! ioAttach(dev,&encoder3,32,DRIVER_MODE_INPUT)) {
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
		plcMessage(1,"M1=%d E1=%d M2=%d E2=%d E3=%d",
		           motore1,encoder1,motore2,encoder2,encoder3);
		timer = t;
	}
}

