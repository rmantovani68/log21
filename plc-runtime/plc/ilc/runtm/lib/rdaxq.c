

/*
* @(#) rdaxq.c 1.1 Thu Mar  2 11:33:20 MET 2000
*
*  Funzione per la lettura stato di un asse.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 07/07/98 GG 1.0 Prima stesura.
* 02/03/00 GG 1.1 Filtrati alcuni codici pericolosi, e aggiunta
*             la conversione del risultato di SAX_CONTAMETRI.
*/

#include <stdio.h>
#include <plcapi.h>

struct rdaxq_t {
	float scale __attribute__((packed));
	long op __attribute__((packed));
	long index __attribute__((packed));
	long p1 __attribute__((packed));
	long p2 __attribute__((packed));
	char *format __attribute__((packed));
} _f_RDAXQ = {
	0.0,
	0,
	0,
	0,
	0,
	"F",
};

#define _SAX_VERS_LEN 16
#define _SAX_VSCOPIO_LEN 16

long RDAXQ(void)
{
union { float f; double d; long l;
	char c[_SAX_VERS_LEN*2 /* Paranoia! */];
	unsigned char b[_SAX_VSCOPIO_LEN*2 /* Paranoia! */]; } dato;
register char c = _f_RDAXQ.format[0];

/* NOTA: Filtrare i codici

	SAX_WATCH_DOG:   restituisce fino a 16 puntatori (= 64 byte)
	SAX_VERSIONE:    restituisce 15 byte di versione (stringa senza NUL)
	SAX_CONTAMENTRI: restituisce un double (8 byte).

	SAX_VSCOPIO produce 16 byte di risultato, ma non ce ne preoccupiamo:
	saranno restituiti i primi quattro byte nell'accumulatore,
	nella remota ipotesi che qualcuno li usi.
*/

	dato.f = 0.0;
	if (CPAXSHM) {
	/* Questo e` uno dei codici pericolosi (e insensati). */
		if (_f_RDAXQ.op == SAX_WATCH_DOG) {
			return 0L;
		}
		read_parm(CPAXSHM,(int)_f_RDAXQ.op,_f_RDAXQ.index,
		          (int)_f_RDAXQ.p1,(int)_f_RDAXQ.p2,(int *)&dato);
		if (_f_RDAXQ.op == SAX_CONTAMETRI) {
	/* Purtroppo il contamentri e` espresso in double. Bisognera`
	 convertirlo a singola precisione. */
		float f;
			f = dato.d;
			dato.f = f;
		}
		else if (_f_RDAXQ.op == SAX_VERSIONE) {
	/* Si restituisce la versione come stringa statica, con tutte
	 le limitazioni inerenti. Ad aiutarci c'e` il fatto che la
	 versione non cambia, quindi non dovrebbe cambiare mai nemmeno
	 il risultato. */
		static char _RDAXQ_sax_versione[_SAX_VERS_LEN];
			dato.c[_SAX_VERS_LEN-1] = '\0';
			memcpy(_RDAXQ_sax_versione,dato.c,_SAX_VERS_LEN);
			return (long) _RDAXQ_sax_versione;
		}
		if (c == 'F' || c == 'f')
			return dato.l;
		else if (c == 'I' || c == 'i')
			return (long)(dato.f);
		else
			return (long)(dato.f * _f_RDAXQ.scale);
	}
	return dato.l;
}

