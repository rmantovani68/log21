/*
* @(#) iosass.h 1.0 Tue Feb 14 10:09:12 WET 1995
*
* Funzioni di accesso ai moduli assi remoti. Si e` cercati di mantenere,
* per quanto possibile, la compatibilita` con il PLC 480.
*/

/*----------------------------------------------------------------------------

	Dichiarazione FUNZIONI per COLLOQUIO ASSI REMOTI su IOSPC

	Le funzioni contenute in iosass.a realizzano
	il colloquio con i modulini assi remoti connessi alla IOS

----------------------------------------------------------------------------*/

int istatm(unsigned char,unsigned char *);

/*
	Funzione che ritorna lo stato del modulino seriale.
	Dichiarazione 'C':
		int istatm(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

void itxcom(unsigned char,unsigned char,long,unsigned char *);

/*
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

unsigned long irxcom(unsigned char,unsigned char *);

/*
	Funzione che gestisce la ricezione comando dall' asse
	trasmette la risposta con attributo di "immediato".
	Ritorna zero se non c'e' nessun comando ricevuto
	Dichiarazione 'C':
		unsigned long irxcom(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

long irxquo(unsigned char,unsigned char *);

/*
	Funzione che gestisce la lettura della quota dall' asse
	Ritorna la quota dell'asse esterno.
	Dichiarazione 'C':
		long irxquo(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

unsigned char irxsta(unsigned char,unsigned char *);

/*
	Funzione che gestisce la lettura dello stato dall' asse
	Ritorna la stato dell'asse esterno.
	Dichiarazione 'C':
		unsigned char irxsta(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/

unsigned int irxerr(unsigned char,unsigned char *);

/*
	Funzione che gestisce la lettura dell' errore dall' asse
	Ritorna l' errore dell'asse esterno.
	Dichiarazione 'C':
		unsigned int irxerr(ass,ios)
		unsigned char ass;		numero asse da 0 a n
		unsigned char *ios;		address base della IOS

----------------------------------------------------------------------------*/
