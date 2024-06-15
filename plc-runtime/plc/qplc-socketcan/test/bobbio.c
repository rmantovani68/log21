
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

#if 0
#define SELCO
#endif
#if 1
#define DAC 
#endif

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "test hardware 1.0 Fri Jul 18 17:54:46 MET DST 1997";

static unsigned char led[24];
static unsigned char in_selco[24];
static unsigned char out_selco[24];
static int compatto = 1;
static int nled = 8;
static int on = 1;
static int off = 0;
static long ser1_handle = 0;
static long ser2_handle = 0;
static short dac_out = 0;

static unsigned char byte_in = 0;
static unsigned char old_byte_in = 0;

int UserStart(int * code)
{
char dev[40];
int i;

#ifdef DAC
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.1.dac");
	if (! ioAttach(dev,&dac_out,sizeof(dac_out)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.byte.13");
	if (! ioAttach(dev,&byte_in,sizeof(byte_in)*8,DRIVER_MODE_INPUT)) {
		plcError(5,dev);
	}
#endif

#ifdef SELCO
	for (i = 0; i < 24; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.1.0.%d",i);
		if (! ioAttach(dev,&in_selco[i],1,DRIVER_MODE_INPUT)) {
			plcError(5,dev);
		}
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.1.0.%d",i + 24);
		if (! ioAttach(dev,&out_selco[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}
#endif

	compatto = 1;
	nled = 24;
#if 0
	for (i = 0; i < nled; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"keyb.0.led.%d",i);
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			if (i == 0) {
				compatto = 0;
				nled = 24;
				break;
			}
			plcError(5,dev);
		}
	}
#else
	compatto = 0;
	on = 0;
	off = 1;
#endif
	for (i = 0; i < nled; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.31.0.%d",i);
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}
	strcpy(dev,"ser.com1.19200n81");
	if (! ioAttach(dev,&ser1_handle,
	               sizeof(ser1_handle)*8,DRIVER_MODE_INPUT)) {
		plcError(5,dev);
	}
	strcpy(dev,"ser.com2.19200n81");
	if (! ioAttach(dev,&ser2_handle,
	               sizeof(ser2_handle)*8,DRIVER_MODE_INPUT)) {
		plcError(5,dev);
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
static char old_t[5];
char tmp;
int i;

	if (start) {
		for (i = 0; i < 6; ++i)
			plcSetBVect(plc_AZZOK,i,1);
		for (i = 0; i < 6; ++i)
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
* Questa funzione traduce i codici di tasto.
*/

static int trans(int k)
{
static int kb_compatto[] =
{ 0,1,6,7,12,13,17,18,2,3,8,9,14,15,19,20,4,5,10,11,16,21,22,23 };

	if (compatto)
		return kb_compatto[k];
	else
		return k;
}

int UserMain(int start)
{
unsigned long t,l;
char tmp;
int i;
static unsigned long timer;
static unsigned long tx1_timer;
static unsigned long tx2_timer;
static int curr_led;
static int curr_led_selco;
char msg[80];
static char rmsg1[80];
static char rmsg2[80];
static stream_rxtx_t tx1,rx1;
static stream_rxtx_t tx2,rx2;

	cn_minimo(start);

	t = plcTime();
	if (start) {
		tx2_timer = tx1_timer = timer = t;
		curr_led = curr_led_selco = 0;
		rx1.msg_curr_pos = 0;
		rx2.msg_curr_pos = 0;
	}

	if (t - tx1_timer >= 2000) {
		tx1_timer = t;
		util_safe_sprintf(msg,sizeof(msg),
		                  "Sono le %d e tutto va bene.\n",t);
		tx1.msg_size = strlen(msg);
		tx1.msg_curr_pos = 0;
		tx1.message = msg;
		if (! ioExtendedOp(ser1_handle,DREXTOP_STREAM_OP_SEND,&tx1))
			plcFault();
	}

	rx1.msg_size = 20;
	rx1.message = rmsg1;
	if (! ioExtendedOp(ser1_handle,DREXTOP_STREAM_OP_RECV,&rx1))
		plcFault();
	if (rx1.status & DREXTOP_STREAM_F_DONE) {
		rmsg1[rx1.msg_size] = '\0';
		plcMessage(100,"Ricevuto \"%s\" da %x",rmsg1,ser1_handle);
		rx1.msg_curr_pos = 0;
	}

	if (t - tx2_timer >= 1000) {
		tx2_timer = t;
		util_safe_sprintf(msg,sizeof(msg),
		                  "Ciclo %d - Sono le %d e tutto va bene.\n",
		                  plcNCycle(),t);
		tx2.msg_size = strlen(msg);
		tx2.msg_curr_pos = 0;
		tx2.message = msg;
		if (! ioExtendedOp(ser2_handle,DREXTOP_STREAM_OP_SEND,&tx2))
			plcFault();
	}

	rx2.msg_size = 5;
	rx2.message = rmsg2;
	if (! ioExtendedOp(ser2_handle,DREXTOP_STREAM_OP_RECV,&rx2))
		plcFault();
	if (rx2.status & DREXTOP_STREAM_F_DONE) {
		rmsg2[rx2.msg_size] = '\0';
		plcMessage(100,"Ricevuto \"%s\" da %x",rmsg2,ser2_handle);
		rx2.msg_curr_pos = 0;
	}

	if (t - timer > 300) {
		timer = t;
		led[curr_led] = off;
		curr_led = (curr_led + 1) % nled;
		led[curr_led] = on;
		out_selco[curr_led_selco] = 0;
		curr_led_selco = (curr_led_selco + 1) % 24;
		out_selco[curr_led] = 1;
	}
	for (i = 0; i < nled; ++i) {
		plcGetBVect(plc_TASTI,trans(i),&tmp);
		if (tmp)
			led[i] = on;
	}
#ifdef SELCO
	for (i = 0; i < 24; ++i)
		if (in_selco[i])
			out_selco[i] = 1;
	plcGetLVect(plc_OVR,0,&l);
	out_selco[(l * 24) / 256] = 1;
#endif
#ifdef DAC
	plcGetLVect(plc_OVR,0,&l);
	dac_out = (short)((l - 128) * 8);
	if (byte_in != old_byte_in) {
		plcMessage(99,"byte_in = %x",(int)byte_in);
		old_byte_in = byte_in;
	}
#endif
}

