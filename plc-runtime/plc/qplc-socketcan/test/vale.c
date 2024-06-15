
/*
* @(#) vale.c 1.0 Fri Jul 18 17:53:24 MET DST 1997
*
* Piccolo programma di prova, che esemplifica l'uso delle principali
* primitive di qplc.
*
* 18/07/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

#define MAX_AX 20
#define MAX_AX_STR "AX=20"

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "Test tempi 1.0 " __DATE__;

static unsigned char led[24];
static int nled = 24;
static int on = 1;
static int off = 0;
static int su_e_giu = 0;
static unsigned char byte_in = 0;
static unsigned char old_byte_in = 0;

/*
* UserStart e` chiamata all'entrata in GO dell'esecutore PLC.
*/

int UserStart(int * code)
{
char dev[40];
int i;

	on = 0;
	off = 1;

	for (i = 0; i < nled; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.31.0.%d",i);
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}

	return 1;
}

/*
* UserStop e` chiamata all'entrata in HALT.
*/

void UserStop(void)
{
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

/*
* Funzione principale. E` chiamata ogni 10 millisecondi.
*/

static void outb(unsigned short port, unsigned char val)
{
	__asm__ volatile ("outb %%al,%%dx"::"d"(port),"a"(val):"eax","edx");
}

int UserMain(int start)
{
unsigned long t,l;
char tmp;
int i;
static unsigned long timer;
static int curr_led;

	outb(0x378,0xFF);

	cn_minimo(start);

	t = plcTime();
	if (start) {
		timer = t;
		curr_led = 0;
	}

	if (t - timer > 300) {
		su_e_giu = !su_e_giu;
		timer = t;
		led[curr_led] = off;
		curr_led = (curr_led + 1) % nled;
		led[curr_led] = on;
	}

	for (i = 0; i < nled; ++i) {
		plcGetBVect(plc_TASTI,i,&tmp);
		if (tmp)
			led[i] = on;
	}

	outb(0x378,0x00);
}

