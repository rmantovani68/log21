
/*
* @(#) p250.c 1.0 Tue Mar 12 16:20:33 MET 2002
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
char ver_user[] = "test P250 1.0";

static short out[4];

static double (*p_sin)(double);

/*
* Funzioncina per ricavare l'indirizzo di un simbolo esterno
* da moduli di qplc.
*/

static void * symbol_address(const char *name)
{
int cambiato;
int chiave;
long v;
shv_t segnale;
void *rv;
static void *(*symaddr_fn)(const char *) = (void *(*)(const char *)) NULL;
static int init = 0;

	if (! init) {
	/* Cerca il segnale di scambio magico, che contiene
	 l'indirizzo del risolutore di simboli di qplc. */
		strcpy(segnale.name,"$PLC_SYMBOL_ADDRESS");
		segnale.dim1 = 1;
		segnale.dim2 = 1;
		segnale.key = -1;
		segnale.mode = SHV_MODE_TEST;

		chiave = shvDefine(&segnale,&cambiato);

		if (chiave != -1) {
		/* Il segnale c'e`! Ora basta estrarre il valore. */
			plcGetLScal(chiave,&v);
			symaddr_fn = (void *(*)(const char *)) v;
		}
	/* Memorizza "inizializzato", anche se qualche operazione
	 e` fallita, cosi` almeno non si tentera` piu` di trovare il
	 segnale di scambio: se non c'e`, non c'e`!... */
		init = 1;
	}

	if (symaddr_fn) {
		rv = (*symaddr_fn)(name);
	}
	else {
		rv = (void *) NULL;
	}

	return rv;
}


int UserStart(int * code)
{
int i;
char dev[40];

	for (i = 0; i < 4; ++i) {
		util_safe_sprintf(dev,sizeof(dev),"ybn.0.3.4.p250.%d",i);
		if (! ioAttach(dev,&out[i],16,DRIVER_MODE_OUTPUT)) {
			/* Errore... */
			plcError(5,dev);
		}
	}
	p_sin = (double (*)(double)) symbol_address("sin");
	if (! p_sin) {
		plcError(180,"Function \"sin\"");
		return 0;
	}
	else {
		return 1;
	}
}

void UserStop(void)
{
}

int UserMain(int start)
{
unsigned long t;
char tmp;
int i;

#define sin(x) (*p_sin)(x)

	t = plcTime();

	out[0] = (short) (0x7FFF * sin(((double)t/1000) * M_PI * 2 * 1));
	out[1] = (short) (0x7FFF * sin(((double)t/1000) * M_PI * 2 * 2));
	out[2] = (short) (0x7FFF * sin(((double)t/1000) * M_PI * 2 * 3));
	out[3] = (short) (0x7FFF * sin(((double)t/1000) * M_PI * 2 * 4));
}

