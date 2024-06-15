/*
* @(#) shvar.c 3.8 Mon Sep 28 13:54:17 MET 1998
*
*  Primitive di gestione delle variabili condivise, altrimenti
* dette "segnali di scambio".
*
* 05/06/96 GG 1.0 Prima stesura.
* 10/06/96 GG 1.1 Corretto il mancato assegnamento del campo "size" in
*             "shvCreate". Aggiunta la funzione "shvKeyToAddr".
* 04/09/96 GG 1.2 Aggiunta la funzione "shvMakeEdge".
* 10/09/96 GG 1.3 Modificate le chiamate alla primitive di creazione
*             delle variabili a rilevamento di fronte. La funzione
*             "shvMakeEdge" e` stata ribattezzata "shvInit", ed e`
*             utilizzata all'inizializzazione del Plc, oppure per reset
*             globali "al volo".
* 26/11/96 GG 2.0 Aggiunta la possibilita` di sistemare variabili in
*             RAM non volatile.
* 11/12/96 GG 2.1 I nomi possono ora contenere "$". E` poi stato aggiunto
*             un tipo "EXTENDED", per associare aree di dimensione arbitraria
*             ad un segnale di scambio. Quando si crea una variabile di questo
*             tipo, si deve specificare la dimensione di un elemento nel
*             campo "size" della struttura di controllo.
* 12/12/96 GG 2.2 Documentata (ma non mi sogno nemmeno di eliminarla) una
*             limitazione della funzione "shvKeyToAddr".
* 13/02/97 GG 2.3 La funzione "shvCreate" e` stata resa piu` tollerante
*             verso le differenze tra tipo richiesto e tipo ottenuto.
*             Differenze riguardanti "LONG-ULONG" e "CHAR-UCHAR" sono
*             considerate ora trascurabili.
* 19/02/97 GG 2.4 Evitata la creazione di variabili RETAIN se la RAM non
*             volatile non c'e`.
* 30/05/97 GG 2.5 E` stata introdotta la funzione "shvDefine", che e`
*             un'evoluzione di "shvCreate". Si usa allo stesso modo, ma
*             l'intero puntato dal secondo argomento si interpreta come
*             una maschera di bit, ciascuno dei quali segnala un conflitto
*             tra proprieta` richesta e proprieta` ottenuta. Vedere le
*             macro "SHV_CHG_...". La funzione "shvCreate" non e` ora altro
*             un'interfaccia verso "shvDefine".
*             E` stato risolto il problema legato all'attributo "EDGE". Ora
*             la ridefinizione di una variabile che non aveva questo
*             atttributo non produce una segnalazione di attributo diverso,
*             ma rende "EDGE" la variabile stessa.
*             Il codice e` stato riorganizzato per raccogliere qualche
*             funzioncina utile (ed e` anche stato rallentato :-( ).
* 03/06/97 GG 3.0 Aggiunta una tabella di conversione chiave --> descrittore
*             esteso per i segnali di scambio. Grazie ad essa, la funzione
*             "shvKeyToAddr" non soffre piu` di limitazioni.
*             Aggiunto l'attributo "SHV_MODE_RESET", che caratterizza le
*             variabili che si azzerano ad ogni GO del PLC. Di conseguenza,
*             e` stato aggiunto il flag "SHV_CHG_RESET" (e` attivato solo se
*             si richiede la creazione senza RESET per una variabili che
*             possiede gia` l'attributo).
*             Aggiunta la funzione "shvRestart", che azzera tutte le
*             variabili con  l'attributo "RESET".
* 03/06/97 GG 3.1 Aggiunte le funzioni "shvSet" ed "shvGet", utile per
*             scrivere o leggere in blocco segnali di scambio.
* 11/06/97 GG 3.2 Aggiunta la possibilita` di indicare un puntatore nullo
*             come secondo argomento di "shvCreate" ed "shvDefine".
* 13/11/97 GG 3.3 Corretto uno schianto in caso di shvDefine applicata a
*             nome non conforme. Inoltre, la segnalazione di errore non
*             ha piu` luogo se si e` impostato il flag SHV_MODE_TEST.
* 19/11/97 GG 3.4 Modificati l'azzeramento e la copia di variabili per
*             gestire correttamente le variabili in RAM non volatile
*             (problema floppy...).
* 21/11/97 GG 3.5 Iniziati i lavori per permettere la cancellazione di
*             variabili.
* 24/11/97 GG 3.6 Uno schifoso, rivoltante, abominevole trucco. Si tratta di
*             una pezza "ad hoc" per risolvere "in qualche modo" il problema
*             della non cancellabilita` delle variabili RETAIN. Ora, ad ogni
*             GO tali variabili sono marcate "DELETED". Si provvede qui a
*             ridefinire una variabile che possieda gli attributi RETAIN e
*             DELETED. Se non ci sono cambiamenti nella configurazione della
*             RAM non volatile, la ridefinizione delle variabili RETAIN al GO
*             produce identici assegnamenti di indirizzi ogni volta.
* 20/03/98 GG 3.7 Aggiunta la funzione "shvStat", che fornisce una statistica
*             di occupazione dell'area delle variabili condivise.
* 28/09/98 GG 3.8 Aggiunta la funzione "shvAddrToVar", che trova (se esiste)
*             la variabile corrispondente all'indirizzo dato. NOTA: e` lenta.
*/

#include <stdio.h>
#include <string.h>

#include "qplc.h"

#include "compat.h"
#include "util.h"

#include "shvar.h"

/*
* Struttura descrivente una chiave libera.
*/

typedef struct _keyfree_t {
	int key;	/* Prima chiave libera. */
	int size;	/* Dimensione del blocco di chiavi libere. */
	struct _keyfree_t * next; /* Puntatore al blocco successivo. */
} keyfree_t;

#define LEN_KEYFREE_BLOCK 80

/* Lista dei descrittori liberi di blocco libero (ARRGGH !). */

static keyfree_t * keyfree_freelist = (keyfree_t *) 0;

/* Lista dei descrittori di blocchi di chiavi libere. */

static keyfree_t * keyfree = (keyfree_t *) 0;

/* Flag indicante che la lista dei descrittori di blocco e` gia`
 stata ricostruita. */

static int keyfree_ok = 0;

/********************************/
/* GESTIONE DELL'HEAP CONDIVISO */
/********************************/

/*
* Funzione per l'allocazione di un blocco di memoria condivisa.
*/

static void * shv_malloc(int size)
{
void *p;

	if (size <= 0 || size > *main_status.p_shheap_size)
		return (void *) 0;

	p = (void *)((char *)main_status.shm_addr + *main_status.p_shheap);
	*main_status.p_shheap_size -= size;
	*main_status.p_shheap += size;
	*main_status.p_shvn_size += size;
	return p;
}

/*
* Funzione per la deallocazione di un blocco di memoria condivisa.
*/

static void shv_free(int size)
{
	*main_status.p_shheap_size += size;
	*main_status.p_shheap -= size;
	*main_status.p_shvn_size -= size;
}

/*
* Funzione per l'allocazione di un blocco di memoria non volatile.
*/

static void * shv_nvmalloc(int size)
{
void *p;

	if (size <= 0
	    || ! main_status.nvram2.addr
	    || size > main_status.nvram2.size
	              - (main_status.nvram2_free - main_status.nvram2.addr))
		return (void *) 0;

	p = (void *)main_status.nvram2_free;
	main_status.nvram2_free += size;
	return p;
}

/*
* Funzione per la deallocazione di un blocco di memoria non volatile.
*/

static void shv_nvfree(int size)
{
	main_status.nvram2_free -= size;
}

/*************************/
/* GESTIONE DELLE CHIAVI */
/*************************/

/*
* Aggiunta di un descrittore di blocco di chiavi libere alla lista.
*/

static int shv_addfkblk(int first, int len)
{
keyfree_t * k;

	if (! len)
		return 1;

	k = (keyfree_t *) util_alloc_cell((void **) &keyfree_freelist,
		                          sizeof(keyfree_t),
			                  GET_STRUCT_OFFSET(keyfree_t,next),
	                                  LEN_KEYFREE_BLOCK);
	if (! k) {
	/* ERRORE. Memoria esaurita. */
		plcError(108,"5");
		return 0;
	}

	k -> key = first;
	k -> size = len;

	util_add_cell((void *) k,
	              (void **) &keyfree,
	              GET_STRUCT_OFFSET(keyfree_t,next));

	return 1;
}

/*
* Ricostruzione della lista dei blocchi di chiavi libere.
*/

static int shv_mkfkbkl(void)
{
int i, first, len;

	if (! keyfree_ok) {

		for (i = len = 0, first = -1;
		     i < main_status.shvk_tab_len;
		     ++i) {

		/* Se il campo "off" della chiave e` 0, la chiave e` libera. */

			if (main_status.shvk[i].off == 0) {

			/* Registra la chiave se e` la prima del blocco. */

				if (! len)
					first = i;
				++len;
			}
			else {

			/* Registra la coppia chiave-lunghezza (se len==0,
			 non succede niente). */

				if (! shv_addfkblk(first,len)) {
				/* ERRORE. Memoria esaurita. */
					plcError(108,"6");
					return 0;
				}
				first = -1;
				len = 0;
			}
		}

	/* Registra l'ultima coppia chiave-lunghezza. */

		if (! shv_addfkblk(first,len)) {
		/* ERRORE. Memoria esaurita. */
			plcError(108,"7");
			return 0;
		}

	/* Marca "inizializzata" la lista. */

		keyfree_ok = 1;

	}

	return 1;
}

/*
* Funzione "shvStat"
* ------------------
* 
* Questa funzione restituisce la dimensione dei blocchi liberi nell'area
* delle variabili condivise. Il risultato e` scaricato nella struttura
* di tipo "shvstat_t" puntata dall'argomento.
* Risultato: 0 se tutto bene, -1 se errore.
*/

int shvStat(shvstat_t *s)
{
keyfree_t *p;

/* Errore se la tabella non e` stata ancora inizializzata. */

	if (! keyfree_ok) {
		return -1;
	}

	s -> free_nvram = main_status.nvram2.addr
	                ? main_status.nvram2.size
	                  - (main_status.nvram2_free - main_status.nvram2.addr)
	                : 0;
	s -> free_heap = *main_status.p_shheap_size;
	s -> free_keys = 0;
	s -> max_free_bkey = 0;

/* Esplora la lista dei blocchi. */

	for (p = keyfree; p; p = p -> next) {
		s -> free_keys += p -> size;
		if (p -> size > s -> max_free_bkey)
			s -> max_free_bkey = p -> size;
	}

	return 0;
}

/*
* Allocazione di un blocco di "n" chiavi a partire dalla "k". Se "k" vale -1,
* si prende il primo blocco disponibile.
*/

static int shv_kalloc(int n, int k)
{
int first,k1,s1,k2,s2;
keyfree_t *p, *q, *pk;

/* Se necessario, ricostruisce la lista dei blocchi. */

	if (! shv_mkfkbkl()) {
	/* ERRORE. Memoria esaurita. */
		plcError(108,"8");
		return -1;
	}

/* Esplora la lista dei blocchi. */

	for (p = keyfree, q = (keyfree_t *) 0; p; q = p, p = p -> next) {

	/* Calcola gli estremi del blocco. Se k == -1, l'inizio
	 di ogni blocco va bene come come primo estremo. */

		if (k == -1)
			first = p -> key;
		else
			first = k;

		k1 = p -> key;
		s1 = first - k1;
		k2 = first + n;
		s2 = p -> size - s1 - n;

		if (k1 <= first && k2 <= k1 + p -> size) {

		/* Trovato ! */

			if (!s1 && !s2) {

			/* Primo caso : il residuo e` vuoto.
			  Bisogna rimuovere il nodo. */

				util_move_cell(
				       (void **) &keyfree,
				       (void **) q,
				       (void **) &keyfree_freelist,
				       (void **) 0,
			               GET_STRUCT_OFFSET(keyfree_t,next));
			}
			else if (s1 && s2) {

			/* Secondo caso : il residuo e` composta da due
			 parti. Bisogna creare un nuovo nodo. */

			/* Alloca la nuova cella. */

				pk = (keyfree_t *) util_insert_cell(
				       (void **) &keyfree,
				       (void **) p,
				       (void **) &keyfree_freelist,
		                       sizeof(keyfree_t),
			               GET_STRUCT_OFFSET(keyfree_t,next),
	                               LEN_KEYFREE_BLOCK);

				if (! pk) {
				/* ERRORE. Memoria esaurita. */
					plcError(108,"9");
					return -1;
				}

			/* Corregge le dimensioni dei due blocchi. */

				p -> size = s1;
				pk -> key = k2;
				pk -> size = s2;
			}
			else if (s1) {

			/* Terzo caso : il residuo e` l'inizio del vecchio
			 blocco. Basta aggiornare la lunghezza. */

				p -> size = s1;
			}
			else {

			/* Quarto caso : il residuo e` la fine del vecchio
			 blocco. Basta aggiornare lunghezza e chiave. */

				p -> key = k2;
				p -> size = s2;
			}

		/* Fine. */

			return first;
		}
	}

/* Non c'e`... */

	return -1;
}

/*******************************/
/* GESTIONE DELLA SYMBOL TABLE */
/*******************************/

/*
* Funzione per il calcolo dell'indice di hash.
*/

static int shv_hash(char * name)
{
char c;
unsigned long l;

	l = 0;

	while ( (c = *(name++)) ) {
		l += c * (l % 37 + 1);
	}

	return (int) (l % main_status.shvh_tab_len);
}

/*
* Ricerca di un nome.
*
* Vale NULL se il nome non e` stato trovato, altrimenti riporta il puntatore
* al descrittore della variabile.
* "hash" conterra` l'indice di hash calcolato dalla funzione,
* "pred" conterra` il puntatore all'elemento precedente nella lista di hash
* (NULL se non c'e` l'elemento precedente).
* Il nome deve essere normalizzato.
*/

static shv_t * shv_find(char * name, int * hash, shv_t ** pred)
{
shv_t *p;
long offset;
int f;

	*hash = shv_hash(name);

	*pred = (shv_t *) 0;

	for (offset = main_status.shvh[*hash];
	     offset;
	     *pred = p, offset = p -> next_hash) {
		p = (shv_t *)((char *)main_status.shm_addr + offset);
		f = strcmp(p -> name, name);
		if (f >= 0) {
			if (f == 0)
				return p;
			break;
		}
	}

	return (shv_t *) 0;
}

/*
* Aggiunta di una variabile alla symbol table.
* Vuole l'indirizzo della nuova cella, l'indice di hash ed il puntatore
* all posizione precedente (tutti dati ricavati dalla "shv_find").
*/

static void shv_add(shv_t * var, int hash, shv_t * pred)
{
register long *dest;

	if (pred)
		dest = &(pred -> next_hash);
	else
		dest = &(main_status.shvh[hash]);
	var -> next_hash = *dest;
	var -> next = main_status.shvn -> next;
	main_status.shvn -> next
	 = *dest
	  = (char *)(var) - (char *)(main_status.shm_addr);
}

/*
* Controllo e normalizzazione di un nome di variabile.
* Il risultato e` riportato in una variabile statica.
* NULL se il nome non e` conforme.
*/

static char * shv_parse(char *name)
{
static char nn[MAX_SHV_NAME+1];
char c;
int i;

	for (i = 0; name[i] && i < MAX_SHV_NAME; ++i) {
		nn[i] = c = toupper(name[i]);
		if ((c < 'A' || c > 'Z')
		    && (c != '$')
		    && (i == 0 || !(c == '_' || (c >= '0' && c <= '9'))))
			return (char *) 0;
	}

	nn[i] = '\0';

	return nn;
}

/*
* Funzioncina di comodo, per esplorare tutte le chiavi di una
* variabile e farci "delle cosine".
*/

static void shv_explore_keys(shv_t * var,
                      int (*cb)(shv_t *var, int idx, void * st),
                      void * st)
{
int nk,i;

	nk = var -> dim1 * var -> dim2;

	for (i = 0; i < nk; ++i)
		if (! (*cb)(var,i,st))
			break;
}

/*
* Funzioncina per azzerare tutte le celle di un segnale di scambio.
*/

/* Nucleo della funzione. */
static int shv_clear_cell(shv_t * var, int idx, void * st)
{
char *addr;

	if (var -> mode & SHV_MODE_RETAIN)
		addr = (char *)(main_status.nvram2.addr)
		       + main_status.shvk[var -> key + idx].off;
	else
		addr = (char *)(main_status.shm_addr)
		       + main_status.shvk[var -> key + idx].off;
	util_memset(addr, 0, var -> size);
	return 1;
}

static void shv_clear_var(shv_t * var)
{
	shv_explore_keys(var,shv_clear_cell,(void *) 0);
}

/*
* Funzioncina che aggiunge le celle di un segnale di scambio alla lista
* delle variabili a rilevamento di fronte.
*/

/* Nucleo della funzione. */
static int shv_register_edge_cell(shv_t * var, int idx, void * st)
{
char *addr;

	addr = (char *)(main_status.shm_addr)
	       + main_status.shvk[var -> key + idx].off;
	compat_register_edge((compat_edge_t *) st, idx, addr);
	return 1;
}

static int shv_edge_var(shv_t * var)
{
compat_edge_t edge;

	edge.n = var -> dim1 * var -> dim2;
	edge.size = var -> size;
	if (! compat_add_edge(&edge)) {
	/* ERRORE. Esaurite le celle per i fronti. */
		plcError(109,var -> name);
		return 0;
	}
	shv_explore_keys(var,shv_register_edge_cell,(void *) &edge);
	return 1;
}

/*
* Funzioncina per riempire la tabella inversa chiave/descrittore.
*/

/* Nucleo della funzione. */
static int shv_assign_revkey(shv_t * var, int idx, void * st)
{
	main_status.shvkey_to_descr[var -> key + idx] = var;
	return 1;
}

static void shv_fill_reverse_ktab(shv_t * var)
{
	shv_explore_keys(var,shv_assign_revkey,(void *) 0);
}

/*
* Funzione per copiare da/verso un segnale di scambio a/da un buffer.
*/

static void shv_copy(shv_t * var, char * buf, int dir)
{
long off;
char * base;
int i,nk,size;

	if (var -> mode & SHV_MODE_RETAIN)
		base = (char *) main_status.nvram2.addr;
	else
		base = (char *) main_status.shm_addr;
	size = var -> size;
	nk = var -> dim1 * var -> dim2;
	if (var -> mode & SHV_MODE_CONTIG) {
		size *= nk;
		nk = 1;
	}
	nk += var -> key;
	for (i = var -> key; i < nk; ++i) {
		off = main_status.shvk[i].off;
		if (dir)
			util_memcpy(base + off, buf, size);
		else
			util_memcpy(buf, base + off, size);
		buf += size;
	}
}

/*
* Funzione shvSet
* ---------------
*
*  Questa funzione inizializza una variabile a partire da un'immagine
* data.
*/

void shvSet(shv_t * var, char * img)
{
	shv_copy(var,img,1);
}

/*
* Funzione shvGet
* ---------------
*
*  Questa funzione copia in un buffer una variabile.
*/

void shvGet(shv_t * var, char * img)
{
	shv_copy(var,img,0);
}

/*
* Funzione shvInit
* ----------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* reinizializzarle, se richiesto, e, in ogni caso, per registrare
* quelle con l'attributo "EDGE" nella lista delle variabili a
* rilevamento di fronte. Questa
* funzione deve essere chiamata all'inizializzazione del Plc.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int shvInit(void)
{
shv_t * var;
long v;
int nb;

/* Alloca la tabella di conversione inversa chiave/descrittore esteso. */

	nb = sizeof(shv_t *) * main_status.shvk_tab_len;
	if (! main_status.shvkey_to_descr) {
		main_status.shvkey_to_descr = (shv_t **) malloc(nb);
		if (! main_status.shvkey_to_descr) {
			plcError(108,"10");
			return -1;
		}
	}
	memset((char *)(main_status.shvkey_to_descr),0,nb);

	for (v = main_status.shvn -> next; v; v = var -> next) {
		var = (shv_t *)((char *)(main_status.shm_addr) + v);

	/* Registra l'indirizzo del descrittore esteso nella
	 tabella inversa. */

		shv_fill_reverse_ktab(var);

	/* Azzera, se richiesto. */

		if (main_status.shv_init)
			shv_clear_var(var);

	/* Rigenera, se non ancora fatto. */

		if (main_status.shv_mkedge && var -> mode & SHV_MODE_EDGE)
			shv_edge_var(var);
	}

/* La prossima volta non saranno registrate altre variabili a rilevamento
 di fronte. */

	main_status.shv_mkedge = 0;

	return 0;
}

/*
* Funzione shvRestart
* -------------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* azzerare tutte quelle con l'attributo "SHV_MODE_RESET".
*  Questa funzione dovrebbe essere chiamata ad ogni GO del PLC.
*/

void shvRestart(void)
{
shv_t * var;
long v;

	for (v = main_status.shvn -> next; v; v = var -> next) {
		var = (shv_t *)((char *)(main_status.shm_addr) + v);
		if (var -> mode & SHV_MODE_RESET)
			shv_clear_var(var);
	}
}

/*
* Funzione shvDeleteRetain
* ------------------------
*
*  Questa funzione esplora la lista delle variabili condivise per
* marcare "cancellate" tutte quelle con l'attributo "SHV_MODE_RETAIN".
*  Questa funzione dovrebbe essere chiamata ad ogni GO del PLC.
*/

void shvDeleteRetain(void)
{
shv_t * var;
long v;

	for (v = main_status.shvn -> next; v; v = var -> next) {
		var = (shv_t *)((char *)(main_status.shm_addr) + v);
		if (var -> mode & SHV_MODE_RETAIN)
			var -> mode |= SHV_MODE_DELETED;
	}
/* Riporta il puntatore alla RAM non volatile libera all'inizio.
  Lascia quattro byte all'inizio dell'area per il timestamp. */
	main_status.nvram2_free = main_status.nvram2.addr+sizeof(long);
}

/*
* Funzione "shvDefine"
* --------------------
*
*  Crea la variabile condivisa "name", di "dim1*dim2" elementi.
*  Se "key" e` diversa da -1, tenta di assegnargliela,
* altrimenti la chiave sara` scelta d'ufficio. "mode" decide la
* modalita` di creazione della variabile e la dimensione della cella.
* NOTA : Tutti questi parametri sono campi della struttura "v".
*  Il flag "change" indica all'uscita se le caratteristiche della
* variabile sono diverse da quelle che l'utente si aspettava. E` una
* composizione di bit, ciascuno dei quali segnala una differenza in
* una certa proprieta`. Le maschere dei vari bit sono le macro SHV_CHG_...
*/

/* Funzioncina per il controllo della compatibilita` tra tipi diversi. */
static int shv_compare_types(int m1,int m2)
{
	if (m1 == SHV_MODE_TYPE_UCHAR)
		m1 = SHV_MODE_TYPE_CHAR;
	else if (m1 == SHV_MODE_TYPE_ULONG)
		m1 = SHV_MODE_TYPE_LONG;
	if (m2 == SHV_MODE_TYPE_UCHAR)
		m2 = SHV_MODE_TYPE_CHAR;
	else if (m2 == SHV_MODE_TYPE_ULONG)
		m2 = SHV_MODE_TYPE_LONG;
	return m1 == m2;
}

/* Struttura "ad hoc" per il passaggio dei parametri al nucleo della
 procedura di assegnamento dell'indirizzo delle celle. */
struct shv_asgn_addr_t {
	char *base_addr;
	char *var_addr;
};

/* Funzioncina per l'assegnamento dell'indirizzo ad una cella. */
static int shv_assign_cell(shv_t * var, int idx, void * st)
{
shvar_t * t;
struct shv_asgn_addr_t *b = (struct shv_asgn_addr_t *) st;
char *addr;

	t = &main_status.shvk[var -> key + idx];

/* Usa, se possibile, il campo "filler". */

	if (b -> var_addr)
		addr = (char *)(b -> var_addr) + idx*(var -> size);
	else
		addr = (char *)(&(t -> filler));
	t -> off = addr - b -> base_addr;
	t -> filler = 0;
	t -> dim = 1;

	return 1;
}

int shvDefine(shv_t * v, int * change)
{
int hash;
shv_t * pred;
shv_t * var;
int dim1,dim2,size,mode,key,dim,nk,retain;
char *name,*base,*p;
struct shv_asgn_addr_t b;
int f = 0; /* Immagine del risultato che andra` a finire in "*change" */

/* Copia i dati dalla struttura. */

	dim = 0;
	name = v -> name;
	dim1 = v -> dim1;
	dim2 = v -> dim2;
	mode = v -> mode & ~(SHV_MODE_DELETED);
	key = v -> key;
	retain = mode & SHV_MODE_RETAIN;

/* Controlla e normalizza il nome. */

	p = shv_parse(name);

	if (! p) {
	/* ERRORE. Nome non conforme. */
		if (!(mode & SHV_MODE_TEST))
			plcError(102,"%s",name);
		return -1;
	}
	name = p;

/* Controlla la sensatezza dei dati. */

	if (!(mode & SHV_MODE_TEST) && (dim1 <= 0 || dim2 <= 0)) {
	/* ERRORE. Dati scorretti. */
		plcError(102,name);
		return -1;
	}

/* Altro controllo di congruenza. */

	switch (mode & SHV_MODE_TYPE) {
	case SHV_MODE_TYPE_BOOL:
	case SHV_MODE_TYPE_CHAR:
	case SHV_MODE_TYPE_UCHAR:
		size = sizeof(char);
		break;
	case SHV_MODE_TYPE_LONG:
	case SHV_MODE_TYPE_ULONG:
		size = sizeof(long);
		break;
	case SHV_MODE_TYPE_FLOAT:
		size = sizeof(float);
		break;
	case SHV_MODE_TYPE_DOUBLE:
		size = sizeof(double);
		break;
	case SHV_MODE_TYPE_EXTENDED:
	default:
		size = v -> size;
		break;
	}

/* Trova il descrittore della variabile. */

	var = shv_find(name, &hash, &pred);

/* Nella modalita` "TEST" si controlla solo che la variabile esista gia`. */

	if (mode & SHV_MODE_TEST) {
		if (! var) {
		/* ERRORE. La variabile non c'e`. */
			return -1;
		}
		else {

		/* Restituisce le caratteristiche della variabile. */

			*v = *var;
			return var -> key;
		}
	}

/* Errore se la variabile esiste gia` e si e` impostata la
 creazione esclusiva. */

	if ((mode & SHV_MODE_EXCL) && var) {
	/* ERRORE. La variabile esiste gia`. */
		return -1;
	}

/* Allocazione della variabile, se necessario. */

	if (! var) {
		var = (shv_t *) shv_malloc(sizeof(*var));
		if (! var) {
		/* ERRORE. Memoria esaurita. */
			plcError(108,"10");
			return -1;
		}

	/* Copia le caratteristiche della variabile. */

		*var = *v;

	/* Assegna la dimensione. */

		var -> size = size;

	/* Calcola il numero di chiavi richiesto. */

		nk = dim1 * dim2;

	/* Se gli elementi sono di un byte, la variabile non e`
	 di tipo "RETAIN", e non e` richiesta
	 l'allocazione contigua, si puo` riciclare il campo "filler"
	 nella tabella delle chiavi. */

		if (size != 1 || (mode & SHV_MODE_CONTIG) || retain) {

	/* Altrimenti si alloca lo spazio necessario, contiguo per
	 forza. La contiguita` e` debitamente segnalata tra le proprieta`. */

			var -> mode |= SHV_MODE_CONTIG;

		/* Calcola l'ammontare di memoria necessario a contenere la
		 variabile. */

			dim = size * nk;

		/* Alloca l'area e la azzera, se richiesto. */

			if (retain)
				base = (char *) shv_nvmalloc(dim);
			else
				base = (char *) shv_malloc(dim);
			if (! base) {
			/* ERRORE. Memoria esaurita. */
				plcError(108,"11");
				shv_free(sizeof(*var));

				return -1;
			}
			if (! retain)
				util_memset(base,0,dim);
		}
		else {
			base = (char *) 0;
		}

	/* Alloca il numero di chiavi richiesto. */

		key = shv_kalloc(nk, key);

		if (key == -1) {
		/* ERRORE. Non c'e` piu` spazio nella tabella. */
			plcError(100,name);

			if (base)
				if (retain)
					shv_nvfree(dim);
				else
					shv_free(dim);

			shv_free(sizeof(*var));

			return -1;
		}

	/* Registra la chiave nella struttura. */

		var -> key = key;

	/* Aggiunge il descrittore nella tabella inversa. */

		shv_fill_reverse_ktab(var);

	/* Inizializza le chiavi. Per le variabili in RAM non
	 volatile si deve usare la base dell'area nvram. */

		b.base_addr = retain ? (char *) main_status.nvram2.addr
		                     : (char *) main_status.shm_addr;
		b.var_addr = base;
		shv_explore_keys(var,shv_assign_cell,(void *) &b);

	/* Se la variabile e` a rilevamento di fronte, riserva il numero
	 di celle richiesto. */

		if (mode & SHV_MODE_EDGE) {
			if (! shv_edge_var(var)) {
		/* In caso di insuccesso, rimuove la proprieta` "EDGE"
		 e lo segnala in "*change". */
				f |= SHV_CHG_EDGE;
				var -> mode &= ~SHV_MODE_EDGE;
			}
		}

	/* Le chiavi matriciali hanno il numero di colonne
	 nell'elemento [0][0]. */

		main_status.shvk[key].dim = dim2;

	/* Aggiunge la variabile alle liste che le competono. */

		shv_add(var, hash, pred);

	}
	else {

	/* Controlla che i dati siano congruenti, o, almeno, compatibili
	 con quelli della variabile predefinita. I confiltti sono
	 segnalati dalla variabile "*change", ma non per questo la
	 funzione fallisce. E` compito del chiamante decidere cosa
	 fare se le cose non vanno come ci si aspetta. */

		if (size != var -> size)
			f |= SHV_CHG_SIZE;
		if (dim1 != var -> dim1)
			f |= SHV_CHG_DIM1;
		if (dim2 != var -> dim2)
			f |= SHV_CHG_DIM2;
		if (key != -1 && key != var -> key)
			f |= SHV_CHG_KEY;
		if (!shv_compare_types(mode & SHV_MODE_TYPE,
		                       var -> mode & SHV_MODE_TYPE))
			f |= SHV_CHG_TYPE;
		if ((mode & SHV_MODE_TYPE) != (var -> mode & SHV_MODE_TYPE))
			f |= SHV_CHG_ITYPE;
		if ((mode & SHV_MODE_RETAIN)!=(var -> mode & SHV_MODE_RETAIN))
			f |= SHV_CHG_RETAIN;
		if ((mode & SHV_MODE_CONTIG)
		    && !(var -> mode & SHV_MODE_CONTIG))
			f |= SHV_CHG_CONTIG;
		if ((mode & SHV_MODE_EDGE) && !(var -> mode & SHV_MODE_EDGE)) {
		/* Caso particolare: prima di dire che non sono compatibili,
		 conviene provare a farli diventare ! */
			if (! shv_edge_var(var))
				f |= SHV_CHG_EDGE;
			else
				var -> mode |= SHV_MODE_EDGE;
		}
		if ((mode & SHV_MODE_RESET)!=(var -> mode & SHV_MODE_RESET)) {
		/* Altro caso particolare: l'attributo "RESET", se richiesto,
		 e` sempre impostato. */
			if (mode & SHV_MODE_RESET)
				var -> mode |= SHV_MODE_RESET;
			else
				f |= SHV_CHG_RESET;
		}

	/* Uno schifoso, rivoltante, abominevole trucco. Si tratta di una
	 pezza "ad hoc" per risolvere "in qualche modo" il problema della
	 non cancellabilita` delle variabili RETAIN. Ora, ad ogni GO
	 tali variabili sono marcate "DELETED". Si provvede qui a ridefinire
	 una variabile che possieda gli attributi RETAIN e DELETED. Se
	 non ci sono cambiamenti nella configurazione della RAM non volatile,
	 la ridefinizione delle variabili RETAIN al GO produce
	 identici assegnamenti di indirizzi ogni volta. */

		if ((var -> mode & (SHV_MODE_RETAIN | SHV_MODE_DELETED))
		   == (SHV_MODE_RETAIN | SHV_MODE_DELETED)) {
			if ((f & (SHV_CHG_RETAIN | SHV_CHG_SIZE
			        | SHV_CHG_DIM1 | SHV_CHG_DIM2))) {

			/* Se le caratteristiche sono irreparabilmente diverse,
			 si cancella brutalmente il nome precedente, si tenta
			 la rigenerazione ex novo, e pazienza per la memoria
			 buttata via... */
			 
				var -> name[0] = '\0';
				return shvDefine(v, change);
			}
			else {

			/* Se le caratteristiche sono ragionevolmente
			 compatibili, si riassegna l'indirizzo alla
			 variabile. */

				var -> mode &= ~SHV_MODE_DELETED;

			/* Calcola l'ammontare di memoria necessario a
			 contenere la variabile. */

				key = var -> key;
				nk = dim1 * dim2;
				dim = size * nk;

			/* Alloca l'area e la azzera, se richiesto. */

				base = (char *) shv_nvmalloc(dim);

			/* Inizializza le chiavi. */

				b.base_addr = (char *) main_status.nvram2.addr;
				b.var_addr = base;
				shv_explore_keys(var,shv_assign_cell,
				                 (void *) &b);

			/* Le chiavi matriciali hanno il numero di colonne
			 nell'elemento [0][0]. */

				main_status.shvk[key].dim = dim2;

			}
		}
	}

	if (change)
		*change = f;

	*v = *var;

	return var -> key;
}

/*
* Funzione "shvCreate"
* --------------------
*
*  Crea la variabile condivisa "name", di "dim1*dim2" elementi.
*  Se "key" e` diversa da -1, tenta di assegnargliela,
* altrimenti la chiave sara` scelta d'ufficio. "mode" decide la
* modalita` di creazione della variabile e la dimensione della cella.
* NOTA : Tutti questi parametri sono campi della struttura "v".
*  Il flag "change" indica all'uscita se le caratteristiche della
* variabile sono diverse da quelle che l'utente si aspettava.
*/

int shvCreate(shv_t * v, int * change)
{
int chg,rv;

	rv = shvDefine(v,&chg);
	if (rv >= 0 && change) {
		*change = (  chg
		           & (  SHV_CHG_SIZE
		              | SHV_CHG_DIM1
		              | SHV_CHG_DIM2
		              | SHV_CHG_KEY
		              | SHV_CHG_RETAIN
		              | SHV_CHG_CONTIG
		              | SHV_CHG_TYPE)
		          ) != 0;
	}
	return rv;
}

/*
* Funzione "shvKeyToAddr"
* -----------------------
*
*  Questa funzione calcola l'indirizzo della variabile condivisa di
* chiave "key" ed indici "n" ed "m". Se la variabile e` scalare, gli indici
* devono valere 0 e 0. Se la variabile e` un vettore, il secondo indice
* deve valere 0. Se la chiave non coincide con quella della base di
* una variabile multidimensionale, "n" ed "m" devono valere 0.
*  Se la variabile e` un vettore o una matrice, ed e` contigua (flag
* SHV_MODE_CONTIG acceso nel campo della proprieta`) l'indirizzo dell'elemento
* 0,0 e` la base dell'area che le compete, vale cioe` la regola :
*
*     addr(v[i][j]) == addr(v[0][0]) + dim2(v)*i*size(v) + j*size(v)
*
*  Se la variabile non e` contigua, la regola si applica solo alle chiavi :
*
*     key(v[i][j]) == key(v[0][0]) + dim2(v)*i + j
*
*  Quest'ultima regola e` sempre valida.
*
*  La funzione restituisce l'indirizzo richiesto, oppure NULL in caso di
* errori. Si ha errore se la chiave non esiste e se gli indici sono
* maggiori delle dimensioni impostate.
*/

void *shvKeyToAddr(int key, int n, int m)
{
shv_t * v;
long off;

	if (key < 0
	    || key >= main_status.shvk_tab_len
	    || (!(v = main_status.shvkey_to_descr[key]))
	    || (v -> key != key && (n != 0 || m != 0))
	    || n < 0
	    || v -> dim1 <= n
	    || m < 0
	    || v -> dim2 <= m) {
	/* ERRORE. Chiave non assegnata o dati scorretti. */
		return (void *) 0;
	}
	off = main_status.shvk[key + v -> dim2 * n + m].off;
	if (v -> mode & SHV_MODE_RETAIN)
		return (void *)((char *) main_status.nvram2.addr + off);
	else
		return (void *)((char *) main_status.shm_addr + off);
}

/*
* Funzione shvAddrToVar
* ---------------------
*
*  Questa funzione esplora la lista delle variabili condivise
* alla ricerca dell'elemento piu` vicino all'indirizzo "addr".
*  Il descrittore della variabile e` riportato in "ret". "key" e` il
* puntatore ad un intero, che all'uscita conterra` la chiave
* associata all'elemento trovato. Serve per elementi vettoriali,
* nei quali la chiave puo` differire da quella dell'elemento [0][0].
* La differenza tra chiave corrente e chiave base permette di ricavare gli
* indici dell'elemento.
*  La funzione vale 0 in caso di successo, -1 in caso di errore.
*/

int shvAddrToVar(void *addr, shv_t *ret, int *key)
{
shv_t *var;
long v;
int k,dim;

	for (v = main_status.shvn -> next; v; v = var -> next) {
		var = (shv_t *)((char *)(main_status.shm_addr) + v);
		dim = var -> key + var -> dim1 * var -> dim2;
		for (k = var -> key; k < dim; ++k) {
			if (addr == shvKeyToAddr(k,0,0)) {
				*ret = *var;
				*key = k;
				return 0;
			}
		}
	}

	return -1;
}

