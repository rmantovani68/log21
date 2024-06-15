/*
* @(#) symtab.c 1.7 Mon Sep 22 09:58:28 MET DST 1997
* @(#) Rif. Giuseppe Guerrini
*
*  Gestione della symbol table del compilatore ILC.
*
* 04/07/96 GG 1.0 Prima stesura.
* 17/07/96 GG 1.1 Eliminato il campo "list" dagli elementi delle tabelle
*             di hash. Ora "list" e` una proprieta` del contesto.
* 18/07/96 GG 1.2 Aggiunta la funzione "st_explore_context".
* 18/07/96 GG 1.3 Migliorata la politica di creazione dei tipi predefiniti.
* 29/08/96 GG 1.4 Modificate le dimensioni dedi tipi "tempo".
* 03/09/96 GG 1.5 Corretta una voragine nelle routine di allocazione dei
*             dati inizializzati e delle liste di parametri.
* 14/10/96 GG 1.6 Iniziati i lavori per la gestione delle stringhe.
* 22/09/97 GG 1.7 Un piccolo aggiustamento per compilazione su NT.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sysdep.h"
#include "conf.h"
#include "util.h"
#include "symtab.h"
#include "ilc.h"

/* FUNZIONI */

/*
* Allocazione di descrittori di contesto.
*/

typedef struct _context_pool_t {
	memory_block_t header;
	context_t block[LEN_CONTEXT_BLOCK];
} context_pool_t;

static context_pool_t *context_pool = (context_pool_t *) 0;

static context_t *st_context_alloc(void)
{
context_t *p;

	p = (context_t *) util_alloc(LEN_CONTEXT_BLOCK,
	                             sizeof(context_t),
	                             (memory_block_t **) &context_pool);

	if (p) {
		p -> n_hash = 1;
		p -> hash = p -> mini_hash;
	}

	return p;
}

/*
* Allocazione di descrittori di simbolo.
*/

typedef struct _symbol_pool_t {
	memory_block_t header;
	symbol_t block[LEN_SYMBOL_BLOCK];
} symbol_pool_t;

static symbol_pool_t *symbol_pool = (symbol_pool_t *) 0;

static symbol_t *st_symbol_alloc(void)
{
	return (symbol_t *) util_alloc(LEN_SYMBOL_BLOCK,
	                             sizeof(symbol_t),
	                             (memory_block_t **) &symbol_pool);
}
/*
* Allocazione di descrittori di tipo.
*/

typedef struct _type_pool_t {
	memory_block_t header;
	type_t block[LEN_TYPE_BLOCK];
} type_pool_t;

static type_pool_t  *type_pool = (type_pool_t *) 0;

static type_t *st_type_alloc(void)
{
	return (type_t *) util_alloc(LEN_TYPE_BLOCK,
	                             sizeof(type_t),
	                             (memory_block_t **) &type_pool);
}

/*
* Allocazione di descrittori di variabile.
*/

typedef struct _var_pool_t {
	memory_block_t header;
	var_t block[LEN_VAR_BLOCK];
} var_pool_t;

static var_pool_t  *var_pool = (var_pool_t *) 0;

static var_t *st_var_alloc(void)
{
	return (var_t *) util_alloc(LEN_VAR_BLOCK,
	                             sizeof(var_t),
	                             (memory_block_t **) &var_pool);
}

/*
* Allocazione di descrittori di valore iniziale, per l'inizializzazione
* di vettori.
*/

typedef struct _inival_pool_t {
	memory_block_t header;
	inival_t block[LEN_INIVAL_BLOCK];
} inival_pool_t;

static inival_pool_t  *inival_pool = (inival_pool_t *) 0;

inival_t *st_inival_alloc(void)
{
	return (inival_t *) util_alloc(LEN_INIVAL_BLOCK,
	                             sizeof(inival_t),
	                             (memory_block_t **) &inival_pool);
}

/*
* Allocazione di descrittori di contesto "per file".
*/

typedef struct _filescope_pool_t {
	memory_block_t header;
	filescope_t block[LEN_FILESCOPE_BLOCK];
} filescope_pool_t;

static filescope_pool_t  *filescope_pool = (filescope_pool_t *) 0;

filescope_t *st_filescope_alloc(void)
{
	return (filescope_t *) util_alloc(LEN_FILESCOPE_BLOCK,
	                             sizeof(filescope_t),
	                             (memory_block_t **) &filescope_pool);
}

/*
* Allocazione di elementi di lista di parametri per funzioni, programmi
* e blocchi funzionali.
*/

typedef struct _paramlist_pool_t {
	memory_block_t header;
	paramlist_t block[LEN_PARAMLIST_BLOCK];
} paramlist_pool_t;

static paramlist_pool_t  *paramlist_pool = (paramlist_pool_t *) 0;

paramlist_t *st_paramlist_alloc(void)
{
	return (paramlist_t *) util_alloc(LEN_PARAMLIST_BLOCK,
	                             sizeof(paramlist_t),
	                             (memory_block_t **) &paramlist_pool);
}

/*
* Funzione di hash per i nomi. Vuole un nome e la dimensione della
* tabella di hash in cui dovra` essere inserito.
*/

static int st_hash(char *name, int n)
{
char c;
unsigned long l;

	if (n == 1)
		return 0;

	l = 0;

	while ( (c = *(name++)) ) {
		l += c * (l % 37 + 1);
	}

	/* Con tabella di hash di dimensione variabile : */
	/* return (int) (((l % MAX_HASH_LEN) * n) / MAX_HASH_LEN); */

	return (int) (l % MAX_HASH_LEN);
}

/*
* Funzione "st_find_symbol"
* -------------------------
*
*  Questa funzione cerca un simbolo nel contesto dato.
*  Riporta in "pos" le informazioni relative alla posizione.
*/

static symbol_t *st_find_symbol(char *name, context_t *context, symbol_position_t *pos)
{
symbol_t *p, **q;
int h,f,d;

	pos -> name = name;
	pos -> context = context;
	pos -> hash = h = st_hash(name,context -> n_hash);

	q = &(context -> hash[h].tree);

	d = 0;

	while ( (p = *q) ) {
		f = strcmp(name,p -> name);
		if (f == 0)
			break;
		++d;
		if (f < 0)
			q = &(p -> inf);
		else
			q = &(p -> sup);
	}

	pos -> parent = q;
	pos -> depth = d;
	pos -> node = p;

	return p;
}

/*
* Funzione "st_make_list_from_tree"
* --------------------------------
*
*  Questa funzione costruisce una lista ordinata sull'albero binario dato.
*/

/* Variabili statiche. */
static int st_list_length = 0;
static symbol_t ** st_current_hook = (symbol_t **) 0;

/* Nucleo della procedura. */
static void st_make_list_from_tree_core(symbol_t *tree)
{
	while (tree) {
		++st_list_length;
		st_make_list_from_tree_core(tree -> inf);
		tree -> inf = *st_current_hook;
		*st_current_hook = tree;
		st_current_hook = &(tree -> inf);
		tree = tree -> sup;
	}
}

/* Funzione principale. */
static symbol_t * st_make_list_from_tree(symbol_t *tree, symbol_t **list, int *n)
{
	*list = (symbol_t *) 0;
	st_current_hook = list;
	st_list_length = 0;
	st_make_list_from_tree_core(tree);
	*n = st_list_length;
	return *list;
}

/*
* Funzione "st_make_tree_from_list"
* --------------------------------
*
*  Questa funzione ricrea un albero binario bilanciato a partire dalla
* lista "list" di "nn" elementi.
*/

/* Variabili statiche. */
static symbol_t * st_current_node = (symbol_t  *) 0;

/* Nucleo della procedura. */
static symbol_t * st_make_tree_from_list_core(int n)
{
symbol_t *inf,*curr;

	if (n == 0)
		return (symbol_t *) 0;
	inf = st_make_tree_from_list_core(n / 2);
	curr = st_current_node;
	st_current_node = curr -> inf;
	curr -> inf = inf;
	curr -> sup = st_make_tree_from_list_core((n - 1) / 2);
	return curr;
}

static symbol_t * st_make_tree_from_list(symbol_t *list, int n)
{
	st_current_node = list;
	return st_make_tree_from_list_core(n);
}

/*
* Funzione "st_optimize_tree"
* --------------------------
*
*  Questa funzione bilancia l'albero binario dato.
*/

static void st_optimize_tree(hash_t *hash)
{
symbol_t *list;

	st_make_list_from_tree(hash -> tree, &list, &(hash -> nn));
	hash -> tree = st_make_tree_from_list(list, hash -> nn);
}

/*
* Funzione "st_add_node"
* ----------------------
*
*  Questa funzione aggiunge un nodo alla posizione di hash data.
*/

static void st_add_node(symbol_position_t *pos)
{
symbol_t *node;
hash_t *hash;
unsigned long theor_limit;

	hash = &(pos -> context -> hash[pos -> hash]);
	node = pos -> node;
	*(pos -> parent) = node;
	node -> inf = node -> sup = (symbol_t *) 0;
	theor_limit = 1L << pos -> depth;
	if ( theor_limit / (++(hash -> nn)) > MAX_TREE_UNB) {

	/* L'albero e` troppo sbilanciato. Bisogna riequilibrarlo. */

		st_optimize_tree(hash);
	}
}

/*
* Funzione "st_add_sym"
* ----------------------
*
*  Questa funzione aggiunge un simbolo alla posizione di hash ed al contesto
* dati.
*/

static void st_add_sym(symbol_position_t *pos)
{
symbol_t *node;
context_t *context;

	node = pos -> node;
	context = pos -> context;

	if (context -> list)
		context -> list_tail -> next = node;
	else
		context -> list = node;
	context -> list_tail = node;

	node -> next = (symbol_t *) 0;

	st_add_node(pos);
}

/*
* Funzione "st_optimize_context"
* ------------------------------
*
*  Questa funzione aumenta la dimensione di un contesto se la profondita`
* raggiunta dagli alberi che contiene e` superiore ad una certa soglia.
*/

static int _st_reinsert_node(context_t * context, symbol_t * sym, void *user)
{
symbol_position_t pos;

	st_find_symbol(sym -> name,context,&pos);
	pos.node = sym;
	st_add_node(&pos);

	return 1;
}

static void st_optimize_context(context_t *context)
{
hash_t *nh;

	if (context -> n_hash != 1 || context -> hash[0].nn < HASH_THRESHOLD)
		return;

/* Alloca la tabella di hash "grande". */

	nh = (hash_t *) util_malloc(MAX_HASH_LEN * sizeof(hash_t));

/* Esplora la lista e redistribuisce i nodi sugli "alberini"
 che fanno capo agli elementi della tabella di hash. */

	/* Assegna la nuova tabella di hash al contesto. */
	context -> hash = nh;
	context -> n_hash = MAX_HASH_LEN;

	st_explore_context(context, _st_reinsert_node, NULL);
}

/*
* Funzione "st_add_new_name"
* --------------------------
*
*  Questa funzione aggiunge un nuove nome al contesto dato, nella posizione
* ed alla profondita` indicati.
*  Riporta l'indirizzo del nodo aggiunto.
*/

static symbol_t *st_add_new_name(symbol_position_t *pos)
{
symbol_t *p;

/* Alloca un nodo. */
 
	p = st_symbol_alloc();

/* Assegna il nome al nodo. */

	p -> name = pos -> name;
	pos -> node = p;

/* Aggiunge il nuovo nodo al contesto. */

	st_add_sym(pos);

/* Riorganizza la tabella di hash. */

	st_optimize_context(pos -> context);

	return p;
}

/*
* Funzione "st_add_name"
* ----------------------
*
*  Questa funzione aggiunge un nome al contesto dato.
*  Riporta l'indirizzo del nodo aggiunto.
*  In "found" riporta se il simbolo esisteva gia`.
*/

symbol_t *st_add_name(char *name, context_t *context, int *found)
{
symbol_position_t pos;
symbol_t *p;

/* Cerca il simbolo. */

	p = st_find_symbol(name, context, &pos);
	if (p) {

	/* Trovato. Il simbolo era gia` presente. */

		*found = 1;
		return p;
	}

/* Il simbolo non era presente. Bisogna aggiornare la struttura. */

	*found = 0;

/* Alloca ed aggiunge il nodo. */

	return st_add_new_name(&pos);
}

/*
* Funzione "st_create_context"
* ----------------------------
*
*  Questa funzione crea un contesto di nome dato. Il nome
* ha solo un valore documentativo.
*/

context_t *st_create_context(char *name)
{
context_t *p;

	p = st_context_alloc();

	p -> name = name;
	p -> list = (symbol_t *) 0;
	p -> list_tail = (symbol_t *) 0;
	p -> n_hash = 1;
	p -> hash = p -> mini_hash;
	p -> mini_hash[0].tree = (symbol_t *) 0;
	p -> mini_hash[0].nn = 0;

	return p;
}

/*
* Funzione "st_reset_context"
* ----------------------------
*
*  Questa funzione crea o reinizializza un contesto di nome dato. Il nome
* ha solo un valore documentativo.
*/

void st_reset_context(context_t ** context, char *name)
{
int i;

	if (*context) {
		(*context) -> name = name;
		(*context) -> list = (symbol_t *) 0;
		(*context) -> list_tail = (symbol_t *) 0;
		for (i = 0; i < (*context) -> n_hash; ++i) {
			(*context) -> hash[i].tree = (symbol_t *) 0;
			(*context) -> hash[i].nn = 0;
		}
	}
	else {
		*context = st_create_context(name);
	}
}

/*
* Funzioni "st_push_context", "st_pop_context", "st_reset_context_stack"
* ----------------------------------------------------------------------
*
*  Queste funzioni manipolano la lista dei contesti di ricerca dei nomi
* attivi. La lista e` gestita come uno stack. L'ultimo contesto inserito
* e` il primo ad essere controllato durante la ricerca dei nomi.
*/

static int st_active_context_num = 0;
static context_t *st_active_context_list[MAX_ACTIVE_CONTEXT];

void st_reset_context_stack(void)
{
	st_active_context_num = 0;
}

void st_push_context(context_t * context)
{
	if (st_active_context_num >= MAX_ACTIVE_CONTEXT - 1) {
		util_fatal(1000);
	}
	st_active_context_list[st_active_context_num++] = context;
}

void st_pop_context(void)
{
	if (st_active_context_num <= 0) {
		util_fatal(1001);
	}
	--st_active_context_num;
}

/*
* Funzione "st_explore_context"
* -----------------------------
*
*  Questa funzione esplora la lista globale dei simboli di un contesto e,
* per ciascun elemento, chiama la funzione "fn", passando il contesto, 
* il simbolo corrente ed un puntatore generico "user" come parametri.
* L'esplorazione termina alla fine della
* lista o quando la funzione chiamata restituisce 0.
*/

void st_explore_context(context_t *context, int (*fn)(context_t *, symbol_t *, void *), void *user)
{
symbol_t *loc_sym;

	for (loc_sym = context -> list; loc_sym; loc_sym = loc_sym -> next) {
		if (! (*fn)(context, loc_sym, user))
			break;
	}
}

/*
* Funzione "st_create_type"
* -------------------------
*
*  Questa funzione alloca ed inizializza una struttura descrivente un tipo
* della categoria specificata. Se il tipo e` uno dei predefiniti, il
* descrittore viene prima cercato nella tabella apposita, e creato solo
* se assente.
*/

static type_t * st_predef_types[TYPE_STRING + 1] = { (type_t *)0 };
static int st_predef_types_init = 0;
static char st_predef_types_size[TYPE_STRING + 1] =
{
0, 1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 4, 8, 4, 4, 4, 8, 4,
};
static char st_predef_types_flags[TYPE_STRING + 1] =
{
0, TYSCAL_ARITHM, TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_ARITHM,
0, TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_ARITHM, 0,
TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_ARITHM, 0, TYSCAL_SIGNED | TYSCAL_ARITHM,
TYSCAL_ARITHM, 0, TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_SIGNED | TYSCAL_ARITHM,

TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_SIGNED | TYSCAL_ARITHM,
TYSCAL_SIGNED | TYSCAL_ARITHM, TYSCAL_SIGNED | TYSCAL_ARITHM,
0,
};

int st_is_scalar_type(type_t * type)
{
	return type && type -> code >= TYPE_BOOL && type -> code <= TYPE_STRING;
}

int st_is_array_type(type_t * type)
{
	return type && type -> code == TYPE_ARRAY;
}

int st_is_int_type(type_t * type)
{
	return type -> code >= TYPE_BOOL && type -> code < TYPE_REAL;
}

int st_is_unsigned_type(type_t * type)
{
	return !(st_predef_types_flags[type -> code] & TYSCAL_SIGNED);
}

int st_is_binary_type(type_t * type)
{
	return !(st_predef_types_flags[type -> code] & TYSCAL_ARITHM);
}

int st_is_float_type(type_t * type)
{
	return type -> code == TYPE_REAL || type -> code == TYPE_LREAL;
}

int st_get_array_size(type_t * type)
{
int n,i;

	n = 1;
	if (st_is_array_type(type))
		for (i = 0; i < type -> d.a.n_dim; ++i)
			n *= type -> d.a.dim[i];
	return n;
}

type_t *st_create_type(int type_code)
{
type_t *p;

	if (type_code >= TYPE_NONE && type_code <= TYPE_STRING) {
		if (! st_predef_types_init) {
			memset((char *) st_predef_types, 0,
			       sizeof(st_predef_types));
			st_predef_types_init = 1;
		}
		p = st_predef_types[type_code];
		if (! p) {
			p = st_type_alloc();
			st_predef_types[type_code] = p;
			p -> code = type_code;
			p -> size = st_predef_types_size[type_code];
			p -> d.scal_flags = st_predef_types_flags[type_code];
		}
	}
	else {
		p = st_type_alloc();
		p -> code = type_code;
		p -> size = 0;
	}
	return p;
}

/*
* Funzione "st_register_name"
* ---------------------------
*
*  Questa funzione trova un nome nel contesto indicato, e memorizza
* in un'area statica informazioni utili alla sua manipolazione.
*  Riporta l'indirizzo della struttura statica utilizzata come cache.
*  Se il contesto e` NULL, il nome viene cercato nei contesti correnti.
*/

static symbol_position_t st_symbol_cache;

symbol_position_t *st_register_name(char *name, context_t *context)
{
symbol_t *p;
int i;

	st_symbol_cache.context = (context_t *) 0;
	st_symbol_cache.parent = (symbol_t **) 0;
	st_symbol_cache.hash = -1;
	st_symbol_cache.depth = -1;
	st_symbol_cache.name = name;
	st_symbol_cache.node = (symbol_t *) 0;

	if (context) {
		st_find_symbol(name, context, &st_symbol_cache);
	}
	else {
		for (i = st_active_context_num; i--; ) {
			p = st_find_symbol(name,
					   st_active_context_list[i],
					   &st_symbol_cache);
			if (p)
				break;
		}
	}

	return &st_symbol_cache;
}

/*
* Funzione "st_add_registered_name"
* ---------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete.
*/

symbol_t *st_add_registered_name(void)
{
	if (! st_symbol_cache.node)
		st_add_new_name(&st_symbol_cache);
	return st_symbol_cache.node;
}

/*
* Funzione "st_add_registered_name_as_var"
* ----------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, ed alloca, se necessario, un descrittore di variabile.
*/

symbol_t *st_add_registered_name_as_var(void)
{
symbol_t *p;

	p = st_add_registered_name();
	if (! (p -> var)) {
		p -> var = st_var_alloc();
	}
	p -> class = SYMCL_VAR;
	return p;
}

/*
* Funzione "st_add_registered_name_as_type"
* ----------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, ed alloca, se necessario, un descrittore di tipo.
*/

symbol_t *st_add_registered_name_as_type(void)
{
symbol_t *p;

	p = st_add_registered_name();
	if (! (p -> type)) {
		p -> type = st_type_alloc();
	}
	p -> class = SYMCL_TYPE;
	return p;
}

/*
* Funzione "st_add_registered_name_as_string"
* -------------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, e lo marca di classe "stringa".
*/

symbol_t *st_add_registered_name_as_string(void)
{
symbol_t *p;

	p = st_add_registered_name();
	p -> class = SYMCL_STRING;
	return p;
}

