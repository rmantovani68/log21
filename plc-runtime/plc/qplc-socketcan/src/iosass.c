/*
* @(#) iosass.c 1.0 Tue Feb 14 10:09:12 WET 1995
*
* Funzioni di accesso ai moduli assi remoti. Si e` mantenuta la
* compatibilita` con il PLC 480.
*/

#include "iosass.h"

/*
---------------------------------------------
COSTANTI PER COLLOQUIO ASSI <--> UTENTE
---------------------------------------------
*/

/*---------- Equate generali ----------*/

#define	VERSION	1.0		/*versione della libreria*/

/*---------- Equate di configurazione e della IOS ----------*/

#define	TXQ	0		/*pos. IOS zona byte di tx,stato asse e quota*/
#define	STA	0x200		/*pos. IOS zona byte di stato modulini*/
#define	IMM	0x220		/*pos. IOS byte di output immediato*/
#define	RXE	0x300		/*pos. IOS zona byte di rx ed errori*/
#define	NBM	8		/*num. byte per ogni modulo sulla IOS*/


/*
--------------------------------------------------------
CODICE PER COLLOQUIO DA E PER GLI ASSI REMOTI SU CPZ/SLV
--------------------------------------------------------
*/

/*----------------------------------------------------------------------------

	FUNZIONI per COLLOQUIO ASSI REMOTI su IOSPC

	Le funzioni contenute in iosass.a realizzano
	il colloquio con i modulini assi remoti connessi alla IOS

----------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------

istatm
------

	Funzione che ritorna lo stato del modulino seriale.
	Dichiarazione 'C':
		int istatm(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

int istatm(unsigned char ass, unsigned char * ios)
{
	return ios[STA+ass];
}

/*--------------------------------------------------------------------------

itxcom
------

	Funzione che invia il comando all' asse.
	Se il bit sette del numero asse in input e' posto a 1
	non trasmette il comando con attributo di "immediato".
	Dichiarazione 'C':
		void itxcom(ass,cod,dat,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char cod;		codice comando
		long dat;			dato del comando (3 byte utili)
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

void itxcom(unsigned char ass, unsigned char cod, long dat, unsigned char *ios)
{
volatile unsigned char (*p);

/* Calcola l'indirizzo della parola di comando dell'asse. */

	p = ios + (ass & 0x7F) * NBM + TXQ;

/* Attende che il registro di stato indichi "libero". */

	while (p[0]) {
	}

/* Scrive il dato */

	p[1] = (unsigned char) dat;
	p[2] = (unsigned char) (dat / 0x100);
	p[3] = (unsigned char) (dat / 0x10000);

/* Scrive il comando. Cio` inneschera` la trasmissione. */

	p[0] = cod;

/* Gestisce l'attributo di "immediato". */

	if (! (ass & 0x80)) {

	/* Calcola l'indirizzo del registro di output immediato. */

		p = ios + IMM;

	/* Attende che il dispositivo sia pronto. */

		while (*p != 0xFF) {
		}

	/* Innesca la trasmissione. */

		*p = ass;
	}
}

/*--------------------------------------------------------------------------

irxcom
------

	Funzione che gestisce la ricezione comando dall' asse
	trasmette la risposta con attributo di "immediato".
	Ritorna zero se non c'e' nessun comando ricevuto
	Dichiarazione 'C':
		unsigned long irxcom(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

unsigned long irxcom(unsigned char ass, unsigned char * ios)
{
volatile unsigned char (*p);
unsigned long rv = 0;
unsigned char *tmp;

/* Calcola l'indirizzo della parola di comando dell'asse. */

	p = ios + ass * NBM + RXE;

/* Verifica se c'e` un comando da gestire. */

	if (p[0]) {

	/* Truccaccio per costruire il valore di ritorno componendone
	 i byte. Funziona su Z80, Z280 e Intel 80x86. */

		tmp = (unsigned char *)&rv;

	/* Legge dato e codice comando. */

		tmp[0] = p[1];
		tmp[1] = p[2];
		tmp[2] = p[3];
		tmp[3] = p[0];

	/* Azzera il byte di comando. */

		p[0] = 0;

	/* Sposta il puntatore nella zona di trasmissione dei comandi. */

		p = p - RXE + TXQ;

	/* Attende che il dispositivo sia pronto. */

		while (p[0]) {
		}

	/* Trasmette la risposta. */

		p[0] = tmp[3];

	/* Calcola l'indirizzo del registro di output immediato. */

		p = ios + IMM;

	/* Attende che il dispositivo sia pronto. */

		while (*p != 0xFF) {
		}

	/* Innesca la trasmissione immediata della risposta. */

		*p = ass;
	}

	return rv;
}

/*--------------------------------------------------------------------------

irxquo
------

	Funzione che gestisce la lettura della quota dall' asse
	Ritorna la quota dell'asse esterno.
	Dichiarazione 'C':
		long irxquo(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

long irxquo(unsigned char ass, unsigned char *ios)
{
volatile unsigned char (*p);
long rv = 0;
unsigned char *tmp;

/* Calcola l'indirizzo della quota dell'asse. */

	p = ios + ass * NBM + TXQ + 5;

/* Truccaccio per costruire il valore di ritorno componendone
 i byte. Funziona su Z80, Z280 e Intel x86. */

	tmp = (unsigned char *)&rv;

/* Legge la quota, controllando che sia stabilizzata (ma siamo sicuri ?). */

	tmp[0] = p[0];

	do {
		tmp[1] = p[1];
		tmp[2] = p[2];
	} while (tmp[1] != p[1] || tmp[2] != p[2]);

/* Aggiusta il segno del risultato. */

	tmp[3] = (tmp[2] & 0x80) ? 0xFF : 0x00;

	return rv;
}

/*--------------------------------------------------------------------------

irxsta
------

	Funzione che gestisce la lettura dello stato dall' asse
	Ritorna la stato dell'asse esterno.
	Dichiarazione 'C':
		unsigned char irxsta(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

unsigned char irxsta(unsigned char ass, unsigned char * ios)
{
	return ios[ass * NBM + TXQ + 4];
}

/*--------------------------------------------------------------------------

irxerr
------

	Funzione che gestisce la lettura dell' errore dall' asse
	Ritorna l' errore dell'asse esterno.
	Dichiarazione 'C':
		unsigned int irxerr(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

unsigned int irxerr(unsigned char ass, unsigned char * ios)
{
unsigned char *p;

	p = ios + ass * NBM + RXE + 4;

	return p[0] + p[1] * 0x100;
}

