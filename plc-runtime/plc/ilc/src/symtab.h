/*
* @(#) symtab.h 1.3 Mon Oct 14 12:53:25 MET 1996
* @(#) Rif. Giuseppe Guerrini
*
*  Gestione della symbol table del compilatore ILC.
*
* 04/07/96 GG 1.0 Prima stesura.
* 17/07/96 GG 1.1 Eliminato il campo "list" dagli elementi delle tabelle
*             di hash. Ora "list" e` una proprieta` del contesto.
* 18/07/96 GG 1.2 Aggiunta la funzione "st_explore_context".
* 14/10/96 GG 1.3 Iniziati i lavori per la compilazine delle stringhe.
*/

#ifndef _SYMTAB_H_

/*
* Questo e` il tipo base che descrive un contesto.
* Un contesto non e` altro che un elenco di simboli.
*/

typedef struct _context_t context_t;

/*
* Possibili tipi.
*/

enum _type_code_t {
	TYPE_NONE,

/* 1 bit (in realta`, un byte). */

	TYPE_BOOL,

/* 8 bit. */

	TYPE_SINT,
	TYPE_USINT,
	TYPE_BYTE,

/* 16 bit. */

	TYPE_INT,
	TYPE_UINT,
	TYPE_WORD,

/* 32 bit. */

	TYPE_DINT,
	TYPE_UDINT,
	TYPE_DWORD,

/* 64 bit. */

	TYPE_LINT,
	TYPE_ULINT,
	TYPE_LWORD,

/* 32 floating point. */

	TYPE_REAL,

/* 64 floating point. */

	TYPE_LREAL,

/* Formato probabilmente variabile. */

	TYPE_TIME,
	TYPE_DATE,
	TYPE_TOD,
	TYPE_DT,
	TYPE_STRING,

/* Tipo ottenuto per applicazione del costrutto "TYPE...END_TYPE". */

	TYPE_USER,

/* Vettore o matrice. */

	TYPE_ARRAY,

/* Struttura. */

	TYPE_STRUCT,

/* Programma. */

	TYPE_PROGRAM,

/* Funzione. */

	TYPE_FUNCTION,

/* Blocco funzionale. */

	TYPE_FUNCTION_BLOCK,

/* Configurazione. */

	TYPE_CONFIGURATION,

/* Risorsa. */

	TYPE_RESOURCE,

/* Tipo indefinito (referenziato ma non trovato). */

	TYPE_UNDEF
};


/*
* Strutture descriventi vari tipi... di tipi.
*/

typedef struct _type_t {
/* Codice (TYPE_...). */
	enum _type_code_t code;
/* Dimensione in byte. */
	int size;
/* Dati specifici del tipo. */
	union {

	/* Tipi scalari predefiniti. */
		int scal_flags;
/* Proprieta` dei tipi scalari predefiniti. */
#define TYSCAL_SIGNED 0x01	/* Il tipo e` da intendersi con segno. */
#define TYSCAL_ARITHM 0x02	/* Il tipo e` aritmetico (non e` un "byte"). */

	/* TYPE_STRING */
		/* Lunghezza. */
		int str_len;

	/* TYPE_USER */
	/* Puntatore al descrittore. */
		/* typedescr_t * descr; */

	/* TYPE_ARRAY */
		struct {
		/* Tipo base. */
			struct _type_t * type;
		/* Numero di dimensioni. */
			int n_dim;
		/* Dimensioni. */
			int dim[MAX_ARRAY_DIM]; 
		/* Indici base. */
			int idx_base[MAX_ARRAY_DIM];
		} a;

	/* TYPE_STRUCT */
	/* Puntatore al "contesto" associato. */
		context_t *struct_list;

	/* TYPE_PROGRAM */
	/* TYPE_FUNCTION_BLOCK */
	/* TYPE_FUNCTION */
		struct {
		/* Puntatore al "contesto" associato. */
			context_t *context;
		/* Puntatore al simbolo associato al tipo. */
			struct _symbol_t *sym;
		/* Lista ordinata dei parametri di input. */
			struct _paramlist_t *input;
		/* Lista ordinata dei parametri di output. */
			struct _paramlist_t *output;
		/* Puntatore al tipo di ritorno (funzioni). */
			struct _type_t * rv_type;
		/* Puntatore al simbolo associato all'area statica (funz.) */
			struct _symbol_t * statics;
		/* Punto di definizione. Se indefinito, manca il corpo
		 della procedura. */
			char *file;
			int line;
		} fb;

	/* TYPE_CONFIGURATION */
		struct {
		/* Punto di definizione. */
			char *file;
			int line;
		} conf;

	/* TYPE_RESOURCE */
		struct {
		/* Nome della CPU (cio` che segue "ON"). */
			char *cpu;
		/* Punto di definizione. */
			char *file;
			int line;
		} res;

	/* TYPE_NONE */
	/* TYPE_UNDEF */
		/* Nome del simbolo associato al tipo. */
		char * undef_name;
	} d;
} type_t;

/*
* Struttura costituente un elemento di inizializzazione per vettori
* e matrici
*/

typedef struct _inival_t {
	union {
		long l;
		double d;
	} data;
	int count;
	struct _inival_t * next;
} inival_t;

/*
* Struttura contenente le proprieta` di un simbolo.
*/

/* Flag di proprieta`. */

/* Variabile gia` definita. */
#define VAR_F_DEFINED 0x0001
/* Modalita` di utilizzo della variabile (valori e maschera). */
#define VAR_F_PARAM_NONE 0x0000
#define VAR_F_PARAM_IN 0x0002
#define VAR_F_PARAM_OUT 0x0004
#define VAR_F_PARAM_IO 0x0006
#define VAR_F_PARAM_M 0x0006
/* Modalita` di connessione ad I/O o memoria (valori e maschera). */
#define VAR_F_CONNECTION_NONE 0x0000
#define VAR_F_CONNECTION_IN 0x0008
#define VAR_F_CONNECTION_OUT 0x0010
#define VAR_F_CONNECTION_MEMORY 0x0018
#define VAR_F_CONNECTION_M 0x0018
/* Variabile retantiva. */
#define VAR_F_RETAIN 0x0020
/* Costante. */
#define VAR_F_CONSTANT 0x0040
/* Inizializzazione specificata. */
#define VAR_F_INIT 0x0080
/* Variabile utilizzata almeno una volta. */
#define VAR_F_USED 0x0100
/* Variabile assegnata (per MSD). */
#define VAR_F_ASSIGNED 0x0200
/* Variabile locale al file. */
#define VAR_F_LOCAL_FL 0x0400
/* Variabile locale al blocco funzionale. */
#define VAR_F_LOCAL_FB 0x0800

typedef struct _var_t {
/* Proprieta` della variabile. */
	int flags;
/* Punto di definizione. */
	char *file;
	int line;
/* Dati di associazione di I/O o memoria, oppure posizione nella lista
 dei parametri di una funzione, valore di ritorno di una funzione,
 livello di annidamento di un'etichetta. */
	union {
		char * direct_io; /* %M, %I, %Q */
		int param_num;
		type_t *rv_type;
		struct {
			unsigned short base;
			unsigned short serial;
		} ilexpr_level;
	} at;
/* Indirizzo di localizzazione, ad uso del compilatore. */
	unsigned long location;
/* Valore iniziale. */
	union {
	/* Tipi interi. */
		long l;
	/* Tipi a virgola mobile. */
		double d;
	/* Vettori e matrici. */
		inival_t *a;
	} init;
} var_t;

/*
* Possibili classi di apparteneza di un simbolo.
*/

enum _symcl_code_t {
	SYMCL_NULL,
	SYMCL_TYPE,
	SYMCL_VAR,
	SYMCL_LABEL,
	SYMCL_FUNCTION,
	SYMCL_STRING,
};

/*
* Union descrivente le proprieta` di un simbolo.
*/

/*
* Struttura descrivente un nodo dell'albero dei simboli.
*/

typedef struct _symbol_t {
/* Nome del simbolo. */
	char *name;
/* Puntatori ai nodi inferiore e superiore. */
	struct _symbol_t *inf;
	struct _symbol_t *sup;
/* Puntatore al nodo successivo dell'albero, per le esplorazioni
 disordinate. */
	struct _symbol_t *next;
/* Classe del simbolo. */
	enum _symcl_code_t class;
/* Campo descrivente un tipo, interpretato diversamente a seconda
 della classe. In particolare (TRUCCACCIO !) contiene l'indice dell'etichetta
 locale associata a ciascuna stringa (cioe` un INTERO !!!). */
	type_t * type;
/* Puntatore ai campi di descrizione della variabile, se richiesti. */
	var_t * var;
} symbol_t;

/*
* Elemento della tabella di hash.
*/

typedef struct {
/* Puntatore all'albero dei nomi. */
	symbol_t *tree;
/* Numero di nodi dell'albero. */
	int nn;
} hash_t;

/*
* Questo e` il tipo base che descrive un contesto.
* Un contesto non e` altro che un elenco di simboli.
*/

struct _context_t {
/* Nome associato al contesto, per debug. */
	char *name;
/* Puntatore alla lista dei simboli (in ordine di definizione). */
	symbol_t *list;
/* Puntatore all'ultimo elemento della lista (posizione di inserimento). */
	symbol_t *list_tail;
/* Dimensione della tabella di hash. */
	int n_hash;
/* Tabella dei puntatori agli alberi dei simboli. */
	hash_t *hash;
/* Mini-tabella di hash per gli alberi piccoli. */
	hash_t mini_hash[1];
};

/*
* Questo tipo serve a tenere traccia dei diversi ambienti indotti
* da ciascun sorgente elaborato.
*/

typedef struct _filescope_t {
	char * name;
	context_t * global;
	context_t * local;
	struct _filescope_t *next;
} filescope_t;

/*
* Questa struttura contiene le informazioni ottenute dalla "st_find_name"
* e simili. Sintetizza la posizione di inserimento di un simbolo in un
* contesto.
*/

typedef struct {
	context_t *context;
	symbol_t **parent;
	int hash;
	int depth;
	char *name;
	symbol_t *node;
} symbol_position_t;

/*
* Questa struttura rappresenta un elemento di una lista di parametri.
* Le liste di parametri fanno parte delle caratteristiche di funzioni,
* programmi e blocchi funzionali.
*/

typedef struct _paramlist_t {
	symbol_t * sym;
	struct _paramlist_t * next;
} paramlist_t;

/*
* Struttura descrivente una costante stringa.
*/

typedef struct _string_t {
	char * bytes;
	int len;
	int location;
	struct _string_t * next;
} string_t;

/*
* Allocazione di descrittori di valore iniziale, per l'inizializzazione
* di vettori.
*/

inival_t *st_inival_alloc(void);

/*
* Allocazione di descrittori di contesto "per file".
*/

filescope_t *st_filescope_alloc(void);

/*
* Funzione "st_add_name"
* ----------------------
*
*  Questa funzione aggiunge un nome al contesto dato.
*  Riporta l'indirizzo del nodo aggiunto.
*  In "found" segnala se il nome era gia` presente.
*/

symbol_t *st_add_name(char *name, context_t *context, int *found);

/*
* Funzione "st_reset_context"
* ----------------------------
*
*  Questa funzione crea o reinizializza un contesto di nome dato. Il nome
* ha solo un valore documentativo.
*/

void st_reset_context(context_t ** context, char *name);

/*
* Funzione "st_create_context"
* ----------------------------
*
*  Questa funzione crea un contesto di nome dato. Il nome
* ha solo un valore documentativo.
*/

context_t *st_create_context(char *name);

type_t *st_create_type(int type_code);

int st_is_scalar_type(type_t * type);

int st_is_array_type(type_t * type);

int st_get_array_size(type_t * type);

int st_is_int_type(type_t * type);

int st_is_unsigned_type(type_t * type);

int st_is_binary_type(type_t * type);

int st_is_float_type(type_t * type);

/*
* Funzioni "st_push_context", "st_pop_context", "st_reset_context_stack"
* ----------------------------------------------------------------------
*
*  Queste funzioni manipolano la lista dei contesti di ricerca dei nomi
* attivi. La lista e` gestita come uno stack. L'ultimo contesto inserito
* e` il primo ad essere controllato durante la ricerca dei nomi.
*/

void st_reset_context_stack(void);

void st_push_context(context_t * context);

void st_pop_context(void);

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

void st_explore_context(context_t *context, int (*fn)(context_t *, symbol_t *, void *), void *user);

/*
* Funzione "st_register_name"
* ---------------------------
*
*  Questa funzione trova un nome nel contesto indicato, e memorizza
* in un'area statica informazioni utili alla sua manipolazione.
*  Riporta l'indirizzo della struttura statica utilizzata come cache.
*  Se il contesto e` NULL, il nome viene cercato nei contesti correnti.
*/

symbol_position_t *st_register_name(char *name, context_t *context);

/*
* Funzione "st_add_registered_name"
* ---------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete.
*/

symbol_t *st_add_registered_name(void);

/*
* Funzione "st_add_registered_name_as_var"
* ----------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, ed alloca, se necessario, un descrittore di variabile.
*/

symbol_t *st_add_registered_name_as_var(void);

/*
* Funzione "st_add_registered_name_as_type"
* ----------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, ed alloca, se necessario, un descrittore di tipo.
*/

symbol_t *st_add_registered_name_as_type(void);

/*
* Funzione "st_add_registered_name_as_string"
* -------------------------------------------
*
*  Questa funzione aggiunge il nome registrato nella cache al contesto
* che gli compete, e lo marca di classe "stringa".
*/

symbol_t *st_add_registered_name_as_string(void);

paramlist_t *st_paramlist_alloc(void);

#define _SYMTAB_H_

#endif

