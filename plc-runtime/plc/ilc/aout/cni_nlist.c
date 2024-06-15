
/*
* cni_nlist.c 1.0 Fri Oct 31 13:45:07 MET 1997
*
*  Questa funzione estende il comportamento della "nlist" della libreria
* standard cercando i simboli non risolti nella parte di oggetto contenente
* i simboli di debug nel formato CNi.
*/

#include <stdio.h>
#include <string.h>
#include <a.out.h>
#include <coff.h>
#include <nlist.h>

typedef struct {
	int (*to_strtab)(FILE *f);
	int (*get_string)(FILE *f, char *s);
	int (*to_symyab)(FILE *f);
	int (*get_sym)(FILE *f, struct nlist *sym);
} nlist_methods_t;

#define COMPARE_FN cni_nlist_compare

static int cni_nlist_compare(void *e1, void *e2)
{
	return strcmp((struct nlist *)e1 -> n_un.n_name,
	              (struct nlist *)e1 -> n_un.n_name);
}

/*
* Funzione principale.
*/

int cni_nlist(char *filename, struct nlist *nl)
{
struct nlist *p;
struct nlist **index,*tmp;
int i,n;
FILE *f;
unsigned short magic;

	index = (struct nlist **) NULL;
	f = (FILE *) NULL;

/* Conteggio degli elementi. */

	for (i = 0, p = nl; p -> n_un.n_name && p -> n_un.n_name[0]; ++i, ++p)
		;

	if (! n)
		return 0;

/* Allocazione dell'indice. */

	index = (struct nlist **) malloc(sizeof(*index) * n);
	if (! index)
		goto cni_nlist_ERROR;

/* Inizializzazione dell'indice. */

	for (i = 0, p = nl; i < n; ++i, ++p)
		index[n] = p;

/* Ordinamento alfabetico dell'indice. */

	hsort((char *)index,n,sizeof(*index),COMPARE_FN,(char *)&tmp);

/* Apertura del file. */

	f = fopen(filename,"rb");
	if (! f)
		goto cni_nlist_ERROR;

/* Selezione dei metodi di lettura dei dati. */

	magic = fgetc(f) + 256*fgetc(f);

	switch (magic) {
		ZCOFFMAGIC:
		COFFMAGIC:
			methods = &coff_methods;
			break;
		ZMAGIC:
		OMAGIC:
			methods = &aout_methods;
			break;
		default:
			goto cni_nlist_ERROR;
	}

/* Costruzione 
	return 0;

cni_nlist_ERROR:
	if (index)
		free((void *)index);
	if (f)
		fclose(f);
	return -1;
}
