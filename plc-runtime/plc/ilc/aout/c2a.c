/*
* @(#) c2a.c 1.2 Thu Nov 23 10:50:28 MET 2000
*
* Convertitore di formato oggetto da COFF a Lynx-a.out, con estrazione
* delle informazioni somboliche alla moda di "astrip" e risoluzione,
* quando possibile, dei riferimeti interni.
*
* 28/10/97 GG 1.0 Prima stesura.
* 22/11/00 GG 1.1 Aggiustamenti per ricompilazione su Linux.
* 23/11/00 GG 1.2 Aggiunto un azzeramento dei campi inutilizzati dei
*             descrittori di rilocazione, cosi` il binario prodotto e` sempre
*             lo stesso.
*             Rigenerato il progetto per Win32.
*/

#if 0
#define DEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef Linux
#include <lynx-coff.h>
#include <lynx-a.out.h>
#else
#include <coff.h>
#include <a.out.h>
#endif
/*
#include <nlist.h>
*/

static const char VERSIONE[] = "1.2";

/* Per rappezzare un buco del sistema di sviluppo (o dei .h di Lynx...) */

#define sizeof_coff_rel (sizeof(coff_rel.r_vaddr) \
                        +sizeof(coff_rel.r_symndx) \
                        +sizeof(coff_rel.r_type))
#define sizeof_coff_sym (sizeof(coff_sym._n) \
                        +sizeof(coff_sym.n_value) \
                        +sizeof(coff_sym.n_scnum) \
                        +sizeof(coff_sym.n_type) \
                        +sizeof(coff_sym.n_sclass) \
                        +sizeof(coff_sym.n_numaux))
 
/* Indice di nome speciale per indicere che il nome, in effetti,
 non interessa. */
#define NULL_NAME 0
/* Indice fittizio da assegnare temporaneamente ai simboli
 durante la prima passata. */
#define DUMMY_NAME 3 /* Perche` 3 ? Perche` punta al byte piu` significativo
                      di un long, valore probabilmente nullo. La stringa
                      risultante, nel malaugurato caso che al secondo passo
                      vi fossero ancora simboli associati a questo nome,
                      sarebbe con ogni probabilita` la stringa vuota. */

 
/* File COFF originale. */
/* nome */
static char *ifile;
/* file handle */
static FILE *iobj;

/* File A.OUT. */
/* nome */
static char *ofile;
/* file handle */
static FILE *oobj;

/* File A.OUT per le informazioni id debug aggiuntive. */
/* nome. Puo` coincidere con il precedente. */
static char *aofile;
/* file handle. Puo` coincidere con il precedente.  */
static FILE *aoobj;

/* Flag che abilita l'aggiunta delle informazioni estese per "astrip". */
static int save_astrip_info;
/* Flag che abilita l'emissine di warning. */
static int warning_on;
/* Flag che abilita la rimozione del carattere "_" all'inizio dei nomi. */
static int remove_underscore;
/* Flag di rimozione dei simboli di debug IL (cominciano per "$"). */
static int remove_debug;
/* Flag di rimozione di tutti i simboli inutili. */
static int remove_unused;

/* Strutture di lavoro. */

/* Header del file in ingresso ed in uscita. */
static struct filehdr coff_header;
static struct aouthdr optaout_header;
static struct exec aout_header;

/* Copia in memoria della tabella di rilocazione e sua lunghezza. */
static struct relocation_info *rel_tab;
static int rel_tab_len = 0;

/* Copia in memoria della symbol table originale e sua lunghezza. */
static struct nlist *sym_tab;
static int sym_tab_len = 0;
static int sym_tab_reduced_len = 0;
static int sym_tab_remaining_len = 0;

/* Copia in memoria delle string table, sua lunghezza totale,
 lunghezza della parte originale,
 e lunghezza della parte aggiunta dalla conversione in a.out. */
static char *str_tab;
static long str_tab_len = 0;
static long str_tab_orig_len = 0;
static long str_tab_add_len = 0;
/* Copia in memoria delle string table risultatnti e loro lunghezze. */
static char *str_tab_reduced;
static long str_tab_reduced_len = 0;
static char *str_tab_remaining;
static long str_tab_remaining_len = 0;

/* Tabella di conteggio dei riferimenti simbolici. */
static long *ref_tab;

/* Tabelle diretta e inversa di permutazione dei simboli
 dopo il riordinamento. */
static long *tr_tab;
static long *itr_tab;

/* Tabella delle sezioni "ammissibili". */

struct ldsect_t {
	int index;
	struct scnhdr header;
};
static struct ldsect_t good_sect[] = {
{ -1, { ".text", 0, 0, 0, 0, 0, 0, 0, 0, 0, } },
{ -1, { ".data", 0, 0, 0, 0, 0, 0, 0, 0, 0, } },
{ -1, { ".bss", 0, 0, 0, 0, 0, 0, 0, 0, 0, } },
};
#define N_GOOD_SECT (sizeof(good_sect)/sizeof(good_sect[0]))
#define SECT_TEXT (good_sect[0])
#define SECT_DATA (good_sect[1])
#define SECT_BSS (good_sect[2])

/*
* Funzioncine di utilita`
*/

/* Apertura di file + messaggio di errore. */
static FILE *c2a_open_file(const char *name, const char *mode)
{
FILE *f;

	f = fopen(name,mode);
	if (! f) {
		perror(name);
		exit(1);
	}
	return f;
}

/* Erroraccio di lettura o di posizionamento. */
static void c2a_readerr(void)
{
	if (errno == 0)
		fprintf(stderr,"%s: end of file unexpected.\n",ifile);
	else
		perror(ifile);
	exit(1);
}

/* Posizionamento assoluto sul file in lettura. */
static void c2a_seek(long pos)
{
	if (fseek(iobj,pos,0))
		c2a_readerr();
}

/* Lettura di un insieme di byte. */
static void c2a_read(char *dest, unsigned int size)
{
	if (fread(dest, size, 1, iobj) != 1)
		c2a_readerr();
}

/* Scrittura di un insieme di byte. */
static void c2a_write(char *src, unsigned int size)
{
	if (! src || ! size)
		return;
	if (fwrite(src, size, 1, oobj) != 1) {
		if (errno == 0)
			fprintf(stderr,"%s: write error.\n",ofile);
		else
			perror(ofile);
		exit(1);
	}
}

/* Allocazione dinamica + errore. */
static char *c2a_alloc(unsigned int size)
{
char *p;

	p = (char *) malloc(size);
	if (! p) {
		fprintf(stderr,"c2d: no free memory.\n");
		exit(1);
	}
	memset(p,0,size);
	return p;
}

/* Liberazione di un'area. */
static void c2a_free(char *p)
{
	if (p)
		free(p);
}

/* Errore di formato in ingresso. */
static void c2a_badcoff(void)
{
	fprintf(stderr,"Bad COFF format.\n");
	exit(1);
}

/* Avvertimenti. */
static void c2a_warning_d(char *s, long d)
{
	if (warning_on) {
		printf("Warning: ");
		printf(s,d);
		printf("\n");
	}
}
static void c2a_warning_s(char *s, char * d)
{
	if (warning_on) {
		printf("Warning: ");
		printf(s,d);
		printf("\n");
	}
}


/* Line di comando scorretta. */
static void c2a_usagerr(void)
{
	fprintf(stderr,
"c2a versione %s\n"
"  Convertitore di oggetti i386-COFF in Lynx-i386-A.OUT con rimozione\n"
" opzionale dei simboli superflui e creazione della sezione di debug\n"
" nel formato CNi.\n"
"  Uso: c2a [-opzioni] <file_coff> [-o] <file_a.out>\n"
"  Opzioni:\n"
"   -c : Rimuove o sposta nella sezione di debug tutti i simboli di debug\n"
"        generati dal compilatore IL (simboli locali inizianti con \"$\").\n"
"        Quest'opzione e` attiva per default.\n"
"   -d file : Specifica il file contenente la sezione di debug. Se \"file\"\n"
"        e` \"-\", non e` prodotta alcuna sezione di debug. Se l'opzione\n"
"        non e` specificata affatto, la sezione di debug e` accodata al\n"
"        file a.out prodotto.\n"
"   -k : Non rimuove alcun simbolo dalla symbol table.\n"
"   -n : Rimuove il carattere \"_\" all'inizio dei nomi.\n"
"   -o file : specifica il nome dell'oggetto a.out. Obbligatoria.\n"
"   -u : Rimuove o sposta nella sezione di debug tutti i simboli non\n"
"        necessari al caricamento dell'oggetto.\n"
"   -w : Emette alcuni messaggi di avvertimento in situazioni anomale.\n"
	,
	VERSIONE);

	exit(1);
}

/*
* Interpretazione della linea di comando.
*/
static void process_command_line(int argc, char **argv)
{
int i;
char *p,*s;

	warning_on = 0;
	remove_underscore = 0;
	save_astrip_info = 1;
	remove_debug = 1;
	remove_unused = 0;
	ifile = NULL;
	ofile = NULL;
	aofile = NULL;

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			p = argv[i] + 1;
			do {
				switch (*p) {

				case '\0':
					if (p == argv[i] + 1)
						c2a_usagerr();
					break;

				case 'c':
/*
   -c : Rimuove o sposta nella sezione di debug tutti i simboli di debug
        generati dal compilatore IL (simboli locali inizianti con "$").
        Quest'opzione e` attiva per default.
*/
					remove_debug = 1;
					remove_unused = 0;
					break;

				case 'd':
/*
   -d file : Specifica il file contenente la sezione di debug. Se "file"
        e` "-", non e` prodotta alcuna sezione di debug. Se l'opzione
        non e` specificata affatto, la sezione di debug e` accodata al
        file a.out prodotto.
*/
					if (p[1]) {
						s = ++p;
						p += strlen(p);
					}
					else {
						s = argv[++i];
					}
					if (s == NULL || aofile)
						c2a_usagerr();
					else if (strcmp(s,"-") == 0)
						save_astrip_info = 0;
					aofile = s;
					break;

				case 'k':
/*
   -k : Non rimuove alcun simbolo dalla symbol table.
*/
					remove_debug = 0;
					remove_unused = 0;
					break;

				case 'n':
/*
   -n : Rimuove il carattere "_" all'inizio dei nomi.
*/
					remove_underscore = 1;
					break;

				case 'o':
/*
   -o file : specifica il nome dell'oggetto a.out. Obbligatoria.
*/
					if (p[1]) {
						s = ++p;
						p += strlen(p);
					}
					else {
						s = argv[++i];
					}
					if (s == NULL || ofile)
						c2a_usagerr();
					ofile = s;
					break;

				case 'u':
/*
   -u : Rimuove o sposta nella sezione di debug tutti i simboli non
        necessari al caricamento dell'oggetto.
*/
					remove_debug = 0;
					remove_unused = 1;
					break;

				case 'w':
/*
   -w : Emette alcuni messaggi di avvertimento in situazioni anomale.
*/
					warning_on = 1;
					break;

				default:
					c2a_usagerr();
					break;
				}
			} while (*(p++));
		}
		else {
			if (! ifile)
				ifile = argv[i];
			else if (! ofile)
				ofile = argv[i];
			else
				c2a_usagerr();
		}
	}

	if (! ifile && ! ofile)
		c2a_usagerr();
}

/*
* Apertura del file di ingresso.
*/
static void open_coff_file(void)
{
	iobj = c2a_open_file(ifile,"rb");
}

/*
* Caricamento dei descrittori di sezione.
*/
static void load_coff_section_headers(void)
{
int i,secn;
struct scnhdr s_header,*s;
char sn[sizeof(s_header.s_name)+1];

	for (i = 0; i < coff_header.f_nscns; ++i) {

	/* Lettura del descrittore di sezione. */

		c2a_read((char *)&s_header, sizeof(s_header));

	/* Ricerca del nome nella tabella delle sezioni ammesse. */

		for (secn = N_GOOD_SECT; secn--; ) {
			if (strncmp(s_header.s_name,
			            good_sect[secn].header.s_name,
			            sizeof(s_header.s_name)) == 0) {
				break;
			}
		}

	/* Copia il nome in una stringa terminante con '\0' (il campo
	 originale puo` non avere il '\0' finale. Comunque questo e` solo
	 un problema di visualizzazione). */
		memcpy(sn,s_header.s_name,sizeof(s_header.s_name));
		sn[sizeof(sn)-1] = '\0';

		if (secn >= 0) {

		/* Trovata la sezione. Si procede. */

			if (good_sect[secn].index != -1) {
				c2a_warning_s("duplicated section \"%s\".",sn);
			}
			else {
				good_sect[secn].header = s_header;
				good_sect[secn].index = i;
			}
		}
		else {
			c2a_warning_s("cannot translate section \"%s\".", sn);
		}
	}
}

/*
* Caricamento dell'header e controllo di congruenza.
*/
static void load_coff_header(void)
{

/* Legge l'header COFF. */
	c2a_read((char *)&coff_header, sizeof(coff_header));
	if (coff_header.f_magic != COFFMAGIC
	 && coff_header.f_magic != ZCOFFMAGIC) {
		c2a_badcoff();
	}

/* Toglie di mezzo l'header opzionale, se necessario. */
	if (coff_header.f_opthdr != 0) {
		c2a_read((char *)&optaout_header, sizeof(optaout_header));
		if (optaout_header.magic != OMAGIC)
			c2a_badcoff();
	}

/* Legge i descrittori di sezione ed isola quelli interessanti. */
	load_coff_section_headers();
}

/*
* Caricamento della tabella di rilocazione della sezione indicata
*/
static void load_coff_reltab_as_aout(struct ldsect_t *sect,
                                     struct relocation_info *p)
{
int i;
struct reloc coff_rel;
struct relocation_info aout_rel;

/* Se la sezione non contiene una tabella di rilocazione, esce. */
	if (!sect -> header.s_relptr || !sect -> header.s_nreloc)
		return;

/* Posizionamento all'inizio della tabella della sezione. */

	c2a_seek(sect -> header.s_relptr);

/* Lettura degli elementi, traduzione (provvisoria) e copia in memoria
 nella forma a.out provvisoria. */

/* Azzeramento preventivo, per rendere prevedibili i campi inutilizzati. */
	memset((char *)&aout_rel,0,sizeof(aout_rel));

	for (i = 0; i < sect -> header.s_nreloc; ++i) {

		c2a_read((char *)&coff_rel, sizeof_coff_rel);

	/* Incrementa il contatore di riferimento del simbolo coinvolto. */
		++ref_tab[coff_rel.r_symndx];

		aout_rel.r_address = coff_rel.r_vaddr;
		aout_rel.r_symbolnum = coff_rel.r_symndx;
		aout_rel.r_extern = 1;

		switch (coff_rel.r_type) {
		case R_DIR16:
			aout_rel.r_pcrel = 0;
			aout_rel.r_length = 1;
			break;
		case R_REL16:
			aout_rel.r_pcrel = 1;
			aout_rel.r_length = 1;
			break;
		case R_DIR32:
			aout_rel.r_pcrel = 0;
			aout_rel.r_length = 2;
			break;
		case R_PCRLONG:
			aout_rel.r_pcrel = 1;
			aout_rel.r_length = 2;
			break;
		default:
			aout_rel.r_pcrel = 0;
			aout_rel.r_length = 2;
			c2a_warning_d("unknown relocation type 0x%02lx.",
			            (long)coff_rel.r_type);
			break;
		}

		*p = aout_rel;
		++p;
	}
}

/*
* Caricamento delle tabelle di rilocazione con conversione in a.out.
*  Questa funzione crea anche la tabella di conteggio dei riferimenti
* ai simboli. Sara` utilizzata per decidere che simboli far sparire
* dalla symbol table ridotta.
*/
static void load_coff_reltabs_as_aout(void)
{

/* Allocazione della tabella di conteggio dei riferimenti. */
	ref_tab = (long *) c2a_alloc(sizeof(long) * coff_header.f_nsyms);

/* Alloca memoria bastante a contenere tutti i record di rilocazione. */
	rel_tab_len = SECT_TEXT.header.s_nreloc + SECT_DATA.header.s_nreloc;
	rel_tab = (struct relocation_info *)
	            c2a_alloc(sizeof(struct relocation_info) * rel_tab_len);

/* Carica le informazioni di .text... */
	load_coff_reltab_as_aout(&SECT_TEXT,
	                         &rel_tab[0]);
/* ...e di .data . */
	load_coff_reltab_as_aout(&SECT_DATA,
	                         &rel_tab[SECT_TEXT.header.s_nreloc]);
/* Non sono previsti altri casi. */
	if (SECT_BSS.header.s_relptr && SECT_BSS.header.s_nreloc)
		c2a_warning_d("relocation record in .bss - Ignored.",0L);
}

/* Questa funzione marca "cancellabile" il simbolo dato, rendendolo
 di tipo "N_ABS", locale, senza nome (n_strx = NULL_NAME). */
static void make_aout_removeable_sym(struct nlist * aout_sym)
{
	aout_sym -> n_un.n_strx = NULL_NAME;
	aout_sym -> n_type = N_ABS;
	aout_sym -> n_other = 0;
	aout_sym -> n_desc = 0;
}

/*
* Caricamento della symbol table con conversione in a.out. I nomi
* dei simboli saranno elaborati piu` avanti. Per ora tutti i simboli sono
* "anonimi". Questa funzione, comunque, calcola la dimensione della
* parte di string table da aggiungere a quella originale, per contenere
* i nomi brevi, che nel COFF sono registrati in un campo del descrittore
* di simbolo.
*/
static void load_coff_symtab_as_aout_unnamed(void)
{
struct syment coff_sym;
struct nlist aout_sym;
struct nlist *p;
int i;
int n_ext;

/* Allocazione dello spazio per la copia della symbol table. */

	sym_tab_len = coff_header.f_nsyms;
	if (! sym_tab_len)
		c2a_readerr();
	sym_tab = (struct nlist *)
	            c2a_alloc(sizeof(struct nlist) * sym_tab_len);

/* Posizionamento. */

	c2a_seek(coff_header.f_symptr);

/* Lettura degli elementi e loro traduzione in forma provvisoria. I record
 che costituiscono entry estese sono tradotti in simboli locali con
 associazione assoluta. Saranno rimossi dopo. */

	p = sym_tab;
	str_tab_add_len = 0;
	for (i = n_ext = 0; i < sym_tab_len; ++i, ++p) {

		c2a_read((char *)&coff_sym,sizeof_coff_sym);

		if (n_ext--) {
		/* Stiamo esplorando un'estensione. Niente di interessante.
		 Questo simbolo sparira`, dopo. */
			make_aout_removeable_sym(&aout_sym);
		/* Puo` essere utile per debug dargli il suo indice
		 per valore. */
			aout_sym.n_value = i;
		}
		else {

		/* Entry ordinaria o la prima di una sequenza estesa. */

		/* Registra il numero di estensioni. */
			n_ext = coff_sym.n_numaux;

		/* Se il simbolo ha un "nome breve" (8 caratteri o meno),
		 aggiunge la lunghezza del nome al conteggio dell'incremento
		 di dimensione della string table. */

			if (coff_sym._n._n_n._n_zeroes != 0) {
				str_tab_add_len
				 += (coff_sym._n._n_name[SYMNMLEN-1]
				     ? (SYMNMLEN + 1)
				     : (strlen(coff_sym._n._n_name) + 1));
			}

		/* Traduzione parziale in a.out. Manca il riferimento
		 al nome. */
			aout_sym.n_un.n_strx = DUMMY_NAME;
			aout_sym.n_type = 0;
			aout_sym.n_other = 0;
			aout_sym.n_desc = 0;
			aout_sym.n_value = coff_sym.n_value;

			switch (coff_sym.n_sclass) {
			case C_EXT:
				aout_sym.n_type = N_EXT;
			case C_LABEL:
			case C_STAT:
				switch (coff_sym.n_scnum) {
				case -1: aout_sym.n_type |= N_ABS; break;
				case 0: break;
				case 1: aout_sym.n_type |= N_TEXT; break;
				case 2: aout_sym.n_type |= N_DATA; break;
				case 3: aout_sym.n_type |= N_BSS; break;
				default: make_aout_removeable_sym(&aout_sym);
					break;
				}
				break;
			default:
				make_aout_removeable_sym(&aout_sym);
				break;
			}
		}

	/* Copia nella tabella l'elemento appena costruito. */

		*p = aout_sym;
	}

/* Fine della symbol table. Segue, giusto giusto, il campo di quattro
 byte contenente la lunghezza della string table. Sara` letto nella
 prossima funzione. */

}

/*
* Caricamento della string table originale, con allocazione della parte
* aggiuntiva calcolata dalla funzione precedente.
* Si suppone che la posizione di lettura sia gia` stata posta alla fine
* della symbol table.
*/
static void load_coff_stringtab_extended(void)
{

/* Legge la lunghezza. */
	c2a_read((char *)&str_tab_orig_len, sizeof(str_tab_orig_len));

	str_tab_len = str_tab_orig_len + str_tab_add_len;

/* Alloca il necessario. */

	str_tab = (char *) c2a_alloc((unsigned int)str_tab_len);

/* Assegna la nuova lunghezza alla copia in memoria. */

	*(long *)str_tab = str_tab_len;

/* Legge la string table, a meno della lunghezza. */

	c2a_read(str_tab + sizeof(long),
	         (unsigned int)str_tab_orig_len - sizeof(long));
}

/*
* Rilettura della symbol table originale, per assegnare un nome a
* ciascun simbolo e copiare i nomi brevi nella parte di string table in
* eccesso.
*/
static void load_short_coff_names(void)
{
struct syment coff_sym;
struct nlist aout_sym;
struct nlist *p;
int i;
int n_ext;
int len;
long nstr;
long nm;
char *s;

/* Posizionamento. */

	c2a_seek(coff_header.f_symptr);

/* Lettura degli elementi e aggiunta delle informazioni riguardanti il nome.
  I nomi lunghi sono indici a stringhe gia` esistenti in str_tab, quelli
 brevi devono diventarlo. Le estensioni restano anonime, naturalmente. */

	p = sym_tab;
	nstr = str_tab_orig_len;
	for (i = n_ext = 0; i < sym_tab_len; ++i, ++p) {

		c2a_read((char *)&coff_sym,sizeof_coff_sym);

		if (n_ext--) {
		/* Stiamo esplorando un'estensione. Niente di interessante.
		 Questo simbolo sparira` molto presto. */
		}
		else {

		/* Entry ordinaria o la prima di una sequenza estesa. */

		/* Registra il numero di estensioni. */
			n_ext = coff_sym.n_numaux;

		/* Che tipo di nome ha il simbolo ? */

			if (coff_sym._n._n_n._n_zeroes != 0) {

			/* Nome breve. Bisogna copiare la stringa
			 nella string table e assegnarne l'offset
			 al simbolo. */

				len = SYMNMLEN;
				nm = nstr;
				s = coff_sym._n._n_name;
				while (len-- && *s)
					str_tab[nstr++] = *(s++);
				str_tab[nstr++] = '\0';
			}
			else {

			/* Nome lungo. L'offset e` gia` buono. */

				nm = coff_sym._n._n_n._n_offset;
			}

		/* Assegnamento del nome, a meno che il simbolo
		 non sia stato marcato "rimovibile". */

			if (p -> n_un.n_strx != NULL_NAME)
				p -> n_un.n_strx = nm;
		}
	}
}

/*
* Risoluzione dei riferimenti locali con correzione 
*/
static void resolve_section_local_references(int start, int end, long offset)
{
int i,ns;
static struct relocation_info *p;
struct nlist *sym;
struct ldsect_t *sect;

	for (i = start, p = &rel_tab[start]; i < end; ++i, ++p) {

	/* Aggiusta l'offset dell'indirizzo da rilocare: in A.OUT le
	 rilocazioni sono relative alla sezione, non alla base del codice. */

		p -> r_address -= offset;

	/* Riconduce i riferimenti simbolici locali a riferimenti alla
	 base della sezione. A quanto pare, il valore del simbolo
	 coinvolto e` ignorato. */

		if (p -> r_extern) {

			ns = p -> r_symbolnum;
			sym = &sym_tab[ns];

			switch (sym ->  n_type & N_TYPE) {
			case N_TEXT: sect = &SECT_TEXT; break;
			case N_DATA: sect = &SECT_DATA; break;
			case N_BSS: sect = &SECT_BSS; break;
			default: sect = (struct ldsect_t *) NULL; break;
			}

			if (sect) {

				p -> r_extern = 0;
				p -> r_symbolnum = sym -> n_type & N_TYPE;

			/* Il simbolo perde un riferimento, ovviamente. */

				--ref_tab[ns];
			}
		}
	}
}

/*
* Risoluzione dei riferimenti locali, che il COFF lascia sospesi
* perche` li riconduce a riferimenti indiretti ai nomi delle sezioni.
*/
static void resolve_local_references(void)
{
int i,ns;
static struct relocation_info *p;
struct nlist *sym;
struct ldsect_t *sect;

/* Risolve le rilocazioni in ".text". */
	resolve_section_local_references(0,
	                                 SECT_TEXT.header.s_nreloc,
	                                 0L);
/* Risolve le rilocazioni in ".data". */
	resolve_section_local_references(SECT_TEXT.header.s_nreloc,
	                                 SECT_TEXT.header.s_nreloc
	                                  + SECT_DATA.header.s_nreloc,
	                                 SECT_TEXT.header.s_size);
}

/*
*  Questa funzione dice se il simbolo ha un nome. E` utilizzata
* durante l'eliminazione delle entry che rappresentavano estensioni
* nel file COFF.
*/
static int is_named_symbol(int i)
{
	return sym_tab[i].n_un.n_strx != NULL_NAME;
}

/*
*  Questa funzione dice se il simbolo di indice dato
* e` utilizzato o no. Si considerano "utilizzati" tutti i simboli
* globali e quelli coinvolti in rilocazioni.
*  Fa anche un piccolo controllo di congruenza. Non si sa mai.
*/
static int is_used_symbol(int i)
{
int rv;

	rv = (ref_tab[i] != 0)
	  || ((sym_tab[i].n_type & N_EXT) != 0);

	if (rv && !is_named_symbol(i)) {
	/* Strano. Un simbolo anonimo ma utilizzato.
	  Questo non dovrebbe mai avvenire. */
		c2a_warning_d("bad symbol reference %ld.", (long)i);
	}

	return rv;
}
/*
*  Questa funzione dice se il simbolo di indice dato
* e` ordinario o di debug. Si considerano "di debug" tutti i simboli
* locali il cui nome inizia per "$" e che non sono coinvolti in
* rilocazioni.
*/
static int is_nondebug_symbol(int i)
{
	return (ref_tab[i] != 0)
	     || ((sym_tab[i].n_type & N_EXT) != 0)
	     || (is_named_symbol(i)
	      && str_tab[sym_tab[i].n_un.n_strx] != '$');
}
/*
*  Funzione utilizzata per raccogliere all'inizio della parte di
* symbol table iniziante all'indice "start" tutti i simboli che
* soddisfano la condizione "method".
*  La tabella di permutazione e` aggiornata di conseguenza.
*  Il valore di ritorno della funzione e` l'indice del primo
* simbolo che non soddisfa la condizione specificata.
*/
static int collect_symbols(int start, int (*method)(int index))
{
int i,ff;

/* Caso speciale. Il metodo NULL indica che tutti gli elementi sono
 accettabili. */

	if (! method)
		return sym_tab_len;

/* Ciclo dalla posizione specificata alla fine della symbol table. */

	for (i = start, ff = -1; i < sym_tab_len; ++i) {

		if (! (*method)(i)) {

		/* Simbolo "inutile". Marca la sua posizione, se e` il
		 primo della sequenza. Il prossimo simbolo "utile"
		 incontrato sara` permutato con questo. */

			if (ff == -1)
				ff = i;
		}
		else {

		/* Simbolo "utile". Se segue uno o piu` simboli "inutili",
		 e` permutato col primo di essi. */

			if (ff != -1) {

		/* Scambia i simboli. */

			{ struct nlist t;
				t = sym_tab[i];
				sym_tab[i] = sym_tab[ff];
				sym_tab[ff] = t; }
			{ long t;
				t = ref_tab[i];
				ref_tab[i] = ref_tab[ff];
				ref_tab[ff] = t; }

		/* Aggiorna la tabella di traslazione. */

			{ long t;
				t = tr_tab[i];
				tr_tab[i] = tr_tab[ff];
				tr_tab[ff] = t; }

		/* Aggiorna la posizione di copia. */

				++ff;
			}
		}
	}

/* "ff" vale -1 se non c'e` stata alcuna permutazione. In questo caso
 la posizione da restituire e` la fine della symbol table. */

	if (ff == -1)
		ff = sym_tab_len;

	return ff;
}

/*
*  Generazione di una string table compatta a partire dalla porzione
* di symbol table che comincia a "start" e termina ad "end" escluso,
* e assegnamento di indici in questa ai simboli coinvolti.
*  L'indirizzo della string table generata e` copiato in "*tab", la sua
* lunghezza in "*len".
*/
static void create_strtab(int start, int end, char **tab, long *len)
{
int i;
struct nlist *p;
register char *s,*d,c;
long n;

/* Calcolo della lunghezza della string table. */

	*len = sizeof(long); /* 4 byte di lunghezza all'inizio. */

	for (i = start, p = &sym_tab[start]; i < end; ++i, ++p) {
		*len += strlen(&str_tab[p -> n_un.n_strx]) + 1;
	/* Non tiene conto dell'underscore, se c'e` e se lo si
	 vuole eliminare. */
		if (remove_underscore && str_tab[p -> n_un.n_strx] == '_')
			--(*len);
	}

/* Allocazione della tabella. */

	*tab = d = (char *)c2a_alloc(*len);

/* Trasferimento dei nomi e correzione degli indici di nome dei simboli. */

	*(long *)d = *len;
	n = sizeof(long);
	d += n;
	for (i = start, p = &sym_tab[start]; i < end; ++i, ++p) {
		s = &str_tab[p -> n_un.n_strx];
	/* Non tiene conto dell'underscore, se c'e` e se lo si
	 vuole eliminare. */
		if (remove_underscore && *s == '_')
			++s;
		p -> n_un.n_strx = n;
		do {
			++n;
			*(d++) = c = *(s++);
		} while (c);
	}
}

/*
* Eliminazione dei simboli considerati inutili. Sono "inutili"
* tutti i nomi non coinvolti in rilocazioni e non globali. I simboli
* non sono in realta` rimossi: la symbol table e` semplicemente riorganizzata
* in modo che i simboli "utili" compaiano per primi. E` creato un vettore
* di corrispondenze che servira` a riallineare la tabella di rilocazione
* con i nuovi indici dei simboli. Anche quest'operazione e` compiuta 
* qui.
*/
static void remove_unuseful_symbols(void)
{
int i,ff;
static struct relocation_info *p;
long len;
int (*method)(int index);

/* Allocazione della tabella di permutazione, che sara` utilizzata
 per ritradurre le rilocazioni. */
	tr_tab = (long *) c2a_alloc(sizeof(long) * sym_tab_len);
/* Ovvia inizializzazione. */
	for (i = 0; i < sym_tab_len; ++i) 
		tr_tab[i] = i;

/* Primo passo. Sono raccolti all'inizio tutti i simboli "utili".
  Il concetto di "utilita`" dipende dalle opzioni scelte. */

	if (remove_debug)
		method = is_nondebug_symbol;
	else if (remove_unused)
		method = is_used_symbol;
	else
		method = is_named_symbol;

	sym_tab_reduced_len = collect_symbols(0,method);

/* Secondo passo. Eliminazione delle entry che costituivano estensioni.
  Si riconoscono dal nome nullo. Se le opzioni di lancio richiedono
 il mantenimento di tutti i somboli, l'operazione non e` necessaria,
 essendo gia` stata compiuta al primo passo. La lunghezza della tabella
 residua sara` zero. */

	if (method == is_named_symbol)
		sym_tab_remaining_len = sym_tab_reduced_len;
	else
		sym_tab_remaining_len = collect_symbols(sym_tab_reduced_len,
							is_named_symbol);
	sym_tab_remaining_len -= sym_tab_reduced_len;

/* Ora la symbol table ha un blocco iniziale di simboli "necessari",
 nel senso che partecipano attivamente alla costruzione del codice
 finale, o esportando funzioni o variabili, o costituendo riferimenti
 per rilocazioni. Segue un blocco di simboli "di debug", tipicamente simboli
 locali non coinvolti in rilocazioni; infine si ha un blocco di simboli
 "di scarto", risultato dell'eliminazione dei descrittori delle sezioni
 di debug e delle estensioni del COFF. */

#ifdef DEBUG
{
int i;

	printf("---------\n");
	for (i = 0; i < sym_tab_len; ++i) {
		if (sym_tab_reduced_len == i
		 || sym_tab_reduced_len + sym_tab_remaining_len == i)
			printf("*********\n");
		printf("%d %s %ld\n",i,
		       sym_tab[i].n_un.n_strx != NULL_NAME
		        ? &str_tab[sym_tab[i].n_un.n_strx] : "<REMOVED>",
		       ref_tab[i]);
	}
}     
#endif

/* Ora si deve riallineare la tabella di rilocazione, dato che
 sono cambiati gli indici dei simboli. Prima di tutto, pero` si deve
 ricavare la tabella di permutazione inversa a partire da quella diretta.
  Esiste un algoritmo che permette di invertire la tabella diretta su
 se stessa, ma e` complicato, percio` si alloca semplicemente un'altra
 tabella ("Graecus est, non legitur"). */

#if 0
{
int next_i;

	for (i = 0; i < sym_tab_len; i = next_i) {
	int j,next_j;

		for (j = i, next_i = -1; tr_tab[j] != i; j = next_j) {
			next_j = tr_tab[j];
			tr_tab[next_j] = j;
		}
	}
}
#else
	itr_tab = (long *) c2a_alloc(sizeof(long) * sym_tab_len);
	for (i = 0; i < sym_tab_len; ++i) 
		itr_tab[tr_tab[i]] = i;
#endif

	for (i = 0, p = rel_tab; i < rel_tab_len; ++i, ++p) {
		if (p -> r_extern)
			p -> r_symbolnum = itr_tab[p -> r_symbolnum];
	}

/* Ora le tabelle non servono piu`. */

	c2a_free((char *)tr_tab);
	tr_tab = (long *) NULL;
	c2a_free((char *)itr_tab);
	itr_tab = (long *) NULL;

/* Riallineamento della string table. Sono calcolate le dimensioni
 delle string table ridotta e residua, sono allocate aree bastanti, e
 sono riassegnati gli indici dei nomi ai simboli. */

	create_strtab(0,
	              sym_tab_reduced_len,
	              &str_tab_reduced,
	              &str_tab_reduced_len);
	create_strtab(sym_tab_reduced_len,
	              sym_tab_reduced_len + sym_tab_remaining_len,
	              &str_tab_remaining,
	              &str_tab_remaining_len);
}

/*
* Creazione del file a.out.
*/
static void open_aout_file(void)
{
	oobj = c2a_open_file(ofile,"wb");
	if (save_astrip_info) {
		if (aofile) {
			aoobj = c2a_open_file(aofile,"wb");
		}
		else {
			aoobj = oobj;
			aofile = aofile;
		}
	}
	else {
		aoobj = (FILE *) NULL;
	}
}

/*
* Scrittura dell'header a.out.
*/
static void store_aout_header(void)
{
	aout_header.a_magic = OMAGIC;
	aout_header.a_text = SECT_TEXT.header.s_size;
	aout_header.a_data = SECT_DATA.header.s_size;
	aout_header.a_bss = SECT_BSS.header.s_size;
	aout_header.a_entry = 0x00000000;
	aout_header.a_trsize = SECT_TEXT.header.s_nreloc
	                     * sizeof(struct relocation_info);
	aout_header.a_drsize = SECT_DATA.header.s_nreloc
	                     * sizeof(struct relocation_info);
	aout_header.a_syms = sym_tab_reduced_len * sizeof(struct nlist);

	c2a_write((char *)&aout_header,sizeof(aout_header));
}

/*
* Copia del codice di una sezione.
*/
static void copy_sect(struct ldsect_t *sect)
{
int n;

/* Se la sezione contiene dati... */
	if (sect -> header.s_scnptr) {

	/* Posizionamento e copia del blocco. */

		c2a_seek(sect -> header.s_scnptr);

		for (n = sect -> header.s_size; n--;)
			fputc(fgetc(iobj),oobj);
	}
}

/*
* Copia del codice e dei dati dal COFF all'a.out.
*/
static void copy_text_and_data(void)
{
	copy_sect(&SECT_TEXT);
	copy_sect(&SECT_DATA);
}

/*
* Scrittura della tabella di rilocazione tradotta.
*/
static void store_aout_reltab(void)
{
	c2a_write((char *)rel_tab,sizeof(*rel_tab)*rel_tab_len);
}

/*
* Scrittura della symbol table ridotta e calcolo della lunghezza
* della string table ridotta. Gli indici di stringa sono
* ricalcolati.
*/
static void store_reduced_symtab(void)
{
	c2a_write((char *)sym_tab,sizeof(*sym_tab)*sym_tab_reduced_len);
}

/*
* Scrittura della string table ridotta.
*/
static void store_reduced_stringtab(void)
{
	c2a_write((char *)str_tab_reduced,str_tab_reduced_len);
}

/*
* Scrittura dell'header a.out della parte residua.
*/
static void store_remaining_aout_header(void)
{
	aout_header.a_magic = OMAGIC;
	aout_header.a_text = 0;
	aout_header.a_data = 0;
	aout_header.a_bss = 0;
	aout_header.a_entry = 0x00000000;
	aout_header.a_trsize = 0;
	aout_header.a_drsize = 0;
	aout_header.a_syms = sym_tab_remaining_len * sizeof(struct nlist);

	c2a_write((char *)&aout_header,sizeof(aout_header));
}

/*
* Scrittura della symbol table residua e calcolo della lunghezza
* della string table residua.
*/
static void store_remaining_symtab(void)
{
	c2a_write((char *)&sym_tab[sym_tab_reduced_len],
	          sizeof(*sym_tab)*sym_tab_remaining_len);
}

/*
* Scrittura della string table residua.
*/
static void store_remaining_stringtab(void)
{
	c2a_write((char *)str_tab_remaining,str_tab_remaining_len);
}

/*
* Chiusura di tutto, e fine.
*/
static void close_all(void)
{
	if (aoobj && aoobj != oobj)
		fclose(aoobj);
	fclose(oobj);
	fclose(iobj);
}

int main(int argc, char **argv)
{
/* Analiso della linea di comando. */
	process_command_line(argc,argv);
/* Apertura del file di ingresso. */
	open_coff_file();
/* Caricamento dell'header e controllo di congruenza. */
	load_coff_header();
/* Caricamento delle tabelle di rilocazione con conversione in a.out.
  Questa funzione crea anche la tabella di conteggio dei riferimenti
 ai simboli. Sara` utilizzata per decidere che simboli far sparire
 dalla symbol table ridotta. */
	load_coff_reltabs_as_aout();
/* Caricamento della symbol table con conversione in a.out. I nomi
 dei simboli saranno elaborati piu` avanti. Per ora tutti i simboli sono
 "anonimi". Questa funzione, comunque, calcola la dimensione della
 parte di string table da aggiungere a quella originale, per contenere
 i nomi brevi, che nel COFF sono registrati in un campo del descrittore
 di simbolo. */
	load_coff_symtab_as_aout_unnamed();
/* Caricamento della string table originale, con allocazione della parte
 aggiuntiva calcolata dalla funzione precedente. */
	load_coff_stringtab_extended();
/* Rilettura della symbol table originale, per assegnare un nome a
 ciascun simbolo e copiare i nomi brevi nella parte di string table in
 eccesso. */
	load_short_coff_names();
/* Risoluzione dei riferimenti locali, che il COFF lascia sospesi
 perche` li riconduce a riferimenti indiretti ai nomi delle sezioni. */
	resolve_local_references();
/* Eliminazione dei simboli considerati inutili. Sono "inutili"
 tutti i nomi non coinvolti in rilocazioni e non globali. I simboli
 non sono in realta` rimossi: la symbol table e` semplicemente riorganizzata
 in modo che i simboli "utili" compaiano per primi. E` creato un vettore
 di corrispondenze che servira` a riallineare la tabella di rilocazione
 con i nuovi indici dei simboli. */
	remove_unuseful_symbols();

/* Creazione del file a.out. */
	open_aout_file();
/* Scrittura dell'header a.out. */
	store_aout_header();
/* Copia del codice e dei dati dal COFF all'a.out. */
	copy_text_and_data();
/* Scrittura della tabella di rilocazione tradotta. */
	store_aout_reltab();
/* Scrittura della symbol table ridotta e calcolo della lunghezza
 della string table ridotta. */
	store_reduced_symtab();
/* Scrittura della string table ridotta. */
	store_reduced_stringtab();

/* Il file A.OUT e` ora completo. Nulla vieta pero` di aggiungere,
 come estensione CNi, invisibile, un altro blocco di dati in forma A.OUT.
  Questo conterra` solo la parte rimanente della simbol table e della string
 table. Se richiesto. Il comando "astrip" e` in grado di isolare questa
 parte ad uso del debugger CNi. */
	if (save_astrip_info) {
	/* Usa (eventualmente) il terzo file aperto. */
		{ FILE *tmp; tmp = oobj; oobj = aoobj; aoobj = tmp; }
		{ char *tmp; tmp = ofile; ofile = aofile; aofile = tmp; }
	/* Scrittura dell'header a.out della parte residua. */
		store_remaining_aout_header();
	/* Scrittura della symbol table residua e calcolo della lunghezza
	 della string table residua. */
		store_remaining_symtab();
	/* Scrittura della string table residua. */
		store_remaining_stringtab();
	/* Rimette a posto i file. */
		{ FILE *tmp; tmp = oobj; oobj = aoobj; aoobj = tmp; }
		{ char *tmp; tmp = ofile; ofile = aofile; aofile = tmp; }
	}

/* Chiusura di tutto, e fine. */
	close_all();
	exit(0);

	return 0;
}

