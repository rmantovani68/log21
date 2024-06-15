
/*
* @(#) p260.c 1.0 Fri Jan 25 15:36:31 MET 2002
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 25/01/02 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test P260 1.0";

static char out[16];
static char in[16];

int UserStart(int * code)
{
int i;
char dev[40];

/* Aggancio dei quattro LED del modulo di indirizzo 1 collegato
 al canale 3 della prima scheda. Le variabile "led[n]" diventano
 le immagini dei LED corrispondenti. */
	for (i = 0; i < 16; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.0.p260.%d",i);
		if (! ioAttach(dev,&in[i],1,DRIVER_MODE_INPUT)) {
			/* Errore... */
			plcError(5,dev);
		}
		if (! ioAttach(dev,&out[i],1,DRIVER_MODE_OUTPUT)) {
			/* Errore... */
			plcError(5,dev);
		}
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

	for (i = 0; i < 16; ++i) {
		out[i] = in[i];
	}

	if (t - timer > 500) {
		timer = t;
	}

}

