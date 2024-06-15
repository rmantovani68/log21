
/*
* @(#) piani.c 1.0 Wed Apr 15 18:34:09 MET DST 1998
*
* Piccolo programma di collaudo dei moduli speciali
* utilizzati per le quote dei piani mobili BiEsse.
*
* 15/04/98 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

#define MAX_AX 20

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test-PDL 1.0 " __DATE__;

static unsigned char comandi[32*4];
static unsigned char dati[32*4][3];

int UserStart(int * code)
{
char dev[40];
int i,j;

	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 32; ++j) {
			util_safe_sprintf(dev,sizeof(dev),"ios.%d.%d.%d.%d",
				i/2, i%2, j, 0);
			if (! ioAttach(dev,&comandi[i*32+j],8,
				DRIVER_MODE_OUTPUT)) {
			/* Che fare ? */
				continue;
			}
			util_safe_sprintf(dev,sizeof(dev),"ios.%d.%d.%d.%d",
				i/2, i%2, j, 1);
			if (! ioAttach(dev,&dati[i*32+j][0],8,
				DRIVER_MODE_OUTPUT)) {
			/* Che fare ? */
				continue;
			}
			util_safe_sprintf(dev,sizeof(dev),"ios.%d.%d.%d.%d",
				i/2, i%2, j, 2);
			if (! ioAttach(dev,&dati[i*32+j][1],8,
				DRIVER_MODE_OUTPUT)) {
			/* Che fare ? */
				continue;
			}
			util_safe_sprintf(dev,sizeof(dev),"ios.%d.%d.%d.%d",
				i/2, i%2, j, 3);
			if (! ioAttach(dev,&dati[i*32+j][2],8,
				DRIVER_MODE_OUTPUT)) {
			/* Che fare ? */
				continue;
			}
		}
	}
	return 1;
}

void UserStop(void)
{
#if 0
/* Solo un esperimento... */
	if (plcHaltCycle() < 10) {
		plcError(2000,"%d",plcHaltCycle());
		plcRepeatHalt();
	}
#endif
}

/*
* Questa funzione fa il minimo che ci si aspetta per poter mettere
* una lavorazione in START.
*/

static void cn_minimo(int start)
{
static char old_str;
static char old_pic;
static char old_t[16];
char tmp;
int i;

	if (start) {
		for (i = 0; i < MAX_AX; ++i)
			plcSetBVect(plc_AZZOK,i,1);
		for (i = 0; i < MAX_AX; ++i)
			plcSetBVect(plc_HOLDAX,i,0);
		old_str = 0;
		old_pic = 0;
		for (i = 0; i < 5; ++i)
			old_t[i] = 0;
	}

	plcGetBVect(plc_PIC,0,&tmp);
	plcSetBVect(plc_RICFIN,0,!tmp);
	plcGetBVect(plc_DATVAL,0,&tmp);
	plcSetLVect(plc_ORIG,0,tmp);

	plcGetBVect(plc_STROBE,0,&tmp);
	if (tmp && !old_str) {
		for (i = 0; i < 5; ++i)
			plcGetBArr(plc_T,0,i,&old_t[i]);
	}
	if (!tmp && old_str)
		plcSetBVect(plc_PAESE,0,0);
	old_str = tmp;

	plcGetBVect(plc_PIC,0,&tmp);
	if (!tmp && old_pic) {
		for (i = 0; i < 5; ++i)
			old_t[i] = 0;
	}
	old_pic = tmp;

	plcGetBVect(plc_AXPOS,0,&tmp);
	if (tmp && old_str)
		plcSetBVect(plc_PAESE,0,1);
}

static int fase = 0;
static int pattern_quota;
static int pattern_punto;
static int pattern_display;
static int pattern_led;
static int quota;
static unsigned long timer;

int UserMain(int start)
{
unsigned long t;
int i;

	cn_minimo(start);

	t = plcTime();
	switch (fase) {
/**/
	case 0:
	/* Attesa iniziale prima del reset reset. */
		if (t - timer >= 1000)
			fase = -1;
		break;

/**/
	case -1:
	/* Reset completo. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x19;
		fase = -2;
		pattern_quota = 1234;
		pattern_punto = 0;
		pattern_display = 0xFF;
		pattern_led = 1;
		quota = 1;
		break;
	case -2:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 1;
		timer = t;
		break;
	case 1:
	/* Attesa completamento reset. */
		if (t - timer >= 1000)
			fase = 22;
		break;
/**/
	case 22:
	/* Impostazione del pattern sul campo quota n. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = (unsigned char) (quota * 1111);
			dati[i][1] = (unsigned char) ((quota * 1111) >> 8);
			dati[i][2] = 0;
		}
		fase = 23;
		break;
	case 23:
	/* Invio comando quota 1. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = quota;
		fase = -23;
		break;
	case -23:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 24;
		timer = t;
		break;
	case 24:
	/* Attesa completamento impostazione quota n. */
		if (t - timer >= 250) {
			fase = 255;
		}
		break;
/**/
	case 255:
	/* Visualizzazione quota n. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = (unsigned char) quota;
			dati[i][1] = 0;
			dati[i][2] = 0;
		}
		fase = 25;
		break;
	case 25:
	/* Invio comando visualizzazione quota n. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x1A;
		fase = -25;
		break;
	case -25:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 26;
		timer = t;
		break;
	case 26:
	/* Attesa completamento visualizzazione quota n. */
		if (t - timer >= 250) {
			if (++quota > 8)
				fase = 355;
			else
				fase = 22;
		}
		break;
/**/
	case 355:
	/* Visualizzazione quota 1. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = 1;
			dati[i][1] = 0;
			dati[i][2] = 0;
		}
		fase = 35;
		break;
	case 35:
	/* Invio comando visualizzazione quota 1. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x1A;
		fase = -35;
		break;
	case -35:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 36;
		timer = t;
		break;
	case 36:
	/* Attesa completamento visualizzazione quota 1. */
		if (t - timer >= 250) {
			fase = 2;
		}
		break;
/**/
	case 2:
	/* Impostazione del pattern sul campo quota 1. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = (unsigned char) pattern_quota;
			dati[i][1] = (unsigned char)(pattern_quota >> 8);
			dati[i][2] = 0;
		}
		fase = 3;
		break;
	case 3:
	/* Invio comando quota 1. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x01;
		if (pattern_quota % 100 >= 90)
			pattern_quota -= 100;
		if (pattern_quota % 10 >= 9)
			pattern_quota -= 10;
		pattern_quota = (pattern_quota * 100
		               + pattern_quota % 100 + 11) % 10000;
		fase = -3;
		break;
	case -3:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 4;
		timer = t;
		break;
	case 4:
	/* Attesa completamento impostazione quota 1. */
		if (t - timer >= 250)
			fase = 5;
		break;
/**/
	case 5:
	/* Accensione del punto decimale su tutte le quote. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = (unsigned char) pattern_punto;
			dati[i][1] = 0;
			dati[i][2] = 0;
		}
		fase = 6;
		break;
	case 6:
	/* Invio comando posizione punto. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x20;
		pattern_punto = (pattern_punto + 1) % 4;
		fase = -6;
		break;
	case -6:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 7;
		timer = t;
		break;
	case 7:
	/* Attesa completamento impostazione quota 1. */
		if (t - timer >= 250)
			fase = 8;
		break;
/**/
	case 8:
	/* Impostazione del pattern sul display quota 1. */
		for (i = 0; i < sizeof(comandi); ++i) {
			dati[i][0] = (unsigned char) pattern_display;
			dati[i][1] = (unsigned char) pattern_led;
			dati[i][2] = 0;
		}
		fase = 9;
		break;
	case 9:
	/* Invio comando pattern display. */
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x31;
		if (pattern_display == 0xFF)
			pattern_display = 0x00;
		else if (pattern_display == 0x00)
			pattern_display = 0x01;
		else if (pattern_display == 0x80)
			pattern_display = 0xFF;
		else
			pattern_display *= 2;
		pattern_led = pattern_led % 4 + 1;
		fase = -9;
		break;
	case -9:
		for (i = 0; i < sizeof(comandi); ++i)
			comandi[i] = 0x00;
		fase = 10;
		timer = t;
		break;
	case 10:
	/* Attesa completamento impostazione pattern display 1. */
		if (t - timer >= 250) {
			if (pattern_quota > 8000)
				fase = 355;
			else
				fase = 2;
		}
		break;
/**/
	default:
		fase = 0;
		break;
	}
}

