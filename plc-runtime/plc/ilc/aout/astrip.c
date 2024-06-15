/*
* @(#) astrip.c 1.4 Wed Nov 22 12:02:44 CET 2000
*
* Questo programma, dato un oggetto in formato Lynx a.out, genera due file,
* il primo dei quali contiene lo stesso oggetto privato dei simboli il cui
* nome inizia per  "$", il secondo contiene solo la parte della symbol table
* rimossa. Si fa l'ipotesi che i simboli "$..." non partecipino attivamente
* alla costruzione del programma, siano locali e definiti.
*
* 02/12/96 GG 1.0 Prima stesura.
* 03/12/96 GG 1.1 Ora sono considerati anche i simboli locali con
*             riferimento "TEXT". E` poi stato corretto un "buco"
*             nella raccolta dei simboli di debug.
* 05/12/96 GG 1.2 Incredibile ! non funziona ancora. Buco grande come una casa
*             nel riempimento della tabella di traslazione.
* 31/01/96 GG 1.3 Aggiunto un truccaccio per togliermi di mezzo i simboli che
*             cominciano per ".", che sono i nomi delle sezioni derivati
*             dalla conversione da COFF ad A.OUT fatta con objcopy.
* 22/11/00 GG 1.4 Aggiustamenti per ricompilazione su Linux e Win32.
*             Rigenerato il progetto per Win32.
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
#endif

static const char VERSIONE[] = "1.4";

static struct nlist *SymTab = (struct nlist *) 0;
static char *StrTab = (char *) 0;
static long *TransTab = (long *) 0;
static long nSyms = 0;
static long nElimSyms = 0;
static char *iname;
static char *oname;
static char *dname;


static void memswap(char *p, char *q, int n)
{
register char tmp;
register int i;

	for (i = n; --i >= 0;) {
		tmp = *p;
		*(p++) = *q;
		*(q++) = tmp;
	}
}

static int load_header(FILE * obj, struct exec *header)
{
/* Lettura dell'header a.out. */

	if (fread((char *)header, sizeof(*header), 1, obj) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

	return 0;
}

static long strtab_len(FILE * obj, struct exec *header)
{
long st_pos,val;
unsigned char c[4];

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

	return c[0] + c[1]*0x100 + c[2]*0x10000 + c[3]*0x1000000;
}

static int load_symtab(FILE * obj, struct exec *header)
{
long st_pos,val;
unsigned char c[4];
char *p;
long *ptab;
long i,n;

	val = strtab_len(obj,header);
	if (val == -1) {
		return -1;
	}

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

/* Calcola il numero di simboli. */

	n = header -> a_syms / sizeof(struct nlist);

/* Alloca la tabella di permutazione, e la inizializza. */

	ptab = (long *) malloc(n * sizeof(*ptab));
	if (! ptab) {
/* ERRORE : qualcosa e` andato storto. */
		free(p);
		return -1;
	}

	for (i = 0; i < n; ++i)
		ptab[i] = i;

/* "ufficializza" il risultato. */

	SymTab = (struct nlist *) p;
	StrTab = p + header -> a_syms;
	TransTab = ptab;
	nSyms = n;

	return 0;
}

static void collect_deb_syms(void)
{
long i;
long primo;
long lunb;
long t;
int type;
struct nlist *p;

	primo = 0;
	lunb = 0;
	for (i = 0; i < nSyms; ++i) {
		p  = &SymTab[i];
		type  = (p -> n_type) & N_TYPE;
		if (((type == N_ABS || type == N_TEXT)
		     && *(StrTab + p -> n_un.n_strx) == '$')
		/* Unica modifica introdotta con la versione 1.3.
		 Togliere la linea seguente se non va un tubo. */
		    || (*(StrTab + p -> n_un.n_strx) == '.')) {
			if (! (lunb++))
				primo = i;
			++nElimSyms;
		}
		else if (lunb) {
			memswap((char *)p,
			        (char *)&SymTab[primo],
			        sizeof(*p));
		/* Algoritmo semplificato. Si registrano solo le
		 corrispondenze tra vecchie e nuove posizioni dei simboli
		 che devono rimanere. Questo evita di mantenere due tabelle,
		 oppure di esplorare a ritroso la tabella per aggiornare anche
		 gli indici dei simboli da eliminare, che richiede un tempo
		 di elaborazione, nel caso peggiore, quadratico. Tutto
		 funziona perche` si e` fatta l'ipotesi che i simboli da
		 eliminare non abbiano riferimenti nella tabella di
		 rilocazione (senno`, non sarebbero eliminalbili !). */
			TransTab[i] = primo;
			++primo;
		}
	}
}

static int copy_data(FILE *dest, FILE *src, int dataseg, struct exec *header)
{
long i,n,pos;

	pos = sizeof(*header);
	n = header -> a_text;
	if (dataseg) {
		pos += n;
		n = header -> a_data;
	}

/* Posizionamento e lettura del blocco di dati. */

	if (fseek(src,pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

	for (i = 0; i < n; ++i)
		fputc(fgetc(src),dest);

	return 0;
}

static int fix_and_copy_reltab(FILE *dest, FILE *src, struct exec *header)
{
long i,pos,n;
struct relocation_info rel;
char *section;

/* Calcolo della posizione di intervento nel codice (text o data),
  del numero di elementi nella tabella di rilocazione,
  e della posizione della tabella di rilocazione nel file. */

	pos = header -> a_text + header -> a_data + sizeof(*header);
	n = (header -> a_trsize + header -> a_drsize)
	    / sizeof(struct relocation_info);

/* Posizionamento all'inizio della tabella di rilocazione. */

	if (fseek(src, pos, 0))
		return -1;

	for (i = 0; i < n; ++i) {

	/* Lettura di un elemento. */

		fread((char *)&rel, sizeof(rel), 1, src);

		if (rel.r_extern) {

		/* Riferimento ad un simbolo. Ne corregge l'indice. */

			rel.r_symbolnum = TransTab[rel.r_symbolnum];
		}

	/* Scrittura dell'elemento corretto. */

		fwrite((char *)&rel, sizeof(rel), 1, dest);

	}

	return 0;
}

static void write_syms_and_strings(FILE *obj, long pos, long ns)
{
long i,len,n;
long strl;
char *p;

	if (! ns)
		return;

	strl = sizeof(long);

	n = ns + pos;

/* Riassegna le posizioni alle stringhe. Ricicla la tabella
 di conversione per memorizzarvi le vecchie posizioni. */

	for (i = pos; i < n; ++i) {
		len = strlen(StrTab + (TransTab[i]=SymTab[i].n_un.n_strx)) + 1;
		SymTab[i].n_un.n_strx = strl;
		strl += len;
	}

/* Scrive il tratto di symbol table. */

	fwrite((char *)&SymTab[pos],sizeof(SymTab[0]),ns,obj);

/* Scrive la lunghezza della string table. */

	fwrite((char *)&strl,sizeof(strl),1,obj);

/* Scrive la string table. */

	for (i = pos; i < n; ++i) {
		p = StrTab + TransTab[i];
		do {
			fputc(*p,obj);
		} while (*(p++));
	}

}

static int copy_extension(FILE *dest, FILE *src, struct exec *header)
{
long pos;
int c;

	pos = strtab_len(src,header);
	pos += header -> a_text + header -> a_data +
			 header -> a_trsize + header -> a_drsize +
			 header -> a_syms + sizeof(*header);

/* Posizionamento e lettura del blocco di dati. */

	if (fseek(src,pos,0)) {
/* ERRORE : qualcosa e` andato storto. */
		return -1;
	}

	while ( (c = fgetc(src)) != EOF )
		fputc(c,dest);

	return 0;
}

static void nonaout(char *s)
{
	fprintf(stderr,"%s: not in Lynx a.out format.\n");
	exit(1);
}

static void usagerr(void)
{
	fprintf(stderr,"astrip versione %s\n"
	               "    uso: astrip obj_in obj_out [obj_deb]\n"
	               " oppure: astrip -d obj_in obj_deb\n",VERSIONE);
	exit(1); 
}

int main(int argc, char **argv)
{
FILE *ifile,*ofile,*dfile;
struct exec header;
struct exec oheader;
int append,extract;

	if (argc < 3)
		usagerr();

	iname = argv[1];
	oname = argv[2];
	dname = argv[3];

	if (! dname || strcmp(dname,oname) == 0)
		append = 1;
	else
		append = 0;

	if (strcmp(iname,"-d") == 0) {
		if (append)
			usagerr();
		iname = oname;
		extract = 1;
	}
	else {
		extract = 0;
	}

/* Apertura dell'oggetto. */

	ifile = fopen(iname,"rb");
	if (! ifile) {
	/* ERRORE. File inesistente o inaccessibile. */
		perror(iname);
		exit(1);
	}

/* Lettura dell'header a.out. */

	if (load_header(ifile,&header)) {
	/* ERRORE. Problemi nella lettura del file. */
		nonaout(iname);
	}

/* Controlla il formato. */

	if (header.a_magic != OMAGIC) {
	/* ERRORE. Formato dell'oggetto imprevisto. */
		nonaout(iname);
	}

/* Se c'e` "-d", non fa altro che copiare la "coda" di "iname" in "dname". */

	if (extract) {
	/* Generazione dell'oggetto che contiene solo i simboli di debug. */

		dfile = fopen(dname,"wb");
		if (! dfile) {
		/* ERRORE. File inesistente o inaccessibile. */
			perror(dname);
			exit(1);
		}

		copy_extension(dfile,ifile,&header);

		fclose(ifile);
		fclose(dfile);

		exit(0);
	}

/* Carica la symbol table. */

	if (load_symtab(ifile,&header)) {
	/* ERRORE. Problemi nella lettura della symbol table. */
		nonaout(iname);
	}

/* Riorganizza i simboli, spostando in fondo alla symbol table
 quelli che iniziano per "$". "TransTab" conterra` le corrispondenze
 tra vecchio e nuovo ordinamento. "nElimSyms" il numero di simboli
 eliminabili. */

	collect_deb_syms();

/* Riproduce header, codice e dati nel file d'uscita. */

	ofile = fopen(oname,"wb");
	if (! ofile) {
	/* ERRORE. File inesistente o inaccessibile. */
		perror(oname);
		exit(1);
	}

	oheader = header;
	oheader.a_syms -= nElimSyms * sizeof(struct nlist);

	fwrite((char *)&oheader,sizeof(oheader),1,ofile);

	copy_data(ofile,ifile,0,&header);
	copy_data(ofile,ifile,1,&header);

/* Corregge e scrive la tabella di rilocazione. */

	fix_and_copy_reltab(ofile, ifile, &header);

/* Il file di ingresso non serve piu`. */

	fclose(ifile);

/* Scarica i simboli rimasti e le corrispondenti stringhe, riassegnando
 gli indici di stringa. */

	write_syms_and_strings(ofile, 0, nSyms - nElimSyms);

	if (append) {

	/* Se il nome dell'oggetto "strippato" e quello del file
	 dei simboli di debug coincidono, le informazioni di debug sono
	 accodate. */

		dfile = ofile;

	}
	else {

	/* L'oggetto "strippato" e` completato. */

		fclose(ofile);

	/* Generazione dell'oggetto che contiene solo i simboli di debug. */

		dfile = fopen(dname,"wb");
		if (! dfile) {
		/* ERRORE. File inesistente o inaccessibile. */
			perror(dname);
			exit(1);
		}
	}

	oheader.a_text = oheader.a_data = oheader.a_bss = oheader.a_entry
	               = oheader.a_trsize = oheader.a_drsize = 0;

	oheader.a_magic = OMAGIC;
	oheader.a_syms = nElimSyms * sizeof(struct nlist);

	fwrite((char *)&oheader,sizeof(oheader),1,dfile);

/* Scarica i simboli "eliminati" e le corrispondenti stringhe, riassegnando
 gli indici di stringa. */

	write_syms_and_strings(dfile, nSyms - nElimSyms, nElimSyms);

	fclose(dfile);

	exit(0);

	return 0;
}


