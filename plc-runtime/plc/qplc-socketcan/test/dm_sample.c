

/*
* Esempio di codice utente eseguito da Isagraf.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "C_CODE";
/* Aggiornare la versione ! */
char ver_prog[] = "sbarlucc 2.1 Mon Jan 29 15:27:34 MET 1996";

/*
* Tabelle di corrispondenza tra gruppi di 8 variabili di input o di
* output e posizione corrispondente nello spazio della IOS.
*
* Questa tabella non e` obbligatoria se si intende caricare dinamicamente
* l'oggetto.
*/

/* Esempio : si dispone della seguente configurazione hardware :
*
* - Una scheda IOSPC, installata come unita` 0, contenente
*   il solo controller 0,
* - Un modulo con 8 output e 16 input all'insirizzo IOS 0,
* - Un modulo con 8 input e 16 output all'insirizzo IOS 1.
*
* Una possibile tabella di corrispondenza e` la seguente :
*/


BEGIN_MSD_INPUT
	IOREG(0,0,0,1)	/* Modulo 0, primo input -> regA[first_input] */
	IOREG(0,0,0,2)	/* Modulo 0, secondo input -> regA[first_input + 1] */
	IOREG(0,0,1,0)	/* Modulo 1, primo input -> regA[first_input + 2] */
END_MSD

BEGIN_MSD_OUTPUT
	IOREG(0,0,0,0)	/* Modulo 0, primo output -> regA[first_output] */
	IOREG(0,0,1,1)	/* Modulo 1, primo output -> regA[first_output + 1] */
	IOREG(0,0,1,2)	/* Modulo 1, secondo output -> regA[first_output + 2] */
	IOREG(0,0,31,0)	/* LED PLC (0-7) */
	IOREG(0,0,31,1)	/* LED PLC (8-15) */
	IOREG(0,0,31,2)	/* LED PLC (16-23) */
END_MSD

/*
* Lettura dei dati macchina.
*/

static int index = 0;
static int nindex = 0;

void leggi_dm(void)
{
float f;

	if (GetGenericData(MData,"Plc",1,"dato1",&f) < 0) {
		f = 7.0;
/*
		sys_err(200,1);
*/
	}

	nindex = (int) f;
}

/*
* Fanno comodo.
*/

static unsigned char *Input,*Output;

/*
* Esempio di programma plc.
*/

void PlcInit(void)
{
	printf("Per passarci, ci passo...\n");
}

void PlcMain(void)
{
char t;
int i;
static int conta;
static int ledtas = 0;
long l;
char appo;
static char old_str;
static char old_pic;
static char memT[5];
int  attr;

static int c1 = 0;
static int c2 = 0;

	if (qualeloop == 0) {

/*
		strcpy(NVRAM,"BEPPE");
*/

		Input = regA[first_input];
		Output = regA[first_output];

		for (i = 0; i < 6; ++i)
			plcSetBVect(plc_AZZOK,i,1);
		for (i = 0; i < 6; ++i)
			plcSetBVect(plc_HOLDAX,i,0);

		conta = 0;
		old_str=0;
		old_pic=0;

		memset(memT,0,sizeof(memT));
	}

/* I dati macchina devono essere riletti all'inizio ed ogni volta
 che cambiano. */

	if (MData && (GetStatoDm(MData) == ID_SDM_DM_MODIFICATI || qualeloop == 0))
		leggi_dm();

	if (index != nindex)
		Output[index] = 0;
	index = nindex;

	for (i = 0; i < 24; ++i) {
		plcGetBVect(plc_TASTI,i,&appo);
		if (appo)
			Output[i + 24] = 0;
	}

	plcGetBScal(plc_RESPLC,&appo);
	if (appo) {
		printf("Plc : RESET !\n"); fflush(stdout);
		c1 = 50;
		Output[6] = 1;
	}
	else {
		if (c1)
			--c1;
		else
			Output[6] = 0;
	}
	plcGetBScal(plc_STOP,&appo);
	if (appo) {
		printf("Plc : STOP !\n"); fflush(stdout);
		c2 = 50;
		Output[4] = 1;
	}
	else {
		if (c2)
			--c2;
		else
			Output[4] = 0;
	
	}

	plcGetBVect(plc_PIC,0,&appo);
	plcSetBVect(plc_RICFIN,0,!appo);
	plcGetBVect(plc_DATVAL,0,&appo);
	plcSetLVect(plc_ORIG,0,appo);

	plcSetLVect(plc_GENERAL,1,(Input[0] ? 1:0) << 6);

	if (++conta == 15) {
		Output[index] = !Output[index];
		Output[ledtas + 24] = 1;
		ledtas = (ledtas + 1) % 24;
		Output[ledtas + 24] = 0;
		conta = 0;
	}

/* Rilevazione fronte strobe e memorizzazione T's */

	plcGetBVect(plc_STROBE,0,&appo);

	if (appo && !old_str) {
		plcGetBArr(plc_T,0,0,&memT[0]);
		plcGetBArr(plc_T,0,1,&memT[1]);
		plcGetBArr(plc_T,0,2,&memT[2]);
		plcGetBArr(plc_T,0,3,&memT[3]);
		plcGetBArr(plc_T,0,4,&memT[4]);
	}
	if (!appo & old_str) plcSetBVect(plc_PAESE,0,0);
	old_str=appo;

	if (!appo && old_pic) {
		memset(memT,0,sizeof(memT));
	}
	old_pic=appo;

/* Output assi posizionati e T's */

	plcGetBVect(plc_AXPOS,0,&appo);
	if (appo) {
		if (old_str)
			plcSetBVect(plc_PAESE,0,1);
	}

	for (i = 0; i < 5; ++i)
		plcGetBVect(plc_ASSCOM,i,&Output[i]);
}

