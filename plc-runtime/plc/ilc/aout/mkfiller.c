/*
* @(#) mkfiller.c 1.0 Wed Nov 11 13:58:28 MET 1998
*
* Questo programma crea un oggetto COFF contenente una sezione .text
* di dimensioni date e composta da byte aventi il valore indicato.
*/

#include <stdio.h>
#include <types.h>
#include <timeb.h>
#include <errno.h>
#include <coff.h>

#define VERSIONE "1.0"

static void usagerr(void)
{
	fprintf(stderr,"mkfiller %s: usage: mkfiller <file> <[0x]size> [<[0x]fill>]\n", VERSIONE);
	exit(1);
}

void main(int argc, char **argv)
{
int filler;
int textlen;
char *file;
FILE *f;
struct filehdr hdr;

	if (argc < 2 || argc > 4)
		usagerr();
	if ((sscanf(argv[2],"0x%x",&textlen) != 1
	  && sscanf(argv[2],"%u",&textlen) != 1)
	 || textlen <= 0)
		usagerr();
	if (argv[3]) {
		if ((sscanf(argv[3],"0x%x",&filler) != 1
		  && sscanf(argv[3],"%u",&filler) != 1)
		 || filler < 0 || filler > 255)
			usagerr();
	}
	else {
		filler = 0x90; /* NOP */
	}

	file = argv[1];

	f = fopen(file,"w");
	if (! f) {
		perror(file);
		exit(1);
	}

	hdr.f_magig = COFFMAGIC;
	hdr.f_nscns = 1;
	hdr.f_timdat = time((long *)NULL);
	hdr.f_symptr = 0;
	hdr.f_nsyms = 0;
	hdr.f_opthdr = 0;
	hdr.f_flags = 0x104;

	if (fwrite((char *)&hdr,sizeof(hdr),1,f) != 1) {
		perror(file);
		exit(1);
	}

	fclose(f);
}

