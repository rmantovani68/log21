/*
* @(#) fixaout.c 1.2 Wed Nov 22 12:03:50 CET 2000
*
* Questo programma corregge la rappresentazione della tabella di
* rilocazione di un oggetto a.out prodotto da objcopy.
*
* 31/01/97 GG 1.0 Prima stesura.
* 26/09/97 GG 1.1 Aggiunta la riscrittura del magic number.
* 22/11/00 GG 1.2 Aggiustamenti per ricompilazione su Linux.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef Linux
#include <lynx-a.out.h>
#include <lynx-nlist.h>
#include <sys/stat.h>
#else
#include <a.out.h>
#include <nlist.h>
#include <stat.h>
#endif

static const char VERSIONE[] = "1.2";

static char *iname;
static char *oname;
static FILE *iobj;
static FILE *oobj;
static struct exec aout_header;


static int load_aout_header(void)
{
/* Lettura dell'header a.out. */

	if (fread((char *)&aout_header, sizeof(aout_header), 1, iobj) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

	return 0;
}


static void nonaout(void)
{
	fprintf(stderr,"%s: not in Lynx a.out format.\n",iname);
	exit(1);
}

static void usagerr(void)
{
	fprintf(stderr,"fixaout versione %s\n"
	               "    uso: fixaout obj_in obj_out\n",VERSIONE);
	exit(1); 
}

int main(int argc, char **argv)
{
int n,c;
struct relocation_info rel;
unsigned long t;

	if (argc != 3)
		usagerr();

	iname = argv[1];
	oname = argv[2];

	iobj = fopen(iname,"r");
	if (! iobj) {
		perror(iname);
		exit(1);
	}

/* Lettura dell'header a.out. */

	if (load_aout_header()) {
	/* ERRORE. Problemi nella lettura del file. */
		nonaout();
	}

/* Apertura del file d'uscita. */

	oobj = fopen(oname,"w");
	if (! oobj) {
		perror(oname);
		exit(1);
	}

/* Scrittura dell'header. */

	aout_header.a_magic = OMAGIC;

	if (fwrite((char *)&aout_header,sizeof(aout_header),1,oobj) != 1) {
		perror(oname);
		exit(1);
	}

/* Copia delle sezioni. */

	n = aout_header.a_text + aout_header.a_data;

	while (n--) {
		c = fgetc(iobj);
		if (c == EOF)
			nonaout();
		fputc(c,oobj);
	}

/* Aggiustamento della tabella di rilocazione. */

	n = (aout_header.a_trsize + aout_header.a_drsize)
	    / sizeof(struct relocation_info);

	while (n--) {

	/* Lettura dell'elemento. */

		if (fread((char *)&rel, sizeof(rel), 1, iobj) != 1) {
			perror(iname);
			exit(1);
		}

	/* Aggiustamento. */

		t = ((unsigned long *)&rel)[1];
		((unsigned long *)&rel)[1] = ((t & 0x00FFFFFF) << 8)
		                           | ((t & 0x08000000) >> 23)
		                           | ((t & 0x06000000) >> 20)
		                           | ((t & 0x01000000) >> 17);

	/* Scrittura dell'elemento corretto. */

		if (fwrite((char *)&rel, sizeof(rel), 1, oobj) != 1) {
			perror(oname);
			exit(1);
		}
	}

/* Copia del resto. */

	while ( (c = fgetc(iobj)) != EOF ) {
		fputc(c,oobj);
	}

/* Chiusura dei file (ma non e` finita qui). */

	fclose(oobj);
	fclose(iobj);

	exit(0);

	return 0;
}

