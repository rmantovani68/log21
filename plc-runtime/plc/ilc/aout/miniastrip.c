/*
* @(#) miniastrip.c 1.0 Tue Dec  2 15:21:34 MET 1997
*
* 02/12/96 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <a.out.h>
#include <nlist.h>
#include <stat.h>

static const char VERSIONE[] = "1.0";

struct nlist *SymbolTable = (struct nlist *) 0;
char *StringTable = (char *) 0;
long NumeroSimboli = 0;


static long strtab_len(FILE * src, struct exec *header, long base)
{
long st_pos,val;

/* Naturalmente, qualche simbolo ci vuole... */

	if (! header -> a_syms) {
/* ERRORE : neanche un simbolo. */
		return -1;
	}

/* Calcolo della posizione del campo contenente la lunghezza
 della string table. */

	st_pos = header -> a_text + header -> a_data +
	         header -> a_trsize + header -> a_drsize +
		 header -> a_syms + sizeof(*header) + base;

	if (fseek(src,st_pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

/* Lettura della lunghezza della tabella. */

	if (fread((char *)&val, sizeof(val), 1, src) != 1) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

	return val;
}

static int load_extension(FILE *src)
{
long pos,val;
char *p;
int n;
struct exec header;

/* Lettura dell'header a.out. */

	if (fread((char *)&header, sizeof(header), 1, src) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

/* Controlla il formato. */

	if (header.a_magic != OMAGIC) {
/* ERRORE. Formato dell'oggetto imprevisto. */
		return -1;
	}

	pos = strtab_len(src,&header,0);
	pos += header.a_text + header.a_data +
	       header.a_trsize + header.a_drsize +
	       header.a_syms + sizeof(header);

/* Posizionamento all'inizio dell'estensione di debug. */

	if (fseek(src,pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

/* BENE, SIAMO ALL'INIZIO DEL BLOCCO DI DEBUG. */

/* Lettura dell'header a.out secondario. */

	if (fread((char *)&header, sizeof(header), 1, src) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

/* Lettura della lunghezza della string table secondaria. */

	val = strtab_len(src,&header,pos);
	if (val == -1) {
		return -1;
	}

/* Allocazione dell'area necessaria a contenere simboli e nomi. */

	val += header.a_syms;
	p = malloc((size_t)val);
	if (! p) {
/* ERRORE : non c'e` abbastanza memoria. */
		return -1;
	}

/* Riposizionamento e lettura del blocco di dati. */

	pos += sizeof(header);

	if (fseek(src,pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

/* Lettura della symbol tabel + string table. */

	if (fread(p, (int) val, 1, src) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

/* Risultato : */

	NumeroSimboli = header.a_syms / sizeof(struct nlist);
	SymbolTable = (struct nlist *) p;
	StringTable = p + header.a_syms;

	return 0;
}

void main(int argc, char **argv)
{
FILE *ifile;
int i;

	if (! argv[1]) {
		fprintf(stderr,"Vorrei un file. Grazie.\n");
		exit(1);
	}

	ifile = fopen(argv[1],"r");

	if (! ifile) {
		fprintf(stderr,
		        "Il file %s non e` leggibile. Dinne un'altra.\n",
		        argv[1]);
		exit(1);
	}

	if (load_extension(ifile) == -1) {
		fprintf(stderr,"Il file %s contiene schifezze.\n",argv[1]);
		exit(1);
	}

	fclose(ifile);

	printf("Il file contiene %ld simboli di debug.\nEccoli:\n",
	       NumeroSimboli);
	for (i = 0; i < NumeroSimboli; ++i) {
		printf("%08lx %02x %s\n",
		       SymbolTable[i].n_value,
		       SymbolTable[i].n_type,
		       &StringTable[SymbolTable[i].n_un.n_strx]);
	}

	exit(0);
}


