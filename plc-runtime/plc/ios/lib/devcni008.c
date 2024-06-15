/* devcni008.c */
#include "devcni.h"

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#ifdef Lynx
#include <mmap.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <ios.h>
#include <hwcni.h>
#include <nvram.h>

extern char *getenv(const char *);

/*
* Funzione "AttachNvram"
* ----------------------
*
*  Questa funzione provvede a collegare il processo all'area di
* ram non volatile indicata dall'etichetta "name". Le caratteristiche
* dell'area sono restituite in "area". Il parametro "base" e` un puntatore
* alla base del dispositivo "NVRAM", che puo` essere ottenuto per esempio
* dalla funzione "iosOpen". Se vale NULL (e` il caso piu` comune), la
* funzione stessa provvede ad apire e mappare il dispositivo. Il parametro
* "ios" Cintiene l'indice della IOS alla quale "base" fa riferimento, ed
* e` ignorato se "base" vale NULL. In caso contrario, la funzione controlla
* se l'indice coincide con quello previsto dalla configurazione. Se questo
* avviene, sono calcolati i campi di "area" e la funzione termana con successo,
* altrimenti si ottiene un errore. In ogni caso, se "base" non e` NULL,
* non si avranno aperture di dispositivi IOS.
*  Chiamate successive di "AttachNvram" per ottenere aree risiedenti nello
* stesso dispositivo non provocano nuove aperture di IOS, perche` la
* funzione registra gli indirizzi di base dei chip utilizzati di volta in
* volta, e li ricicla.
*
*  La funzione vale 0 in caso di successo, oppure -1 in caso di errore.
*  Data la delicatezza delle azioni compiute, questa funzione emette
* sempre messaggi su "stderr" in caso di situazioni anomale.
*/

typedef struct _nvram_descr_t {
	unsigned int brd;
	unsigned int off;
	unsigned int siz;
	char *lbl;
} nvram_descr_t;

/* "default dei default". E` la configurazione "vecchio stile". */

#define _NVCONF(x) { NVRAM_BRD_##x,NVRAM_OFF_##x,NVRAM_SIZ_##x,NVRAM_LBL_##x }

nvram_descr_t fallback[] = {
	_NVCONF(SERVERDM),
	_NVCONF(AX),
	_NVCONF(PLC480),
	_NVCONF(PLC),
	_NVCONF(GMM),
};

/* Massima lunghezza di una linea del file di configurazione. */
#define MAX_NVRAM_CONF_LINE 200

/* Minima dimensione di un'area. */
#define MIN_AREA_SZ 8

/* Emissione di messaggi. */

static void _AttachNvram_warning(int n, ...)
{
va_list ap;
static struct { char * msg; int warned; } wl[] = {
	{ "using file %s as description\n", 0 },
	{ "file %s not found\n", 0 },
	{ "syntax error in file %s, line %d\n", 0 },
	{ "area %s out of space\n", 0 },
	{ "overlap detected for area %s\n", 0 },
	{ "bad description for area %s\n", 0 },
};

	va_start(ap,n);
	if (! wl[n].warned) {
		wl[n].warned = 1;
		fprintf(stderr, "PID %d : NVRAM : ", getpid());
		vfprintf(stderr,wl[n].msg,ap);
	}
	va_end(ap);
}

/* Verifica che la stringa data sia un commento. */

static int _AttachNvram_is_comment(char *line)
{
char filler;

	return (line[0] == '\0' || line[0] == '\n' ||
	        (sscanf(line," %c",&filler) == 1 && filler == '#') ||
	        (sscanf(line,"%*[ \t]%[\n]",&filler) == 1 && filler));
}

/* Controlla che l'area descritta da "d" non sia gia` occupata. */

static int _AttachNvram_check_area(
                    nvram_descr_t *d,
                    unsigned char bitmap[MAX_IOS_BOARD]
				        [IOS_NVRAM_SZ / (MIN_AREA_SZ * 8)])
{
int i, m_bit, start8, end8, rv;
unsigned char *p;

	if (d -> brd >= MAX_IOS_BOARD
	    || d -> siz == 0 || d -> siz + d -> off > IOS_NVRAM_SZ
	    || d -> off >= IOS_NVRAM_SZ) {

		_AttachNvram_warning(5,d -> lbl);

		d -> off = d -> siz = d -> brd = 0;

		return 0;
	}

	rv = 1;
	start8 = d -> off / MIN_AREA_SZ;
	end8 = start8 + d -> siz / MIN_AREA_SZ;

	for (i = start8; i < end8; ++i) {
		m_bit = 1 << (i % 8);
		p = &bitmap[d -> brd][i / 8];
		if (*p & m_bit) {
			_AttachNvram_warning(4,d -> lbl);
			rv = 0;
		}
		*p |= m_bit;
	}

	return rv;
}

/* Funzione principale. */

int AttachNvram(char *name, nvram_t *area, void *base, int ios)
{
char *path,*p;
FILE *f;
nvram_descr_t d;
char line[MAX_NVRAM_CONF_LINE + 1];
char id[MAX_NVRAM_CONF_LINE + 1];
char tmp[MAX_NVRAM_CONF_LINE + 1];
unsigned char *addr;
char filler;
int brd,off,err,ln,found,fd,i,rv;
struct iosboard info;

/* Cache per registrare gli indirizzi dei dispositivi gia` utilizzati. */
static unsigned char *cache[MAX_IOS_BOARD] = { 0, /*...*/ };
/* Bitmap per trovare le sovrapposizioni di aree. */
static unsigned char bitmap[MAX_IOS_BOARD][IOS_NVRAM_SZ / (MIN_AREA_SZ * 8)];


/* Cerca il percorso per il file di configurazione. */

	path = getenv("NVRAM_CONF");

	if (! path) {

	/* Non c'e` la variabile di shell. Forse converrebbe dirlo... */
		path = "/home/xnc/bin/nvram.conf";

/*
		_AttachNvram_warning(0,path);
*/
	}

/* Apre il file (se c'e` !). */

	f = fopen(path,"r");

	brd = off = err = found = 0;

	if (! f) {

	/* Niente file : conviene avvertire. */

		_AttachNvram_warning(1,path);

	/* In assenza di file, ricava le informazioni dal "default
	 dei default". */

		for (i = sizeof(fallback)/sizeof(fallback[0]); --i >= 0;) {
			if (strcmp(fallback[i].lbl,name) == 0) {
				break;
			}
		}

		if (i >= 0) {

		/* Registra la configurazione in "d". */

			found = 1;

			d = fallback[i];
		}
	}
	else {

	/* Lettura del file di configurazione. */

		ln = 0;
		d.siz = 0;
		d.off = 0;
		d.brd = 0;
		memset((char *)bitmap,0, sizeof(bitmap));

		while (fgets(line, MAX_NVRAM_CONF_LINE, f)) {

			++ln;

		/* Commento o linea vuota. */

			if (_AttachNvram_is_comment(line))
				continue;

		/* Cerca "  Nome Valore" o "   Nome 0xValore". */

			if ((rv = sscanf(line," %[a-zA-Z0-9_] 0x%x%s",
			           id,&d.siz,tmp)) < 2
			 && (rv = sscanf(line," %[a-zA-Z0-9_] %d%s",
			           id,&d.siz,tmp)) < 2) {
			/* Errore di sintassi. Che fare ? */
				if (! err) err = ln;
				continue;
			}

		/* Normalizza la dimensione. Si conviene che debba
		 essere un multiplo di 8 (mi fa risparmiare memoria
		 nella mappa di bit delle aree occupate...). */

			d.siz = ((d.siz + MIN_AREA_SZ - 1) / MIN_AREA_SZ)
			         * MIN_AREA_SZ;
			d.lbl = id;
			if (rv == 2 || _AttachNvram_is_comment(tmp)) {

			/* Linea terminata o commento a seguire. */
			/* Offset e scheda non sono specificati. Si
			 utilizzano i valori correnti. */

				d.off = off;
				d.brd = brd;
			}
			else {

				if (sscanf(line," %*s %*s 0x%x",&d.brd)!=1
				 && sscanf(line," %*s %*s %d",&d.brd)!=1) {
				/* Errore di sintassi... */
					if (! err) err = ln;
					continue;
				}
				if ((rv = sscanf(line," %*s %*s %*s 0x%x%s",
				           &d.off,tmp)) < 1
				 && (rv = sscanf(line," %*s %*s %*s %d%s",
				           &d.off,tmp)) < 1) {
				/* Errore di sintassi... */
					if (! err) err = ln;
					continue;
				}
				if (rv != 1 && ! _AttachNvram_is_comment(tmp)) {
				/* Errore di sintassi... */
					if (! err) err = ln;
					continue;
				}
			/* Offset e scheda sono specificati. Sono questi
			 i nuovi valori correnti. L'offset e` allineato
			 ad un multiplo di 8. */

				d.off = (d.off / MIN_AREA_SZ) * MIN_AREA_SZ;
				off = d.off;
				brd = d.brd;
			}

		/* Controlla le sovrapposizioni. */

			rv = _AttachNvram_check_area(&d,bitmap);

		/* Aggiorna l'offset (e intanto controlla l'area). */

			off += d.siz;
			if (off >= IOS_NVRAM_SZ) {
				++brd;
				if (off > IOS_NVRAM_SZ
				    || (brd >= MAX_IOS_BOARD && off)) {
				/* L'area corrente "sfora".
				 Bisogna segnalarlo. */
					_AttachNvram_warning(3,d.lbl);
				}
				off = 0;
			}

		/* Esce se il nome e` quello cercato. */

			if (strcmp(name,d.lbl) == 0) {

			/* Si` ! E` questo il nome che cerco ! Ma se
			 ci sono sovrapposizioni o valori non
			 credibili, niente ! */

				found = rv;
				break;
			}

		}

		fclose(f);
	}

/* Errori di sintassi ? Segnalare ! */

	if (err)
		_AttachNvram_warning(2,path,err);

/* Insomma, l'abbiamo trovato o no ? */

	if (! found) {

	/* Il nome non c'e`. Pazienza. */

		goto ERROR;

	}

/* Bene. Ora in "d" c'e` la descrizione completa dell'area.
 Adesso bisogna mapparla, oppure vedere se si puo` riciclare una
scheda gia` mappata, o, ancora, utilizzare il puntatore "base". */

	if (base) {

	/* Se "base" non e` NULL, si deve controllare che la scheda "ios"
	 coincida con quella prevista dalla configurazione. */

		if (ios == d.brd) {

		/* Coincide. "base" puo` essere utilizzato. */

			addr = base;
		}
		else {

		/* Spiacente. La scheda mappata da "base" non coincide
		 con quella richiesta dalla configurazione. */

			goto ERROR;
		}
	}
	else {

	/* "base" non e` dato. Se non e` gia` stato fatto, bisognera`
	 mappare il dispositivo IOS. */

		if (! cache[d.brd]) {

		/* Pare che non ci si possa esimere... */

			if (_iosOpen(d.brd, &fd, &info) < 0)
				goto ERROR;

		/* La IOS e` aperta. Ma, la NVRAM, c'e` ? Perche`, senno`
		 errore, e ciao ! */

			if (info.nvram_size < d.off + d.siz) {
				close(fd);
				goto ERROR;
			}

			if (_iosMap(fd, (int *)0, &addr,
			            info.nvram_size, info.nvram_off) < 0)
				goto ERROR;

		}
		else {

		/* Area gia` mappata. Tanto meglio. */

			addr = cache[d.brd];
		}

	}

/* Ora si puo` prendere nota della nuova area mappata. */

	cache[d.brd] = addr;

/* Finalmente ! "addr" punta all'area richiesta. Si produce il
 risultato, e fine. */

	area -> addr = addr + d.off;
	area -> size = d.siz;
	return 0;

ERROR:
	area -> addr = NULL;
	area -> size = 0;
	return -1;
}

