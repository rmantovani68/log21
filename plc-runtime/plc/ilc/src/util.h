/*
* @(#) util.h 1.2 Mon Oct 14 13:02:31 MET 1996
* @(#) Rif. Giuseppe Guerrini
*
*  Funzioni di varia utilita`.
*
* 04/07/96 GG 1.0 Prima stesura.
* 24/07/96 GG 1.1 Aggiunte le funzioni util_stralloc ed util_strcatdup.
* 14/10/96 GG 1.2 Aggiunta la funzione util_memdup.
*/

#ifndef _UTIL_H_

enum {
	UTIL_E_ERRNO,
	UTIL_E_NOMEM,

	UTIL_E_MAX,
};
	
void util_fatal(int n);

/*
* Gestione della memoria
* ----------------------
*
* Si tenta qui di non frammentare troppo la memoria (malloc e` cosi`
* sprecona...).
*/

/*
* Allocazione di strutture generiche, costituite da due campi fissi
* ed un campo fittizio di lunghezza indefinita, che sara` a disposizione
* del chiamante.
*/

typedef struct _memory_block_t {
	int n_byte;
	int n_free;
	struct _memory_block_t *next;
	char data[sizeof(long)]; /* Lunghezza fittizia. */
} memory_block_t;


char *util_malloc(int n);

char *util_alloc(int n, int size, memory_block_t **list);

/*
* Allocazione di stringhe. E` la mia versione di "strdup".
*/

#define LEN_STR_BLOCK 0x2000

char *util_stralloc(int l);

char *util_strdup(char *s);

char *util_memdup(char *s, int n);

char *util_strcatdup(char *s1, char *s2);

#define _UTIL_H_

#endif

