/* ALLINEAMI ! */
char ver_user[] = "2.2.0.1\t\t[14/07/1997 17:22:21]";
/*
* @(#) plc480.c 2.2.0.1 Mon Jul 14 17:22:21 MET DST 1997
*
* Pacchetto di compatibilita` col plc per 480.
* NON MODIFICARE QUESTO FILE !
*
* 150295 GG Versione 1.0.
* 030395 GG Versione 1.1. Aggiunte le funzioni UserStart ed UserStop.
* 100595 GG Versione 1.2. Aggiornamento a seguito di modifiche hardware.
*           Cambiato il tipo alla funzione UserStart. Ora e` "int" e,
*           se vale 0, fa fallire lo start del plc. Inoltre ammette un
*           parametro di tipo "void *". Questa prestazione e` qui utilizzta
*           per impedire lo start su macchine sprovviste dell'hardware
*           richiesto dalla tabella MSD.
* 190695 GG Versione 1.3. Utilizzate le variabili "pMSD_input" e "pMSD_output"
*           anziche` "MSD_input" ed "MDS_output". Questo permette di caricare
*           dinamicamente nuove tabelle di associazione.
* 290196 GG Versione 1.4. Aggiunto numero di versione ed etichetta per
*           caricamneto dinamico.
* 200596 GG Versione 2.0.0.0. Rimaneggiato per inserimento nel nuovo PLC.
* 22/10/96 GG 2.0.0.1 Aggiunto il controllo sulla configurazione degli I/O
*             in stile isagraf.
* 30/10/96 GG 2.0.0.2 Raccolto un po' il codice.
* 09/01/97 GG 2.1.0.0 Aggiunta la chiamata ad una funzione opzionale
*             di inizializzazione a livello di programma 480.
* 15/01/97 GG 2.2.0.0 Aggiunto l'aggiornamento dell'indice del programma
*             corrente (gestione provvisoria).
* 14/07/97 GG 2.2.0.1 Sostituite le "sprintf" con "util_safe_sprintf".
*/

#include <stdio.h>

#include "qplc.h"
#include "plcapi.h"

/* Da commentare per eventuale caricamento dinamico : */
/*
 */
static
/*
 */
char module_name[] = "U_CODE";

unsigned char regA[0700][010];
unsigned char regB[0200][010];
unsigned char regC[0600];
unsigned int first_nc_plc;
unsigned char num_ss_nc_plc;
unsigned int first_plc_nc;
unsigned char num_ss_plc_nc;
unsigned int first_input;
unsigned char num_input;
unsigned int first_output;
unsigned char num_output;
unsigned int first_const;
unsigned char num_const;
unsigned int tim_att;
unsigned int tim_old;
unsigned char qualeloop;

/*
* Distribuzione dei registri e preparazione delle tabelle di corrispondenza.
* Questa funzione deve essere chiamata allo start.
* Vale 1 se tutto e` andato bene, 0 altrimenti (assenza del controllore IOS
* selezionato). "code" contiene l'indice del controllore che non e` stato
* trovato.
*/

static int read_msd_core(int *code, unsigned int *msd, unsigned int first, int io, unsigned char *num)
{
int i,j,k;
char byte_name[30];
char bit_name[40];

	for (i = 0; msd[i] != (unsigned int) (_END_MSD); ++i) {
		j = msd[i];
		util_safe_sprintf(byte_name,sizeof(byte_name),
		        "ios.%d.%d.%d.%d",
		        j / 0x8000,
		        (j & 0x7800) / 0x0800,
			(j & 0x00FF) / 0x0008,
		        j & 0x0007);
		for (k = 0; k < 8; ++k) {
			util_safe_sprintf(bit_name,sizeof(bit_name),
			                  "%s.%d",byte_name,k);
			regA[i+first][k] = 0;
			if (! ioAttach(bit_name,
			               &regA[i+first][k],
			               1,
			               io)) {
			/* ERRORE. Dispositivo assente o gia` occupato. */
				*code = j;
			/*
			Mi dicono che cio` e` male.

				plcError(120,bit_name);
				return 0;
			*/
			}
		}
		++(*num);
	}

	return 1;
}

static int read_msd(int * code)
{
/* Per ora non sono previsti segnali di scambio, ma le variabili
 associate sono mantenute per compatibilita`. */
	first_nc_plc = 0;
	num_ss_nc_plc = 0;
	first_plc_nc = first_nc_plc + num_ss_nc_plc;
	num_ss_plc_nc = 0;
/* Calcolo del numero di variabili di input. */
	first_input = first_plc_nc + num_ss_plc_nc;
	num_input = 0;
	if (! read_msd_core(code,
	                    main_status.mod_plc480_op.msdin,
	                    first_input,
	                    DRIVER_MODE_INPUT,
	                    &num_input))
		return 0;
/* Calcolo del numero di variabili di output. */
	first_output = first_input + num_input;
	num_output = 0;
	if (! read_msd_core(code,
	                    main_status.mod_plc480_op.msdout,
	                    first_output,
	                    DRIVER_MODE_OUTPUT,
	                    &num_output))
		return 0;
/* Posizione della costanti. Per ora non sono previste. */
	first_const = first_output + num_output;
	num_const = 0;

	return 1;
}

int UserStart(int * code)
{
/* Controlla la configurazione hardware riportata in NVRAM. */
	if (! compat_check_io())
		return 0;
/* Chiama l'inizializzatore del programma 480 (tipicamente si trattera`
 di un costruttore di tabella MSD). */
	if (main_status.mod_plc480_op.init) {
		main_status.mod_plc480_op.init();
	}
/* Inizializza le tabelle. */
	if (! read_msd(code))
		return 0;
	return 1;
}

int UserMain(int start)
{
	if (start) {
	/* Indica "start". */
		qualeloop = 0;
	/* Inizializza i timer. */
		tim_att = 0;
		tim_old = (unsigned int) -1;
	}

/* Aggiusta l'indice del programma corrente (provvisorio, ed un po' grezzo). */

	main_status.curr_prog_id = -1;

/* Lancia la procedura 480-compatibile. */
	main_status.mod_plc480_op.main();

/* Aggiusta l'indice del programma corrente (provvisorio, ed un po' grezzo). */

	main_status.curr_prog_id = 0;

/* Aggiorna il timer ed il flag di start. */
	tim_old = tim_att++;
	qualeloop = 1;

	return 0;
}

void UserStop(void)
{
}

