/*
* @(#) compiler.h 1.5 Mon Nov 20 19:52:52 MET 2000
*
* Livello medio del processo di generazione del codice.
*
* 16/07/96 GG 1.0 Prima stesura.
* 14/10/96 GG 1.1 Iniziati i lavori per la gestione delle stringhe.
* 17/10/96 GG 1.2 Iniziati i lavori per l'ottimizzazione di codice.
* 18/11/97 GG 1.3 Aggiunto il flag di indirizzo in NVRAM, utilizzato
*             per evitare che il compilatore produca scritture a 32 bit
*             sul bus ISA.
* 26/11/98 GG 1.4 Aggiunta la definizione di un flag di "puntatore a blocco
*             funzionale".
* 20/11/00 GG 1.5 Aggiunte le funzioni "co_end_instruction" e
*             "co_begin_instruction", per eseguire aggiustamenti a inizio
*             e fine istruzione (per il debugger CNi).
*/

#ifndef _COMPILER_H_


/*
* Tipo descrivente un operando (o una parte di esso).
*/

enum _co_argtype_t {
	CO_ARG_SYM,	/* Riferimento ad una variabile. */
	CO_ARG_NUM,	/* Costante numerica. */
	CO_ARG_STRING,	/* Costante stringa. */
	CO_ARG_EXPR,	/* Espressione da valutare. */
	CO_ARG_CVAL,	/* Risultato di un'espressione compilata. */
	CO_ARG_ACC,	/* Valore gia` presente nell'accumulatore. */
};
enum _co_exprop_t {
	CO_EOP_INDEX,	/* Operatore di indicizzazione "[]" */
	CO_EOP_EXTRACT,	/* Operatore di estrazione "." */
	CO_EOP_EVAL,	/* Operatore di ri-valutazione. */
	CO_EOP_ADD,	/* Addizione. */
	CO_EOP_MUL	/* Moltiplicazione. */
};

enum _cop_t {
	CO_O_AND,
	CO_O_OR,
	CO_O_XOR,
	CO_O_ADD,
	CO_O_SUB,
	CO_O_MUL,
	CO_O_DIV,
	CO_O_MOD,
/* Non alterare quest'ordine ! */
	CO_O_GT,
	CO_O_GE,
	CO_O_LT,
	CO_O_LE,
	CO_O_EQ,
	CO_O_NE
};

/*
* Struttura descrivente lo stato di un registro.
*/

/* Significato dei flag di stato del registro . */

#define RSTS_LOCKED	0x01	/* Registro riservato, non utilizzabile. */
#define RSTS_USED	0x02	/* Registro in uso. */
#define RSTS_CACHE	0x04	/* Registro non in uso, ma contenente dati
				riutilizzabili utili. */

typedef struct {
/* Flag di stato. */
	int flags;
/* Numero di salvataggi nello stack. */
	int count;
/* Ultimo livello di stack utilizzato per il salvataggio del registro. */
	int level;
/* Sono previsti altri campi (cache...). */
} reg_status_t;

/*
* Struttura descrivente il risultato di una valutazione-compilazione.
*/

#define CO_CEX_REG1	0x01 /* Registro 1 significativo. */
#define CO_CEX_REG2	0x02 /* Registro 2 significativo. */
#define CO_CEX_SYMOFF	0x04 /* Offset simbolico significativo. */
#define CO_CEX_NUMOFF	0x08 /* Offset numerico significativo. */
#define CO_CEX_SCALE  	0x10 /* Fattore di scala significativo. */
#define CO_CEX_NVRAM  	0x20 /* Variabile RETAIN. */

typedef struct {
	long numoff;	/* Offset numerico. */
	char *symoff;	/* Offset simbolico. */
	char flags;	/* Flag di validita` dei campi. */
	char r1;	/* Primo registro. */
	char r2;	/* Secondo registro. */
	char scale;	/* Fattore di scala per il secondo registro. */
} cval_t;

/* Significato del campo "format". */
#define CO_CEX_F_DIM_M     0x03 /* Dimensione del dato. */
#define CO_CEX_F_DIM_1     0x00
#define CO_CEX_F_DIM_2     0x01
#define CO_CEX_F_DIM_4     0x02
#define CO_CEX_F_DIM_8     0x03
#define CO_CEX_F_FMT_M     0x0C	/* Formato del dato. */
#define CO_CEX_F_FMT_U     0x00	/* Unsigned. */
#define CO_CEX_F_FMT_S     0x04	/* Signed. */
#define CO_CEX_F_FMT_B     0x08	/* Boolean (che probabilmente non serve). */
#define CO_CEX_F_FMT_F     0x0C	/* Float. */
#define CO_CEX_F_SPC_F     0x10	/* Flag di puntatore a blocco funzionale. */

typedef struct _co_arg_t {
	enum _co_argtype_t type;
	union {
	/* L'operando e` un simbolo. */
		symbol_t *sym;
	/* L'operando e` una costante stringa. */
		string_t *string;
	/* L'operando e` una costante. */
		VAL num;
	/* L'operando e` un'espressione (codice + tre argomenti al massimo). */
		struct {
			enum _co_exprop_t code;
			int nargs;
			struct _co_arg_t *args[3];
		} expr;
	/* L'operando e` il risultato di un'espressione compilata. */
		cval_t cexpr;
	} val;
	char format;
} co_arg_t;

void co_init(void);

int co_begin(file_action_t *act);

void co_end(void);

void co_set_data(void);

void co_set_text(void);

void co_assign_offset(type_t *type);

void co_def_extern(symbol_t *sym);

void co_def_global_var(symbol_t *sym);

void co_def_local_var(symbol_t *sym);

void co_begin_fb(symbol_t *fb);

void co_end_fb(symbol_t *fb);

void co_def_label(symbol_t *sym);

/*
* Caricamento di un valore come costante, estratto dallo stack del
* valutatore di espressioni costanti.
*/

co_arg_t * co_expr_push_num(void);

/*
* Caricamento di un valore come simbolo.
*/

co_arg_t * co_expr_push_sym(symbol_t *sym);

/*
* Caricamento di un valore come CVAL.
*/

co_arg_t * co_expr_push_cval(cval_t *v, int fmt);

/*
* Inizia la generzione di un'espressione di tipo "index", creando
* l'espressione "(CO_EOP_INDEX op1)".
*/

void co_expr_index_begin(void);

/*
* Registra un nuovo indice nell'espressione di tipo "index", corrente.
* L'espressione passa da "(CO_EOP_INDEX op1)" (per esempio) a
* "(CO_EOP_INDEX op1 op2)"
*/

void co_expr_index_register(void);

/*
* Registra un nuovo campo nell'espressione di tipo "extract", corrente.
* L'espressione passa da "(CO_EOP_EXTRACT op1)" (per esempio) a
* "(CO_EOP_EXTRACT op1 op2)"
*/

void co_expr_extract(void);

/*
* Valuta l'espressione che si trova in cima allo stack
* (in pratica, la compila), reinizializza lo stack e libera tutti
* gli argomenti.
*/

void co_expr_eval(void);

/*
* Salto incondizionato.
*/

void co_jmp(symbol_t *sym);

void co_jmpc(symbol_t *sym);

void co_jmpnc(symbol_t *sym);

/*
* Return incondizionato.
*/

void co_ret(void);

void co_retc(void);

void co_retnc(void);

void co_begin_def_configuration(char *name);

void co_end_def_configuration(void);

void co_begin_def_resource(char *name, char *on);

void co_end_def_resource(void);

void co_def_program_instance(symbol_t *sym, symbol_t *proc);

void co_end_def_program_instance(void);

void co_add_msd_in(symbol_t *sym, symbol_t *prog, char *io);

void co_add_msd_out(symbol_t *sym, symbol_t *prog, char *io);

void co_add_msd_const(symbol_t *sym, symbol_t *prog);

void co_object_header(void);

/*
* Negazione implicita. "bo" indica se la negazione sia logica o a bit.
*/

void co_not(int bo);

void co_new_string(symbol_t *);

void co_label(void);

void co_expr_eval_ld(void);

void co_expr_store(void);

void co_expr_set(void);

void co_expr_reset(void);

void co_expr_bitop(enum _cop_t cop, int neg, int bo);

void co_expr_arop(enum _cop_t cop, int uns);

void co_cal(symbol_t *sym);

void co_do_fncall(void);

void co_calc(symbol_t *sym);

void co_calcn(symbol_t *sym);

void co_calfn(symbol_t *sym);

void co_fnarg(void);

void co_il_expr_begin(void);

void co_il_expr_end(void);

void co_il_newline(void);

void co_add_string(symbol_t *sym);

void co_object_end(void);

void _co_def_global_var_core(symbol_t *sym);

void _co_expr_sr(int sr);

void co_expr_nstore(int bo);

co_arg_t * co_expr_push_string(symbol_t *sym);

void co_expr_push_expr(enum _co_exprop_t code);

void co_expr_addarg(void);

void co_begin_instruction(void);

void co_end_instruction(void);

#define _COMPILER_H_

#endif

