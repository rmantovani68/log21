
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef Linux
#include "lynx-a.out.h"
#include "lynx-nlist.h"
#include <sys/stat.h>
#else
#include <a.out.h>
#include <nlist.h>
#include <stat.h>
#endif

#ifdef Linux
#if 0
extern const struct {
	unsigned long fn;
	char *name;
} qplc_symbol_table[];
extern int qplc_symbol_table_size;

static int nlist_internal(struct nlist *nl)
{
int i;
struct nlist *pl;

	for (pl = nl; pl->n_un.n_name && pl->n_un.n_name[0]; ++pl) {
		pl -> n_type = N_UNDF;
		pl -> n_value = (unsigned long) 0;
		for (i = 0; i < qplc_symbol_table_size; ++i) {
#if 0
printf("nlist: n=\"%s\", m=\"%s\"\n",pl->n_un.n_name,qplc_symbol_table[i].name);
#endif
			if (strcmp(pl->n_un.n_name,qplc_symbol_table[i].name) == 0) {
				pl -> n_type = N_ABS;
				pl -> n_value = (unsigned long) qplc_symbol_table[i].fn;
				break;
			}
		}
	}
	return 1;
}
#else
#include <dlfcn.h>
static int nlist_internal(struct nlist *nl)
{
int i;
struct nlist *pl;
void *dl;


	dl = dlopen(NULL,RTLD_NOW|RTLD_GLOBAL);
	if (!dl) {
	  util_fatal(dlerror());
	 }
	for (pl = nl; pl->n_un.n_name && pl->n_un.n_name[0]; ++pl) {
		pl -> n_type = N_UNDF;
		pl -> n_value = (unsigned long) 0;
		if (dl) {
			pl -> n_value = (unsigned long) dlsym(dl,pl->n_un.n_name);
			if (!dlerror())
				pl -> n_type = N_ABS;
		}
	}
	if (dl)
		dlclose(dl);
	return 1;
}
#endif

static int _dyLoadHeader(FILE * obj, struct exec *header)
{
/* Lettura dell'header a.out. */

	if (fread((char *)header, sizeof(*header), 1, obj) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

	return 0;
}


static struct nlist *SymTab = (struct nlist *) 0;
static char *StrTab = (char *) 0;
static int nSyms = 0;

static int _dyLoadSymTab(FILE * obj, struct exec *header)
{
long st_pos,val;
unsigned char c[4];
char *p;

/* Naturalmente, qualche simbolo ci vuole... */

	if (! header -> a_syms) {
/* ERRORE : neanche un simbolo. */
		return -1;
	}

/* Calcolo della posizione del campo contenente la lunghezza
 della string table. */

	st_pos = header -> a_text + header -> a_data +
			 header -> a_trsize + header -> a_drsize +
			 header -> a_syms + sizeof(*header);

	if (fseek(obj,st_pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

/* Lettura della lunghezza della tabella. */

	if (fread(c, sizeof(c), 1, obj) != 1) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

/* Decifra la lunghezza (precauzione forse eccessiva). */

	val = c[0] + c[1]*0x100 + c[2]*0x10000 + c[3]*0x1000000;

/* Allocazione dell'area necessaria a contenere simbolo e nomi. */

	val += header -> a_syms;
	p = malloc((size_t)val);
	if (! p) {
/* ERRORE : non c'e` abbastanza memoria. */
		return -1;
	}

/* Calcolo della posizione della symbol table. */

	st_pos = header -> a_text + header -> a_data +
			 header -> a_trsize + header -> a_drsize +
			 sizeof(*header);

/* Posizionamento e lettura di simboli e nomi. */

	if (fseek(obj,st_pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		free(p);
		return -1;
	}

	if (fread(p, (size_t) val, 1, obj) != 1) {
/* ERRORE : qualcosa e` andato storto. */
		free(p);
		return -1;
	}

/* "ufficializza" il risultato. */

	SymTab = (struct nlist *) p;
	StrTab = p + header -> a_syms;
	nSyms = header -> a_syms / sizeof(struct nlist);

	return 0;
}

static void _dyFreeSymTab(void)
{
	if (SymTab)
		free(SymTab);

	SymTab = (struct nlist *) 0;
	StrTab = (char *) 0;
	nSyms = 0;
}


static int nlist_file(char *objname, struct nlist *nl)
{
FILE *obj;
struct exec header;
int i;
struct nlist *pl;

/* Apertura dell'oggetto. */

	obj = fopen(objname,"r");
	if (! obj) {
	/* ERRORE. File inesistente o inaccessibile. */
		goto ERROR;
	}

/* Lettura dell'header a.out. */

	if (_dyLoadHeader(obj,&header)) {
	/* ERRORE. Problemi nella lettura del file. */
		goto ERROR;
	}

/* Controlla il formato. */

	if (header.a_magic != OMAGIC) {
	/* ERRORE. Formato dell'oggetto imprevisto. */
		goto ERROR;
	}

/* Carica la symbol table. */

	if (_dyLoadSymTab(obj,&header)) {
	/* ERRORE. Problemi nella lettura della symbol table. */
		goto ERROR;
	}

	for (pl = nl; pl->n_un.n_name && pl->n_un.n_name[0]; ++pl) {
		pl -> n_type = N_UNDF;
		pl -> n_value = (unsigned long) 0;
		for (i = 0; i < nSyms; ++i) {
			if (strcmp(pl->n_un.n_name,StrTab+SymTab[i].n_un.n_strx) == 0) {
				pl -> n_type = SymTab[i].n_type;
				pl -> n_value = SymTab[i].n_value;
				break;
			}
		}
	}

	_dyFreeSymTab();

ERROR:
	if (obj)
		fclose(obj);
	return -1;
}

int nlist(char *path, struct nlist *nl)
{

	if ((! path) || (! path[0])) {
		return nlist_internal(nl);
	}
	else {
		return nlist_file(path,nl);
	}
}
#endif

