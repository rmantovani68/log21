
/*
* @(#) p810.c 1.1 Fri Jan 25 09:56:57 MET 2002
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 18/01/02 GG 1.0 Prima stesura.
* 25/01/02 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test display P810 1.1";

static char led[4];
static char tasto;
static int display = -1;

int UserStart(int * code)
{
int i;
char dev[40];

/* Aggancio dei quattro LED del modulo di indirizzo 1 collegato
 al canale 3 della prima scheda. Le variabile "led[n]" diventano
 le immagini dei LED corrispondenti. */
	for (i = 0; i < 4; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.1.p810.%d",i);
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			/* Errore... */
			plcError(5,dev);
		}
	}
/* Aggancio del tasto del modulo. */
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.1.p810.0");
	if (! ioAttach(dev,&tasto,1,DRIVER_MODE_INPUT)) {
		/* Errore... */
		plcError(5,dev);
	}
/* Aggancio dello pseudo-ingresso indeice del modulo. */
	util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.1.p810");
	if (! ioAttach(dev,&display,32,DRIVER_MODE_INPUT)) {
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
		led[0] = 1;
	}

	if (t - timer > 500) {
		timer = t;
		tmp = led[0];
		for (i = 0; i < 4; ++i)
			led[i] = led[i+1];
		led[3] = tmp;
	}
	if (tasto) {
		Drawnum810(display,'1',plcTime() % 1000);
	}
	else {
		Drawchar810(display,'0',
		            led[0]+'0',led[1]+'0',led[2]+'0',led[3]+'0');
	}
}

