/*
* @(#) dr_ios.h 4.9 mer dic 16 22:17:21 CET 2009
*
* Driver della scheda CNI IOS per QPLC.
*
* Forma dei parametri specifici :
*
*        8 bit : <cpu>.<modulo>.<byte>
*        1 bit : <cpu>.<modulo>.<byte>.<bit>
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
* 24/05/96 GG 1.0 Prima stesura.
* 18/02/97 GG 1.1 Aggiunto supporto per il CN compatto (LED...).
* 17/04/97 GG 1.2 Aggiunti due flag di attivazione del trigger e del
*             controllo della scheda (per debug).
* 22/05/97 GG 1.3 Aggiunta la gestione di alcuni moduli speciali, come
*             ta tastiera Selco.
* 28/07/97 GG 2.0 Riorganizzazione per accesso a basso livello ai moduli.
* 01/08/97 GG 2.1 Aggiunta un'opzione di configurazione del driver che
*             impedisce l'accesso a byte arbitrari della IOS. Sara` normalmente
*             abilitata, perche` questo tipo di accesso puo` pregiudicare
*             la sicurezza del sistema.
* 21/01/98 GG 2.2 Aggiunto il supporto per la scheda UDC3.
* 06/02/98 GG 2.3 Aggiunto un flag per ignorare l'eventuale stato di fault che
*             manifesta fisiologicamente la prima volta che si legge il byte
*             di stato dell'UDC3 (succede che si legge lo stato del watchdog
*             senza averlo mai rinfrescato).
* 13/03/98 GG 2.4 Aggiunta la possibilita` di mantenere lo stato dei bit
*             non utilizzati di un byte di output tra una forzatura e
*             l'altra (attivo solo se AZZERA_NON_USATI_IOS vale 0).
* 20/03/98 GG 2.5 Migliorata la gestione della UDC3 (e MPP3) parametrizzando
*             il numero di ingressi e di uscite presenti (due nuovi campi
*             nella struttura di stato).
* 27/03/98 GG 2.6 Aggiunto il campo "model", contenete il codice di modello
*             della scheda.
* 01/04/98 GG 3.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
* 08/06/98 GG 3.1 Aggiunto il supporto per il DAC della scheda CX.
* 15/06/98 GG 3.2 Aggiunto il supporto per i moduli ADC.
* 30/06/98 GG 3.3 Aggiunta la variabile di configurazione "wdtime", che
*             contiene il tempo d'intervento del watchdog della IOS espresso
*             in cicli di PLC.
* 25/08/98 GG 3.4 Aggiunto un contatore di cicli di timeout per la IOS,
*             necessario per evitare segnalazioni di timeout erronee quando
*             il ciclo di plc e` molto breve (timer INAX).
* 20/11/98 GG 3.5 Pesanti modifiche per moduli LAE.
* 30/01/01 GG 4.0 Pesanti modifiche per moduli DAC doppi.
* 15/03/01 GG 4.1 Aggiunto l'accesso al CNOK della scheda IOSLK2 (ios.0.cnok).
* 19/03/01 GG 4.2 Aggiunto l'accesso ai moduli SDIS.
* 19/03/01 GG 4.3 Continuano i lavori sugli SDIS.
* 23/03/01 GG 4.4 Corretto un orrendo bug nella gestione dei moduli XDAC
*             e SDIS. Era sbagliata la definizione dei puntatori delle
*             liste dei moduli di questo tipo. Era privo di conseguenze
*             solo perche` il tipo utilizzato si riveriva ad una struttura
*             piu` grande.
* 11/05/01 GG 4.5 Aggiunte due nuove variabili di configurazione,
*             ("chkwd={0|1}" e "chkdis={0|1}") che controllano il test
*             passivo dello stato della CPU e il test sulla disconnessione
*             dei moduli. E` anche stata riabilitata la variabile "wd=<n>".
* 02/04/08 GG 4.6 Tentativo di supporto degli encoder CN104.
* 08/07/08 GG 4.7 Aggiunto un flag per la disattivazione della rimappatura
*             dei PDO sui moduli CNI Informatica.
* 04/08/08 GG 4.8 Aggiunto un flag per la disattivazione del canaler CAN.
* 16/12/09 GG 4.9 Aggiunta la variabile di configurazione "only_can"
*             che permette di ignorare il cnale IOS tradizionale
*             anche se presente per passare direttamente al canale CAN.
*             E' a 1 per default.
*/

#ifndef _DR_IOS_H_

/* Purtroppo e` necessario... */
#include "dr_can.h"

/*
* Ponendo a 0 questa macro si fa si` che lo stato degli I/O
* venga preservato durante l'HALT tra una forzatura e l'altra.
*/

#define AZZERA_NON_USATI_IOS 0

#define MAX_IOS_MOD 32

/*
*  Definizione della struttura interna descrivente un byte coinvolto
* nelle operazioni di I/O orientate al bit. Ogni bit del byte
* e` rappresentato da una maschera di accesso e dall'indirizzo del byte
* a cui e` associato. I bit nella struttura sono rappresentati in
* modo disordinato. Il primo elemento con maschera nulla termina l'insieme.
*/

typedef struct {
	unsigned long m; /* Maschera. */
	unsigned char * var; /* indirizzo della variabile. */
} ios_bit_t;

typedef struct _ios_direct_byte {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
	unsigned char * var; /* indirizzo della variabile. */
	struct _ios_direct_byte * next; /* Puntatore al prossimo elemento. */
} ios_direct_byte_t;

typedef struct _ios_byte {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
#if !AZZERA_NON_USATI_IOS
	unsigned int m;		/* Maschera globale per il byte. */
#endif
	int mod;		/* Modulo di riferimento (per CAN). */
	int idx;		/* Posizione nel modulo (per CAN). */
	ios_bit_t bit[9];	/* 8 bit (+1 per terminare la lista). */
	struct _ios_byte * next; /* Puntatore al prossimo elemento. */
} ios_byte_t;

typedef struct _ios_w12bit {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
	unsigned long image;	/* Immagine dell'uscita + parola di comando. */
	short * var; /* indirizzo della variabile. */
	struct _ios_w12bit * next; /* Puntatore al prossimo elemento. */
} ios_w12bit_t;

typedef ios_w12bit_t ios_ain_t;

typedef struct _ios_enc {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
	long * var; /* indirizzo della variabile. */
	struct _ios_enc * next; /* Puntatore al prossimo elemento. */
} ios_enc_t;

typedef struct _ios_aout {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
	unsigned long image;	/* Immagine dell'uscita + parola di comando. */
	short * var; /* indirizzo della variabile. */
	int mod;		/* Modulo di riferimento (per CAN). */
	int idx;		/* Posizione nel modulo (per CAN). */
	struct _ios_aout * next; /* Puntatore al prossimo elemento. */
} ios_aout_t;

typedef struct _ios_w24bit {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
	unsigned long image;	/* Immagine dei led + parola di comando. */
	ios_bit_t bit[25];	/* 24 bit (+1 per terminare la lista). */
	struct _ios_w24bit * next; /* Puntatore al prossimo elemento. */
} ios_w24bit_t;

typedef struct _ios_xdac {
	unsigned char * addr;	/* Indirizzo dei byte del modulo. */
	unsigned long image;	/* Immagine dei dati + parola di comando. */
	struct {
		int mode; /* Modalita` di accesso. */
#define _IOS_XDAC_MODE_NONE 0
#define _IOS_XDAC_MODE_UNIPOLAR 1
#define _IOS_XDAC_MODE_BIPOLAR 2
#define _IOS_XDAC_MODE_SIGNED 3
#define _IOS_XDAC_MODE_MASK 3
#define _IOS_XDAC_MODE_TRASH 0x0F /* Valore "impossibile". */
		unsigned short * val; /* Indirizzo della variabile "valore". */
		unsigned char * dir; /* Indirizzo della variabile "segno". */
	} vars[2];	/* Descrittore delle variabili agganciate al modulo. */
	struct _ios_xdac * next; /* Puntatore al prossimo elemento. */
} ios_xdac_t;

typedef struct _ios_sdis {
	unsigned char * addr;	/* Indirizzo dei byte del modulo. */
	unsigned char dtab[16]; /* Tabella degli indici di display. */
	int n_disp; /* Numero di display utilizzati. */
	int disp_map; /* Bitmap dei display utilizzati. */
	unsigned long dots; /* Immagine dello stato dei punti. */
	struct {
		long last_val; /* Immagine del dato corrente. */
		long * val; /* Indirizzo della variabile "valore". */
		unsigned char * dec; /* Indirizzo della variabile "punto". */
	} vars[16];	/* Descrittore delle variabili agganciate al modulo. */
	struct _ios_sdis * next; /* Puntatore al prossimo elemento. */
} ios_sdis_t;

typedef struct _ios_laetm {
	unsigned short * var; /* Indirizzo della variabile. */
} ios_laetm_t;

typedef struct _ios_laepol {
	unsigned char * var; /* Indirizzo della variabile. */
} ios_laepol_t;

typedef struct _ios_lae {
	unsigned char * addr;	/* Indirizzo della zona del modulo. */
	int nused;		/* Numero di segnali utilizzati. */
	int usedidx[8];		/* Lista dei segnali utilizzati. */
	long dl_pol_img[8];	/* Immagini dei comandi ritardo+polarita` */
	long pulse_img[8];	/* Immagini dei comandi per durata impulsi */
	ios_laetm_t delay[8];	/* Lista dei ritardi sugli ingressi. */
	ios_laetm_t pulse[8];	/* Lista delle durate degli impulsi. */
	ios_laepol_t trpol[8];	/* Lista delle polarita` di trigger. */
	ios_laepol_t oupol[8];	/* Lista delle polarita` di uscita. */
	struct _ios_lae * next; /* Puntatore al prossimo elemento. */
} ios_lae_t;

/*
* Definizione del tipo per la struttura di stato del driver ios.
*/

typedef struct _dr_ios {
/* Indirizzo di base della RAM. */
	unsigned char *base;
/* Puntatori alle CPU. */
	unsigned char *cpu[MAX_IOS_CPU];
/* Puntatore alla NVRAM (grezza). */
	unsigned char *nvram;
/* File descriptor (se utilizzato). */
	int fd;
/* Codice di modello della scheda. */
	int model;
/* Modalita` di manipolazione della porta LED (CN compatto). */
	void (*write_leds)(struct _dr_ios *, int);
/* Porta di I/O che controlla i LED. */
	int led_port;
/* Flag di presenza del dispositivo LED. */
	int led_present;
/* Immagine dei LED. */
	int led_image;
#if !AZZERA_NON_USATI_IOS
	int led_out_mask;
#endif
/* Numero di segnali collegati all'uscita LED. */
	int n_leds;
/* Flag di presenza del dispositivo CX DAC. */
	int cxdac_present;
/* Immagine del DAC CX. */
	int cxdac_image;
/* Indirizzo della variabile associata al DAC CX. */
	unsigned char * cxdac_var;
/* Modalita` di manipolazione della porta DAC CX. */
	void (*write_cxdac)(struct _dr_ios *, int);
/* Flag di presenza del dispositivo CNOK. */
	int cnok_present;
/* Immagine del CNOK. */
	int cnok_image;
/* Indirizzo della variabile associata al CNOK. */
	unsigned char * cnok_var;
/* Modalita` di manipolazione della porta CNOK. */
	void (*write_cnok)(struct _dr_ios *, int);
/* Modalita` di manipolazione delle porte UDC3. */
	int (*read_udc3)(struct _dr_ios *);
	void (*write_udc3)(struct _dr_ios *, int);
	int (*check_udc3)(struct _dr_ios *);
	void (*trig_udc3)(struct _dr_ios *);
/* Porte di I/O che controllano gli ingressi e le uscite UDC3. */
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[2];
	} udc3_in_ports;
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[2];
	} udc3_out_ports;
/* Porte di I/O per il trigger del watchdog e la lettura dello stato. */
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[1];
	} udc3_trigwd_ports;
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[1];
	} udc3_wdsts_ports;
/* Porta di I/O per il DAC della scheda CX. */
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[1];
	} cxdac_ports;
/* Porta di I/O per il CNOK della scheda IOSLK2. */
	struct {
		struct ios_ports_t p;
		struct ios_port_descr_t filler[1];
	} cnok_ports;
/* Flag di presenza del dispositivo UDC3. */
	int udc3_present;
/* Numero di ingressi e uscite presenti sulla scheda. */
	int udc3_num_in;
	int udc3_num_out;
/* Immagine delle uscite UDC3. */
	int udc3_out_image;
#if !AZZERA_NON_USATI_IOS
	int udc3_out_mask;
#endif
/* Contatore di "prima lettura" dello stato del bit di stato del watchdog.
  Serve ad ignorare l'eventuale "non ok" fisiologico al primo accesso
 in assoluto. */
	int udc3_firstchk;
/* Stato del bit di rinfresco del watchdog della UDC3. */
	int udc3_wdout_image;
/* Numero di segnali collegati agli ingressi e alle uscite UDC3. */
	int n_udc3_in;
	int n_udc3_out; 
/* Lista di input a bit. */
	ios_byte_t *bit_in_list;
/* Lista di output a bit. */
	ios_byte_t *bit_out_list;
/* Lista di input a byte. */
	ios_direct_byte_t *byte_in_list;
/* Lista di output a byte. */
	ios_direct_byte_t *byte_out_list;
/* Lista di output a bit per i moduli speciali (tastiera Selco). */
	ios_w24bit_t *w24bit_out_list;
/* Lista di output a word per i moduli speciali (convertitori D/A). */
	ios_w12bit_t *w12bit_out_list;
/* Lista di input a word per i moduli speciali (convertitori A/D). */
	ios_w12bit_t *adc_in_list;
/* Lista di input a word per ingressi analogici CANOpen. */
	ios_ain_t *ain_list;
/* Lista di output a word per uscite analogiche CANOpen. */
	ios_aout_t *aout_list;
/* Lista di input a long per encoder CANOpen. */
	ios_enc_t *enc_list;
/* Lista di output speciale per le uscite speciali dei moduli LAE. */
	ios_lae_t *lae_out_list;
/* Lista di output speciale per le uscite dei moduli XDAC. */
	ios_xdac_t *xdac_out_list;
/* Lista di output speciale per le uscite dei moduli SDIS. */
	ios_sdis_t *sdis_out_list;
/* Lista di output speciale per l'uscita LED. */
	ios_bit_t led_out_list[8];
/* Lista di input speciale per gli ingressi UDC3. */
	ios_bit_t udc3_in_list[16];
/* Lista di output speciale per gli ingressi UDC3. */
	ios_bit_t udc3_out_list[16];

/* Numero di CPU presenti. */
	int n_cpu;
/* Puntatori alle CPU attive. */
	unsigned char *acpu[MAX_IOS_CPU];
/* Puntatori agli ingressi di trigger. */
	unsigned char *wdin[MAX_IOS_CPU];
/* Puntatori ai flag di "CPU OK". */
	unsigned char *wdout[MAX_IOS_CPU];
/* Puntatori ai flag di "modulo disconnesso". */
	unsigned char *disc[MAX_IOS_CPU];
/* Contatori di timeout per "CPU OK". */
	int curr_count[MAX_IOS_CPU];

/* Tabella di conversione da indice interno ad indice esterno di CPU, per
 la visualizzazione degli errori. */
	signed char rev[MAX_IOS_CPU];

/* Flag di configurazione derivati dallal linea di comando (debug). */
	int do_trig; /* Rinfresca la scheda. */
	int do_chk;  /* Controlla la scheda. */
	int do_chk_wd;  /* Controlla la CPU della scheda (passivo). */
	int do_chk_dis;  /* Controlla lo sgancio dei moduli. */
	int direct_byte_access; /* Permette l'accesso a qualsiasi byte */
	                        /* della IOS. */
	int wdtime; /* Tempo d'intervento del watchdog espresso in */
	            /* cicli di PLC. */
	int max_count; /* Massimo del contatore di timeout per "CPU OK". */
	int canspeed; /* Velocita` dell'eventuale CAN. */
	int candebug; /* Flag di debug del gestore CAN. */
	int cansync; /* Flag di emissione del SYNC controllata. */
	int cancniremap; /* Flag di non rimappatura PDO per CNI Inf. */
	int forcesync; /* Flag di emissione del SYNC forzata. */
	int bitoffset; /* Offset per taroccamento degli indici di bit. */
	int enable_can; /* Abilitazione del canale CAN. */
	int only_can; /* Abilitazione del SOLO canale CAN (niente ios). */

/* Flag di ripetizione della fase di HALT. */
	int repeat_me;

/* Descrittore dell'eventuale controllore CAN. NULL nel caso IOS. */
	canboard_t *canbus;
/* Indice del dispositivo CAN in uso. -1 nel caso IOS. */
	int candevice;
/* Cache delle caratteristiche dei moduli. */
	struct {
		int present;
		unsigned int in;
		unsigned int inb;
		unsigned int out;
		unsigned int outb;
		unsigned int ain;
		unsigned int aout;
		unsigned int enc;
	} canmod[128];
} dr_ios_t;

#define _DR_IOS_H_

#endif

