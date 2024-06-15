/*
* @(#) dynaload.c 2.7 Thu Nov  9 18:10:41 MET 2000
* @(#) Rif. Guerrini
*
*  Questo modulo contiene le primitive per il caricamento dinamico
* di oggetti da parte del Plc.
*
* 14/04/95 GG : Versione 1.0. E` gestito il solo formato "a.out".
* 26/04/95 GG : Versione 1.1. Si e` separato il nome del modulo da
*               quello dei simboli.
* 19/05/95 GG : Versione 1.2. Si e` data la possibilita` di definire piu`
*               moduli anonimi.
* 19/06/95 GG : Versione 1.3. Ordinati (in parte) gli errori. Aggiunta la
*               possibilita` di avere simboli opzionali.
* 20/06/95 GG : Versione 1.4. Accelerato il caricamento raccogliendo in un
*               array temporaneo i simboli esterni per chiamare una sola
*               volta "nlist()".
* 18/09/95 GG : Versione 1.5. Condizionato il tutto alla definizione di "Lynx",
*               per unificare senza fatica le compilazioni su LynxOS e Solaris.
* 30/01/96 GG : Versione 1.6. Aggiunta la memorizzazione del nome del file
*               oggetto caricato in un modulo, ed un flag di "modulo caricato",
*               in modo che il campo "text" possa mantenere l'indirizzo di
*               base dell'oggetto. Inoltre e` stata aggiunta la memorizzazione
*               della dimensione del modulo.
* 01/02/96 GG : Versione 1.7. Modificato l'algoritmo di rilocazione, che non
*               gestiva correttamente le rilocazione nella sezione "data" con
*               riferimenti alla sezione stessa.
* 02/02/96 GG : Versione 1.8. Modificata la struttura descrittiva di un modulo
*               aggiungendo informazioni riguardanti l'inizio dell'area "bss",
*               "data" e "common".
* 17/05/96 GG : Versione 2.0. Rimaneggiamenti per passaggio a nuovo PLC.
* 19/09/96 GG : Versione 2.1. Filtrato il caso "mod == NULL" in "dyUnload".
* 22/10/96 GG 2.2 Aggiunta la possibilita` di immagazzinare e reinizializzare
*             l'area dati di un modulo.
* 04/12/96 GG 2.3 Aggiunta una "pezza" nella "_dySetHooks", per gestire
*             correttamente i "ganci" con simboli nella sezioni BSS. Dico
*             "pezza" perche` la correzione e` basata su di un'ipotesi mai
*             smentita, ma nemmeno mai dimostrata.
* 05/12/96 GG 2.4 Spostato il backup del valore iniziale dell'area "data"
*             dopo la risoluzione dei riferimenti esterni, senno`, crash !
* 02/04/99 GG 2.5 Aggiunta la chiusura dell'oggetto alla fine del caricamento.
*             Incredibile! Ad ogni caricamento si consumava un file descriptor,
*             e nessuno si e` mai lamentato!
* 28/07/00 GG 2.6 Aggiustamenti per compilazione su Linux.
* 09/11/00 GG 2.7 Limitata a 200 byte la lunghezza del nome di file
*             riportato da "dyModProp". Dava problemi su Linux (troppi byte
*             trasmessi all'interfaccia locale, perche` i path di Linux
*             sono piu` lunghi).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef Linux
#include "lynx-a.out.h"
#include "lynx-nlist.h"
#include <sys/stat.h>
#include <sys/mman.h>
#else
#include <a.out.h>
#include <nlist.h>
#include <stat.h>
#endif

#include "qplc.h"

/* Tabella dei moduli caricati. Ogni oggetto
 caricato dinamicamente deve contenere tutti i simboli
 indicati in un elemento di questa tabella. Se questo non avviene,
 il modulo non e` puo` essere accettato. */

module_t * Modules = (module_t *) 0;

static struct nlist *SymTab = (struct nlist *) 0;
static char *StrTab = (char *) 0;
static int nSyms = 0;

static int iDyStatus = 0;

/*
* Funzione _dyLoadHeader
* ----------------------
*
* Copia l'header dell'oggetto "obj" in "header".
* Vale 0 in caso di successo, -1 in caso di errore.
*/

static int _dyLoadHeader(FILE * obj, struct exec *header)
{
/* Lettura dell'header a.out. */

	if (fread((char *)header, sizeof(*header), 1, obj) != 1) {
/* ERRORE : sembra che il file sia troppo corto... */
		return -1;
	}

	return 0;
}

/*
* Funzione _dyLoadSymTab
* ----------------------
*
* Copia in un'area allocata dinamicamente i simboli ed i nomi dal file "obj",
* basandosi sui dati contenuti in "header".
* Vale 0 in caso di successo, -1 in caso di errore.
*/

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

/*
* Funzione _dyFreeSymTab
* ----------------------
*
* Questa funzione libera l'area occupata dalla tabella dei simboli e dei nomi.
*/

static void _dyFreeSymTab(void)
{
	if (SymTab)
		free(SymTab);

	SymTab = (struct nlist *) 0;
	StrTab = (char *) 0;
	nSyms = 0;
}

/*
* Funzione _dyUninitVarSize
* -------------------------
*
*  Questa funzione calcola la dimensione dell'area richiesta per
* ospitare le variabili non inizializzate definite nella symbol table.
*  Nel format Lynx objact a.out tali variabili figurano come "extern",
* ed il loro valore ne rappresenta la dimensione. Una dimensione uguale
* a zero indica che il simbolo e` un "extern" ordinario.
*/

static int _dyUninitVarSize(void)
{
int i,size;

/* Ciclo di ricerca delle variabili indefinite con valore non nullo.
  Il totale dei valori da` la quantita` di memoria necessaria a contenerle
 tutte. */

	for (i = size = 0; i < nSyms; ++i) {
		if ((SymTab[i].n_type & N_TYPE) == N_UNDF) {
			size += SymTab[i].n_value;
		}
	}

	return size;
}

/*
* Funzione _dyUninitVarAlloc
* --------------------------
*
*  Questa funzione assegna ai simboli rappresentanti variabili non
* inizializzate un indirizzo assoluto nella sezione BSS. La base di
* allocazione e` "bss".
*/

static void _dyUninitVarAlloc(char * bss)
{
int i,size;

/* Ciclo di assegnamento di un indirizzo definitivo alle variabili non
 inizializzate. Tutte le variabili di questo tipo sono promosse a globali
 della sezione BSS, ed associate ad un indirizzo nell'area allocata.
  I simboli con dimensione nulla non sono considerati, trattandosi
 di "extern" veri e propri. */

	for (i = 0; i < nSyms; ++i) {
		if ((SymTab[i].n_type & N_TYPE) == N_UNDF && SymTab[i].n_value != 0) {
			SymTab[i].n_type = N_BSS;
			size = SymTab[i].n_value;
			SymTab[i].n_value = (unsigned long) bss;
			bss += size;
		}
	}
}


/*
* Funzione "_dyRelocate"
* ----------------------
*
* Questa funzione riloca il codice all'indirizzo "text"
* prelevando la tabella di rilocazione dal file "obj" di header "header".
* La rilocazione si riferisce allo spazio codice o dati a seconda che
* "textflag" valga 1 o 0.
*/

static int _dyRelocate(char *text,FILE * obj,struct exec * header, int textflag)
{
int i,size,nrel;
struct relocation_info rel;
char *pos;
unsigned long val;
char *section;

/* Calcolo della posizione di intervento nel codice (text o data),
  del numero di elementi nella tabella di rilocazione,
  e della posizione della tabella di rilocazione nel file. */

	val = header -> a_text + header -> a_data + sizeof(*header);
	section = text;
	if (textflag) {
		nrel = header -> a_trsize;
	}
	else {
		val += header -> a_trsize;
		section += header -> a_text;
		nrel = header -> a_drsize;
	}
	nrel /= sizeof(struct relocation_info);

/* Posizionamento all'inizio della tabella di rilocazione. */

	if (fseek(obj, val, 0))
		return -1;

	for (i = 0; i < nrel; ++i) {

	/* lettura di un elemento. */

		if (fread((char *)&rel, sizeof(rel), 1, obj) != 1)
			return -1;

	/* pos = posizione della parola da correggere,
	   size = dimensione,
	   val = valore iniziale. */

		pos = section + rel.r_address;
		size = 1 << rel.r_length;
		val = 0;
		memcpy(&val,pos,size);	/* E` una porcheria, lo so. */

		if (rel.r_extern) {

		/* Riferimento ad un simbolo. */

			if ((SymTab[rel.r_symbolnum].n_type & N_TYPE) == N_UNDF) {
				if (rel.r_pcrel)
					val += SymTab[rel.r_symbolnum].n_value-(unsigned long)text;
				else
					val += SymTab[rel.r_symbolnum].n_value;
			}
			else {

			/* Il simbolo e` definito nell'oggetto. */

				/* Corregge il valore interpretando il simbolo come
				 indirizzo assoluto o relativo rispetto alla posizione
				 corrente. */

				if (rel.r_pcrel)
					val += SymTab[rel.r_symbolnum].n_value - (unsigned long)pos;
				else
					val += SymTab[rel.r_symbolnum].n_value;
			}
		}
		else {

		/* La rilocazione non fa riferimento a simboli. */

			if (rel.r_pcrel) {

		/* Il valore e` un offset rispetto alla posizione corrente,
		 qualunque essa sia. Non e` necessaria alcuna elaborazione. */

			}
			else {

		/* Il valore e` un offset rispetto all'inizio del blocco "text",
		 e deve diventare un indirizzo assoluto. */

				val = (unsigned long)(text + val);
			}
		}
		memcpy(pos,&val,size);
	}

	return 0;
}

/*
* Funzione _dyLink
* ----------------
*
*  Questa funzione esegue i passi necessari a rendere eseguibile
* il codice caricato in "text" : interpreta la tabella di rilocazione,
* leggendola da "obj" e risolve i riferimenti esterni ricavando le
* informazioni dal file di nome "exename". Le dimensioni delle sezioni
* sono ricavate da "header".
*  La funzione vale 0 se tutto e` andato bene, -1 in caso di errore.
*/

static int _dyLink(char *text,FILE * obj,struct exec * header,char * exename)
{
int i,nrel,errs;
struct relocation_info rel;
struct nlist *nlext;
int *corr;
int n_ext,j;
char *symset;
int by,bi;

	errs = 0;

/* Raccoglie i simbolo indefiniti e tenta di risolverli "in un colpo solo". */

/* Conta i simboli esterni. */

	if (fseek(obj, header -> a_text + header -> a_data + sizeof(*header),0))
		return -1;

/* Alloca la tabella dei flag di simbolo indefinito (mappa di bit). */

	symset = (char *) malloc(nSyms/8 + 1);
	if (! symset)
		return -1;
	memset(symset,0,nSyms/8 + 1);

/* Calcolo del numero di elementi. */
	nrel = (header->a_trsize+header->a_drsize)/sizeof(struct relocation_info);

	for (i = n_ext = 0; i < nrel; ++i) {

	/* lettura di un elemento. */

		if (fread((char *)&rel, sizeof(rel), 1, obj) != 1) {
			free(symset);
			return -1;
		}

		if (rel.r_extern) {

		/* Riferimento ad un simbolo. */

			if ((SymTab[rel.r_symbolnum].n_type & N_TYPE) == N_UNDF) {
				by = rel.r_symbolnum / 8;
				bi = 1 << (rel.r_symbolnum % 8);
				if (!(symset[by] & bi)) {
					++n_ext;
					symset[by] |= bi;
				}
			}
		}
	}

	if (n_ext) {

/* Alloca un array di dimensione sufficiente. */

		nlext = (struct nlist *) malloc(sizeof(struct nlist)*(n_ext + 2));
		if (! nlext) {
			free(symset);
			return -1;
		}
		memset(nlext,0,sizeof(struct nlist)*(n_ext + 2));

/* Alloca una tabella di corrispondenza tra elemento in SymTab ed in nlext. */

		corr = (int *) malloc(sizeof(int) * (n_ext + 2));
		if (! corr) {
			free(nlext);
			free(symset);
			return -1;
		}

/* Copia tutti i nomi dei simboli esterni nel vettore temporaneo. */

		for (i = j = 0; i < nSyms; ++i) {
			by = i / 8;
			bi = 1 << (i % 8);
			if (symset[by] & bi) {
				nlext[j].n_un.n_name = StrTab+SymTab[i].n_un.n_strx;
				corr[j] = i;
				++j;
			}
		}

/* Chiude il vettore con il nome nullo (il manuale e` ambiguo in merito !). */

		nlext[n_ext].n_un.n_name = "";
		nlext[n_ext + 1].n_un.n_name = (char *)0;

/* Risolve i riferimenti esterni. */

	/* Non si sa che farsene dell'errore di nlist() ! */

		nlist(exename,nlext);

/* Ricopia i risultati nel campo "valore" della symbol table (o errore
 se il simbolo non e` stato risolto). */

		for (i = 0; i < n_ext; ++i) {
			if (nlext[i].n_type) {
				SymTab[corr[i]].n_value = nlext[i].n_value;
			}
			else {
			/* ERRORE. Simbolo indefinito. */
				plcError(180,nlext[i].n_un.n_name);
				errs = 1;
			}
		}

/* Libera le aree temporanee. */

		free(corr);
		free(nlext);
	}

/* Libera la mappa di bit. */

	free(symset);

/* Rilocazione del codice. */

	errs |= _dyRelocate(text,obj,header,1);

/* Rilocazione dei dati. */

	errs |= _dyRelocate(text,obj,header,0);

	return errs ? -1 : 0;
}

/*
* Funzione _dyFindSym
* -------------------
*
*  Questa funzione cerca il simbolo indicato nella tabella dei simboli.
*  La funzione vale NULL se il simbolo non esiste o e` esterno.
*/

static struct nlist * _dyFindSym(char *name)
{
int i;

	for (i = 0; i < nSyms; ++i) {
		if (strcmp(StrTab + SymTab[i].n_un.n_strx, name) == 0) {
			if ((SymTab[i].n_type & N_TYPE) != N_UNDF)
				return & SymTab[i];
			break;
		}
	}
	return (struct nlist *) 0;
}

/*
* Funzione _dyRemove
* ------------------
*
*  Cancella un modulo.
*/

static void _dyRemove(module_t *mod)
{
	if (mod -> base)
		free(mod -> base);
	mod -> base = (char *) 0;
	mod -> text = (char *) 0;
}

/*
* Funzione dyFindModule
* ---------------------
*
*  Questa funzione trova il modulo di nome "name".
*/

module_t * dyFindModule(char * name)
{
module_t *p,**q;

/* Cerca il nome del modulo. */

	for (q = &Modules, p = *q; p; q = &(p -> next), p = *q) {
		if (strcmp(p -> name, name) == 0)
			break;
	}

	return p;
}

/*
* Funzione _dyGetLabel
* --------------------
*
*  Questa funzione estrae il nome del modulo cercando il simbolo "label"
* e leggendo la stringa associata dall'area "text".
*  Restituisce il puntatore all'etichetta, oppure NULL.
*/

static char * _dyGetLabel(char * label, char * text)
{
static struct nlist * ls;

	ls = _dyFindSym(label);
	if (! ls)
		return (char *) 0;
	
	return text + ls -> n_value;
}

/*
* Funzione _dySetHooks
* --------------------
*
*  Questa funzione cerca i simboli globali elencati nella tabella dei moduli.
*  Se il modulo e` definito (un modulo si identifica con la stringa puntata dal
* valore del simbolo "label"), la routine controlla se sono
* presenti tutti i simboli previsti per il modulo. Se la ricerca ha successo,
* il modulo e` registrato (l'eventuale modulo precedentemente caricato e`
* rimosso), altrimenti si ha un errore.
*  E` restituito il puntatore al descrittore di modulo, oppure NULL
* in caso di errore.
*/

static module_t * _dySetHooks(char *label, char *text)
{
int i,nhooks;
struct nlist *st;
hook_t *hooks;
module_t *mod;
char *modname;


/* Ricava il nome del modulo. */

	modname = _dyGetLabel(label,text);

/* Non c'e`. L'oggetto non ha il formato previsto. */

	if (! modname)
		return (module_t *) 0;

/* Cerca il modulo. */

	mod = dyFindModule(modname);

/* Non c'e`. Cerca un modulo anonimo. */

	if (! mod) {

		mod = dyFindModule("");

	/* Non c'e`. Il modulo non e` noto al sistema. */

		if (! mod)
			return (module_t *) 0;
	}

	hooks = mod -> hooks;
	nhooks = mod -> nhooks;

/* Controlla che siano presenti tutti i simboli richiesti. */

	for (i = nhooks; i--;) {
		if (! _dyFindSym(hooks[i].name) &&
            ! (hooks[i].flags & DY_PROP_OPTIONAL))
			return (module_t *) 0;
	}

/* Rimuove il modulo precedentemente caricato. */

	_dyRemove(mod);


/* Battezza il modulo (o ne conferma il nome). */

	dyRename(mod,modname);

/* Assegna l'indirizzo ad ogni simbolo. */

	for (i = nhooks; i--; ) {
		st = _dyFindSym(hooks[i].name);
		if (st) {

		/* Pezza "ad hoc". Se un simbolo e` "BSS", e` stato
		 QUASI sicuramente elaborato dall'allocatore delle aree
		 non inizializzate, ed alla fine si ritrova ad avere associato
		 un indirizzo assoluto. Quel "QUASI" mi toglie il sonno. */

			if (st -> n_type == N_BSS)
				*(void **)(hooks[i].target)
				   = (void *) (st -> n_value);
			else
				*(void **)(hooks[i].target)
				   = (void *) (text + st -> n_value);
		}
		else if (hooks[i].flags & DY_PROP_USE_DEFAULT)
			*(void **)(hooks[i].target) = hooks[i].defaultfn;
	}

	return mod;
}

/*
* Funzione dyLoad
* ---------------
*
*  Questa funzione carica per l'esecuzione "objname", file oggetto in formato
* "Lynx a.out (object)", ne corregge le rilocazioni, risolve i riferimenti
* esterni utilizzando le informazioni registrate nella symbol table
* del file "exename" (che deve essere il nome dell'eseguibile da cui e`
* nato il processo chiamante, ricavato tipicamente da argv[0]).
*  Il modulo deve contenere tutti i simboli definiti nel descrittore
* di modulo corrispondente.
*  Il nome del modulo e` la stringa puntata dal valore del simbolo
* indicato dal parametro "label". Se non esiste alcun modulo con
* quel nome, la funzione cerca il modulo anonimo "" e lo rinomina.
*  La funzione riporta il descrittore del modulo, oppure NULL in caso di
* errore.
*/

module_t * dyLoad(char * objname, char * exename, char * label)
{
FILE *obj;
struct exec header;
char *base,*text,*data,*init_data,*bss,*p;
int inimem, totmem, uivmem, data_size, zeromem;
module_t *mod;

	base = (char *) 0;
	text = (char *) 0;

/* Apertura dell'oggetto. */

	obj = fopen(objname,"r");
	if (! obj) {
	/* ERRORE. File inesistente o inaccessibile. */
		plcError(182,objname);
		goto ERROR;
	}

/* Lettura dell'header a.out. */

	if (_dyLoadHeader(obj,&header)) {
	/* ERRORE. Problemi nella lettura del file. */
		plcError(182,objname);
		goto ERROR;
	}

/* Controlla il formato. */

	if (header.a_magic != OMAGIC) {
	/* ERRORE. Formato dell'oggetto imprevisto. */
		plcError(183,objname);
		goto ERROR;
	}

/* Carica la symbol table una prima volta, per poter calcolare la
 dimensione effettiva dell'area BSS (il dato contenuto nell'header
 non tiene conto di tutte le variabili).
  L'area utilizzata per questo caricamento sara` liberata, ed il
 caricamento sara` ripetuto piu` avanti. Si fa questo nella speranza di
 ridurre la frammentazione della memoria, allocando per ultime
 le aree destinate ad essere liberate per prime. */

	if (_dyLoadSymTab(obj,&header)) {
	/* ERRORE. Problemi nella lettura della symbol table. */
		plcError(184,objname);
		goto ERROR;
	}

/* Calcolo della quantita` di memoria per il codice ed i dati. */

	uivmem = _dyUninitVarSize();
	data_size = header.a_data;
	inimem = header.a_text + data_size;
	zeromem = uivmem + header.a_bss;
	totmem = inimem + zeromem + data_size;

	if (inimem == 0) {
	/* ERRORE. File oggetto vuoto. */
		plcError(183,objname);
		goto ERROR;
	}

/* Libera, per ora, la symbol table. Serve a fare spazio per accogliere
 il codice ed i dati. */

	_dyFreeSymTab();

/* Allocazione dell'area destinata al codice ad ai dati. */

	p = malloc(totmem + 0x2000);
	if (! p) {
	/* ERRORE. Memoria insufficiente. */
		plcError(185,objname);
		goto ERROR;
	}
	memset(p,0,totmem + 0x2000);

	base = p;
	text = (char *)((((long)p) | (0x1000-1)) + 1);
	if (mprotect(text,totmem,PROT_READ|PROT_WRITE|PROT_EXEC) != 0) {
		perror("mprotect");
	}


/* Calcolo delle posizioni delle sezioni predefinite. In realta`,
 "text" e` l'unica sezione utilizzata per i riferimenti interni. */

	data = text + header.a_text;
	bss  = text + inimem;
	init_data = bss + zeromem;

/* Caricamento del codice e dei dati predefiniti. */

	if (fseek(obj,(long)(sizeof(header)),0)) {
	/* ERRORE. Problemi nell'accesso al file. */
		plcError(183,objname);
		goto ERROR;
	}

	if (fread(text, inimem, 1, obj) != 1) {
	/* ERRORE. Problemi nella lettura del file. */
		plcError(183,objname);
		goto ERROR;
	}

/* Ricarica la tabella dei simboli e dei nomi. */

	if (_dyLoadSymTab(obj,&header)) {
	/* ERRORE. Problemi nel caricamento della symbol table. */
		plcError(183,objname);
		goto ERROR;
	}

/* Associa un indirizzo assoluto nella sezione BSS alle variabili
 non inizializzate. */

	_dyUninitVarAlloc(bss + header.a_bss);

/* Rilocazione e risoluzione dei riferimenti esterni. */

	if (_dyLink(text,obj,&header,exename))
		goto ERROR;

/* Prepara la copia di backup dei valori iniziali delle variabili.
  La cosa e` possibile solo ora, perche` sono stati risolte eventuali
 rilocazioni nella sezione "data". */

	if (data_size) {
		memcpy(init_data, data, data_size);
	}

/* Cerca il modulo ed assegna i punti di aggancio. */

	mod = _dySetHooks(label,text);
	if (! mod) {
	/* ERRORE. Modulo imprevisto o non definito. */
		plcError(183,objname);
		goto ERROR;
	}

/* Fine. Libera la tabella dei nomi ed il file pointer. */

	_dyFreeSymTab();

	fclose(obj);
	obj = (FILE *) NULL;

/* Registra in nome del file. */

	if (objname[0] == '/')
		mod -> file[0] = '\0';
	else {
		getwd(mod -> file);
		strcat(mod -> file,"/");
	}
	strcat(mod -> file, objname);
	
/* Registra la dimensione del modulo. */

	mod -> size = totmem;

/* Registra gli indirizzi delle sezioni. */

	mod -> base = base;
	mod -> text = text;
	mod -> data = data;
	mod -> bss = bss;
	mod -> common = bss + header.a_bss;
	mod -> init_data = init_data;
	mod -> data_size = data_size;
	mod -> zero_size = zeromem;

/* Ufficializza il caricamento. */

	mod -> loaded = 1;

	iDyStatus = 0;

	return mod;

ERROR:
/* ERRORE. Caricamento dinamico fallito. */

	if (obj)
		fclose(obj);

	_dyFreeSymTab();

	if (base)
		free(base);

	iDyStatus = 181;

	return (module_t *) 0;
}

/*
* Funzione dyInitData
* -------------------
*
*  Questa funzione ricarica il valore iniziale della parte dati di un modulo
* precedentemente caricato.
*/

void dyInitData(module_t * mod)
{
	if (mod) {
		if (mod -> data && mod -> init_data && mod -> data_size)
			memcpy(mod -> data, mod -> init_data, mod -> data_size);
		if (mod -> bss && mod -> zero_size)
			memset(mod -> bss, 0, mod -> zero_size);
	}
}

/*
* Funzione dyDefineModule
* -----------------------
*
*  Registra un modulo, cioe` aggiunge una nuova definizione alla
* lista dei moduli caricabili. "hooks" e` un vettore di "n"
* punti di aggancio. "name" e` il nome del modulo.
*  Come valore di ritorno da` il puntatore al descrittore di modulo,
* oppure NULL in caso di errore.
*/

module_t * dyDefineModule(char * name, hook_t * hooks, int nhooks)
{
module_t *p;

/* Un minimo di controllo. */

	if (! hooks || nhooks <= 0)
		return (module_t *) 0;

/* Cerca il nome del modulo. Errore se e` gia` presente e non e` anonimo. */

	if (name[0] && dyFindModule(name))
			return (module_t *) 0;

/* Alloca il nuovo descrittore. */

	p = (module_t *) malloc(sizeof(module_t));
	if (! p)
		return p;

/* Inizializza la struttura. */

	p -> base =
	p -> text =
	p -> data =
	p -> bss =
	p -> common =
	p -> init_data = (char *) 0;
	p -> hooks = hooks;
	p -> nhooks = nhooks;
	p -> file[0] = '\0';
	p -> default_file[0] = '\0';
	p -> loaded = 0;
	p -> size = 0;
	p -> data_size = 0;
	p -> zero_size = 0;
	p -> default_size = 0;
	p -> default_text = (char *) 0;

	dyRename(p,name);

/* Aggiunge l'elemento alla lista. */

	p -> next = Modules;
	Modules = p;

	return p;
}

/*
* Funzione dyAllocHooks
* ---------------------
*
*  Questa funzione alloca una copia dell'insieme di agganci "hooks" di
* dimensione "nhooks". Anche i nomi sono riallocati e copiati. E` utile
* ogni volta che l'originale faccia parte di un'area che potrebbe essere
* riciclata, come lo stack o l'area dati di un modulo caricato dinamicamente.
*  Il valore della funzione e` un puntatore alla copia, oppure NULL se
* non c'e` piu` memoria.
*/

hook_t * dyAllocHooks(hook_t * hooks, int nhooks)
{
hook_t *p;
int i;
char *names,*q;
int len;

	if (! hooks || nhooks <= 0)
		return (hook_t *) 0;

/* Calcola la dimensione dello spazio riservato ai nomi. Si eseguira`
 una sola allocazione per ridurre la frammentazione della memoria. */

	for (i = len = 0; i < nhooks; ++i) {
		len += strlen(hooks[i].name) + 1;
	}

/* Alloca il necessario. */

	p = (hook_t *) malloc(sizeof(hook_t)*nhooks + len);
	if (! p)
		return p;

/* Copia tutto (tranne i nomi). */

	for (i = 0; i < nhooks; ++i) {
		p[i].target = hooks[i].target;
		p[i].defaultfn = hooks[i].defaultfn;
	}

/* Copia i nomi. */

	for (names = (char *)(&p[nhooks]), i = 0; i < nhooks; ++i) {
		q = hooks[i].name - 1;
		p[i].name = names;
		do {
			*names = *++q;
			++names;
		} while (*q);
	}

	return p;
}

/*
* Funzione dyInitDefault
* ----------------------
*
*  Questa funzione inizializza il campo "defaultfn" di ogni aggancio del
* modulo "mod", copiandone il valore dalla variabile associata.
*/

void dyInitDefault(module_t * mod)
{
int i;

	for (i = 0; i < mod -> nhooks; ++i)
		mod -> hooks[i].defaultfn = *(void **)(mod -> hooks[i].target);
}

/*
* Funzione dyMakeDefault
* ----------------------
*
*  Questa funzione fa si` che il modulo "mod", correntemente caricato,
* figuri "non caricato", e le funzioni contenute diventino il default.
*  L'effetto di questa funzione e` di rendere "permanente" un codice
* caricato dinamicamente.
*  L'area allocata non puo` piu` essere rilasciata, a meno che non si siano
* presi opportuni provvedimenti.
*/

void dyMakeDefault(module_t * mod)
{
	if (! mod -> loaded)
		return;

	dyInitDefault(mod);

	mod -> loaded = 0;

	strcpy(mod -> default_file, mod -> file);

	mod -> default_text = mod -> text;
	mod -> default_data = mod -> data;
	mod -> default_bss = mod -> bss;
	mod -> default_common = mod -> common;

	mod -> default_size = mod -> size;
}

/*
* Funzione dyUnload
* -----------------
*
*  Questa funzione scarica un modulo, libera l'area associata e riporta
* tutte le variabili associate al valore di default.
*/

void dyUnload(module_t * mod)
{
int i;

	if (mod && mod -> loaded) {
		for (i = 0; i < mod -> nhooks; ++i)
			*(void **)(mod -> hooks[i].target) = mod -> hooks[i].defaultfn;
		_dyRemove(mod);
		mod -> loaded = 0;
		strcpy(mod -> file, mod -> default_file);
		mod -> text = mod -> default_text;
		mod -> data = mod -> default_data;
		mod -> bss = mod -> default_bss;
		mod -> common = mod -> default_common;
		mod -> size = mod -> default_size;
	}
}

/*
* Funzione dyRename
* -----------------
*
*  Questa funzione rinomina un modulo.
*/

void dyRename(module_t * mod, char * name)
{
int i;

	for (i = 0; *name && (i < MAX_MODNAME - 1); ++i, ++name)
		mod -> name[i] = *name;
	while (i < MAX_MODNAME)
		mod -> name[i++] = '\0';
}

/*
* Funzione dyStatus
* -----------------
*
*  Questa funzione copia in "*sts"lo stato del caricatore dinamico.
*  Restituisce il numero di byte copiati.
*/

int dyStatus(int * sts)
{
	*sts = iDyStatus;
	return sizeof(*sts);
}

/*
* Funzione dyModProp
* ------------------
*
*  Questa funzione copia in "*sts" le proprieta` principali del modulo "prop" :
*  - quattro byte per l'indirizzo di base ("text"),
*  - quattro byte per la dimensione ("size"),
*  - duecento byte per il nome del file ("file").
*  Restituisce il numero di byte copiati.
*/

int dyModProp(module_t * mod, modprop_t * sts)
{
int l;
/* Ho aggiunto questo limite per semplificare il codice del comando
 locale che usa questo servizio. Il buffer di transito utilizzato dal
 comando e` di 1 Kb... */
#define MAX_MODPROP_PATH 200

	sts -> text = (long)(mod -> text);
	sts -> data = (long)(mod -> data);
	sts -> bss = (long)(mod -> bss);
	sts -> common = (long)(mod -> common);
	sts -> size = (long)(mod -> size);
	l = strlen(mod -> file);
	if (l > MAX_MODPROP_PATH)
		l = MAX_MODPROP_PATH;
	memcpy(sts -> file, mod -> file, l);
	sts -> file[l] = '\0';
	return sizeof(*sts) - sizeof(sts -> file) + l + 1;
}


#ifdef Linux
#if 1
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

	if ((! path) || (! path[0]) || (strcmp(path,main_status.exe_path)==0)) {
		return nlist_internal(nl);
	}
	else {
		return nlist_file(path,nl);
	}
}
#endif

