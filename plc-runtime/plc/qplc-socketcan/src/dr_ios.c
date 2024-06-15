/*
* @(#) dr_ios.c 4.12 mer dic 16 22:15:50 CET 2009
*
* Driver della scheda CNI IOS per QPLC.
*
* Forma dei parametri specifici :
*
*       16 bit, moduli con convertitore D/A : <cpu>.<modulo>.dac
*       16 bit, moduli con convertitore D/A : <cpu>.<modulo>.sda.<indice>.<modo>
*       16 bit, moduli con convertitore A/D : <cpu>.<modulo>.adc.<indice adc>
*       16 bit, moduli con convertitore A/D : <cpu>.<modulo>.adcn.<indice adc>
*        8 bit : <cpu>.<modulo>.<byte>
*        8 bit, accesso diretto al controllore : <cpu>.byte.<byte>
*        1 bit : <cpu>.<modulo>.<byte>.<bit>
*        1 bit : LED.<bit>
*        8 bit : DAC
*        1 bit : CNOK
*        1 bit : UDC.<bit>
*        1 bit : MPP.<bit>
*
* Esempio di nome di variabile completo (1 bit):
*
*        ios.0.0.2.1.5
*         |  | | | | |
*         |  | | | | bit 5...
*         |  | | | del byte 1...
*         |  | | del modulo 2...
*         |  | collegato alla CPU 0...
*         |  della scheda 0...
*         di tipo IOS.
*
*  NOTA: I moduli speciali Selco sono visti come un modulo a 24
*  ingressi e 24 uscite. I 24 ingressi sono i bit da 0 a 23, le 24 uscite
*  sono i bit da 24 a 47.
*
* 24/05/96 GG 1.0 Prima stesura.
* 10/06/96 GG 1.1 Aggiunta la possibilita` di indicare un indice di bit
*             maggiore di 7, che e` da interpretare come appartenente a
*             byte successivi. In questo modo, l'ultimo bit di un modulo
*             con 24 input puo` essere indicato sia ...2.7 che ...0.23 .
*             Inoltre, con i moduli misti, si puo` ricordare semplicemente
*             che il gruppo di segnali minoritari ha byte 0 e bit 0-7,
*             e quello dei maggioritari ha byte 1 e bit 0-15.
* 22/06/96 GG 1.2 Corretta la funzione "_ios_clear_outs", che non azzerava
*             i moduli da 24 output.
* 25/06/96 GG 1.3 Aggiunta la manipolazione del riaggancio dei moduli.
* 28/08/96 GG 1.4 Modifiche per parametro aggiuntivo di "parse".
* 30/08/96 GG 1.5 Modifiche per uso della funzione "plcPeriod".
* 12/09/96 GG 1.6 Aggiunto qualche commento, in vista della formalizzazione
*             degli errori.
* 19/09/96 GG 1.7 Aggiunta l'emissione di adeguati errori in caso di problemi.
* 18/02/97 GG 1.8 Aggiunto il supporto per la porta LED del CN compatto.
* 20/02/97 GG 1.9 Corretta la logica di controllo dei LED (erano negati,
*             errore di interpretazione dello schema elettrico...).
* 17/04/97 GG 1.10 Aggiunti due flag di attivazione del trigger e del
*             controllo della scheda (per debug).
* 22/05/97 GG 1.11 Aggiunta la gestione di alcuni moduli speciali, come
*             ta tastiera Selco.
* 26/05/97 GG 1.12 Piccola modifica per evitare la confusione tra segnali
*             di input e segnali di output nei moduli speciali Selco. Ora
*             Gli output cominciano dal bit 24 ed arrivano al 47.
* 16/07/97 GG 1.13 Aggiunti i metodi "up", "close" ed "extension".
* 28/07/97 GG 2.0 Riorganizzate "parse" ed "attach" per gestione a byte
*             di moduli qualsiasi. Eliminati gli accessi "a 32 bit", che non
*             hanno in pratica alcuna utilita`. Aggiunti gli accessi diretti
*             a byte ed i convertitori D/A a 12 bit.
* 01/08/97 GG 2.1 Aggiunta un'opzione di configurazione del driver che
*             impedisce l'accesso a byte arbitrari della IOS. Sara` normalmente
*             abilitata, perche` questo tipo di accesso puo` pregiudicare
*             la sicurezza del sistema.
* 01/08/97 GG 2.2 Corretta una voragine nella gestioen del DAC. E anche
*             una pericolosa inesattezza nella scrittura del pacchetto
*             comando+dati, la quale poteva provocare occasionali, improbabili
*             e quindi pericolosissimi malfunzionamenti.
* 19/11/97 GG 2.3 Modificate alcune scritture sui moduli speciali perche`
*             utilizzavano istruzioni a 32 bit, che possono interferire
*             con i floppy. Per lo stesso motivo e` stata utilizzata la nuova
*             funzione "uitl_memset" al posto di "memset".
* 21/01/98 GG 2.4 Aggiunto il supporto pe la scheda UDC3. Ha richiesto la
*             definizione di una nuova categoria di segnali e la revisione
*             della procedura di trigger/check per gestire il watchdog diretto.
* 06/02/98 GG 2.5 Corretta l'acquisizione degli indirizzi delle porte UDC3.
*             Corretto il test sull'indice massimo di uscita UDC3 (deve essere
*             minore di 11).
*             Corrette le chiamate ad alcuni metodi specifici di UDC3.
*             Aggiunto un contatore per ignorare l'eventuale stato di fault
*             fisiologico la prima volta che si legge il byte
*             di stato dell'UDC3 (succede che si legge lo stato del watchdog
*             senza averlo mai rinfrescato).
* 13/02/98 GG 2.6 Aggiunta la variabile di configurazione "udcchk", che
*             contiene la durata del periodo di insensibilita` allo stato
*             di watchdog spento nel byte di stato dell'UDC3. Sembra infatti
*             che quel dispositivo si comporti in un modo un po' strano:
*             sembra che non reagisca immediatamente al retrigger, ma solo
*             dopo un po'. Questa variabile permette di tarare qplc sul
*             ritardo di risposta del watchdog, espressa in millisecondi.
*             Il valore di default e` 1000, vale a dire che qplc all'entrata
*             nel ciclo di esecuzione (o di attesa comandi, se in HALT)
*             ignorera` la segnalazione (evidentemente fasulla) di fault del
*             watchdog per un secondo.
* 16/02/98 GG 2.7 Portato a 1.2 secondi il periodo di insensibilita` allo
*             stato di "CN non ok".
*             Corretta un'imperfezione (che non faceva danni) nella scrittura
*             delle uscite UDC3.
* 11/03/98 GG 2.8 Aggiunto il metodo "detach".
* 13/03/98 GG 2.9 Aggiunto l'azzeramento delle uscita UDC3 in "fatal".
*             Aggiunta la possibilita` di mantenere lo stato dei bit
*             non utilizzati di un byte di output tra una forzatura e
*             l'altra (attivo solo se AZZERA_NON_USATI_IOS vale 0).
* 20/03/98 GG 2.10 Migliorata la gestione della UDC3 (e MPP3) parametrizzando
*             il numero di ingressi e di uscite presenti (due nuovi campi
*             nella struttura di stato).
*             La funzione di scrtittura sulla UDC3 (o MPP3) e` stata resa
*             piu` generale.
* 23/03/98 GG 2.11 Il driver accetta ora anche la stringa "MPP" come sinonimo
*             di "UDC". Questo dovrebbe facilitare la vita all'editor PLC.
* 24/03/98 GG 2.12 Aggiunto il metodo "list".
* 01/04/98 GG 3.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
* 03/04/98 GG 3.1 Aggiunto il metodo "show" (e corrette le immancabili
*             sfighe della versione precedente).
* 08/06/98 GG 3.2 Aggiunto il supporto per il DAC della scheda CX.
* 15/06/98 GG 3.3 Aggiunto il supporto per i moduli ADC.
* 18/06/98 GG 3.4 Mancava la de-associazione dell'uscita DAC CX.
* 30/06/98 GG 3.5 Aggiunta la variabile di configurazione "wdtime", che
*             contiene il tempo d'intervento del watchdog della IOS espresso
*             in cicli di PLC.
* 25/08/98 GG 3.6 Aggiunto un contatore di cicli di timeout per la IOS,
*             necessario per evitare segnalazioni di timeout erronee quando
*             il ciclo di plc e` molto breve (timer INAX). E` anche stato
*             corretto un'errore nella calcolo dell'indice della scheda in
*             timeout (poteva venire indicata la scheda sbagliata).
* 19/11/98 GG 3.7 Modifica per visualizzare anche il numero del modulo
*             andato in fault. Vedere "ioAddError".
* 20/11/98 GG 3.8 Pesanti modifiche per gestire i moduli LAE.
* 21/11/98 GG 3.9 Prime correzioni alle immancabili sfighe (crash, robe
*             da chiodi!).
* 11/12/98 GG 3.10 Supporto per la scheda MPP3 SLAVE (nessuna paura! e` solo
*             una prestazione cosmetica per il debugger remoto. Si tratta
*             dell'identificazione della scheda per nome anziche` per codice).
* 28/07/00 GG 3.11 Aggiustamenti per compilazione su Linux.
* 30/01/01 GG 4.0 Supporto per moduli DAC multipli.
* 02/02/01 GG 4.1 Aggiunti gli ingressi "ADCN", che funzionano come gli ADC,
*             ma normalizzano il valore a 16 bit.
*              Resa piu` solida la procedura di lettura degli ingressi ADC.
* 15/03/01 GG 4.2 Aggiunta la leggibilita` del byte 0 dei moduli speciali,
*             in modo che si possa realizzare da logica PLC l'handshake
*             dei comandi di moduli sconosciuti (es. SDIS). Ma in realta`
*             non puo` essere usato affidabilmente...
*              Aggiunto l'accesso al CNOK della scheda IOSLK2 (ios.0.cnok).
* 19/03/01 GG 4.3 Aggiunta la gestione dei moduli SDIS. Ogni modulo SDIS
*             gestisce fino a 16 display, associati a veriabili a 32 bit,
*             inoltre e` prevista un'uscita di configurazione a un bit per
*             ogni display, che serve ad accendere il punto decimale (e
*             visualizzare i "decimi"). Le uscite si chiamano
*             "ios.<scheda>.<cpu>.<modulo>.SDIS.<display>[.DEC]." Le
*             uscite "....DEC" controllano il punto decimale del
*             modulo corrispondente.
* 23/03/01 GG 4.4 Corretto un orrendo bug nella gestione dei moduli XDAC
*             e SDIS. Era sbagliata la definizione dei puntatori delle
*             liste dei moduli di questo tipo. Era privo di conseguenze
*             solo perche` il tipo utilizzato si riveriva ad una struttura
*             piu` grande.
*              Corretto uno schianto nella scrittura dei moduli SDIS nel caso
*             di associazioni incompleta (mancanza dell'uscita DEC o VALORE).
*             Era dovuto alla mancanza di un controllo su un puntatore NULL.
* 11/05/01 GG 4.5 Aggiunte due nuove variabili di configurazione,
*             ("chkwd={0|1}" e "chkdis={0|1}") che controllano il test
*             passivo dello stato della CPU e il test sulla disconnessione
*             dei moduli. E` anche stata riabilitata la variabile "wd=<n>".
* 09/06/06 GG 4.6 (Ne e` passato del tempo!...) Prima versione
*             documentata della diramazione AEL. Aggiunta l'entrata
*             in preoperational dei moduli CANOpen all'HALT.
* 27/02/08 GG 4.7 (E` passato altro tempo!...) Prima versione documentata
*             con I/O analogici CANOpen. NOTA: e' stata distribuita
*             una versione preliminare in cui gli analogici funzionavano
*             ma non erano riportati dal comando "list".
* 02/04/08 GG 4.8 Tentativo di supporto degli encoder CN104.
* 08/07/08 GG 4.9 Aggiunto un flag per la disattivazione della rimappatura
*             dei PDO sui moduli CNI Informatica.
* 04/08/08 GG 4.10 Riportata dalla versione demo del 6/5/2008 la gestione
*             del flag di abilitazione del canale CAN (opzione
*             enable_can={0|1}). E' stato tolto l'indice del canale dal
*             nome della variabile perche' puo` essere indicato in modo
*             generale dopo la parola "ios".
* 14/09/08 GG 4.11 Aggiunti i function block di lettura e scrittura
*             del dizionario CANOpen (SDO_READ/SDOWRITE).
* 16/12/09 GG 4.12 Aggiunta la variabile di configurazione "only_can"
*             che permette di ignorare il canale IOS tradizionale
*             anche se presente per passare direttamente al canale CAN.
*             E' a 1 per default.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hwcni.h>
#include <ios.h>

#include "qplc.h"

#include "util.h"

#include "dr_ios.h"

#if 0
#define MAX_CAN_IOBYTE 19
#else
#define MAX_CAN_IOBYTE 15
#endif

#define LEN_IOS_ALLOC_BLK 16
#if 1
#define LEN_IOS_ALLOC_W24BIT_BLK 8 /* E quante altre tastiere Selco volete ? */
#define LEN_IOS_ALLOC_W12BIT_BLK 8 /* E quanti altri moduli DAC volete ? */
#define LEN_IOS_ALLOC_AIN_BLK 16
#define LEN_IOS_ALLOC_AOUT_BLK 16
#define LEN_IOS_ALLOC_ENC_BLK 16
#define LEN_IOS_ALLOC_LAE_BLK 8 /* E quanti altri moduli LAE volete ? */
#define LEN_IOS_ALLOC_XDAC_BLK 8 /* E quanti altri moduli XDAC volete ? */
#define LEN_IOS_ALLOC_SDIS_BLK 8 /* E quanti altri moduli SDIS volete ? */
#else
#define LEN_IOS_ALLOC_W24BIT_BLK 32 /* Voglio il massimo! */
#define LEN_IOS_ALLOC_W12BIT_BLK 32
#define LEN_IOS_ALLOC_LAE_BLK 32
#define LEN_IOS_ALLOC_XDAC_BLK 32
#define LEN_IOS_ALLOC_SDIS_BLK 32
#endif

/*
* Definizioni legate alla gerarchia dei dispositivi controllati dal driver.
*/

/* Livello 2: tipo di controllore. */
#define c_bus next.spec[0]
#define t_c_bus tree.spec[0]
#define LEVEL_2_CPU 0 /* Base per le CPU: da 0 a MAX_IOS_CPU escluso. */
	#define c_mod next.spec[1]
	#define t_c_mod tree.spec[1]
	/* Livello 3: Indice modulo (da 0 a MAX_IOS_MOD escluso). */
		/* Livello 4: */
		#define c_byte next.spec[2]
		#define t_c_byte tree.spec[2]
		#define c_sts next.spec[3]
		#define c_sts2 next.spec[6]
		#define t_c_sts tree.spec[3]
		#define t_c_sts2 tree.spec[6]
		/* Byte del modulo (0-MAX_CAN_IOBYTE). */
			#define c_bit next.spec[4]
			#define t_c_bit tree.spec[4]
			#define c_mode next.spec[5]
			#define t_c_mode tree.spec[5]
			/* Livello 5: bit nel byte (0-7). */
		/* DAC */
		#define LEVEL_4_DAC (MAX_CAN_IOBYTE+1)
		/* Tasti tastiera S24T e ingressi LAE. (0.0-0.23). */
		#define LEVEL_4_SELCO_IN 17
		/* LED tastiera S24T e uscite LAE. (0.24-0.47). */
		#define LEVEL_4_SELCO_OUT (17+24)
		/* Livello 5: configurazione moduli speciali LAE. */
			#define c_laeconf next.spec[4]
			#define t_c_laeconf tree.spec[4]
			/* Polarita` dei segnali d'ingresso. */
			#define LEVEL_5_LAE_TL 0
			/* Polarita` dei segnali d'uscita. */
			#define LEVEL_5_LAE_OL 1
			/* Ritardo sugli ingressi. */
			#define LEVEL_5_LAE_DT 2
			/* Durata dell'impulso prodotto dal monostabile. */
			#define LEVEL_5_LAE_PT 3
			/* Fine */
			#define LEVEL_5_LAE_QUIT 4
		/* Ingressi ADC dei moduli speciali. */
		#define LEVEL_4_ADC (17+2*24)
		#define LEVEL_4_ADCN (17+2*24+4)
		/* Moduli DAC multipli (XDAC). */
		#define LEVEL_4_XDAC (17+2*24+8)
		/* Livello 5: Possibili utilizzi di un'uscita XDAC. */
			#define c_xdacconf next.spec[4]
			#define t_c_xdacconf tree.spec[4]
			#define c_xdacmode next.spec[5]
			#define t_c_xdacmode tree.spec[5]
			#define LEVEL_5_XDAC_BP 0
			#define LEVEL_5_XDAC_UP 1
			#define LEVEL_5_XDAC_SG 2
			#define LEVEL_5_XDAC_DR 3
		/* I 16 display controllabili da un modulo SDIS. */
		#define LEVEL_4_SDIS (17+2*24+8+2)
		/* Livello 5: Punto decimale sull'n-esimo display */
			#define c_sdisconf next.spec[4]
			#define t_c_sdisconf tree.spec[4]
			#define c_sdismode next.spec[5]
			#define t_c_sdismode tree.spec[5]
			#define LEVEL_5_SDIS_DEC 0
		/* Ingressi analogici generici (CANOpen, tipicamente). */
		#define LEVEL_4_AIN (17+2*24+8+2+16)
		/* Uscite analogiche generiche (CANOpen, tipicamente). */
		#define LEVEL_4_AOUT (17+2*24+8+2+16+16)
		/* Ingressi encoder (CANOpen, tipicamente). */
		#define LEVEL_4_ENC (17+2*24+8+2+16+16+16)
		/* Fine */
		#define LEVEL_4_QUIT (17+2*24+8+2+16+16+16+16)
	/* Livello 3: accesso a byte qualsiasi del controllore. */
	#define MAX_CAN_MOD 128
	#define LEVEL_3_BYTE (MAX_CAN_MOD+1)
		#define c_byte2 next.spec[6]
		#define t_c_byte2 tree.spec[6]
		/* Livello 4: byte (0-0x3ff). */
#define LEVEL_2_LED (MAX_IOS_CPU)
	/* Livello 3: indice LED (0-7). */
	#define c_ledbit c_mod
	#define t_c_ledbit t_c_mod
#define LEVEL_2_UDC (MAX_IOS_CPU+1)
	/* Valori base per segnali UDC3 o MPP3 input e output. */
	/* Livello 3: Ingressi UDC. */
	#define c_udcbit c_mod
	#define t_c_udcbit t_c_mod
	#define LEVEL_3_UDCIN 0
	/* Livello 3: Uscite UDC. */
	#define LEVEL_3_UDCOUT 64
	#define LEVEL_3_UDCQUIT 128
#define LEVEL_2_MPP (MAX_IOS_CPU+2)
	/* Livello 3: Ingressi UDC (vedi). */
	/* Livello 3: Uscite UDC (vedi). */
#define LEVEL_2_CXDAC (MAX_IOS_CPU+3)
#define LEVEL_2_CNOK (MAX_IOS_CPU+4)
#define LEVEL_2_QUIT (MAX_IOS_CPU+5)

/* Liste delle celle libere. */

static ios_byte_t *ios_byte_free = (ios_byte_t *) 0;
static ios_direct_byte_t *ios_direct_byte_free = (ios_direct_byte_t *) 0;
static ios_w24bit_t *ios_w24bit_free = (ios_w24bit_t *) 0;
static ios_w12bit_t *ios_w12bit_free = (ios_w12bit_t *) 0;
static ios_ain_t *ios_ain_free = (ios_ain_t *) 0;
static ios_aout_t *ios_aout_free = (ios_aout_t *) 0;
static ios_enc_t *ios_enc_free = (ios_enc_t *) 0;
static ios_lae_t *ios_lae_free = (ios_lae_t *) 0;
static ios_xdac_t *ios_xdac_free = (ios_xdac_t *) 0;
static ios_sdis_t *ios_sdis_free = (ios_sdis_t *) 0;

/* Array delle strutture di stato (per istanza, cioe` per scheda). */

static dr_ios_t ios_board[MAX_IOS_BOARD];

static canboard_t *canbus_table[MAX_IOS_BOARD] = { NULL, };

/* Prossimo indice di dispositivo CAN da tentare in mancanza di IOS. */
static int next_can_device = 0;

/* Macro per l'invio di un dato ad un'uscita speciale e
 l'aggiornamento dell'immagine. */

/* Uscita tipo Selco. Il byte di comando e` il codice di STOP degli 
 assi remoti (va bene qualsiasi valore diverso da zero)
 Si e` scelto questo codice perche` cosi` ogni accesso indebito
 (es. se per errore un modulo assi ed un modulo Selco
 sono stati scambiati) provoca l'arresto dell'asse. */
#define IOS_W24BIT_OUT(w,v) { \
register unsigned char * _p_ = (w) -> addr; \
register unsigned long _v_ = (v); \
\
	(w) -> image = _v_; \
	_p_[1] = (unsigned char)(_v_ >>= 8); \
	_p_[2] = (unsigned char)(_v_ >>= 8); \
	_p_[3] = (unsigned char)(_v_ >>= 8); \
	_p_[0] = 0x0C; \
}

/* Uscita tipo DAC. Il byte di comando e` il codice di accesso diretto al
 convertitore degli assi remoti. */
#define IOS_W12BIT_OUT(w,v) { \
register unsigned char * _p_ = (w) -> addr; \
register unsigned long _v_ = (v); \
\
	(w) -> image = _v_; \
	_p_[1] = (unsigned char)(_v_ >>= 8); \
	_p_[2] = (unsigned char)(_v_ >>= 8); \
	_p_[3] = (unsigned char)(_v_ >>= 8); \
	_p_[0] = 0x20; \
}

#define IOS_W32BIT_OUT(w,v) { \
register unsigned char * _p_ = (w) -> addr; \
register unsigned long _v_ = (v); \
\
	(w) -> image = _v_; \
	_p_[1] = (unsigned char)(_v_ >>= 8); \
	_p_[2] = (unsigned char)(_v_ >>= 8); \
	_p_[3] = (unsigned char)(_v_ >>= 8); \
	_p_[0] = (unsigned char)((w)->image); \
}


/* Funzione per l'azzeramento di tutti gli output esistenti. */

static void _ios_clear_leds(dr_ios_t *dr)
{
	dr -> led_image = -1; /* Questo forza la riscrittura della porta */
	(*(dr -> write_leds))(dr,0);
}

static void _ios_clear_cxdac(dr_ios_t *dr)
{
	dr -> cxdac_image = -1; /* Questo forza la riscrittura della porta */
	(*(dr -> write_cxdac))(dr,0);
}

static void _ios_clear_cnok(dr_ios_t *dr)
{
	dr -> cnok_image = -1; /* Questo forza la riscrittura della porta */
	(*(dr -> write_cnok))(dr,0);
}

static void _ios_clear_udc3(dr_ios_t *dr)
{
	dr -> udc3_out_image = -1; /* Questo forza la riscrittura della porta */
	(*(dr -> write_udc3))(dr,0);
}

static void _ios_clear_outs(dr_ios_t *dev)
{
int i,j;
unsigned char sts;

/* Azzera l'uscita LED del modello compatto. */

	_ios_clear_leds(dev);

/* Azzera l'uscita DAC del modello compatto. */

	_ios_clear_cxdac(dev);

/* Azzera l'uscita CNOK. */

	_ios_clear_cnok(dev);

/* Azzera le uscite UDC3. */

	_ios_clear_udc3(dev);

/* Azzera tutto il resto (almeno, le cose normali). */

	if (dev -> canbus) {
		for (i = 1; i < 128; ++i) {
			if (dev -> canmod[i].present) {
				for (j = 0; j < dev -> canmod[i].outb; ++j) {
					canboard_set_output_byte(
						dev -> canbus, i, j, 0);
				}
				for (j = 0; j < dev -> canmod[i].aout; ++j) {
					canboard_set_output_word(
						dev -> canbus, i, j, 0);
				}
			}
		}
	}
	else {
		for (i = 0; i < dev -> n_cpu; ++i) {
			for (j = 0; j < MAX_IOS_MOD; ++j) {
				sts = dev -> acpu[i][IOS_IOSTATUS + j];
				if (! (sts & IOS_STS_M_SPEC)) {
					switch (sts & IOS_STS_M_OUT) {
					case 1:
						dev -> acpu[i][j * 8] = 0;
						break;
					case 3:
						dev -> acpu[i][j * 8] = 0;
					case 2:
						dev -> acpu[i][j * 8 + 1] = 0;
						dev -> acpu[i][j * 8 + 2] = 0;
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

#ifndef Linux
static void _ios_wr_led_fast(register dr_ios_t * dr, register int v)
{
	if (dr -> led_image != v) {
		util_outb( (dr -> led_image = v), dr -> led_port);
	}
}

static void _ios_wr_led_slow(register dr_ios_t * dr, register int v)
{
	if (dr -> led_image != v) {
		ioctl(dr -> fd, IOSWRLED, (void *)(dr -> led_image = v));
	}
}

static void _ios_wr_cxdac_fast(register dr_ios_t * dr, register int v)
{
	if (dr -> cxdac_image != v) {
		dr -> cxdac_image = v;
		util_outb(v << dr -> cxdac_ports.p.ports[0].pos,
		          dr -> cxdac_ports.p.ports[0].ioaddr);
	}
}

static void _ios_wr_cxdac_slow(register dr_ios_t * dr, register int v)
{
	if (dr -> cxdac_image != v) {
		ioctl(dr -> fd, IOSWRCXDAC, (void *)(dr -> cxdac_image = v));
	}
}

static void _ios_wr_cnok_fast(register dr_ios_t * dr, register int v)
{
	v = (v != 0);
	if (dr -> cnok_image != v) {
		dr -> cnok_image = v;
		util_outb(v << dr -> cnok_ports.p.ports[0].pos,
		          dr -> cnok_ports.p.ports[0].ioaddr);
	}
}

#define _ios_wr_cnok_slow _ios_wr_cnok_fast

static int _ios_rd_udc3_fast(register dr_ios_t * dr)
{
register int v;

#if 0
/* In teoria dovrebbe essere cosi`... */
	v = (util_inb(dr -> udc3_in_ports.p.ports[0].ioaddr)
	     >> dr -> udc3_in_ports.p.ports[0].pos)
	  & ((1 << dr -> udc3_in_ports.p.ports[0].nbits) - 1);
	if (dr -> udc3_in_ports.p.nports > 1) {
		v |= ((util_inb(dr -> udc3_in_ports.p.ports[1].ioaddr)
		      >> dr -> udc3_in_ports.p.ports[1].pos)
		      & ((1 << dr -> udc3_in_ports.p.ports[1].nbits) - 1)) << 8;
	}
#else
/* ...ma in pratica si sa che nbits = 8 e pos = 0. */
	v = util_inb(dr -> udc3_in_ports.p.ports[0].ioaddr);
	if (dr -> udc3_in_ports.p.nports > 1) {
		v |= util_inb(dr -> udc3_in_ports.p.ports[1].ioaddr) << 8;
	}
#endif
	return v;
}

static int _ios_rd_udc3_slow(register dr_ios_t * dr)
{
int v;

	ioctl(dr -> fd, IOSRDUDC3, &v);
	return v;
}

static void _ios_wr_udc3_fast(register dr_ios_t * dr, register int v)
{
	if (dr -> udc3_out_image != v) {
		dr -> udc3_out_image = v;
		util_outb(v << dr -> udc3_out_ports.p.ports[0].pos,
		          dr -> udc3_out_ports.p.ports[0].ioaddr);
		if (dr -> udc3_out_ports.p.nports > 1) {
		int d;
			d = dr -> udc3_out_ports.p.ports[0].nbits
			  - dr -> udc3_out_ports.p.ports[1].pos;
			if (d >= 0)
				v >>= d;
			else if (d)
				v <<= -d;
			util_outb(v,dr -> udc3_out_ports.p.ports[1].ioaddr);
		}
	}
}

static void _ios_wr_udc3_slow(register dr_ios_t * dr, register int v)
{
	if (dr -> udc3_out_image != v) {
		ioctl(dr -> fd, IOSWRUDC3, (void *)(dr -> udc3_out_image = v));
	}
}

static int _ios_chk_udc3_fast(register dr_ios_t * dr)
{
	return (util_inb(dr -> udc3_wdsts_ports.p.ports[0].ioaddr)
	     >> dr -> udc3_wdsts_ports.p.ports[0].pos)
	  & ((1 << dr -> udc3_wdsts_ports.p.ports[0].nbits) - 1);
}

static int _ios_chk_udc3_slow(register dr_ios_t * dr)
{
int v;

	ioctl(dr -> fd, IOSRDSTSREG, &v);
	v = (v >> dr -> udc3_wdsts_ports.p.ports[0].pos)
	  & ((1 << dr -> udc3_wdsts_ports.p.ports[0].nbits) - 1);
	return v;
}

static int _ios_chk_udc3(register dr_ios_t * dr)
{
	if (dr -> udc3_firstchk) {
		--(dr -> udc3_firstchk);
		return 1;
	}
	return (dr -> check_udc3)(dr);
}

static void _ios_trig_udc3_slow(register dr_ios_t * dr)
{
#if 0
	ioctl(dr -> fd, IOSWRBM,
	      IOS_MASK(0xFF,dr -> udc3_trigwd_ports.p.ports[0].pos));
	ioctl(dr -> fd, IOSWRBM,
	      IOS_MASK(0x00,dr -> udc3_trigwd_ports.p.ports[0].pos));
#else
	ioctl(dr -> fd, IOSWRBM,
	      IOS_MASK(dr -> udc3_wdout_image,
	               dr -> udc3_trigwd_ports.p.ports[0].pos));
	dr -> udc3_wdout_image ^= 0xFF;
#endif
}
#define _ios_trig_udc3_fast _ios_trig_udc3_slow
#else /* Linux */
static int _ios_nullfn(void*p,...)
{
	return 1;
}

#define  _ios_wr_led_fast _ios_nullfn
#define  _ios_wr_cxdac_fast _ios_nullfn
#define  _ios_wr_cnok_fast _ios_nullfn
#define  _ios_rd_udc3_fast _ios_nullfn
#define  _ios_wr_udc3_fast _ios_nullfn
#define  _ios_chk_udc3_fast _ios_nullfn
#define  _ios_trig_udc3_fast _ios_nullfn
#define  _ios_wr_led_slow _ios_nullfn
#define  _ios_wr_cxdac_slow _ios_nullfn
#define  _ios_wr_cnok_slow _ios_nullfn
#define  _ios_rd_udc3_slow _ios_nullfn
#define  _ios_wr_udc3_slow _ios_nullfn
#define  _ios_chk_udc3_slow _ios_nullfn
#define  _ios_trig_udc3_slow _ios_nullfn
#define  _ios_chk_udc3 _ios_nullfn

#endif /* Linux */

static int ios_install(driver_t * d, int conf)
{
int n = conf;
int i,j;
dr_ios_t *p;
char *q;
int check,trigger,direct,udcchk,wdtime,awd,max_count,checkwd,checkdis;
int use_fd,need_fd;
int canspeed,candebug,cansync,cancniremap;
int forcesync;
int bitoffset;
int enable_can;
int only_can;

	if (d -> instance >= MAX_IOS_BOARD) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	trigger = check = checkwd = checkdis = 1;
	direct = 0;
	udcchk = 1200;
	wdtime = 4;
	awd = 10;
	canspeed = 500;
	candebug = 0;
	cansync = 1;
	cancniremap = 0;
	forcesync = 0;
	bitoffset = 0;
	enable_can = 1;
	only_can = 1;
	for (ioStartResource(d); (q = ioNextResource(d)); ) {
		if (util_strneq(q,"trig=",5)) {
		/* Flag di abilitazione del trigger al watchdog
		 attivo della IOS. Default=1. */
			q += 5;
			trigger = ioGetNum(&q);
		}
		else if (util_strneq(q,"chk=",4)) {
		/* Flag di abilitazione del test periodico
		 della IOS. Default=1. */
			q += 4;
			check = ioGetNum(&q);
		}
		else if (util_strneq(q,"chkwd=",6)) {
		/* Flag di abilitazione del controllo passivo dello stato
		 della CPU della IOS. Default=1. */
			q += 6;
			checkwd = ioGetNum(&q);
		}
		else if (util_strneq(q,"chkdis=",7)) {
		/* Flag di abilitazione del controllo passivo dello stato
		 della CPU della IOS. Default=1. */
			q += 7;
			checkdis = ioGetNum(&q);
		}
		else if (util_strneq(q,"direct=",7)) {
		/* Flag di abilitazione dell'accesso diretto ai byte
		 della IOS. Default=0. */
			q += 7;
			direct = ioGetNum(&q);
		}
		else if (util_strneq(q,"wdtime=",7)) {
		/* Periodo di retrigget del watchdog, in millisecondi,
		 piu` o meno. */
			q += 7;
			wdtime = ioGetNum(&q);
		}
#if 1
		else if (util_strneq(q,"wd=",3)) {
		/* Periodo di tolleranza alla "morte" della scheda IOS.
		 in millisecondi. */
			q += 3;
			awd = ioGetNum(&q);
		}
#endif
		else if (util_strneq(q,"udcchk=",7)) {
		/* Periodo di tolleranza iniziale ai falsi fault della
		 scheda UDC e simili. Default=1200. */
			q += 7;
			udcchk = ioGetNum(&q);
		}
		else if (util_strneq(q,"canspeed=",9)) {
		/* Velocita` dell'eventaule bus CAN. Default=500 Kbit/s. */
			q += 9;
			canspeed = ioGetNum(&q);
		}
		else if (util_strneq(q,"candebug=",9)) {
		/* Flag di debug dell'eventaule bus CAN.  Default=0. */
			q += 9;
			candebug = ioGetNum(&q);
		}
		else if (util_strneq(q,"cansync=",8)) {
		/* Flag di emissione del SYNC su bus CAN.  Default=1. */
			q += 8;
			cansync = ioGetNum(&q);
		}
		else if (util_strneq(q,"cancniremap=",12)) {
		/* Flag di disattivazione della rimappatura dei PDO
		 sui moduli CNI Informatica. */
			q += 12;
			cancniremap = ioGetNum(&q);
		}
		else if (util_strneq(q,"forcesync=",10)) {
		/* Flag di emissione del SYNC su bus CAN.  Default=1. */
			q += 10;
			forcesync = ioGetNum(&q);
		}
		else if (util_strneq(q,"bitoffset=",10)) {
		/* Offset sottratto all'indice di bit, per poter
		 * usare l'MSD generato da EDIT_PLC con i moduli CAN. */
			q += 10;
			bitoffset = ioGetNum(&q);
		}
		else if (util_strneq(q,"enable_can=",11)) {
			q += 11;
			enable_can = ioGetNum(&q);
		}
		else if (util_strneq(q,"only_can=",8)) {
			q += 8;
			only_can = ioGetNum(&q);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}

	p = &ios_board[n];
	p -> fd = -1;
	p -> do_chk = check;
	p -> do_chk_wd = checkwd;
	p -> do_chk_dis = checkdis;
	p -> do_trig = trigger;
	p -> wdtime = wdtime;
	p -> max_count = awd / plcPeriod();
	p -> direct_byte_access = direct;

	p -> bitoffset = bitoffset;
	p -> enable_can = enable_can;
	p -> only_can = only_can;
	p -> canbus = NULL;
	p -> candevice = -1;
	p -> canspeed = canspeed;
	p -> candebug = candebug;
	p -> cansync = cansync;
	p -> cancniremap = cancniremap;
	p -> forcesync = forcesync;
	for (i = 0; i < 128; ++i) {
		p -> canmod[i].present = 0;
		p -> canmod[i].in = 0;
		p -> canmod[i].inb = 0;
		p -> canmod[i].out = 0;
		p -> canmod[i].outb = 0;
		p -> canmod[i].ain = 0;
		p -> canmod[i].aout = 0;
		p -> canmod[i].enc = 0;
	}

	if (p -> only_can
	 || iosOpen(n, &p -> base, p -> cpu, &p -> nvram,
	            (int *) &p -> fd)
	    < 0) {
	/* Niente IOS. Tentiamo col CAN. */
		/*
		* rm 06-05-2008 : gestione abilitazione porte can
		*/
		if(p -> enable_can){
			p -> canbus = canboard_open(next_can_device,
																	p -> canspeed,
																	p -> candebug);
		}
		if (! p -> canbus) {
		/* ERRORE. Il dispositivo non esiste. */
			return 0;
		}
	/* Impostazione dell'opzione di non rimappatura dei PDO
	 sui moduli CNI Informatica. */
		canboard_set_option(p -> canbus,
		                    CANBOARD_OPTION_NOREMAP_CNIINF,
		                    !(p -> cancniremap));
		/* Ma guarda! Abbiamo un controllore CAN! */
		p -> candevice = next_can_device;
		canbus_table[p -> candevice] = p -> canbus;
		++next_can_device;
		/* Inapplicabili nel caso CAN (per ora...) */
		p -> do_chk = 0;
		p -> do_trig = 0;
	}

	use_fd = need_fd = 0;

/* Sceglie il metodo migliore per accedere alle porte. */
	if (util_isrootgroup()) {
	/* Ho il permesso di accedere direttamente alle porte ! */
		p -> write_leds = _ios_wr_led_fast;
		p -> write_cxdac = _ios_wr_cxdac_fast;
		p -> write_cnok = _ios_wr_cnok_fast;
		p -> read_udc3 = _ios_rd_udc3_fast;
		p -> write_udc3 = _ios_wr_udc3_fast;
		p -> check_udc3 = _ios_chk_udc3_fast;
		p -> trig_udc3 = _ios_trig_udc3_fast;
	}
	else {
	/* Dovro` usare ioctl(IOSWR...). Peccato. */
		p -> write_leds = _ios_wr_led_slow;
		p -> write_cxdac = _ios_wr_cxdac_slow;
		p -> write_cnok = _ios_wr_cnok_slow;
		p -> read_udc3 = _ios_rd_udc3_slow;
		p -> write_udc3 = _ios_wr_udc3_slow;
		p -> check_udc3 = _ios_chk_udc3_slow;
		p -> trig_udc3 = _ios_trig_udc3_slow;
	/* Devo lasciare aperto il file descriptor... */
		need_fd = 1;
	}

/* Test per verificare la presenza dell'uscita LED. */
	p -> n_leds = 0;
	p -> led_image = 0;
#ifdef Linux
	p -> led_present = 0;
#else
	p -> led_present = 1;
	if (ioctl(p -> fd, IOSLEDPORT, (void *)(&p -> led_port)) < 0)
		p -> led_present = 0;
	else
		use_fd |= need_fd;
#endif

/* Si ricava il modello. Serve solo al debugger. */
#ifdef Linux
	p -> model = 0;
#else
	{
	struct iosboard_gen b;

		b.struct_size = sizeof(b);
		if (ioctl(p -> fd, IOSGETALL, (void *) &b) < 0)
			p -> model = 0;
		else
			p -> model = b.model;
	}
#endif

/* Test di presenza dei segnali UDC3. */
	
	p -> n_udc3_in = p -> n_udc3_out = 0;
	p -> udc3_firstchk = udcchk / plcPeriod();
	p -> udc3_wdout_image = 0;
	p -> udc3_out_image = 0;
	p -> udc3_num_in = 0;
	p -> udc3_num_out = 0;
#ifdef Linux
	p -> udc3_present = 0;
	p -> udc3_in_ports.p.port_set = 0;
	p -> udc3_in_ports.p.nports = 0;
	p -> udc3_out_ports.p.port_set = 0;
	p -> udc3_out_ports.p.nports = 0;
	p -> udc3_trigwd_ports.p.port_set = 0;
	p -> udc3_trigwd_ports.p.nports = 0;
	p -> udc3_wdsts_ports.p.port_set = 0;
	p -> udc3_wdsts_ports.p.nports = 0;
	p -> cxdac_present = 0;
	p -> cxdac_ports.p.port_set = 0;
	p -> cxdac_ports.p.nports = 0;
	p -> cnok_present = 0;
	p -> cnok_ports.p.port_set = 0;
	p -> cnok_ports.p.nports = 0;
#else
	p -> udc3_present = 1;
	p -> udc3_in_ports.p.port_set = IOS_PORTS_TYPE_UDC3_IN;
	p -> udc3_in_ports.p.nports = (sizeof(p -> udc3_in_ports)
	                             - sizeof(p -> udc3_in_ports.p))
	                            / sizeof(p -> udc3_in_ports.p.ports[0])
	                            + 1;
	p -> udc3_out_ports.p.port_set = IOS_PORTS_TYPE_UDC3_OUT;
	p -> udc3_out_ports.p.nports = (sizeof(p -> udc3_out_ports)
	                             - sizeof(p -> udc3_out_ports.p))
	                            / sizeof(p -> udc3_out_ports.p.ports[0])
	                            + 1;
	p -> udc3_trigwd_ports.p.port_set = IOS_PORTS_TYPE_UDC3_WD;
	p -> udc3_trigwd_ports.p.nports = (sizeof(p -> udc3_trigwd_ports)
	                             - sizeof(p -> udc3_trigwd_ports.p))
	                            / sizeof(p -> udc3_trigwd_ports.p.ports[0])
	                            + 1;
	p -> udc3_wdsts_ports.p.port_set = IOS_PORTS_TYPE_WDSTS;
	p -> udc3_wdsts_ports.p.nports = (sizeof(p -> udc3_wdsts_ports)
	                             - sizeof(p -> udc3_wdsts_ports.p))
	                            / sizeof(p -> udc3_wdsts_ports.p.ports[0])
	                            + 1;
	p -> cxdac_ports.p.port_set = IOS_PORTS_TYPE_CXDAC_OUT;
	p -> cxdac_ports.p.nports = (sizeof(p -> cxdac_ports)
	                             - sizeof(p -> cxdac_ports.p))
	                            / sizeof(p -> cxdac_ports.p.ports[0])
	                            + 1;
#ifndef IOS_PORTS_TYPE_IOSLK2_CNOK
#define IOS_PORTS_TYPE_IOSLK2_CNOK 8
#endif 
	p -> cnok_ports.p.port_set = IOS_PORTS_TYPE_IOSLK2_CNOK;
	p -> cnok_ports.p.nports = (sizeof(p -> cnok_ports)
	                             - sizeof(p -> cnok_ports.p))
	                            / sizeof(p -> cnok_ports.p.ports[0])
	                            + 1;

/* Il fallimento di ioctl e` dovuto a versione del kernel non aggiornata.
  L'assenza del dispositivo si individua, su kernel aggiornati, dal valore
 di "nports", che e` zero. */

	if (ioctl(p -> fd, IOSGETPORTS, (void *)(&p -> cxdac_ports.p)) < 0
	 || p -> cxdac_ports.p.nports == 0) {

		p -> cxdac_present = 0;
	}
	else {
		p -> cxdac_present = 1;
	}
	if (ioctl(p -> fd, IOSGETPORTS, (void *)(&p -> cnok_ports.p)) < 0
	 || p -> cnok_ports.p.nports == 0) {

		p -> cnok_present = 0;
	}
	else {
		p -> cnok_present = 1;
	}

/* Il fallimento di ioctl e` dovuto a versione del kernel non aggiornata.
  L'assenza del dispositivo si individua, su kernel aggiornati, dal valore
 di "nports", che e` zero sia per l'input che per l'output. */

	if (ioctl(p -> fd, IOSGETPORTS, (void *)(&p -> udc3_in_ports.p)) < 0
	 || ioctl(p -> fd, IOSGETPORTS, (void *)(&p -> udc3_out_ports.p)) < 0
	 || (p -> udc3_in_ports.p.nports == 0
	     && p -> udc3_out_ports.p.nports == 0)) {

		p -> udc3_present = 0;
	}
	else {

/* Calcolo del numero di ingressi e uscite presenti sulla scheda. */

		for (i = 0; i < p -> udc3_in_ports.p.nports; ++i)
			p->udc3_num_in += p->udc3_in_ports.p.ports[i].nbits;
		for (i = 0; i < p -> udc3_out_ports.p.nports; ++i)
			p->udc3_num_out += p->udc3_out_ports.p.ports[i].nbits;

/* E adesso si devono calcolare i metodi di check e di trigger del
 watchdog della scheda UDC3. */ 

		ioctl(p -> fd, IOSGETPORTS,
		      (void *)(&p -> udc3_trigwd_ports.p));
		ioctl(p -> fd, IOSGETPORTS,
		      (void *)(&p -> udc3_wdsts_ports.p));

/* Sappiamo che la porta di trigger del watchdog diretto e` sempre
 inaccessibile direttamente. Il test sarebbe inutile, oppure e`
 orrendamente inadeguato. */

		if (p -> udc3_trigwd_ports.p.ports[0].nbits == 0)
			need_fd = 1;

/* Ora bisognerebbe controllare che tutte le porte siamo di tipo "diretto".
  Le porte "non dirette" devono per forza essere pilotate tramite il
 servizio ioctl indicato in "ioaddr", utilizzando come maschera il valore
 che si trova in "pos". Se ci sono porte di questo tipo il file descriptor
 e` sempre necessario, e si deve ricorrere per forza al metodo di accesso
 lento. Ma al momento sappiamo che non ce n'e` bisogno... */

		/* ... */

		use_fd |= need_fd;

	}
#endif /* Linux */

/* Assegna i metodi nulli se i dispositivi corrispondenti non esistono. */
	if (!(p -> led_present))
		p -> write_leds = (void (*)(dr_ios_t *, int)) util_null;
	if (!(p -> cxdac_present))
		p -> write_cxdac = (void (*)(dr_ios_t *, int)) util_null;
	if (!(p -> cnok_present))
		p -> write_cnok = (void (*)(dr_ios_t *, int)) util_null;
	if (!(p -> udc3_present)) {
		p -> read_udc3 = (int (*)(dr_ios_t *)) util_null;
		p -> write_udc3 = (void (*)(dr_ios_t *, int)) util_null;
		p -> check_udc3 = (int (*)(dr_ios_t *)) util_null;
		p -> trig_udc3 = (void (*)(dr_ios_t *)) util_null;
	}

/* Chiude il file descriptor se non serve a nessuno. */
	if (! use_fd)
		close(p -> fd);

	p -> bit_in_list = (ios_byte_t *) 0;
	p -> bit_out_list = (ios_byte_t *) 0;
	p -> byte_in_list = (ios_direct_byte_t *) 0;
	p -> byte_out_list = (ios_direct_byte_t *) 0;
	p -> w24bit_out_list = (ios_w24bit_t *) 0;
	p -> w12bit_out_list = (ios_w12bit_t *) 0;
	p -> adc_in_list = (ios_w12bit_t *) 0;
	p -> ain_list = (ios_ain_t *) 0;
	p -> aout_list = (ios_aout_t *) 0;
	p -> enc_list = (ios_enc_t *) 0;
	p -> lae_out_list = (ios_lae_t *) 0;
	p -> xdac_out_list = (ios_xdac_t *) 0;
	p -> sdis_out_list = (ios_sdis_t *) 0;
#if !AZZERA_NON_USATI_IOS
	p -> led_out_mask = 0;
	p -> udc3_out_mask = 0;
#endif

	d -> device = (void *) p;

	if (! p -> canbus) {
		for (i = j = 0; i < MAX_IOS_CPU; ++i) {
			if (p -> cpu[i]) {
			/* Azzera il flag di riaggancio e scrive la firma di
			 reset sul controllore. */
				p -> cpu[i][IOS_HANDSHAKE] = 0;
				iosRestart(p -> cpu[i]);
			/* Puntatori alla base delle CPU. */
				p -> acpu[j] = p -> cpu[i];
			/* Puntatori ai flag di "CPU OK". */
				p -> wdin[j] = &p -> cpu[i][IOS_CPUOK];
			/* Puntatori agli ingressi di trigger. */
				p -> wdout[j] = &p -> cpu[i][IOS_WDTRIG];
			/* Puntatori ai flag di "modulo disconnesso". */
				p -> disc[j] = &p -> cpu[i][IOS_DISCONN];
			/* Contatori di timout per il flag di "CPU OK". */
				p -> curr_count[j] = 0;
			/* Registra l'indice esterno nella tabella inversa. */
				p -> rev[j] = i;
				++j;
			}
		}

	/* Attende che i controllori si siano inizializzati. */

		util_usleep(1L,0L);

		p -> n_cpu = j;
	}

/* Azzera gli output. */

	_ios_clear_outs(p);

	return 1;
}

static void ios_detach(driver_t * d)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;

/* Rimouve tutti gli assegnamenti di variabile. */
	
	util_add_chain((void *)(dev -> bit_in_list),
	           (void **)&ios_byte_free,
		   GET_STRUCT_OFFSET(ios_byte_t,next));
	util_add_chain((void *)(dev -> bit_out_list),
	           (void **)&ios_byte_free,
		   GET_STRUCT_OFFSET(ios_byte_t,next));
	util_add_chain((void *)(dev -> byte_in_list),
	           (void **)&ios_direct_byte_free,
		   GET_STRUCT_OFFSET(ios_direct_byte_t,next));
	util_add_chain((void *)(dev -> byte_out_list),
	           (void **)&ios_direct_byte_free,
		   GET_STRUCT_OFFSET(ios_direct_byte_t,next));
	util_add_chain((void *)(dev -> w24bit_out_list),
	           (void **)&ios_w24bit_free,
		   GET_STRUCT_OFFSET(ios_w24bit_t,next));
	util_add_chain((void *)(dev -> w12bit_out_list),
	           (void **)&ios_w12bit_free,
		   GET_STRUCT_OFFSET(ios_w12bit_t,next));
	util_add_chain((void *)(dev -> adc_in_list),
	           (void **)&ios_w12bit_free,
		   GET_STRUCT_OFFSET(ios_w12bit_t,next));
	util_add_chain((void *)(dev -> ain_list),
	           (void **)&ios_ain_free,
		   GET_STRUCT_OFFSET(ios_ain_t,next));
	util_add_chain((void *)(dev -> aout_list),
	           (void **)&ios_aout_free,
		   GET_STRUCT_OFFSET(ios_aout_t,next));
	util_add_chain((void *)(dev -> enc_list),
	           (void **)&ios_enc_free,
		   GET_STRUCT_OFFSET(ios_enc_t,next));
	util_add_chain((void *)(dev -> lae_out_list),
	           (void **)&ios_lae_free,
		   GET_STRUCT_OFFSET(ios_lae_t,next));
	util_add_chain((void *)(dev -> xdac_out_list),
	           (void **)&ios_xdac_free,
		   GET_STRUCT_OFFSET(ios_xdac_t,next));
	util_add_chain((void *)(dev -> sdis_out_list),
	           (void **)&ios_sdis_free,
		   GET_STRUCT_OFFSET(ios_sdis_t,next));
	dev -> bit_in_list = dev -> bit_out_list = (ios_byte_t *) 0;
	dev -> byte_in_list = dev -> byte_out_list = (ios_direct_byte_t *) 0;
	dev -> w24bit_out_list = (ios_w24bit_t *) 0;
	dev -> w12bit_out_list = (ios_w12bit_t *) 0;
	dev -> adc_in_list = (ios_w12bit_t *) 0;
	dev -> ain_list = (ios_ain_t *) 0;
	dev -> aout_list = (ios_aout_t *) 0;
	dev -> enc_list = (ios_enc_t *) 0;
	dev -> lae_out_list = (ios_lae_t *) 0;
	dev -> xdac_out_list = (ios_xdac_t *) 0;
	dev -> sdis_out_list = (ios_sdis_t *) 0;

	dev -> cxdac_var = (unsigned char *) 0;
	dev -> cnok_var = (unsigned char *) 0;
	dev -> n_leds = 0;
	dev -> n_udc3_in = 0;
	dev -> n_udc3_out = 0;

#if !AZZERA_NON_USATI_IOS
	dev -> led_out_mask = 0;
	dev -> udc3_out_mask = 0;
#endif

}

static void ios_restart(driver_t * d)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;
int i;
long limit;

/* Rimouve tutti gli assegnamenti di variabile. */

	ios_detach(d);

	if (dev -> canbus) {
	/* TODO */
	}
	else {
	/* Forza il riaggancio dei moduli. */

		for (i = 0; i < dev -> n_cpu; ++i)
			dev -> acpu[i][IOS_HANDSHAKE] = 0;

	/* Attende che il riaggancio sia stato "recepito". */

		util_usleep(0L,500000L);

	/* Azzera i flag di modulo disconnesso e riprogramma i periodi di
	 retrigger. */

		limit = plcPeriod() * dev -> wdtime;
		if (limit > 255)
			limit = 255;

		for (i = dev -> n_cpu; i--; ) {
			*(dev -> disc[i]) = 0;
			dev -> acpu[i][IOS_HANDSHAKE] = 1;
			dev -> acpu[i][IOS_WDPRESET] = (unsigned char)limit;
		}
	}

/* Azzera gli output. */

	_ios_clear_outs(dev);
}

static int _ios_decode_status(int sts, int flags[8],int *spec)
{
int nio,i;

	if (sts & (IOS_STS_M_DIS | IOS_STS_M_WD | IOS_STS_M_ERR)) {
		return -1;
	}
	else if (sts & IOS_STS_M_SPEC) {
		for (i = 8; --i >= 4; )
			flags[i] = DRIVER_MODE_INPUT;
		do {
			flags[i] = DRIVER_MODE_OUTPUT;
		} while (--i >= 0);
	/* Il byte 0 deve essere anche letto,
	 per poter gestire l'handshake del comando. */
		flags[0] |= DRIVER_MODE_INPUT;
		*spec = 1;
	}
	else {
		for (i = 8; --i >= 0; )
			flags[i] = 0;
		nio = (sts & IOS_STS_M_IN)>>2;
		if (nio & 2) {
			flags[1] = DRIVER_MODE_INPUT;
			flags[2] = DRIVER_MODE_INPUT;
		}
		if (nio & 1)
			flags[0] = DRIVER_MODE_INPUT;
		nio = (sts & IOS_STS_M_OUT);
		if (nio & 2) {
			flags[1] = DRIVER_MODE_OUTPUT;
			flags[2] = DRIVER_MODE_OUTPUT;
		}
		if (nio & 1)
			flags[0] = DRIVER_MODE_OUTPUT;
		*spec = 0;
	}
	return 0;
}

/*
* FUNZIONI ASSOCIATE AI METODI "parse" ed "attach".
*/

/*
* I/O UDC ed MPP
*/

static int _ios_attach_udc(dr_ios_t *dev, devnode_t *l, void *var)
{
int i,m,mi,p2;
int *p_n;
ios_bit_t *p_l;

/* Costruisce i parametri da immagazzinare nella
 lista di associazione. */

	mi = 1;
	p2 = l -> c_udcbit;
	if (p2 >= LEVEL_3_UDCOUT) {
		p2 -= LEVEL_3_UDCOUT;
		mi = 0;
	}
	m = (1 << p2);

/* Cerca un posto nella lista delle uscite UDC. */

	if (mi) {
		p_n = &(dev -> n_udc3_in);
		p_l = dev -> udc3_in_list;
	}
	else {
		p_n = &(dev -> n_udc3_out);
		p_l = dev -> udc3_out_list;
	}

	for (i = 0; i < *p_n; ++i, ++p_l) {
		if (m == p_l -> m) {
		/* ERRORE. Uscita gia` assegnata. */
			return 0;
		}
	}

	if (i >= 16) {
	/* ERRORE. Tutto occupato. Peccato. */
		return 0;
	}

#if !AZZERA_NON_USATI_IOS
	if (! mi)
		dev -> udc3_out_mask |= m;
#endif

	p_l -> m = m;
	p_l -> var = var;
	*p_n = ++i;

	return 1;
}

static int _ios_parse_udc(dr_ios_t * dev, devnode_t *l)
{
int udc;
char *name;

	name = l -> pname;

	if (util_strneq(name,"udc",3)) {
		l -> c_bus = LEVEL_2_UDC;
	}
	else if (util_strneq(name,"mpp",3)) {
		l -> c_bus = LEVEL_2_MPP;
	}
	else {
	/* No, non interessano i segnali UDC. */
		return 0;
	}
	if (name[3] != '\0' && name[3] != '.') {
	/* No, non interessano i segnali UDC. */
		return 0;
	}

	name += 3;

	if (!dev -> udc3_present) {
	/* ERRORE. Non esiste il dispositivo. */
		return -1;
	}

	if (name[0] == '\0') {
	/* Fine stringa ?
	  Allora ci siamo fermati al livello "tipo controllore",
	 al quale si trova il nodo esplorabile "ios.n.UDC". */
		l -> ideep = 2;
		l -> flags |= DRIVER_MODE_LIST;
		l -> pname = name;
		return 0;
	}
/* Salta il "." */
	++name;

	udc = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 3;

	if (udc < 0
	 || udc >= ((l -> flags & DRIVER_MODE_OUTPUT) ? dev -> udc3_num_out
	                                        : dev -> udc3_num_in)) {
	/* ERRORE. Il segnale non esiste o e` mal descritto. */
		return -1;
	}

	if (l -> flags & DRIVER_MODE_OUTPUT)
		udc += LEVEL_3_UDCOUT;
	l -> c_udcbit = udc;
	l -> pname = name;

	return (l -> nbit = 1);
}

/*
* LED della scheda CX.
*/

static int _ios_attach_led(dr_ios_t *dev, devnode_t *l, void *var)
{
int i,m;

/* Costruisce i parametri da immagazzinare nella
 lista di associazione. */

	m = (1 << l -> c_ledbit);

/* Cerca un posto nella lista dei led. */

	for (i = 0; i < dev -> n_leds; ++i) {
		if (m == dev -> led_out_list[i].m) {
		/* ERRORE. Uscita gia` assegnata. */
			return 0;
		}
	}

	if (i >= 8) {
	/* ERRORE. Tutto occupato. Peccato. */
		return 0;
	}

#if !AZZERA_NON_USATI_IOS
	dev -> led_out_mask |= m;
#endif

	dev -> led_out_list[i].m = m;
	dev -> led_out_list[i].var = var;
	dev -> n_leds = ++i;

	return 1;
}

static int _ios_parse_led(dr_ios_t * dev, devnode_t *l)
{
int led;
char *name;

	name = l -> pname;

	if (! util_strneq(name,"led",3)
	 || (name[3] != '\0' && name[3] != '.')) {
	/* No, non interessano i LED. */
		return 0;
	}

	l -> c_bus = LEVEL_2_LED;

	name += 3;

	if (!dev -> led_present) {
	/* ERRORE. Non esiste il dispositivo. */
		return -1;
	}

	if (name[0] == '\0') {
		l -> ideep = 2;
		l -> flags |= DRIVER_MODE_LIST;
		l -> pname = name;
		return 0;
	}
/* Salta il "." */
	++name;

	led = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 3;

	if (led < 0 || led >= 8) {
	/* ERRORE. Manca l'indicazione del modulo, oppure la stringa
	 non e` conforme. */
		return -1;
	}
	if (l -> flags & DRIVER_MODE_INPUT) {
	/* ERRORE. La porta LED e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;
	l -> c_ledbit = led;

	return (l -> nbit = 1);
}

/*
* DAC della scheda CX.
*/

static int _ios_attach_cxdac(dr_ios_t *dev, devnode_t *l, void *var)
{

/* Controlla che l'uscita DAC sia gia` assegnata. */

	if (dev -> cxdac_var) {
	/* ERRORE. Tutto occupato. Peccato. */
		return 0;
	}

	dev -> cxdac_var = var;

/* Inizializza il campo immagine ad un valore tale da indurre
 l'immediato aggiornamento dell'uscita. */

	dev -> cxdac_image = -1;

	return 1;
}

static int _ios_parse_cxdac(dr_ios_t * dev, devnode_t *l)
{
char *name;

	name = l -> pname;

	if (! util_strneq(name,"dac",3)
	 || (name[3] != '\0' && name[3] != '.')) {
	/* No, non interessa il DAC. */
		return 0;
	}

	l -> c_bus = LEVEL_2_CXDAC;

	name += 3;

	if (!dev -> cxdac_present) {
	/* ERRORE. Non esiste il dispositivo. */
		return -1;
	}

	l -> ideep = 2;

	if (l -> flags & DRIVER_MODE_INPUT) {
	/* ERRORE. La porta DAC e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;

	return (l -> nbit = 8);
}

/*
* CNOK della scheda IOSLK2.
*/

static int _ios_attach_cnok(dr_ios_t *dev, devnode_t *l, void *var)
{

/* Controlla che l'uscita CNOK sia gia` assegnata. */

	if (dev -> cnok_var) {
	/* ERRORE. Tutto occupato. Peccato. */
		return 0;
	}

	dev -> cnok_var = var;

/* Inizializza il campo immagine ad un valore tale da indurre
 l'immediato aggiornamento dell'uscita. */

	dev -> cnok_image = -1;

	return 1;
}

static int _ios_parse_cnok(dr_ios_t * dev, devnode_t *l)
{
char *name;

	name = l -> pname;

	if (! util_strneq(name,"cnok",4)
	 || (name[4] != '\0' && name[4] != '.')) {
	/* No, non interessa il CNOK. */
		return 0;
	}

	l -> c_bus = LEVEL_2_CNOK;

	name += 4;

	if (!dev -> cnok_present) {
	/* ERRORE. Non esiste il dispositivo. */
		return -1;
	}

	l -> ideep = 2;

	if (l -> flags & DRIVER_MODE_INPUT) {
	/* ERRORE. La porta CNOK e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;

	return (l -> nbit = 1);
}


/*
* Funzioni per parse ed attach di moduli seriali.
*/

static unsigned char * _ios_make_target(dr_ios_t * dev, devnode_t *l)
{
unsigned char *target;

	if (dev -> canbus) {
		if (l -> c_byte >= LEVEL_4_AIN
		 && l -> c_byte < LEVEL_4_AOUT) {
			target = (unsigned char *)
				canboard_get_ain_image(dev -> canbus,
			                                  l -> c_mod);
			target += (l -> c_byte - LEVEL_4_AIN)
			        * (sizeof(unsigned short));
		}
		else if (l -> c_byte >= LEVEL_4_AOUT
		 && l -> c_byte < LEVEL_4_ENC) {
			target = (unsigned char *)
				canboard_get_aout_image(dev -> canbus,
			                                  l -> c_mod);
			target += (l -> c_byte - LEVEL_4_AOUT)
			        * (sizeof(unsigned short));
		}
		else if (l -> c_byte >= LEVEL_4_ENC
		 && l -> c_byte < LEVEL_4_QUIT) {
			target = (unsigned char *)
				canboard_get_enc_image(dev -> canbus,
			                                  l -> c_mod);
			target += (l -> c_byte - LEVEL_4_ENC)
			        * (sizeof(long));
		}
		else if (l -> c_mode & DRIVER_MODE_INPUT) {
			target = canboard_get_input_image(dev -> canbus,
			                                  l -> c_mod);
			target += l -> c_byte;
		}
		else {
			target = canboard_get_output_image(dev -> canbus,
			                                   l -> c_mod);
			target += l -> c_byte;
		}
		return target;
	}

	if (l -> c_bus >= MAX_IOS_CPU)
		return (unsigned char *)0;
	target = dev -> cpu[l -> c_bus];
	if (! target)
		return (unsigned char *)0;

	if (l -> c_mod < MAX_IOS_MOD) {

		target += l -> c_mod * 8;

		if (l -> c_byte < LEVEL_4_DAC) {
			target += l -> c_byte;
		}
		else if (l -> c_byte >= LEVEL_4_SELCO_IN
		      && l -> c_byte < LEVEL_4_SELCO_OUT) {
			target += (l -> c_byte - LEVEL_4_SELCO_IN)/8 + 5;
		}
		else if (l -> c_byte >= LEVEL_4_ADC
		      && l -> c_byte < LEVEL_4_XDAC) {
			switch (l -> c_byte) {
			case LEVEL_4_ADC + 0:
			case LEVEL_4_ADCN+ 0: target += 0x004; break;
			case LEVEL_4_ADC + 1:
			case LEVEL_4_ADCN+ 1: target += 0x006; break;
			case LEVEL_4_ADC + 2:
			case LEVEL_4_ADCN+ 2: target += 0x302; break;
			case LEVEL_4_ADC + 3:
			case LEVEL_4_ADCN+ 3: target += 0x304; break;
			}
		}
		else if (l -> c_byte >= LEVEL_4_XDAC
		      && l -> c_byte < LEVEL_4_SDIS) {
		}
		else if (l -> c_byte >= LEVEL_4_SDIS) {
		}
	}
	else {
		target += l -> c_byte + l -> c_byte2 * 0x100;
	}
	return target;
}

static int _ios_attach_bit(dr_ios_t *dev, devnode_t *l, void *var)
{
int i;
unsigned long m;
ios_byte_t *p;
unsigned char *target;
ios_byte_t **list;

/* Calcola l'indirizzo del byte richiesto e la maschera di accesso. */

	target = _ios_make_target(dev,l);

	if (l -> c_byte >= LEVEL_4_SELCO_IN)
		m = 1 << ((l -> c_byte - LEVEL_4_SELCO_IN) % 8);
	else
		m = 1 << l -> c_bit;

	if (l -> flags & DRIVER_MODE_INPUT)
		list = &(dev -> bit_in_list);
	else
		list = &(dev -> bit_out_list);

	for (p = *list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (ios_byte_t *) util_alloc_cell(
		                          (void **) &ios_byte_free,
		                          sizeof(ios_byte_t),
		                          GET_STRUCT_OFFSET(ios_byte_t,next),
		                          LEN_IOS_ALLOC_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Assegna il campo "indirizzo" e gli indici di appartenenza. */

		p -> addr = target;
		p -> mod = l -> c_mod;
		p -> idx = l -> c_byte;

	/* Azzera il campo "maschera" di tutti gli elementi. */

#if !AZZERA_NON_USATI_IOS
		p -> m = 0;
#endif
		for (i = 0; i < sizeof(p -> bit)/sizeof(p -> bit[0]); ++i)
			p -> bit[i].m = 0;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) list,
		           GET_STRUCT_OFFSET(ios_byte_t,next));
	}

/* Controlla la cella e le aggiunge il bit. */

	for (i = 0; p -> bit[i].m; ++i) {
		if (p -> bit[i].m == m) {
		/* ERRORE. Bit gia` assegnato. */
			return 0;
		}
	}

#if !AZZERA_NON_USATI_IOS
	p -> m |= m;
#endif
	p -> bit[i].m = m;
	p -> bit[i].var = (unsigned char *) var;

	return 1;
}

static int _ios_attach_lae(dr_ios_t *dev, devnode_t *l, void *var)
{
int i,idx;
ios_lae_t *p;
unsigned char *target;
void **q;

/* Calcola l'indirizzo del byte richiesto e la maschera di accesso. */

	target = _ios_make_target(dev,l);

	idx = l -> c_byte - LEVEL_4_SELCO_OUT;

	for (p = dev -> lae_out_list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (ios_lae_t *) util_alloc_cell(
		                       (void **) &ios_lae_free,
		                       sizeof(ios_lae_t),
		                       GET_STRUCT_OFFSET(ios_lae_t,next),
		                       LEN_IOS_ALLOC_LAE_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Inizializza i campi. */

		p -> nused = 0;
		for (i = 0; i < 8; ++i) {
			p -> dl_pol_img[i] = 0xFF;
			p -> pulse_img[i] = 0xFF;
			p -> delay[i].var = (unsigned short *) 0;
			p -> pulse[i].var = (unsigned short *) 0;
			p -> trpol[i].var = (unsigned char *) 0;
			p -> oupol[i].var = (unsigned char *) 0;
			p -> usedidx[i] = 0;
		}

	/* Assegna il campo "indirizzo". */

		p -> addr = target;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) &(dev -> lae_out_list),
		           GET_STRUCT_OFFSET(ios_lae_t,next));
	}

/* Cerca il bit nella tabella dei segnali utilizzati. */

	for (i = 0; i < p -> nused; ++i) {
		if (p -> usedidx[i] == idx) {
		/* Trovato. */
			break;
		}
	}

/* Aggiunge l'elemento, se necessario. */
	if (i == p -> nused) {
		p -> usedidx[i] = idx;
		++(p -> nused);
	}

/* Assegna l'indirizzo della variabile sorgente al campo opportuno. */

	switch (l -> c_laeconf) {
	case LEVEL_5_LAE_PT: q = (void **)&(p -> pulse[idx].var); break;
	case LEVEL_5_LAE_DT: q = (void **)&(p -> delay[idx].var); break;
	case LEVEL_5_LAE_TL: q = (void **)&(p -> trpol[idx].var); break;
	case LEVEL_5_LAE_OL: q = (void **)&(p -> oupol[idx].var); break;
	default:
	/* ERRORE: Non so come sia successo, ma non doveva succedere. */
		return 0;
	}

	if (*q) {
	/* ERRORE: Segnale gia` assegnato. */
		return 0;
	}

	*q = var;

	return 1;
}

static int _ios_attach_w24bit(dr_ios_t *dev, devnode_t *l, void *var)
{
int i;
unsigned long m;
ios_w24bit_t *p;
unsigned char *target;

/* Calcola l'indirizzo del byte richiesto e la maschera di accesso. */

	target = _ios_make_target(dev,l);

	m = 1 << (l -> c_byte - LEVEL_4_SELCO_OUT + 8);

	for (p = dev -> w24bit_out_list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (ios_w24bit_t *) util_alloc_cell(
		                          (void **) &ios_w24bit_free,
		                          sizeof(ios_w24bit_t),
		                          GET_STRUCT_OFFSET(ios_w24bit_t,next),
		                          LEN_IOS_ALLOC_W24BIT_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Inizializza il campo immagine. */

		p -> image = 0xFF; /* I bit significativi sono 8-23, 0-7
		                     sono sempre nulli...salvo all'inizio */

	/* Assegna il campo "indirizzo". */

		p -> addr = target;

	/* Azzera il campo "maschera" di tutti gli elementi. */

		for (i = 0; i < sizeof(p -> bit)/sizeof(p -> bit[0]); ++i)
			p -> bit[i].m = 0;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) &(dev -> w24bit_out_list),
		           GET_STRUCT_OFFSET(ios_w24bit_t,next));
	}

/* Controlla la cella e le aggiunge il bit. */

	for (i = 0; p -> bit[i].m; ++i) {
		if (p -> bit[i].m == m) {
		/* ERRORE. Bit gia` assegnato. */
			return 0;
		}
	}

	p -> bit[i].m = m;
	p -> bit[i].var = (unsigned char *) var;

	return 1;
}

static int _ios_attach_byte(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_direct_byte_t *p;
unsigned char *target;
ios_direct_byte_t **list;

	if (l -> flags & DRIVER_MODE_INPUT)
		list = &(dev -> byte_in_list);
	else
		list = &(dev -> byte_out_list);

	target = _ios_make_target(dev,l);
	for (p = *list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (p) {
	/* ERRORE. Byte gia` assegnato. */
		return 0;
	}

/* Alloca una cella. */
	p = (ios_direct_byte_t *) util_alloc_cell(
		                          (void **) &ios_direct_byte_free,
		                          sizeof(ios_direct_byte_t),
		                          GET_STRUCT_OFFSET(ios_direct_byte_t,
		                                            next),
		                          LEN_IOS_ALLOC_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) list,
		   GET_STRUCT_OFFSET(ios_direct_byte_t,next));

	p -> var = (unsigned char *) var;

	return 1;
}

/* Uscite di configurazione dei moduli LAE. */
static int _ios_parse_lae(dr_ios_t * dev, devnode_t *l)
{
char *name;
int nb,cnf,no;

	name = l -> pname;

	no = 5; /* Lunghezza del nome. E` quasi sempre 5 (TRUCCACCIO!). */
	nb = 16; /* Numero di bit del segnale. Ora 16, ora 1... */
	if (util_streq(name,"delay")) {
		cnf = LEVEL_5_LAE_DT;
	}
	else if (util_streq(name,"pulse")) {
		cnf = LEVEL_5_LAE_PT;
	}
	else if (util_streq(name,"trigl")) {
		cnf = LEVEL_5_LAE_TL;
		nb = 1;
	}
	else if (util_streq(name,"outl")) {
		no = 4;
		cnf = LEVEL_5_LAE_OL;
		nb = 1;
	}
	else {
	/* No, non interessano i segnali di controllo LAE. */
		return 0;
	}


	l -> ideep = 5;
	l -> flags = DRIVER_MODE_OUTPUT;
	l -> pname = name + no;
	l -> c_laeconf = cnf;

	return (l -> nbit = nb);
}

static int _ios_parse_bit(dr_ios_t * dev, devnode_t *l)
{
int byt,bit,spec,inmode,mode;
unsigned char sts;
char *name;
int fb[16];
int max_bit;

	name = l -> pname;

/* Cerca l'indice di byte. */
	byt = ioGetNum(&name);
	if (byt < 0) {
	/* No, grazie, non interessa. */
		return 0;
	}
	if (dev -> canbus) {
 		if (byt > MAX_CAN_IOBYTE) {
			return 0;
		}
	}
	else {
 		if (byt > 7) {
			return 0;
		}
	}

/* Controlla che il byte corrisponda ad un modulo funzionante, e che
 la modalita` di accesso richiesta sia compatibile con la natura del
 modulo stesso.
 NOTA: A questo livello, le uscite di configurazione dei moduli LAE
 sono viste come uscite ordinarie a bit. Poiche` la
 modalita` di accesso e` la stessa (uscita, appunto), questo codice
 funziona anche in questo caso. Comunque e` un TRUCCACCIO. */

	mode = l -> flags;
	inmode = mode & DRIVER_MODE_INPUT;

	sts = l -> c_sts;
	if (dev -> canbus) {
	int i,n_in_byte,n_out_byte;

		n_in_byte = l -> c_sts;
		n_out_byte = l -> c_sts2;
		for (i = 0; i < 16; ++i) {
			fb[i] = 0;
			if (i < n_in_byte) {
		 		fb[i] |= DRIVER_MODE_INPUT;
			}
			if (i < n_out_byte) {
		 		fb[i] |= DRIVER_MODE_OUTPUT;
			}
		}
		spec = 0;
		max_bit = 127;
	}
	else {
		if (_ios_decode_status(sts,fb,&spec) < 0)
			return -1;
		max_bit = 47;
	}

/* Gestione del caso "byte di un modulo". */
	if (name[0] == '\0') {
		if (!(fb[byt] & mode))
			return -1;
		l -> ideep = 4;
		l -> flags = mode | DRIVER_MODE_LIST;
		l -> pname = name;
		l -> c_byte = byt;
		return (l -> nbit = 8);
	}

/* Cerca l'indice di bit. */
	bit = ioGetNum(&name);
	/* Accettiamo anche bit fuori range per rendere piu` trattabili
	 * i moduli CAN tramite EDIT_PLC. */
	if (bit >= dev -> bitoffset && dev -> bitoffset != 0) {
		bit -= dev -> bitoffset;
	}
	if (bit < 0 || bit > max_bit) {
	/* ERRORE. L'indicazione del bit non e` conforme. */
		return -1;
	}

/* Caso "tastiera Selco" o "modulo speciale LAE". */
	if (spec) {

	/* Da 0 a 23 sono ingressi, da 24 a 47 uscite. */
		if (byt != 0 || ((inmode != 0) == (bit >= 24)))
			return -1;

		l -> c_byte = LEVEL_4_SELCO_IN + bit;

		l -> pname = name;

	/* Questi bit sono espandibili, in quanto possono avere
	 componenti di configurazione nel caso LAE. */

		if (bit >= 24 && bit < 24+8) {

			l -> flags |= DRIVER_MODE_LIST; 

		/* Se c'e` qualcosa dopo l'indice di bit, si tratta
		 probabilmente di una delle uscite di configurazione LAE.
		  La parola passa dunque all'apposita procedura. */

			if (name[0])
				return  _ios_parse_lae(dev,l);
		}

	/* Negli altri casi non sono ammesse uteriori indformazioni. */
		if (name[0]) {
		/* ERRORE: schifezze in fondo al nome. */
			return -1;
		}

		l -> ideep = 4;
		return (l -> nbit = 1);
	}

	if (name[0]) {
	/* ERRORE: schifezze in fondo al nome. */
		return -1;
	}

/* Calcola un eventuale spostamento in avanti del byte. */

	byt += bit / 8;
	bit %= 8;

	if (dev -> canbus) {
		if (byt > MAX_CAN_IOBYTE) {
			return -1;
		}
	}
	else {
		if (byt > 7) {
		/* ERRORE. Non esiste niente del genere in CNi. */
			return -1;
		}
	}
	if (!(fb[byt] & mode)) {
		return -1;
	}

	l -> c_byte = byt;
	l -> c_bit = bit;
	l -> c_mode = mode & ~DRIVER_MODE_LIST;
	l -> ideep = 5;
	l -> pname = name;

/* Fine. */

	return (l -> nbit = 1);
}

static int _ios_attach_dac(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_w12bit_t *p;
unsigned char *target;

	target = _ios_make_target(dev,l);

	for (p = *(ios_w12bit_t **)&(dev->w12bit_out_list); p; p = p -> next) {
		if (p -> addr == target) {
		/* ERRORE. DAC gia` assegnato. */
			return 0;
		}
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	p = (ios_w12bit_t *) util_alloc_cell(
	                          (void **) &ios_w12bit_free,
	                          sizeof(ios_w12bit_t),
	                          GET_STRUCT_OFFSET(ios_w12bit_t,next),
	                          LEN_IOS_ALLOC_W12BIT_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Inizializza il campo immagine. */

	p -> image = 0xFF; /* I bit significativi sono 8-23, 0-7
			     sono sempre nulli...salvo all'inizio */

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) &(dev -> w12bit_out_list),
		   GET_STRUCT_OFFSET(ios_w12bit_t,next));

/* Aggiunge la variabile. */

	p -> var = (short *) var;

	return 1;
}

static int _ios_parse_dac(dr_ios_t * dev, devnode_t *l)
{
char *name;

	name = l -> pname;

	if (! util_streq(name,"dac")) {
	/* No, evidentemente non interessa l'uscita DAC. */
		return 0;
	}

	name += 3;

	if (!(l -> flags & DRIVER_MODE_OUTPUT)) {
	/* ERRORE. Il DAC e` in dispositivo di OUTPUT ! */
		return -1;
	}

	l -> c_byte = LEVEL_4_DAC;
	l -> pname = name;
	l -> ideep = 4;

/* Fine. */

	return (l -> nbit = 16);
}

static int _ios_attach_adc(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_w12bit_t *p;
unsigned char *target;

	target = _ios_make_target(dev,l);

	for (p = *(ios_w12bit_t **)&(dev->adc_in_list); p; p = p -> next) {
		if (p -> addr == target) {
		/* ERRORE. ADC gia` assegnato. */
			return 0;
		}
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	p = (ios_w12bit_t *) util_alloc_cell(
	                          (void **) &ios_w12bit_free,
	                          sizeof(ios_w12bit_t),
	                          GET_STRUCT_OFFSET(ios_w12bit_t,next),
	                          LEN_IOS_ALLOC_W12BIT_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Nel campo immagine si registra la modalita` di accesso (vecchio
 stile: 0, normalizzata: 1). */

	p -> image = (l -> c_byte >= LEVEL_4_ADCN);

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) &(dev -> adc_in_list),
		   GET_STRUCT_OFFSET(ios_w12bit_t,next));

/* Aggiunge la variabile. */

	p -> var = (short *) var;

	return 1;
}

static int _ios_parse_adc(dr_ios_t * dev, devnode_t *l)
{
char *name;
int adc;
int base;

	name = l -> pname;

	base = LEVEL_4_ADC;

	if (! util_strneq(name,"adc.",4)) {
		if (! util_strneq(name,"adcn.",5)) {
		/* No, non interessano gli ADC. */
			return 0;
		}
		else {
			name += 5;
			base = LEVEL_4_ADCN;
		}
	}
	else {
		name += 4;
	}

	adc = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 4;

	if (adc < 0 || adc >= 4) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (l -> flags & DRIVER_MODE_OUTPUT) {
	/* ERRORE. La porta ADC e` di sola lettura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = base + adc;

	return (l -> nbit = 16);
}

static int _ios_attach_xdac(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_xdac_t *p;
unsigned char *target;
int idx,i;
void **q;
int new_mode;

	target = _ios_make_target(dev,l);

	idx = l -> c_byte - LEVEL_4_XDAC;

	for (p = dev -> xdac_out_list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (ios_xdac_t *) util_alloc_cell(
		                       (void **) &ios_xdac_free,
		                       sizeof(ios_xdac_t),
		                       GET_STRUCT_OFFSET(ios_xdac_t,next),
		                       LEN_IOS_ALLOC_XDAC_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Inizializza i campi. */

		for (i = 0; i < 2; ++i) {
			p -> vars[i].mode = _IOS_XDAC_MODE_NONE;
			p -> vars[i].val = (unsigned short *) 0;
			p -> vars[i].dir = (unsigned char *) 0;
		}

/* Inizializza l'immagine ad un valore "impossibile", cosi` al
 primo giro viene eseguita sicuramente una scrittura. */
		p -> image = _IOS_XDAC_MODE_TRASH;

	/* Assegna il campo "indirizzo". */

		p -> addr = target;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) &(dev -> xdac_out_list),
		           GET_STRUCT_OFFSET(ios_xdac_t,next));
	}

/* Assegna l'indirizzo della variabile sorgente al campo opportuno. */
/* (e controlla anche qualche vincolo). */

	if (l -> c_xdacconf == LEVEL_5_XDAC_DR)
		q = (void **)&(p -> vars[idx].dir);
	else
		q = (void **)&(p -> vars[idx].val);

	new_mode = _IOS_XDAC_MODE_NONE;
	switch (l -> c_xdacconf) {
	case LEVEL_5_XDAC_BP: new_mode = _IOS_XDAC_MODE_BIPOLAR; break;
	case LEVEL_5_XDAC_UP: new_mode = _IOS_XDAC_MODE_UNIPOLAR; break;
	case LEVEL_5_XDAC_SG: new_mode = _IOS_XDAC_MODE_SIGNED; break;
	case LEVEL_5_XDAC_DR: new_mode = _IOS_XDAC_MODE_SIGNED; break;
	default:
	/* ERRORE: Non so come sia successo, ma non doveva succedere. */
		return 0;
	}

/* Errore se 
 - il canale era gia` assegnato 
 E
  - la variabile era gia` assegnata
  Oppure
  - il modo di funzionamento non e` compatibile con quello corrente. */
 
	if ((p -> vars[idx].mode != _IOS_XDAC_MODE_NONE)
	 && ((*q) || (p -> vars[idx].mode != new_mode))) {
	/* ERRORE: Segnale gia` assegnato o non utilizzabile. */
		return 0;
	}

	p -> vars[idx].mode = new_mode;
	*q = var;

	return 1;
}

static int _ios_parse_xdac(dr_ios_t * dev, devnode_t *l)
{
char *name;
int xdac;
int nb,no,cnf;

	name = l -> pname;

	if (! util_strneq(name,"sda.",4)) {
	/* No, non interessano gli XDAC. */
		return 0;
	}

	name += 4;

	xdac = ioGetNum(&name);

	if (xdac < 0 || xdac >= 2) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (l -> flags & DRIVER_MODE_INPUT) {
	/* ERRORE. La porta XDAC e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = LEVEL_4_XDAC + xdac;
	if (name[0] == '\0') {
		l -> ideep = 4;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}

	nb = 16;
	if (util_streq(name,"bp")) {
		cnf = LEVEL_5_XDAC_BP;
		no = 2;
	}
	else if (util_streq(name,"un")) {
		cnf = LEVEL_5_XDAC_UP;
		no = 2;
	}
	else if (util_streq(name,"us")) {
		cnf = LEVEL_5_XDAC_SG;
		no = 2;
	}
	else if (util_streq(name,"out")) {
		cnf = LEVEL_5_XDAC_DR;
		no = 3;
		nb = 1;
	}
	else {
		return -1;
	}

	l -> c_xdacmode = DRIVER_MODE_OUTPUT;
	l -> pname = name + no;
	l -> c_xdacconf = cnf;
	l -> ideep = 5;

	return (l -> nbit = nb);
}

static int _ios_attach_sdis(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_sdis_t *p;
unsigned char *target;
int idx,i;
void **q;

	target = _ios_make_target(dev,l);

	idx = l -> c_byte - LEVEL_4_SDIS;

	for (p = dev -> sdis_out_list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (ios_sdis_t *) util_alloc_cell(
		                       (void **) &ios_sdis_free,
		                       sizeof(ios_sdis_t),
		                       GET_STRUCT_OFFSET(ios_sdis_t,next),
		                       LEN_IOS_ALLOC_SDIS_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Inizializza i campi. */

		p -> dots = 0xFFFFFFFF;
		p -> n_disp = 0;
		p -> disp_map = 0;
		for (i = 0; i < 16; ++i) {
/* Inizializza le immagini ad un valore "impossibile", cosi` al
 primo giro viene eseguita sicuramente una scrittura. */
			p -> vars[i].last_val = 0x7FFFFFFF;
			p -> vars[i].val = (long *) NULL;
			p -> vars[i].dec = (unsigned char *) NULL;
		}

	/* Assegna il campo "indirizzo". */

		p -> addr = target;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) &(dev -> sdis_out_list),
		           GET_STRUCT_OFFSET(ios_sdis_t,next));
	}

/* Assegna l'indirizzo della variabile sorgente al campo opportuno. */

	if (l -> ideep == 5)
		q = (void **)&(p -> vars[idx].dec);
	else
		q = (void **)&(p -> vars[idx].val);

	if (*q) {
	/* ERRORE: Segnale gia` assegnato o non utilizzabile. */
		return 0;
	}

	*q = var;

/* Aggiornamento della tabella degli indici. */
	if (!(p -> disp_map & (1 << idx))) {
		p -> disp_map |= (1 << idx);
		p -> dtab[p -> n_disp] = idx;
		++(p -> n_disp);
	}

	return 1;
}

static int _ios_parse_sdis(dr_ios_t * dev, devnode_t *l)
{
char *name;
int sdis;
int nb;

	name = l -> pname;

	if (! util_strneq(name,"sdis.",5)) {
	/* No, non interessano gli SDIS. */
		return 0;
	}

	name += 5;

	sdis = ioGetNum(&name);

	if (sdis < 0 || sdis >= 16) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (l -> flags & DRIVER_MODE_INPUT) {
	/* ERRORE. La porta SDIS e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = LEVEL_4_SDIS + sdis;
	if (name[0] == '\0') {
		l -> ideep = 4;
		l -> flags |= DRIVER_MODE_LIST;
		return (l -> nbit = 32);
	}

	if (! util_streq(name,"dec")) {
		return -1;
	}
	l -> pname = name + 3;
/* E` previsto solo questo codice a questo livello. Quindi si
 potrebbe fare a meno di definirlo... */
	l -> c_sdisconf = LEVEL_5_SDIS_DEC;
	l -> c_sdismode = DRIVER_MODE_OUTPUT;
	l -> ideep = 5;

	return (l -> nbit = 1);
}

static int _ios_attach_ain(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_w12bit_t *p;
unsigned char *target;

	target = _ios_make_target(dev,l);

	for (p = *(ios_ain_t **)&(dev->ain_list); p; p = p -> next) {
		if (p -> addr == target) {
		/* ERRORE. Gia` assegnato. */
			return 0;
		}
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	p = (ios_ain_t *) util_alloc_cell(
	                          (void **) &ios_ain_free,
	                          sizeof(ios_ain_t),
	                          GET_STRUCT_OFFSET(ios_ain_t,next),
	                          LEN_IOS_ALLOC_AIN_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Nel campo immagine si registra la modalita` di accesso (vecchio
 stile: 0, normalizzata: 1). */

	p -> image = 1;

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) &(dev -> ain_list),
		   GET_STRUCT_OFFSET(ios_ain_t,next));

/* Aggiunge la variabile. */

	p -> var = (short *) var;

	return 1;
}

static int _ios_parse_ain(dr_ios_t * dev, devnode_t *l)
{
char *name;
int ain;
int base;

	name = l -> pname;

	base = LEVEL_4_AIN;

	if (! util_strneq(name,"ain.",4)) {
	/* No, non interessano gli ingressi analogici. */
		return 0;
	}
	else {
		name += 4;
	}

	ain = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 4;

	if (ain < 0 || ain >= dev -> canmod[l->c_mod].ain) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (l -> flags & DRIVER_MODE_OUTPUT) {
	/* ERRORE. La porta e` di sola lettura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = base + ain;

	return (l -> nbit = 16);
}

static int _ios_attach_aout(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_aout_t *p;
unsigned char *target;
int idx,i;
void **q;
int new_mode;

	target = _ios_make_target(dev,l);

	idx = l -> c_byte - LEVEL_4_AOUT;

	for (p = dev -> aout_list; p; p = p -> next) {
		if (p -> addr == target) {
	/* ERRORE: Segnale gia` assegnato. */
			return 0;
		}
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	p = (ios_aout_t *) util_alloc_cell(
			       (void **) &ios_aout_free,
			       sizeof(ios_aout_t),
			       GET_STRUCT_OFFSET(ios_aout_t,next),
			       LEN_IOS_ALLOC_AOUT_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Inizializza l'immagine ad un valore "impossibile", cosi` al
primo giro viene eseguita sicuramente una scrittura. */
	p -> image = _IOS_XDAC_MODE_TRASH;

	p -> mod = l -> c_mod;
	p -> idx = idx;

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) &(dev -> aout_list),
		   GET_STRUCT_OFFSET(ios_aout_t,next));

/* Aggiunge la variabile. */

	p -> var = (short *) var;

	return 1;
}

static int _ios_parse_aout(dr_ios_t * dev, devnode_t *l)
{
char *name;
int aout;
int base;

	name = l -> pname;

	base = LEVEL_4_AOUT;

	if (! util_strneq(name,"aout.",5)) {
	/* No, non interessano gli ingressi analogici. */
		return 0;
	}
	else {
		name += 5;
	}

	aout = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 4;

	if (aout < 0 || aout >= dev -> canmod[l->c_mod].aout) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (!(l -> flags & DRIVER_MODE_OUTPUT)) {
	/* ERRORE. La porta e` di sola scrittura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = base + aout;

	return (l -> nbit = 16);
}

static int _ios_attach_enc(dr_ios_t *dev, devnode_t *l, void *var)
{
ios_enc_t *p;
unsigned char *target;

	target = _ios_make_target(dev,l);

	for (p = *(ios_enc_t **)&(dev->enc_list); p; p = p -> next) {
		if (p -> addr == target) {
		/* ERRORE. Gia` assegnato. */
			return 0;
		}
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	p = (ios_enc_t *) util_alloc_cell(
	                          (void **) &ios_enc_free,
	                          sizeof(ios_enc_t),
	                          GET_STRUCT_OFFSET(ios_enc_t,next),
	                          LEN_IOS_ALLOC_ENC_BLK);
	if (! p) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"2");
		return 0;
	}

/* Assegna il campo "indirizzo". */

	p -> addr = target;

/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) p,
		   (void **) &(dev -> enc_list),
		   GET_STRUCT_OFFSET(ios_enc_t,next));

/* Aggiunge la variabile. */

	p -> var = (long *) var;

	return 1;
}

static int _ios_parse_enc(dr_ios_t * dev, devnode_t *l)
{
char *name;
int enc;
int base;

	name = l -> pname;

	base = LEVEL_4_ENC;

	if (! util_strneq(name,"enc.",4)) {
	/* No, non interessano gli ingressi analogici. */
		return 0;
	}
	else {
		name += 4;
	}

	enc = ioGetNum(&name);
	if (name[0]) {
	/* ERRORE. Descrizione non conforme. */
		return -1;
	}
	l -> ideep = 4;

	if (enc < 0 || enc >= dev -> canmod[l->c_mod].enc) {
	/* ERRORE. Indice non conforme. */
		return -1;
	}
	if (!(l -> flags & DRIVER_MODE_INPUT)) {
	/* ERRORE. La porta e` di sola lettura. */
		return -1;
	}

	l -> pname = name;
	l -> c_byte = base + enc;

	return (l -> nbit = 32);
}

static int _ios_parse_mod(dr_ios_t * dev, devnode_t *l)
{
int mod;
char *name;
int sts;
int size = 0;

	name = l -> pname;

	mod = ioGetNum(&name);
	if (dev -> canbus) {
		if (mod < 1 || mod >= MAX_CAN_MOD) {
		/* No, non interessano i moduli IOS. */
			return 0;
		}
	}
	else {
		if (mod < 0 || mod >= MAX_IOS_MOD) {
		/* No, non interessano i moduli IOS. */
			return 0;
		}
	}

	l -> c_mod = mod;

	if (dev -> canbus) {
	int n_in,n_out,n_ain,n_aout,n_enc;
	
		n_in = canboard_get_n_input(dev -> canbus, mod);
		n_out = canboard_get_n_output(dev -> canbus, mod);
		n_ain = canboard_get_n_ain(dev -> canbus, mod);
		n_aout = canboard_get_n_aout(dev -> canbus, mod);
		n_enc = canboard_get_n_enc(dev -> canbus, mod);
		if (n_in == -1 || n_out == -1
		 || n_ain == -1 || n_aout == -1
		 || n_enc == -1) {
		/* ERRORE. Non esiste il dispositivo. */
			return -1;
		}
	/* Memorizziamo cosi` le caratteristiche del dispositivo. */
	/* (Ma che senso ha? abbiamo gia` memorizzato tutto nel descrittore,
	 che possiamo recuperare in l -> c_mod!). */
		if (n_in || n_out || n_ain || n_aout || n_enc) {
			dev -> canmod[mod].present = 1;
			dev -> canmod[mod].in = n_in;
			dev -> canmod[mod].inb = (n_in + 7) / 8;
			dev -> canmod[mod].out = n_out;
			dev -> canmod[mod].outb = (n_out + 7) / 8;
			dev -> canmod[mod].ain = n_ain;
			dev -> canmod[mod].aout = n_aout;
			dev -> canmod[mod].enc = n_enc;
		}
		sts = ((n_in + 7) / 8) + 256 * ((n_out + 7) / 8);
	}
	else {
		sts = dev -> cpu[l -> c_bus][IOS_IOSTATUS + mod];
		if ((sts & (IOS_STS_M_DIS | IOS_STS_M_WD | IOS_STS_M_ERR))
		    != 0) {
		/* ERRORE. Non esiste il dispositivo. */
			return -1;
		}
	}

	l -> c_sts = sts % 256;
	l -> c_sts2 = sts / 256;

	l -> pname = name;

	if (name[0] == '\0') {
		l -> ideep = 3;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}
	if (dev -> canbus
	 || !(sts & IOS_STS_M_SPEC)
	 || !(   (size = _ios_parse_dac(dev,l))
	      || (size = _ios_parse_adc(dev,l))
	      || (size = _ios_parse_xdac(dev,l))
	      || (size = _ios_parse_sdis(dev,l))) ) {
		if (dev -> canbus) {
			size = _ios_parse_ain(dev,l);
			if (size == 0) {
				size = _ios_parse_aout(dev,l);
			}
			if (size == 0) {
				size = _ios_parse_enc(dev,l);
			}
		}
		if (size == 0) {
			size = _ios_parse_bit(dev,l);
		}
	}
	return size;
}

static int _ios_parse_rawbyte(dr_ios_t * dev, devnode_t *l)
{
int by;
char *name;

	name = l -> pname;

	if (! util_strneq(name,"byte",4)
	 || (name[4] != '\0' && name[4] != '.')) {
	/* No, non interessano i BYTE. */
		return 0;
	}

	name += 4;

	if (!dev -> direct_byte_access) {
	/* ERRORE. Non esiste il dispositivo. */
		return -1;
	}

	l -> c_mod = LEVEL_3_BYTE;

	if (name[0] == '\0') {
		l -> ideep = 3;
		l -> flags |= DRIVER_MODE_LIST;
		l -> pname = name;
		return 0;
	}
/* Salta il "." */
	++name;
	by = ioGetNum(&name);
	if (by < 0 || by >= 0x400 || name[0]) {
	/* ERRORE. Parametro fuori dai limiti o errore di sintassi. */
		return -1;
	}

	l -> ideep = 4;
	l -> c_byte = by & 0xFF;
	l -> c_byte2 = (by >> 8) & 0xFF;
	l -> pname = name;

	return (l -> nbit = 8);
}

static int _ios_parse_ios(dr_ios_t * dev, devnode_t *l)
{
int cpu;
char *name;
int size;

	name = l -> pname;

	cpu = ioGetNum(&name);
	if (cpu < 0 || cpu >= MAX_IOS_CPU) {
	/* No, non interessano i moduli IOS. */
		return 0;
	}

	l -> c_bus = cpu;

	if (dev -> canbus) {
		if (cpu != 0) {
		/* ERRORE. Non esiste il dispositivo. */
			return -1;
		}
	}
	else {
		if (! dev -> cpu[cpu]) {
		/* ERRORE. Non esiste il dispositivo. */
			return -1;
		}
	}

	l -> pname = name;

	if (name[0] == '\0') {
		l -> ideep = 2;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}

	size = _ios_parse_mod(dev,l);
	if (! size)
		size = _ios_parse_rawbyte(dev,l);
	return size;
}

static int ios_parse(driver_t * d, int i_off, devnode_t *l)
{
int size;
dr_ios_t *dev = (dr_ios_t *) d -> device;

/* Il driver IOS e` istanziabile, percio` ha poco senso interpretare
 in modo ambiguo il campo "istanza". */

	if (i_off != 0) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

	size = _ios_parse_udc(dev,l);
	if (size)
		goto ios_parse_END;
	size = _ios_parse_led(dev,l);
	if (size)
		goto ios_parse_END;
	size = _ios_parse_cxdac(dev,l);
	if (size)
		goto ios_parse_END;
	size = _ios_parse_cnok(dev,l);
	if (size)
		goto ios_parse_END;
	size = _ios_parse_ios(dev,l);
ios_parse_END:
	return size < 0 ? 0 : size;
}

static int ios_attach(driver_t * d, devnode_t * l, void * var)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;

	switch (l -> c_bus) {
	case LEVEL_2_LED:
		return _ios_attach_led(dev,l,var);
	case LEVEL_2_CXDAC:
		return _ios_attach_cxdac(dev,l,var);
	case LEVEL_2_CNOK:
		return _ios_attach_cnok(dev,l,var);
	case LEVEL_2_UDC:
	case LEVEL_2_MPP:
		return _ios_attach_udc(dev,l,var);
	default:
		if (l -> c_mod == LEVEL_3_BYTE)
			return _ios_attach_byte(dev, l, var);
		if (l -> c_byte < LEVEL_4_DAC) {
			if (l -> ideep == 5)
				return _ios_attach_bit(dev, l, var);
			return _ios_attach_byte(dev, l, var);
		}
		if (l -> c_byte == LEVEL_4_DAC)
			return _ios_attach_dac(dev, l, var);
		if (l -> c_byte < LEVEL_4_SELCO_OUT)
			return _ios_attach_bit(dev, l, var);
		if (l -> c_byte >= LEVEL_4_ENC)
			return _ios_attach_enc(dev, l, var);
		if (l -> c_byte >= LEVEL_4_AOUT)
			return _ios_attach_aout(dev, l, var);
		if (l -> c_byte >= LEVEL_4_AIN)
			return _ios_attach_ain(dev, l, var);
		if (l -> c_byte >= LEVEL_4_SDIS)
			return _ios_attach_sdis(dev, l, var);
		if (l -> c_byte >= LEVEL_4_XDAC)
			return _ios_attach_xdac(dev, l, var);
		if (l -> c_byte >= LEVEL_4_ADC)
			return _ios_attach_adc(dev, l, var);
		if (l -> ideep == 5)
			return _ios_attach_lae(dev, l, var);
		return _ios_attach_w24bit(dev, l, var);
	}
	return 0;
}

static int ios_check(driver_t * d)
{
register unsigned char **p = ((dr_ios_t *) d -> device) -> wdin;
register unsigned char **q = ((dr_ios_t *) d -> device) -> disc;
register int i,n;
int rv;

	rv = 1;

	if (! ((dr_ios_t *) d -> device) -> do_chk)
		return rv;

	n = ((dr_ios_t *) d -> device) -> n_cpu;
	for (i = 0; i < n; ++p, ++q, ++i) {
		if (((dr_ios_t *) d -> device) -> do_chk_wd) {
			if (**p) {
				if (++(((dr_ios_t *) d -> device)
				                    -> curr_count[i])
				  >= ((dr_ios_t *) d -> device) -> max_count) {
			/* ERRORE. Timeout nella IOS. */
					rv = 0;
					ioAddError(d,0,
					   ((dr_ios_t *) d -> device)
					                 -> rev[i]);
					continue;
				}
			}
			else {
				((dr_ios_t *) d -> device)->curr_count[i] = 0;
			}
			**p = 0x55;
		}
		if ((((dr_ios_t *) d -> device) -> do_chk_dis)
		  && (**q == 0x55)) {
		/* ERRORE. Modulo sganciato. */
			**q = 0;
			rv = 0;
#if 1
{
int sts,k,f;
unsigned char *s;
			s = & (((dr_ios_t *) d -> device)
			        -> acpu[i][IOS_IOSTATUS]);

			f = 0;

			for (k = 0; k < MAX_IOS_MOD; ++k) {
				sts = s[k];
				if (sts & IOS_STS_M_DIS)
					continue;

				if (sts & (IOS_STS_M_WD | IOS_STS_M_ERR)) {
					f = 1;
					ioAddError(d,1,
						(((dr_ios_t *) d -> device)
						  -> rev[i]) | (k << 16));
				}
			}
		/* Che fare se non c'e` alcun modulo in fault?
		 Si fa vedere un bel 9999? */
			if (! f)
				ioAddError(d,1,
					(((dr_ios_t *) d -> device)
					  -> rev[i]) | (9999 << 16));
}
#else
			ioAddError(d,1, ((dr_ios_t *) d -> device) -> rev[i]);
#endif
		}
	}

	if (!_ios_chk_udc3((dr_ios_t *) d -> device)) {
		rv = 0;
		ioAddError(d,1,0);
	}

	return rv;
}

static void ios_trigger(driver_t * d)
{
register unsigned char **p = ((dr_ios_t *) d -> device) -> wdout;
register int i;

	if (((dr_ios_t *) d -> device) -> do_trig) {
		for (i = ((dr_ios_t *) d -> device) -> n_cpu + 1; --i; ++p)
			**p = 0x55;
		(((dr_ios_t *)d->device) -> trig_udc3)((dr_ios_t *)d->device);
	}
}

static void ios_up(driver_t * d)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;
int i;

	if (dev -> canbus) {
		for (i = 1; i < 128; ++i) {
			if (dev -> canmod[i].present) {
				canboard_activate_module(dev -> canbus, i);
			}
		}
	}
}

static void ios_down(driver_t * d)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;
int n;

	if (plcHaltCycle()) {

	/* Esce se la funzione e` gia` stata chiamata e la ripetizione
	 non e` stata richiesta. */

		if (! dev -> repeat_me)
			return;
	}
	else {

	/* Azzera il flag di ripetizione. */

		dev -> repeat_me = 0;

	/* Tutti i moduli CANOpen sono riportati nello stato
	 PREOPERATIONAL. */
		if (dev -> canbus) {

		int i;

			for (i = 1; i < 128; ++i) {
				if (dev -> canmod[i].present) {
					canboard_preop_module(dev -> canbus,i);
				}
			}
		}

	/* Marca "non allineate" tutte le uscite speciali. */
{
ios_w24bit_t *w;
		for (w = dev -> w24bit_out_list; w; w = w -> next) {
			w -> image = 0xFF;
		}
}
{
ios_xdac_t *w;
		for (w = dev -> xdac_out_list; w; w = w -> next) {
			w -> image &= 0x0000000FF;
		}
}
#if 0
/* Ha davvero senso azzerare dei semplici visualizzatori? */
{
ios_sdis_t *w;
int i;
		for (w = dev -> sdis_out_list; w; w = w -> next) {
			for (i = 0; i < 16; ++i) {
				w -> vars[i].last_val = 0x7FFFFFFFF;
				w -> vars[i].last_dec = 0xFF;
			}
		}
}
#endif

	/* Spegne le uscite ordinarie solo la prima volta. */

		_ios_clear_outs(dev);
	}

/* Spegne le uscite speciali, e registra se ci sono moduli non pronti. */

	n = 0;
{
ios_w24bit_t *w;

	for (w = dev -> w24bit_out_list; w; w = w -> next) {

	/* Invia il comando di scrittura solo ai dispositivi
	 la cui immagine non sia stata ancora azzerata. */

		if (w -> image) {

		/* Attende che il modulo sia pronto. Se no, si ritentera` al
		 prossimo ciclo. */

			if (*(unsigned char *)(w -> addr) == 0) {

			/* Invia il comando di scrittura sulle uscite
			 ed aggiorna l'immagine. */

				IOS_W24BIT_OUT(w,0);
			}
			else {

			/* Incrementa il contatore di moduli non pronti. */

				++n;
			}
		}
	}
}
{
ios_xdac_t *w;

	for (w = dev -> xdac_out_list; w; w = w -> next) {

	/* Invia il comando di scrittura solo ai dispositivi
	 la cui immagine non sia stata ancora azzerata. */

		if (w -> image) {

		/* Attende che il modulo sia pronto. Se no, si ritentera` al
		 prossimo ciclo. */

			if (*(unsigned char *)(w -> addr) == 0) {

			/* Invia il comando di scrittura sulle uscite
			 ed azzera l'immagine. */

				IOS_W32BIT_OUT(w,w -> image);
				w -> image = 0;
			}
			else {

			/* Incrementa il contatore di moduli non pronti. */

				++n;
			}
		}
	}
}
#if 0
/* Ha davvero senso azzerare dei semplici visualizzatori? */
{
ios_sdis_t *w;
int i;
	for (w = dev -> sdis_out_list; w; w = w -> next) {
		for (i = 0; i < 16; ++i) {
		}
	}
}
#endif


/* Se qualche modulo non e` pronto, richiede la ripetizione del ciclo. */

	if (n) {
		plcRepeatHalt();
		dev -> repeat_me = 1;
	}
}

static void ios_fatal(driver_t * d)
{
dr_ios_t *dev = (dr_ios_t *) d -> device;
int i;


	if (dev -> canbus) {
	/* RESET per tutti. */
		canboard_reset_module(dev -> canbus, 0);
	}
	else {
		for (i = 0; i < dev -> n_cpu; ++i) {

	/* Azzera tutto l'azzerabile (e anche molto del non azzerabile). */

			util_memset(dev -> acpu[i], 0, MAX_IOS_MOD * 8);

	/* Programma un periodo di retrigger brevissimo. */

			dev -> acpu[i][IOS_WDPRESET] = 1;
		}
	}

	_ios_clear_udc3(dev);
	_ios_clear_leds(dev);
	_ios_clear_cxdac(dev);
	_ios_clear_cnok(dev);
}

static int _ios_read_adc(volatile unsigned char * p, int norm)
{
int v;
int i;

	v = ((p[0] & 0x07) << 8) | p[1];
	for (i = 0; (v != ((p[0] & 0x07) << 8) | p[1]) && (i < 5); ++i)
		v = ((p[0] & 0x07) << 8) | p[1];
	if (v & 0x0400)
		v |= 0xFC00;
	if (norm)
		v <<= 5;
	return v;
}

#if 0
#define TEST_SDO
#endif

#ifdef TEST_SDO
// TEST SDO CANOPEN
///////////////////
static int sdo_ok = 1;
static long sdo_data;
static void sdo_cb(sdo_t *sdo,
				int err,
            	                unsigned int len,
                                unsigned int abort,
                                void *cb_arg)
{
	printf("SDO terminated: sdo=0x%08lx err=%d len=%u abort=0x%x data=%ld\n",
	       sdo, err, len, abort, sdo_data);
	sdo_ok = 1;
	sdo_data = 0;
}
#endif

static void ios_read(driver_t * d)
{
{
dr_ios_t *dr = (dr_ios_t *)(d -> device);
int rv;

	if (dr -> canbus) {
		canboard_schedule(dr -> canbus, plcPeriod());
#ifdef TEST_SDO
// TEST SDO CANOPEN
///////////////////
		if (sdo_ok) {
			sdo_ok = 0;
			rv = canboard_async_sdo(dr->canbus, 1,
			                  0x6864, 0,
                                          &sdo_data, sizeof(sdo_data),
                                          0, sdo_cb, NULL);
			if (rv < 0) {
				printf("SDO start failed: rv = %d\n",rv);
			}
		}
#endif
	}
}
	{
	register ios_bit_t *q;
	register unsigned char v;
	register unsigned char m;
	ios_byte_t *p = ((dr_ios_t *) d -> device) -> bit_in_list;

		while (p) {
			v = *(p -> addr);
			q = p -> bit;
			while ((m = q -> m)) {
				*(q++ -> var) = ((v & m) != 0);
			}
			p = p -> next;
		}
	}

	{
	register ios_direct_byte_t *p
	 = ((dr_ios_t *) d -> device) -> byte_in_list;

		while (p) {
			*(p -> var) = *(p -> addr);
			p = p -> next;
		}
	}

	{
	register ios_w12bit_t *p
	 = ((dr_ios_t *) d -> device) -> adc_in_list;

		while (p) {
			*(p -> var) = _ios_read_adc((unsigned char *)
			                             (p -> addr),p -> image);
			p = p -> next;
		}
	}

	{
	register ios_ain_t *p
	 = ((dr_ios_t *) d -> device) -> ain_list;

		while (p) {
			*(p -> var) = (*(short *)(p -> addr) << 5);
			p = p -> next;
		}
	}

	{
	register ios_enc_t *p
	 = ((dr_ios_t *) d -> device) -> enc_list;

		while (p) {
			*(p -> var) = (*(long *)(p -> addr));
			p = p -> next;
		}
	}

	{
	register int i = ((dr_ios_t *) d -> device) -> n_udc3_in;
	register ios_bit_t *p = ((dr_ios_t *) d -> device) -> udc3_in_list;
	register unsigned int v;

		if (i) {
			v = ((dr_ios_t *) d -> device)
			     -> read_udc3((dr_ios_t *) d -> device);
			do {
				*(p -> var) = (v & p -> m) != 0;
				++p;
			} while(--i);
		}
	}
}

static void ios_write(driver_t * d)
{
dr_ios_t * dr = ((dr_ios_t *) d -> device);

/* Aggiorna le uscite a bit ordinarie. */

{
ios_byte_t *p;
register ios_bit_t *q;
register unsigned long v;
register unsigned long m;

	for (p = dr -> bit_out_list; p; p = p -> next) {
#if AZZERA_NON_USATI_IOS
		v = 0;
#else
		v = *(p -> addr) & ~(p -> m);
#endif
		q = p -> bit;
		while ((m = q -> m)) {
			if (*(q++ -> var))
				v |= m;
		}
		if (dr -> canbus) {
			canboard_set_output_byte(dr -> canbus,
						p -> mod,
						p -> idx,
						v);
		}
		else {
			*(p -> addr) = (unsigned char) v;
		}
	}
}

/* Aggiorna le uscite speciali. */

/* Uscite di configurazione dei moduli LAE. */
{
register ios_lae_t *p;
int i;
register int idx;
long cmd;

/* Esplorazione della lista dei moduli LAE. */

	for (p = dr -> lae_out_list; p; p = p -> next) {

	/* Esplorazione del vettore degli indici di bit coinvolti
	 in qualcuna delle configurazioni. */

		for (i = 0; i < p -> nused; ++i) {

		/* Estrazione dell'indice del bit. */

			idx = p -> usedidx[i];

		/* Costruzione del comando di configurazione
		 delay+polarita` per il bit "idx". */

			cmd = (0x10 + idx); /* Codice del comando */
			if (p -> delay[idx].var)
				cmd |= (*(p -> delay[idx].var)) << 8;
			if (p -> trpol[idx].var && (*(p -> trpol[idx].var)))
				cmd |= 0x1000000;
			if (p -> oupol[idx].var && (*(p -> oupol[idx].var)))
				cmd |= 0x2000000;

		/* Se il comando cotruito e` uguale all'ultimo inviato,
		 non c'e` bisogno di fare niente. */

			if (cmd != p -> dl_pol_img[idx]) {

			/* Necessario per non farsi fregare. */
			register volatile unsigned char *a;

			/* ...altrimenti bisogna inviare il comando,
			 se possibile, e passare al modulo successivo. */

				a = p -> addr;
				if (a[0] == 0) {

			/* Con questo si tiene occupato il modulo. Eventuali
			 variazioni delle uscite ordinarie non avranno effetto
			 finche` non sono stati elaborati tutti i comandi
			 di configurazione pendenti. E` importante che
			 questo tratto di codice preceda quello in cui si
			 elaborano le uscite ordinarie SELCO/LAE. */

					a[1] = (unsigned char)(cmd >> 8);
					a[2] = (unsigned char)(cmd >> 16);
					a[3] = (unsigned char)(cmd >> 24);
					a[0] = (unsigned char)(cmd);

					p -> dl_pol_img[idx] = cmd;
				}
				else {

			/* Modulo occupato. Conviene continuare?
			  Forse no: Se e` occupato ora, quante probabilita`
			 ci sono che non lo sia tra una manciata di
			 microsecondi? */

				}

		/* USCITA DAL CICLO INTERNO */

				break;

			}

		/* Costruzione del comando di configurazione della
		 durata dell'impulso del monostabile associato al bit "idx".
		  La logica e` la stessa utilizzata poco sopra. */

			cmd = (0x18 + idx); /* Codice del comando */
			if (p -> pulse[idx].var)
				cmd |= (*(p -> pulse[idx].var)) << 8;

			if (cmd != p -> pulse_img[idx]) {

			register volatile unsigned char *a;

				a = p -> addr;
				if (a[0] == 0) {
					a[1] = (unsigned char)(cmd >> 8);
					a[2] = (unsigned char)(cmd >> 16);
					a[3] = (unsigned char)(cmd >> 24);
					a[0] = (unsigned char)(cmd);

					p -> pulse_img[idx] = cmd;
				}
				else {
				}

		/* USCITA DAL CICLO INTERNO */

				break;

			}
		}
	}
}

/* Moduli Selco. */
{
ios_w24bit_t *p;
register ios_bit_t *q;
register unsigned long v;
register unsigned long m;

	for (p = dr -> w24bit_out_list; p; p = p -> next) {
		v = 0;
		q = p -> bit;
		while ((m = q -> m)) {
			if (*(q++ -> var))
				v |= m;
		}

	/* Invia il comando di scrittura solo se ci sono cambiamenti
	 di stato. */

		if (v != p -> image) {

		/* Attende che il modulo sia pronto. Se no, si ritentera` al
		 prossimo ciclo. */

			if (*(unsigned char *)(p -> addr) == 0) {

			/* Invia il comando di scrittura sulle uscite
			 ed aggiorna l'immagine. */

				IOS_W24BIT_OUT(p,v);
			}
		}
	}
}
/* Moduli DAC. */
{
register ios_w12bit_t *p;
register unsigned long v;

	for (p = dr -> w12bit_out_list; p; p = p -> next) {

		v = (*(p -> var)) << 8;

	/* Invia il comando di scrittura solo se ci sono cambiamenti
	 di stato. */

		if (v != p -> image) {

		/* Attende che il modulo sia pronto. Se no, si ritentera` al
		 prossimo ciclo. */

			if (*(unsigned char *)(p -> addr) == 0) {

			/* Invia il comando di scrittura sulle uscite
			 ed aggiorna l'immagine. */

				IOS_W12BIT_OUT(p,v);
			}
		}
	}
}
/* Uscite analogiche CANOpen. */
{
register ios_aout_t *p;
register unsigned short v;

	for (p = dr -> aout_list; p; p = p -> next) {

		//if (v != p -> image)
		{
		v = (*(unsigned short *)(p -> var));
		canboard_set_output_word(dr -> canbus,
					p -> mod,
					p -> idx,
					v);
		}

	}
}
/* Moduli XDAC */
{
register ios_xdac_t *p;
int i,val,dir;
unsigned long v;

/* Esplorazione della lista dei moduli XDAC. */

	for (p = dr -> xdac_out_list; p; p = p -> next) {
		v = ((p -> vars[1].mode) | (p -> vars[0].mode << 4))
		  & ((_IOS_XDAC_MODE_MASK) | (_IOS_XDAC_MODE_MASK << 4));
		for (i = 0; i < 2; ++i) {
			val = dir = 0;
			if ((p -> vars[i].mode) != _IOS_XDAC_MODE_NONE) {
				if (p -> vars[i].val)
					val = *(p -> vars[i].val) & 0xFFF0;
				if (p -> vars[i].dir)
					dir = *(p -> vars[i].dir) != 0;
				if ((p -> vars[i].mode)
				 == _IOS_XDAC_MODE_SIGNED) {
					val = (val >> 1) | (dir << 15);
				}
			}
			if (i == 0)
				v |= (val << 8)   & 0x00FFF000;
			else
				v |= ((val >> 4)  & 0x00000F00)
				   | ((val << 20) & 0xFF000000);
		}
	/* Invia il comando di scrittura solo se ci sono cambiamenti
	 di stato. */

		if (v != p -> image) {

		/* Attende che il modulo sia pronto. Se no, si ritentera` al
		 prossimo ciclo. */

			if (*(unsigned char *)(p -> addr) == 0) {

			/* Invia il comando di scrittura sulle uscite
			 ed aggiorna l'immagine. */

				IOS_W32BIT_OUT(p,v);
			}
		}
	}

}

/* Moduli SDIS */
{
register ios_sdis_t *p;
int i,j,n,idx = 0,dec,m = 0;
long l;

/* Esplorazione della lista dei moduli SDIS. */

	for (p = dr -> sdis_out_list; p; p = p -> next) {
	/* Ad ogni ciclo si inizia da un display diverso, per
	 dare a tutti la possibilita` di essere rinfrescati, prima
	 o poi. In piu`, ogni tanto si da` la priorita` al
	 test sui punti decimali (codificato con i == p -> n_disp). */
		for (n = p -> n_disp + 1, i = plcNCycle() % n; n; --n) {
			if (i == p -> n_disp) {
			/* E` il momento di controllare i punti. */
				for (j = 0, m = 0; j < p -> n_disp; ++j) {
					idx = p -> dtab[j];
					if ((p -> vars[idx].dec)
					 && (*(p -> vars[idx].dec)))
						m |= (1 << idx);
				}
				i = 0;
			/* Uscita dal ciclo se la maschera corrente
			 e` diversa dall'ultima impostata. */
			/* Questo stato e` contrassegnato da "n!=0 && i==0" */
			/* Il dato utile e` in "m". */
				if (m != p -> dots)
					break;
			}
			else {
				idx = p -> dtab[i];
				++i;
				if ((p -> vars[idx].val)
				 && ((*(p -> vars[idx].val) & 0xFFFFFF)
				     != p -> vars[idx].last_val)) {
			/* Uscita dal ciclo se il dato "idx" corrente
			 non coincide con l'ultimo impostato. */
			/* Questo stato e` contrassegnato da "n!=0 && i!=0" */
			/* Il dato utile e` in "idx". */
					break;
				}
			}
		}
		if (n == 0) {
		/* Nulla di fatto: non ci sono evoluzioni. */
		}
		else if (i == 0) {
		/* Rilevato cambiamento di stato dei punti decimali. */
		/* Se il modulo e` occupato, non aggiorna lo stato. */
			if (*(p -> addr) == 0) {
			/* Bit = 1 significa "NON visualizzare i
			 decimali" per il modulo, qindi bisogna
			 invertire la maschera prima di applicarla. */
				p -> dots = m;
				m = ~m;
				p -> addr[1] = m & 0xFF;
				p -> addr[2] = (m >> 8) & 0xFF;
				p -> addr[3] = 0;
				p -> addr[0] = 0x7F;
			}
		}
		else {
		/* Rilevato cambiamento di stato nel valore "idx". */
		/* Se il modulo e` occupato, non aggiorna lo stato. */
			if (*(p -> addr) == 0) {
				p -> vars[idx].last_val
				 = l
				 = *(p -> vars[idx].val) & 0xFFFFFF;
				p -> addr[1] = l & 0xFF;
				p -> addr[2] = (l >> 8) & 0xFF;
				p -> addr[3] = (l >> 16) & 0xFF;
				p -> addr[0] = idx + 1;
			}
		}
	}
}

/* Aggiorna i LED. */

{
register ios_bit_t *q;
register unsigned long v;
register unsigned long m;

	if ( (m = dr -> n_leds) ) {
		q = dr -> led_out_list;
#if AZZERA_NON_USATI_IOS
		v = 0;
#else
		v = dr -> led_image & ~(dr -> led_out_mask);
#endif
		do {
			if (*(q -> var)) {
				v |= (q -> m);
			}
			++q;
		} while (--m);
		(*(dr -> write_leds))(dr,(int)v);
	}
}

/* Aggiorna il DAC CX. */

{
	if (dr -> cxdac_var) {
		(*(dr -> write_cxdac))(dr,*(dr -> cxdac_var));
	}
}

/* Aggiorna il CNOK della IOSLK2. */

{
	if (dr -> cnok_var) {
		(*(dr -> write_cnok))(dr,*(dr -> cnok_var));
	}
}

/* Aggiorna le uscite UDC3. */

{
register ios_bit_t *q;
register unsigned long v;
register unsigned long m;

	if ( (m = dr -> n_udc3_out) ) {
		q = dr -> udc3_out_list;
#if AZZERA_NON_USATI_IOS
		v = 0;
#else
		v = dr -> udc3_out_image & ~(dr -> udc3_out_mask);
#endif
		do {
			if (*(q -> var)) {
				v |= (q -> m);
			}
			++q;
		} while (--m);
		(*(dr -> write_udc3))(dr,(int)v);
	}
}

/* Aggiorna le uscite a byte. */

{
register ios_direct_byte_t *p = ((dr_ios_t *) d -> device) -> byte_out_list;

	while (p) {
		*(p -> addr) = *(p -> var);
		p = p -> next;
	}
}
/* Rinfresco fisico del fieldbus CAN, se presente. */
{
	if (dr -> canbus) {
		canboard_flush_output_ex(dr -> canbus,
		     (dr -> forcesync != 0)
		      || ((dr -> cansync != 0)
		           && ((dr -> ain_list != NULL)
		               || (dr -> enc_list != NULL))));
	}
}
}

/*
* Metodo "list". Complicato assai.
*/

/* Funzione di costruzione del nome (livello 1: DRIVER.ISTANZA). */
static void _ios_list_1_print(driver_t * d, bstring_t *b)
{
	util_sprintf_bstring(b,"%s.%d", d -> op.name, d -> instance);
}

/* Funzione di costruzione del nome
 (livello 2: DRIVER.ISTANZA.{n|LED|UDC|MPP|DAC|CNOK}). */
static void _ios_list_2_print(driver_t * d, bstring_t *b, int param)
{
static const char *str[] = { "LED","UDC","MPP","DAC","CNOK" };

	_ios_list_1_print(d, b);
	util_addch_bstring(b,'.');
	if (param < MAX_IOS_CPU)
		util_ltob_bstring(b,param,10);
	else
		util_cat_bstring(b,str[param-MAX_IOS_CPU]);
}

/* Funzione di esplorazione per il livello 2
 (DRIVER.ISTANZA.{n|LED|UDC|MPP|DAC}). */
static int _ios_list_2(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int param = -1;
dr_ios_t *dev = (dr_ios_t *) d -> device;

	while (!quit && !ok) {
		param = l -> c_bus;
		switch (param) {
#ifdef Linux
#else
		case LEVEL_2_LED:
			ok = dev -> led_present;
			++(l -> c_bus);
			break;
		case LEVEL_2_UDC:
			ok = dev -> udc3_present
			  && (dev -> model == IOS_MODEL_UDC3
			   || dev -> model == IOS_MODEL_UDC3SL);
			++(l -> c_bus);
			break;
		case LEVEL_2_MPP:
			ok = dev -> udc3_present
			  && (dev -> model == IOS_MODEL_MPP3
			   || dev -> model == IOS_MODEL_MPP3SL);
			++(l -> c_bus);
			break;
		case LEVEL_2_CXDAC:
			ok = dev -> cxdac_present;
			++(l -> c_bus);
			break;
		case LEVEL_2_CNOK:
			ok = dev -> cnok_present;
			++(l -> c_bus);
			break;
#endif
		default:
			if (l -> c_bus < MAX_IOS_CPU) {
			if (dev -> canbus) {
				if (l -> c_bus == 0) {
					ok = 1;
					util_safe_sprintf(l -> comment,
						  sizeof(l -> comment),
						  "CANOpen%d",
					          dev -> candevice);
				}
			}
			else {
			char *p = dev -> cpu[l -> c_bus];
				if (p) {
					ok = 1;
					p += IOS_VER;
					util_safe_sprintf(l -> comment,
						  sizeof(l -> comment),
						  "ver=%c%c%c%c",
					          p[0],p[1],p[2],p[3]);
				}
			}
			++(l -> c_bus);
			}
			else {
				quit = 1;
			}
			break;
		}
	}
	if (ok) {
		if (param == LEVEL_2_CXDAC) {
			l -> flags = DRIVER_MODE_OUTPUT;
			l -> nbit = 8;
		}
		else if (param == LEVEL_2_CNOK) {
			l -> flags = DRIVER_MODE_OUTPUT;
			l -> nbit = 1;
		}
		else {
			l -> flags = DRIVER_MODE_LIST;
		}
		l -> t_c_bus = param;
		l -> t_c_mod = 0;
		_ios_list_2_print(d, b, param);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 3:
 DRIVER.ISTANZA.CPU.MODULO o DRIVER.ISTANZA.{LED|UDC|MPP}.SEGNALE} */
static void _ios_list_3_print(driver_t * d, bstring_t *b, int p1, int p2)
{
	_ios_list_2_print(d, b, p1);
	util_addch_bstring(b,'.');
	if (p2 >= LEVEL_3_UDCOUT)
		p2 -= LEVEL_3_UDCOUT;
	util_ltob_bstring(b,p2,10);
}

/* Funzione di esplorazione per il livello 3
 (DRIVER.ISTANZA.CPU.MODULO o DRIVER.ISTANZA.{LED|UDC|MPP}.SEGNALE} */
static int _ios_list_3(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int p1 = -1;
int p2 = -1;
int sts = 0;
dr_ios_t *dev = (dr_ios_t *) d -> device;

	p1 = l -> c_bus;
	while (!quit && !ok) {
		p2 = l -> c_mod; /* Alias c_ledbit, alias c_udcbit */
		switch (p1) {
		case LEVEL_2_LED:
			if (p2 < 8) {
				l -> flags = DRIVER_MODE_OUTPUT;
				l -> nbit = 1;
				ok = 1;
				++(l -> c_ledbit);
			}
			else {
				quit = 1;
			}
			break;
		case LEVEL_2_UDC:
		case LEVEL_2_MPP:
			if (p2 < LEVEL_3_UDCOUT) {
				if (p2 < dev -> udc3_num_in) {
					l -> flags = DRIVER_MODE_INPUT;
					l -> nbit = 1;
					ok = 1;
					++(l -> c_udcbit);
				}
				else {
					l -> c_udcbit = LEVEL_3_UDCOUT;
				}
			}
			else if (p2 < LEVEL_3_UDCQUIT) {
				if (p2-LEVEL_3_UDCOUT < dev->udc3_num_out) {
					l -> flags = DRIVER_MODE_OUTPUT;
					l -> nbit = 1;
					ok = 1;
					++(l -> c_udcbit);
				}
				else {
					l -> c_udcbit = LEVEL_3_UDCQUIT;
				}
			}
			else {
				quit = 1;
			}
			break;
		default:
			if (p1 == 0
			 && dev -> canbus
			 && p2 < MAX_CAN_MOD) {
			int n_in,n_out;
			int n_ain,n_aout;
			int n_enc;
				n_in = dev -> canmod[l->c_mod].in;
				n_out = dev -> canmod[l->c_mod].out;
				n_ain = dev -> canmod[l->c_mod].ain;
				n_aout = dev -> canmod[l->c_mod].aout;
				n_enc = dev -> canmod[l->c_mod].enc; //N.U.
				if (n_in > 0 || n_out > 0) {
					util_safe_sprintf(l -> comment,
						sizeof(l -> comment),
						"DS401-%di%do%dai%dao",
					        n_in,n_out,
					        n_ain,n_aout);
					l -> flags = DRIVER_MODE_LIST;
					ok = 1;
				}
				++(l -> c_mod);
			}
			else if (p1 < MAX_IOS_CPU
			    && dev -> cpu[p1]
			    && p2 < MAX_IOS_MOD) {

			int fb[8];
			int spec;

				sts = dev -> cpu[p1][IOS_IOSTATUS + p2];
				if (_ios_decode_status(sts,fb,&spec) == 0) {

				int nn = 0,v = 0;
				char *f;

					if (spec) {
						f = "SPECIAL";
					}
					else {
						v = fb[0] ? fb[0] : fb[1];
						if (v == DRIVER_MODE_INPUT)
							f = "SI%d";
						else
							f = "SO%d";
						if (fb[0] == fb[2])
							nn = 24;
						else if (fb[0] && fb[2])
							nn = 816;
						else if (fb[0])
							nn = 8;
						else
							nn = 16;
					}
					util_safe_sprintf(l -> comment,
						sizeof(l -> comment),f,nn);
					l -> flags = DRIVER_MODE_LIST;
					ok = 1;
				}
				++(l -> c_mod);
			}
			else {
				quit = 1;
			}
			break;
		}
	}
	if (ok) {
		l -> t_c_mod = p2;
		l -> t_c_byte = 0;
		l -> t_c_sts = sts;
		_ios_list_3_print(d, b, p1, p2);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 4:
 DRIVER.ISTANZA.CPU.MODULO.{byte|DAC|ADC.n|XDAC.n|SDIS.n} ). */
static void _ios_list_4_print(driver_t * d, bstring_t *b,int p1,int p2,int p3)
{
	_ios_list_3_print(d, b, p1, p2);
	util_addch_bstring(b,'.');
	if (p3 == LEVEL_4_DAC) {
		util_cat_bstring(b,"DAC");
	}
	else if (p3 >= LEVEL_4_ENC) {
		util_cat_bstring(b,"ENC.");
		util_ltob_bstring(b,p3 - LEVEL_4_ENC,10);
	}
	else if (p3 >= LEVEL_4_AOUT) {
		util_cat_bstring(b,"AOUT.");
		util_ltob_bstring(b,p3 - LEVEL_4_AOUT,10);
	}
	else if (p3 >= LEVEL_4_AIN) {
		util_cat_bstring(b,"AIN.");
		util_ltob_bstring(b,p3 - LEVEL_4_AIN,10);
	}
	else if (p3 >= LEVEL_4_SDIS) {
		util_cat_bstring(b,"SDIS.");
		util_ltob_bstring(b,p3 - LEVEL_4_SDIS,10);
	}
	else if (p3 >= LEVEL_4_XDAC) {
		util_cat_bstring(b,"SDA.");
		util_ltob_bstring(b,p3 - LEVEL_4_XDAC,10);
	}
	else if (p3 >= LEVEL_4_ADCN) {
		util_cat_bstring(b,"ADCN.");
		util_ltob_bstring(b,p3 - LEVEL_4_ADCN,10);
	}
	else if (p3 >= LEVEL_4_ADC) {
		util_cat_bstring(b,"ADC.");
		util_ltob_bstring(b,p3 - LEVEL_4_ADC,10);
	}
	else if (p3 >= LEVEL_4_SELCO_IN) {
		util_cat_bstring(b,"0.");
		util_ltob_bstring(b,p3 - LEVEL_4_SELCO_IN,10);
	}
	else
		util_ltob_bstring(b,p3,10);
}

/* Funzione di esplorazione per il livello 4
 (DRIVER.ISTANZA.CPU.MODULO.{byte|DAC|ADC.n}. */
static int _ios_list_4(driver_t * d, devnode_t * l, bstring_t *b)
{
int ok = 0;
int p3 = -1;
int spec,sts;
int fb[8];
dr_ios_t *dev = (dr_ios_t *) d -> device;

	sts = l -> t_c_sts + 256 * l -> t_c_sts2;
	if (dev -> canbus) {
		spec = 0;
	}
	else if (_ios_decode_status(sts,fb,&spec) < 0) {
		return -1;
	}

	while (!ok) {
		p3 = l -> c_byte;
		if (p3 < LEVEL_4_DAC) {
			if (fb[p3]) {
				ok = 1;
				l -> flags = fb[p3]
					   | (spec ? 0 : DRIVER_MODE_LIST);
				l -> nbit = 8;
			}
		}
		else if (spec) {
			if (p3 == LEVEL_4_DAC) {
				ok = 1;
				l -> flags = DRIVER_MODE_OUTPUT;
				l -> nbit = 16;
			}
			else if (p3 >= LEVEL_4_SELCO_IN
			      && p3 < LEVEL_4_ADC) {
				ok = 1;
				if (p3 < LEVEL_4_SELCO_OUT)
					l -> flags = DRIVER_MODE_INPUT;
				else if (p3 < LEVEL_4_SELCO_OUT+8)
					l -> flags = DRIVER_MODE_OUTPUT
					           | DRIVER_MODE_LIST;
				else
					l -> flags = DRIVER_MODE_OUTPUT;
				l -> nbit = 1;
			}
			else if (p3 >= LEVEL_4_ADC
			      && p3 < LEVEL_4_XDAC) {
				ok = 1;
				l -> flags = DRIVER_MODE_INPUT;
				l -> nbit = 16;
			}
			else if (p3 >= LEVEL_4_XDAC
			      && p3 < LEVEL_4_SDIS) {
				ok = 1;
				l -> flags = DRIVER_MODE_OUTPUT
				           | DRIVER_MODE_LIST;
				l -> nbit = 0;
			}
			else if (p3 >= LEVEL_4_SDIS
			      && p3 < LEVEL_4_AIN) {
				ok = 1;
				l -> flags = DRIVER_MODE_OUTPUT
				           | DRIVER_MODE_LIST;
				l -> nbit = 32;
			}
		}
		else if (dev -> canbus) {
			if (p3 < LEVEL_4_AIN) {
			}
			else if (p3 >= LEVEL_4_AIN
			      && p3 < LEVEL_4_AOUT) {
				if (p3 - LEVEL_4_AIN
				 < dev -> canmod[l->c_mod].ain) {
					ok = 1;
					l -> flags = DRIVER_MODE_INPUT;
					l -> nbit = 16;
				}
			}
			else if (p3 >= LEVEL_4_AOUT
			      && p3 < LEVEL_4_ENC) {
				if (p3 - LEVEL_4_AOUT
				 < dev -> canmod[l->c_mod].aout) {
					ok = 1;
					l -> flags = DRIVER_MODE_OUTPUT;
					l -> nbit = 16;
				}
			}
			else if (p3 >= LEVEL_4_ENC
			      && p3 < LEVEL_4_QUIT) {
				if (p3 - LEVEL_4_ENC
				 < dev -> canmod[l->c_mod].enc) {
					ok = 1;
					l -> flags = DRIVER_MODE_INPUT;
					l -> nbit = 32;
				}
			}
			else {
				break;
			}
		}
		else {
			break;
		}
		++(l -> c_byte);
	}
	if (ok) {
		l -> t_c_byte = p3;
		l -> t_c_sts = sts % 256;
		l -> t_c_sts2 = sts / 256;
		l -> t_c_bit = 0;
		l -> t_c_mode = l -> flags & ~DRIVER_MODE_LIST;
		_ios_list_4_print(d, b, l -> c_bus,
		                  l -> c_mod, p3);
	}
	return 0;
}

static int _ios_list_5(driver_t * d, devnode_t * l, bstring_t *b)
{
int p3,p4,nb;
char *s;

	p3 = l -> c_byte;
	if (p3 >= LEVEL_4_SELCO_OUT && p3 < LEVEL_4_SELCO_OUT+8) {
		l -> flags = l -> c_mode;
		nb = 16;
		p4 = l -> c_laeconf;
		switch (p4) {
		case LEVEL_5_LAE_PT: s = "PULSE"; break;
		case LEVEL_5_LAE_DT: s = "DELAY"; break;
		case LEVEL_5_LAE_TL: nb = 1; s = "TRIGL"; break;
		case LEVEL_5_LAE_OL: nb = 1; s = "OUTL"; break;
		default:
			return 0;
		}
		++(l -> c_laeconf);
		l -> nbit = nb;
		_ios_list_4_print(d,b,l -> c_bus, l -> c_mod,l -> c_byte);
		util_addch_bstring(b,'.');
		util_cat_bstring(b,s);
	}
	else if (p3 >= LEVEL_4_XDAC && p3 < LEVEL_4_SDIS) {
		l -> flags = l -> c_xdacmode;
		nb = 16;
		p4 = l -> c_xdacconf;
		switch (p4) {
		case LEVEL_5_XDAC_BP: s = "BP"; break;
		case LEVEL_5_XDAC_UP: s = "UN"; break;
		case LEVEL_5_XDAC_SG: s = "US"; break;
		case LEVEL_5_XDAC_DR: nb = 1; s = "OUT"; break;
		default:
			return 0;
		}
		++(l -> c_xdacconf);
		l -> nbit = nb;
		_ios_list_4_print(d,b,l -> c_bus, l -> c_mod,l -> c_byte);
		util_addch_bstring(b,'.');
		util_cat_bstring(b,s);
	}
	else if (p3 >= LEVEL_4_SDIS && p3 < LEVEL_4_AIN) {
		l -> flags = l -> c_sdismode;
		nb = 1;
		p4 = l -> c_sdisconf;
		switch (p4) {
		case LEVEL_5_SDIS_DEC: s = "DEC"; break;
		default:
			return 0;
		}
		++(l -> c_sdisconf);
		l -> nbit = nb;
		_ios_list_4_print(d,b,l -> c_bus, l -> c_mod,l -> c_byte);
		util_addch_bstring(b,'.');
		util_cat_bstring(b,s);
	}
	else if (p3 >= LEVEL_4_AIN && p3 < LEVEL_4_QUIT) {
		_ios_list_4_print(d,b,l -> c_bus, l -> c_mod,l -> c_byte);
		return 0;
	}
	else {
		p4 = l -> c_bit;
		if (p4 < 8) {
			++(l -> c_bit);
			l -> nbit = 1;
			l -> flags = l -> c_mode;
			_ios_list_4_print(d,b,l -> c_bus,
			                  l -> c_mod,l -> c_byte);
			util_addch_bstring(b,'.');
			util_ltob_bstring(b,p4,10);
		}
	}
	return 0;
}

static int ios_list(driver_t * d, devnode_t * l)
{
int rv;
bstring_t b;
dr_ios_t *dev = (dr_ios_t *) d -> device;

	l -> flags = 0;
	l -> nbit = 0;
	l -> name[0] = '\0';

	util_init_bstring(&b,l -> name, sizeof(l -> name));

	switch (l -> ideep) {
	case 1:
		{
		static const char *models[] = {
		       "?","IOS","CX","UDC3","MPP3","UDC3SL","MPP3SL" };
			if (dev -> model >= sizeof(models)/sizeof(models[0]))
				util_safe_sprintf(l -> comment,
				                  sizeof(l -> comment),
				                  "id=%d",dev -> model);
			else
				util_safe_sprintf(l -> comment,
				                  sizeof(l -> comment),
				                  "%s",models[dev -> model]);
			rv = 0;
			break;
		}
	case 2:
	/* Livello degli oggetti CPU, LED, UDC ed MPP. */
		rv = _ios_list_2(d,l,&b);
		break;
	case 3:
	/* Livello degli oggetti CPU.MODULO o {LED|UDC|MPP}.SEGNALE. */
		rv = _ios_list_3(d,l,&b);
		break;
	case 4:
	/* Livello degli oggetti CPU.MODULO.{BYTE|DAC}. */
		rv = _ios_list_4(d,l,&b);
		break;
	case 5:
	/* Livello degli oggetti CPU.MODULO.BYTE.BIT */
		rv = _ios_list_5(d,l,&b);
		break;
	default:
		rv = -1;
		break;
	}
	return rv;
}

static int ios_show(driver_t * d, devnode_t *l, void *dest)
{
unsigned char *target;
dr_ios_t *dev = (dr_ios_t *) d -> device;
int p1,p2,p3;

	l -> nbit = 1;

	switch (l -> ideep) {

	case 5:

		p3 = l -> c_byte;
		if (p3 >= LEVEL_4_SELCO_OUT && p3 < LEVEL_4_SELCO_OUT+8) {


		/* Uscite speciali di configurazione dei moduli LAE.
		  Bisognerebbe forse restituire almeno i
		 valori immagazzinati nelle strutture degli output
		 collegati a variabili, ma non mi e` affatto chiaro
		 se sia la cosa piu` giusta. Potrebbe essere fuorviante.
		  In fondo, il modulo NON E` CAPACE di comunicare la
		 propria configurazione (e se anche lo fosse, sarebbe
		 un bel casino...). */

			switch (l -> c_laeconf) {
			case LEVEL_5_LAE_PT:
			case LEVEL_5_LAE_DT:
				l -> nbit = 16;
				*(short *)dest = 0;
				break;
			case LEVEL_5_LAE_TL:
			case LEVEL_5_LAE_OL:
				*(char *)dest = 0;
				break;
			default:
			/* ERRORE: Non so come sia successo, ma non doveva succedere. */
				return 0;
			}

		}
		else if (p3 >= LEVEL_4_XDAC && p3 < LEVEL_4_XDAC+2) {

		/* DAC multipli. Stesse perplessita` viste sopra. */

			switch (l -> c_xdacconf) {
			case LEVEL_5_XDAC_BP:
			case LEVEL_5_XDAC_UP:
			case LEVEL_5_XDAC_SG:
				l -> nbit = 16;
				*(short *)dest = 0;
				break;
			case LEVEL_5_XDAC_DR:
				*(char *)dest = 0;
				break;
			default:
			/* ERRORE: Non so come sia successo, ma non doveva succedere. */
				return 0;
			}
		}
		else if (p3 >= LEVEL_4_SDIS && p3 < LEVEL_4_SDIS+16) {

		/* Uscite di configurazione dei punti per moduli SDIS.
		 Non rilevabili se non agganciate a variabili. */

			l -> nbit = 1;
			*(unsigned char *)dest = 0;
		}
		else if (p3 >= LEVEL_4_AIN && p3 < LEVEL_4_ENC) {

		/* Ingressi o uscite CANpen e 16 bit. */
	
			target = _ios_make_target(dev,l);
			*(short *)dest = *(short *)target;
			return 1;
		}
		else if (p3 >= LEVEL_4_ENC && p3 < LEVEL_4_QUIT) {

		/* Ingressi encoder CANOpen. */
	
			target = _ios_make_target(dev,l);
			*(long *)dest = *(long *)target;
			return 1;
		}
		else {

		/* Normali ingressi o uscite a bit. Si pesca direttamente
		 dalla mappa sulla scheda IOS. */
	
			target = _ios_make_target(dev,l);
			*(char *)dest = (*target & (1 << (l -> c_bit))) != 0;
			return 1;
		}

	case 4:

	/* Byte della scheda IOS, DAC, segnali della tastiera Selco o 
	 ingressi ADC.
	 Ancora, si pesca dalla scheda, dopo qualche maneggio. */

		p3 = l -> c_byte;
		target = _ios_make_target(dev,l);
		if (p3 < LEVEL_4_DAC) {
			*(unsigned char *)dest = *target;
			l -> nbit = 8;
			return 1;
		}
		else if (p3 == LEVEL_4_DAC) {
			*(short *)dest = *(short *)(target + 1);
			l -> nbit = 16;
			return 1;
		}
		else if (p3 >= LEVEL_4_SELCO_IN && p3 < LEVEL_4_ADC) {
			if (p3 >= LEVEL_4_SELCO_OUT) {
				p3 -= LEVEL_4_SELCO_OUT;
				target += 1 + p3/8;
			}
			else {
				p3 -= LEVEL_4_SELCO_IN;
			}
			*(char *)dest = (*target & (1 << (p3 % 8))) != 0;
			return 1;
		}
		else if (p3 >= LEVEL_4_ADC && p3 < LEVEL_4_XDAC) {
			*(short *)dest = _ios_read_adc(target,
			                               (p3 >= LEVEL_4_ADCN));
			l -> nbit = 16;
			return 1;
		}
		else if (p3 >= LEVEL_4_XDAC && p3 < LEVEL_4_SDIS) {
		/* ??? */
		}
		else if (p3 >= LEVEL_4_SDIS && p3 < LEVEL_4_AIN) {
		/* Alo solito, se il segnale non e` collegato, non c'e`
		 modo di conoscere lo stato dell'uscita. Se e` collegato,
		 non ce n'e` bisogno. */
			l -> nbit = 32;
			*(long *)dest = 0;
		}
		else if (p3 >= LEVEL_4_AIN && p3 < LEVEL_4_AOUT) {
			l -> nbit = 16;
			*(short *)dest = *(short *)(target);
			return 1;
		}
		else if (p3 >= LEVEL_4_AOUT && p3 < LEVEL_4_ENC) {
			l -> nbit = 16;
			*(short *)dest = *(short *)(target);
			return 1;
		}
		else if (p3 >= LEVEL_4_ENC && p3 < LEVEL_4_QUIT) {
			l -> nbit = 32;
			*(long *)dest = *(long *)(target);
			return 1;
		}
		break;

	case 3:

		p1 = l -> c_bus;
		p2 = l -> c_mod; /* Alias c_ledbit, alias c_udcbit */

		switch (p1) {

		case LEVEL_2_LED:
			*(char *)dest = (dev -> led_image & (1 << p2)) != 0;
			return 1;

		case LEVEL_2_UDC:
		case LEVEL_2_MPP:
			l -> nbit = 1;
			if (p2 < LEVEL_3_UDCOUT) {
				p2 -= LEVEL_3_UDCIN;
				*(char *)dest = (dev -> read_udc3(dev)
				                 & (1L << p2)) != 0;
				return 1;
			}
			else if (p2 < LEVEL_3_UDCQUIT) {
				p2 -= LEVEL_3_UDCOUT;
				*(char *)dest = (dev -> udc3_out_image
				                 & (1L << p2)) != 0;
				return 1;
			}
			break;

		default:
			break;
		}

	case 2:
	/* A questo livello ci sono solo il DAC della scheda CX e il CNOK
	 della IOSLK2. */

		p1 = l -> c_bus;

		if (p1 == LEVEL_2_CXDAC) {
			*(unsigned char *)dest = dev -> cxdac_image;
			return 1;
		}
		else if (p1 == LEVEL_2_CNOK) {
			*(unsigned char *)dest = dev -> cnok_image;
			return 1;
		}

		break;

	default:
		break;
	}

	return 0;
}

driver_op_t driver_ios = {
	"ios",
	ios_install,
	ios_restart,
	ios_parse,
	ios_list,
	ios_attach,
	ios_detach,
	ios_check,
	ios_trigger,
	ios_up,
	ios_down,
	(void (*)(driver_t *)) 0, /* ios_close */
	ios_fatal,
	ios_read,
	ios_write,
	ios_show,
	(int (*)(driver_t *,void *,int,void *)) 0, /* ios_extension */
};


typedef struct sdo_rw_fb sdo_rw_fb;
struct sdo_rw_fb {
/* INPUT */
	unsigned long val_wr __attribute__((packed));
	unsigned short bus __attribute__((packed));
	unsigned short id __attribute__((packed));
	unsigned short index __attribute__((packed));
	unsigned short subindex __attribute__((packed));
	unsigned short size __attribute__((packed));
	char enable __attribute__((packed));
	char reserv_1[9] __attribute__((packed));
/* OUTPUT */
	unsigned long val_rd __attribute__((packed));
	unsigned long abort __attribute__((packed));
	char err __attribute__((packed));
	char done __attribute__((packed));
	char reserv_2[8] __attribute__((packed));
/* LOCALI */
	sdo_t *sdo __attribute__((packed));
	unsigned long abort_ret __attribute__((packed));
	unsigned long val __attribute__((packed));
	char enable_old __attribute__((packed));
	char wr __attribute__((packed));
	char done_ret __attribute__((packed));
	char err_ret __attribute__((packed));
	char reserv_3[16] __attribute__((packed));
};

static canboard_t *get_canbus(unsigned int bus)
{
	if (bus >= sizeof(canbus_table)/sizeof(canbus_table[0])) {
		return NULL;
	}
	return canbus_table[bus];
}

static void sdo_rw_fb_cb(sdo_t *sdo,
                      int err,
            	      unsigned int len,
                      unsigned int abort,
                      void *cb_arg)
{
sdo_rw_fb *st = (sdo_rw_fb *)cb_arg;

	if (st -> sdo != sdo) {
	// ???
		return;
	}
	st -> sdo = NULL;
	st -> done_ret = 1;
	st -> err_ret = err;
	st -> abort_ret = abort;
}

static long sdo_rw_fb_core(sdo_rw_fb *st, int wr)
{
unsigned int sz;

	st -> wr = wr;

	if (st -> done_ret) {
		st -> done = st -> done_ret;
		st -> err = st -> err_ret;
		st -> abort = st -> abort_ret;
		if ((! wr) && (! st -> err)) {
			st -> val_rd = st -> val;
		}
		st -> done_ret = 0;
	}

	if (st -> enable) {
		if (! st -> enable_old) {
			if (wr) {
				st -> val = st -> val_wr;
			}
			else {
				st -> val = 0;
			}
			sz = st -> size;
			if (sz > sizeof(st -> val)) {
				sz = sizeof(st -> val);
			}
			st -> done = 0;
			st -> err = 0;
			st -> abort = 0;
			st -> sdo = canboard_async_sdo(get_canbus(st -> bus),
			                  st -> id,
			                  st -> index, st -> subindex,
                                          &(st -> val), sz,
                                          wr, sdo_rw_fb_cb, st);
			if (st -> sdo == NULL) {
				st -> done = 1;
				st -> err = 1;
				st -> abort = 0;
			}
		}
	}
	else {
		if (st -> enable_old) {
			if (! st -> done) {
				canboard_async_sdo_cancel(st -> sdo);
				st -> sdo = NULL;
				st -> done = 1;
				st -> err = 1;
				st -> abort = 0;
			}
		}
	}

	st -> enable_old = st -> enable;

	return st -> done;
}

long SDO_READ(sdo_rw_fb *st)
{
	return sdo_rw_fb_core(st, 0);
}

long SDO_WRITE(sdo_rw_fb *st)
{
	return sdo_rw_fb_core(st, 1);
}

