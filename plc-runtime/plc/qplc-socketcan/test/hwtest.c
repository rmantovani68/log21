
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

#define OLD_PLC
#define MAX_AX 20
#define MAX_AX_STR "AX=20"

#if 0
#define SER
#define SER_STR "+SER"
#else
#define SER_STR ""
#endif
#if 0
#define SELCO
#define SELCO_STR "+SELCO"
#else
#define SELCO_STR ""
#endif
#if 0
#define DAC
#define DAC_STR "+DAC"
#else
#define DAC_STR ""
#endif
#if 0
#define UDC3
#define UDC3_STR "+UDC3"
#else
#define UDC3_STR ""
#endif
#if 0
#define UDC3SL
#define UDC3SL_STR "+UDC3SL"
#else
#define UDC3SL_STR ""
#endif
#if 0
#define BGAGE
#define BGAGE_STR "+BGAGE"
#else
#define BGAGE_STR ""
#endif
#if 0
#define MPPDAC
#define MPPDAC_STR "+MPPDAC"
#else
#define MPPDAC_STR ""
#endif
#if 0
#define CXDAC
#define CXDAC_STR "+CXDAC"
#else
#define CXDAC_STR ""
#endif
#if 0
#define ADC
#define ADC_STR "+ADC"
#else
#define ADC_STR ""
#endif
#if 0
#define LAE
#define LAE_STR "+LAE"
#else
#define LAE_STR ""
#endif
#if 1
#define SDA
#define SDA_STR "+SDA"
#else
#define SDA_STR ""
#endif


/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "testhw("
MAX_AX_STR SELCO_STR DAC_STR UDC3_STR UDC3SL_STR
BGAGE_STR SER_STR MPPDAC_STR CXDAC_STR ADC_STR SDA_STR
") 1.5 " __DATE__;

static unsigned char led[24];
static unsigned char led2[24];
static unsigned char in_udc3[16];
static unsigned char in_udc3sl[16];
static unsigned char in_selco[24];
static unsigned char out_selco[24];
static int compatto = 1;
static int nled = 8;
static int on = 1;
static int off = 0;
static long ser1_handle = 0;
static long ser2_handle = 0;
static short dac_out = 0;
static short bgage_in = 0;
static short bgage_out = 0;
static short asse_mpp = 0;
static unsigned char cxdac_out = 0;
static short adc_in[4];
static short sda_val[2];
static char sda_out[2];

unsigned char ix[20] = { 0 };
unsigned char ox[20] = { 0 };

static unsigned short lae_delay;
static unsigned short lae_pulse;
static char lae_trigl = 1;
static char lae_outl;
static char lae_out1;
static char lae_out2;

static unsigned char byte_in = 0;
static unsigned char old_byte_in = 0;

#if 0
int uno;
int due;
int tre;
int quattro;
int cinque;
#endif

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

#ifdef CXDAC
	util_safe_sprintf(dev,sizeof(dev),"ios.0.dac");
	if (! ioAttach(dev,&cxdac_out,sizeof(cxdac_out)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
#endif

#ifdef LAE
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.28.delay");
	if (! ioAttach(dev,&lae_delay,sizeof(lae_delay)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.28.pulse");
	if (! ioAttach(dev,&lae_pulse,sizeof(lae_pulse)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.28.outl");
	if (! ioAttach(dev,&lae_outl,1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.28.trigl");
	if (! ioAttach(dev,&lae_trigl,1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.28");
	if (! ioAttach(dev,&lae_out1,1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.0.0.32");
	if (! ioAttach(dev,&lae_out2,1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
#endif

#ifdef ADC
	for (i = 0; i < 4; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.3.adc.%d",i);
		if (! ioAttach(dev,&adc_in[i],16,DRIVER_MODE_INPUT)) {
			plcError(5,dev);
		}
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
#if 1
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

#ifdef UDC3SL
	for (i = 0; i < 11; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.1.udc.%d",i);
		if (! ioAttach(dev,&led2[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}
	for (i = 0; i < 16; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.1.udc.%d",i);
		if (! ioAttach(dev,&in_udc3sl[i],1,DRIVER_MODE_INPUT)) {
			plcError(5,dev);
		}
	}
#endif
#ifdef UDC3
	for (i = 0; i < 11; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.udc.%d",i);
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}
	for (i = 0; i < 16; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ios.0.udc.%d",i);
		if (! ioAttach(dev,&in_udc3[i],1,DRIVER_MODE_INPUT)) {
			plcError(5,dev);
		}
	}
#else
	for (i = 0; i < nled; ++i) {
#ifdef OLD_PLC
		strcpy(dev,"ios.0.0.31.0.00");
		if (i < 10) {
			dev[13] = '0' + i;
			dev[14] = '\0';
		}
		else {
			dev[13] = '0' + i / 10;
			dev[14] = '0' + i % 10;
		}
#else
		util_safe_sprintf(dev,sizeof(dev),"ios.0.0.31.0.%d",i);
#endif
		if (! ioAttach(dev,&led[i],1,DRIVER_MODE_OUTPUT)) {
			plcError(5,dev);
		}
	}
#endif
#ifdef SER
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
#endif

#ifdef MPPDAC
	strcpy(dev,"ax.0.1.mpp.2");
	if (! ioAttach(dev,&asse_mpp,
	               sizeof(asse_mpp)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
#endif

#ifdef BGAGE
	strcpy(dev,"bgage.0");
	if (! ioAttach(dev,&bgage_in,
	               sizeof(bgage_in)*8,DRIVER_MODE_INPUT)) {
		plcError(5,dev);
	}
	strcpy(dev,"bgage.0");
	if (! ioAttach(dev,&bgage_out,
	               sizeof(bgage_out)*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
#endif

#ifdef SDA
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.3.sda.0.us");
	if (! ioAttach(dev,&sda_val[0],
	               sizeof(sda_val[0])*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.3.sda.0.out");
	if (! ioAttach(dev,&sda_out[0],1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.3.sda.1.bp");
	if (! ioAttach(dev,&sda_val[1],
	               sizeof(sda_val[1])*8,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
	util_safe_sprintf(dev,sizeof(dev),"ios.0.0.3.sda.1.out");
	if (! ioAttach(dev,&sda_out[1],1,DRIVER_MODE_OUTPUT)) {
		plcError(5,dev);
	}
#endif
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

int su_e_giu = 0;

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
		lae_delay = 2000;
		lae_pulse = 2000;
	}

#ifdef LAE
{
static int conta = 0;
static int conta2 = 0;
long l;
static long ol = 0;

#if 1
	if (++conta == 100) {
		lae_out2 = !lae_out2;
		conta = 0;
	}
#endif
	plcGetLVect(plc_OVR,0,&l);
	if (l != ol) {
		ol = l;
		plcError(9000,"OVR = %d",l);
	}
	lae_pulse = (unsigned short)(l * 10);
#if 1
	if (++conta2 == 20) {
		lae_out1 = !lae_out1;
		conta2 = 0;
	}
#else
	lae_out1 = 1;
#endif
}
#endif

#ifdef SER
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
#endif

#ifdef BGAGE
{
static int azzera = 0;
static int passo_azz = 0x800;
static int val_azz = 0x800;
static int filtro[10];
static int nf = 0;
static int conta = 0;
char tmp;

	plcGetBVect(plc_TASTI,0,&tmp);
	if (tmp) {
	/* Azzeramento. */
		nf = 0;
		if (azzera) {
			if (passo_azz) {
				passo_azz /= 2;
				if (bgage_in == 0) {
					passo_azz = 0;
				}
				else if (bgage_in > 0) {
					val_azz += passo_azz;
				}
				else {
					val_azz -= passo_azz;
				}
				if (! passo_azz)
					plcMessage(201,"Azzeramento BGAGE = %d = %f V",
					           val_azz,
						   val_azz * 0.004882813);
			}
		}
		else {
			azzera = 1;
		}
		bgage_out = val_azz;
	}
	else {
		filtro[nf % 10] = bgage_in;
		++nf;
		azzera = 0;
		val_azz = passo_azz = 0x800;
		if (++conta >= 50) {

		int i,s;

			conta = 0;
			for (i = s = 0; i < 10; ++i)
				s += filtro[i];
			s /= 10;
			plcMessage(200,"Input BGAGE = %d = %f V, media = %d = %f V",
			           bgage_in,
			           bgage_in * 0.004882813,
			           s,
			           s * 0.004882813);
		}
	}
}
#endif

#if 0
{
static int ccc = 0;
static int iii = -1000;
static float fff = 1024.0;


	if (++ccc >= 50) {
		ccc = 0;
		plcMessage(200,"intero = %d, float = %f", iii, fff);
		iii += 123;
		fff /= 2;
	}
}
#endif

#ifdef UDC3SL
	for (i = 1; i < 8; ++i)
		led2[i] = in_udc3sl[i];
	if (t - timer > 300) {
		led2[0] = !led2[0];
		timer = t;
	}
	for (i = 8; i < 16; ++i) {
		if (in_udc3sl[i]) {
			led2[8] = (i & 1) != 0;
			led2[9] = (i & 2) != 0;
			led2[10] = (i & 4) != 0;
			break;
		}
	}
#endif
#ifdef UDC3
	for (i = 1; i < 8; ++i)
		led[i] = in_udc3[i];
	if (t - timer > 300) {
		led[0] = !led[0];
		timer = t;
	}
	for (i = 8; i < 16; ++i) {
		if (in_udc3[i]) {
			led[8] = (i & 1) != 0;
			led[9] = (i & 2) != 0;
			led[10] = (i & 4) != 0;
			break;
		}
	}
#else
	if (t - timer > 300) {
		su_e_giu = !su_e_giu;
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
#if 0
	if (uno)
		led[0] = on;
	if (due)
		led[2] = on;
	if (tre)
		led[4] = on;
	if (quattro)
		led[6] = on;
	if (cinque)
		led[8] = on;
#endif
#endif
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

#ifdef CXDAC
{
static int cxdac_step = 1;

	if (cxdac_out == 0xFF)
		cxdac_step = -1;
	else if (cxdac_out == 0x00)
		cxdac_step = 1;
	cxdac_out += cxdac_step;
}
#endif

#ifdef ADC
{
static int conta = 0;

	if (++conta >= 50) {
		conta = 0;
		plcMessage(1,"Input ADC = %d %d %d %d",
		           adc_in[0], adc_in[1], adc_in[2], adc_in[3]);
	}
}
#endif

#ifdef SDA
{
static short val = 0;
static int conta = 0;

	sda_val[0] = -(sda_val[1] = val);
	++val;
	if (++conta >= 50) {
		sda_out[0] = !(sda_out[1] = sda_out[0]);
		conta = 0;
	}
}
#endif

#if 0
{
	plcGetLVect(plc_OVR,0,&l);
	l *= 10000;
	while (l > 0)
		--l;
}
#endif

#ifdef MPPDAC
	plcGetLVect(plc_OVR,0,&l);
	asse_mpp = (short)((l - 128) * 8);
#endif

{
static int conta = 0;
static int posto = 0;

	if (++conta == 200) {
		conta = 0;
		ox[posto] = 0;
		if (++posto == 20) {
			posto = 0;
		}
		ox[posto] = 1;
	}
}

}

