/*
* @(#) util.c 1.5 Thu Jan 25 10:16:38  2001
* @(#) Rif. Giuseppe Guerrini
*
*  Funzioni di varia utilita`.
*
* 04/07/96 GG 1.0 Prima stesura.
* 24/07/96 GG 1.1 Aggiunte le funzioni util_stralloc ed util_strcatdup.
* 14/10/96 GG 1.2 Aggiunta la funzione util_memdup.
* 22/09/97 GG 1.3 Un piccolo aggiustamento per compilazione su NT.
* 20/04/99 GG 1.4 Corretto un imperdonabile buco nella funzione "util_alloc".
*             Restituiva piu` memoria del necessario se applicata a strutture
*             piccole. Incredibile che nessuno se ne fosse accorto!
* 25/01/01 GG 1.5 Un piccolo adattamento per compilazione su CYGWIN32.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "sysdep.h"
#include "conf.h"
#include "util.h"
#include "symtab.h"
#include "ilc.h"

extern char *sys_errlist[];

/*
* Uscita forzata in caso di errori non recuperabili.
*/

static const char *util_fatal_errlist[] = {
	"(errno)",
	"no memory",
};

void util_fatal(int n)
{
	if (n >= UTIL_E_MAX)
		fprintf(stderr,"Internal error %d\n", n);
	else
		fprintf(stderr,"Fatal error : %s\n", n ? util_fatal_errlist[n]
		                                       :
#ifdef __CYGWIN32__
						         strerror(errno)
#else
						         sys_errlist[errno]
#endif
		                                       );
	exit(1);
}

/*
* Gestione della memoria
* ----------------------
*
* Si tenta qui di non frammentare troppo la memoria (malloc e` cosi`
* sprecona...).
*/

char *util_malloc(int n)
{
char *p;

	p = (char *) malloc( (unsigned int)n );
	if (! p)
		util_fatal(UTIL_E_NOMEM);
	memset(p, 0, (unsigned int)n);
	return p;
}

/*
* Allocazione di strutture generiche, costituite da due campi fissi
* ed un campo fittizio di lunghezza indefinita, che sara` a disposizione
* del chiamante.
*/

char *util_alloc(int n, int size, memory_block_t **list)
{
memory_block_t *p;
char *q;
int nb;

	p = *list;

	if (! p || p -> n_free < size) {
		nb = sizeof(*p) + n*size;
		p = (memory_block_t *) util_malloc(nb);
		memset((char *) p, 0, (unsigned int)nb);
		nb -= sizeof(*p);
		p -> n_free = nb;
		p -> n_byte = nb;
		p -> next = *list;
		*list = p;
	}

	q = &(p -> data[p -> n_byte - p -> n_free]);

	p -> n_free -= size;

	return q;
}

/*
* Allocazione di stringhe. E` la mia versione di "strdup".
*/

#define LEN_STR_BLOCK 0x2000

typedef struct _string_block_t {
	char * free;
	int n_free;
	struct _string_block_t *next;
	char data[LEN_STR_BLOCK];
} string_block_t;

static string_block_t  *string_block = (string_block_t *) 0;

char *util_stralloc(int l)
{
string_block_t *p;
char *r;

	if (l > LEN_STR_BLOCK)
		return (char *) 0;
	for (p = string_block; p; p = p -> next) {
		if (l <= p -> n_free) {
			r = p -> free;
			p -> free += l;
			p -> n_free -= l;
			return r;
		}
	}
	p = (string_block_t *) util_malloc(sizeof(*p));
	p -> next = string_block;
	string_block = p;
	r = p -> data;
	p -> n_free = sizeof(p -> data) - l;
	p -> free = r + l;
	return r;
}

char *util_strdup(char *s)
{
char *r;

	r = util_stralloc( (int)(strlen(s) + 1) );
	if (r)
		strcpy(r,s);
	return r;
}

char *util_memdup(char *s, int n)
{
char *r;

	r = util_stralloc(n);
	if (r)
		memcpy(r,s, (unsigned int)n);
	return r;
}

char *util_strcatdup(char *s1, char *s2)
{
char *r;
int l;

	l = strlen(s1);
	r = util_stralloc( (int)(l + strlen(s2) + 1) );
	if (r) {
		strcpy(r,s1);
		strcpy(r + l,s2);
	}
	return r;
}

