/*
* @(#) compiler.c 1.32 Wed Jan 24 17:38:24 MET 2001
*
* Livello medio del processo di generazione del codice.
* Le funzioni qui contenute realizzano la compilazione vera e propria.
* Si avvalgono dei servizi del modulo "emitter.c" per produrre linee
* assembler nella forma corretta. Non e` di competenza di questo
* modulo la formattazione del codice prodotto, ne` il rispetto delle
* regole sintattiche dell'assembler.
*  Le funzioni di questo modulo sono chiamate dal supervisore
* del passo 2 ("pass2.c") su sollecitazione del parser. Qui si
* realizza anche parte del controlli semantici.
*
* 16/07/96 GG 1.0 Prima stesura.
* 23/09/96 GG 1.1 Aggiunta la compilazione delle chiamate di funzioni
*             (lavori in corso).
* 25/09/96 GG 1.2 Corretto un problema nella compilazione di espressioni
*             con parentesi. Lo stack restava sbilanciato alla prima
*             istruzione dopo la parentesi chiusa.
* 27/09/96 GG 1.3 Aggiunto l'azzeramento di %edx nella divisione, per
*             evitare overflow.
* 07/10/96 GG 1.4 Aggiunta la compilazione di "S" ed "R". Corretto un
*             bug nel caso di bound_check == 0.
* 15/10/96 GG 1.5 Aggiunta la gestione delle stringhe.
* 16/10/96 GG 1.6 Aggiunto il calcolo delle locazioni dei parametri di
*             una funzione al momento della sua chiamata. Questo facilitera`
*             i riferimenti in avanti (uso prima della definizione).
* 17/10/96 GG 1.7 Aggiunto lo svuotamento dello stack di valutazione
*             dopo la chiamata di un blocco funzionale.
* 17/10/96 GG 1.8 Raccolta la generazione del codice per i segnali di scambio,
*             in vista dell'aggiunta di un meccanismo di ottimizzazione degli
*             accessi a "pchMem" e "pKeyTab".
* 18/10/96 GG 1.9 Migliorato l'uso dei registri nel caso di accesso a matrice
*             con due indici variabili.
* 18/10/96 GG 1.10 Corretta l'indebita occupazione dei registri cache (a
*             distribuzione gia` avvenuta...)
* 26/11/96 GG 1.11 Iniziata la gestione delle variabili "RETAIN".
* 28/11/96 GG 1.12 Iniziata la produzione delle informazioni di debug.
* 29/11/96 GG 1.13 Modifiche per la lettura di costanti stringa.
* 02/12/96 GG 1.14 Aggiunta la costruzione degli inizializzatori per le
*             variabili "RETAIN".
* 03/12/96 GG 1.15 Lavori in corso per le informazioni di debug.
* 05/12/96 GG 1.16 Utilizzate le solite etichette ".Ln" come riferimenti
*             per certe informazioni di debug. "as" ed "ld", a quanto pare
*             non ignorano affatto le etichette temporanee "0:" !
* 11/12/96 GG 1.17 La generazione delle informazioni di debug riguardanti i
*             file e` stata corretta : non funzionava bene con i path assoluti.
* 13/01/97 GG 1.18 Evitata l'emissione di "$VA.." con MUL/DIV/MOD di una
*             costante.
* 13/01/97 GG 1.19 Modificata la sintassi dei simboli "$BF..." e "$EF...",
*             in modo che possano rappresentare nome di file qualsiasi. Il nome
*             del file e` ora codificato come sequenza di cifre esadecimali
*             rappresentante successivi caratteri.
* 20/01/97 GG 1.20 Aggiunta la gestione dei flag di compilazione
*             "begin_lineinfo" ed "end_lineinfo" (vedi "main.c").
* 06/05/97 GG 1.21 Corretto un problema nella gestione della cache delle
*             variabili "pchMem" e "pKeyTab" nel caso di istruzioni "S" ed "R".
* 04/06/97 GG 1.22 E` stata aggiunta la possibilita` di generare segnali
*             di scambio (non RETAIN) inizializzati.
*             E` stato evitato che il valore iniziale di un segnale di
*             scambio a chiave indefinita comparisse anche nella variabile
*             "ombra". Non aveva mai dato fastidio, ma era una inaccettabile
*             concessione alla casualita`.
* 09/06/97 GG 1.23 E` stata aggiunta la codifica di cinque NOP prima di ogni
*             etichetta e di ogni salto, per evitare lo scherzo dei punti di
*             salto a cavallo di un breakpoint.
* 11/06/97 GG 1.24 Aggiunta la definizione di una nuova etichetta di debug,
*             che codifica il punto "consigliato" per introdurre un break
*             in caso di etichette e salti. Aggiunto un "buco" di NOP
*             anche a fine funzione/blocco funzionale.
* 12/06/97 GG 1.25 Modificata l'emissione delle etichette "$LJ", che ora
*             caratterizzano solo i salti e i RET, e contengono anche
*             l'informazione dell'accumulatore. Per le etichette ed il codice
*             di fine funzione e` ora emesso "$LL".
* 22/09/97 GG 1.26 Un piccolo aggiustamento per compilazione su NT.
* 18/11/97 GG 1.27 Aggiunto il flag di indirizzo in NVRAM, utilizzato
*             per evitare che il compilatore produca scritture a 32 bit
*             sul bus ISA.
* 26/11/98 GG 1.28 Correzioni nelle primitive che fanno capo ad LD, ST
*             e alle funzioni aritmetiche per rilevare l'erroneo utilizzo
*             di blocchi funzionali (o altro ancora) come solo argomento
*             di questi operatori.
* 01/12/98 GG 1.29 Corretta la compilazione della divisione con segno, che
*             era esposta al pericolo di overflow in caso di dividendo
*             negativo.
* 02/11/00 GG 1.30 Aggiunta l'emissione dell'etichetta "$VA..." nel caso
*             di S e R.
* 20/11/00 GG 1.31 Aggiunte le funzioni "co_end_instruction" e
*             "co_begin_instruction", per eseguire aggiustamenti a inizio
*             e fine istruzione ("buchi di codice" per il debugger CNi).
*             Allungate le "zone di sicurezza" (blocchi di NOP) nei
*             punti critici (prime due istruzioni di un ramo, JMP*, RET*,
*             etichette, fine procedura).
* 24/01/01 GG 1.32 Modificata la generazione di codice di SET e RES,
*             in modo che l'indirizzo dell'operando sia calcolato sempre,
*             cioe` anche quando l'operando non deve essere alterato.
*             La vecchia modalita` di funzionamento e` ancora ottenlibile
*             con l'opzione da linea di comando "-optsr".
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "conf.h"
#include "sysdep.h"
#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "util.h"
#include "ilc.h"
#include "pass1.h"
#include "pass2.h"
#include "cexpr.h"
#include "debug.h"
#include "errs.h"
#include "rdfile.h"
#include "compiler.h"
#include "emitter.h"
#include "msd.h"

/*
* VARIABILI LOCALI
*/

/* Registri del manipolatore formale di operandi. */

static co_arg_t co_args[MAX_CO_REG_ARGS * 2];
static co_arg_t *co_stack[MAX_CO_REG_ARGS];
static int co_expr_sp = 0;
static int co_expr_argp = 0;
static int co_fb_count = 0;
static symbol_t *co_curr_fb = (symbol_t *) 0;

static void _co_expr_eval_ld(void);

static void _co_reset_cache(void);

static void _co_expr_eval_core(void);


/* Accumulatore corrente. */

static int co_curr_acc = R_NULL;

/* Sezione correntamente utilizzata.  */

enum { CO_SECT_NONE, CO_SECT_DATA, CO_SECT_TEXT };
static int co_curr_section = CO_SECT_NONE;

/* Indice per la generazione di etichette assembler uniche. */

static int co_local_label_count = 0;
static int co_string_label_count = 0;
static int co_local_db_label_count = 0;

/* Contatore di istruzioni contenute in un ramo di codice.
 Serve a individuare i tratti di codice che il debugger puo` tracciare
 con una sola lista di break. Il compilatore deve garantire al debugger
 lo spazio necessario a contenere le istruzione di break (5 byte), con il
 vincolo aggiuntivo che il primo e l'ultimo elemento di una lista di
 break devono distare almeno 5 byte. A questo scopo, sono prodotte
 delle sequenze di cinque NOP prima di ogni etichetta, dopo ogni salto,
 e all'inizio della seconda istruzione di ogni ramo.  */

static int co_branch_instr_count = 0;

/* FUNZIONI */

/*****************************************************\
* INIZIALIZZAZIONE, APERTURA E CHIUSURA DI UN OGGETTO *
\*****************************************************/

void co_init(void)
{
	em_init();
	co_curr_section = CO_SECT_NONE;
	co_local_label_count = 0;
	co_string_label_count = 0;
	co_local_db_label_count = 0;
	co_expr_sp = 0;
	co_expr_argp = 0;
	co_fb_count = 0;
	co_branch_instr_count = 0;
}

int co_begin(file_action_t *act)
{
	if (! em_start(act)) {
		return 0;
	}

	co_curr_section = CO_SECT_NONE;
	co_local_label_count = 0;
	co_string_label_count = 0;
	co_local_db_label_count = 0;
	co_expr_sp = 0;
	co_expr_argp = 0;
	co_fb_count = 0;
	co_branch_instr_count = 0;

	return 1;
}

void co_end(void)
{
	em_end();
}

/**************************************************\
* FUNZIONI DI INTERFACCIA PER IL MODULO EMETTITORE *
\**************************************************/

/*
* Emissione di un'istruzione nella forma "opcode nome"
*/

static void _co_op_1name(int code, char *name)
{
em_opr_t op;

	op.type = EM_OP_DIR;
	op.val.vtype = EM_VTYPE_STR;
	op.val.v.s = name;
	em_op(code,&op);
}

/*
* Emissione di un'istruzione nella forma "opcode nome_in_esadecimale"
*/

static void _co_op_1hexname(int code, char *name)
{
em_opr_t op;

	op.type = EM_OP_DIR;
	op.val.vtype = EM_VTYPE_HEXSTR;
	op.val.v.s = name;
	em_op(code,&op);
}

#ifdef FFSS
static void _co_op_1const(int code, long val)
{
em_opr_t op;

	op.type = EM_OP_IMM;
	op.val.vtype = EM_VTYPE_NUM;
	op.val.v.n = val;
	em_op(code,&op);
}
#endif /* FFSS */

/*
* Applicazione dell'operatore "op" ad un'etichetta temporanea di indice "n".
*/

static void _co_op_1tlbl(int code, long n)
{
em_opr_t op;

	op.type = EM_OP_DIR;
	op.val.vtype = EM_VTYPE_TLBL;
	op.val.v.n = n;
	em_op(code,&op);
}

static void _co_op_1strbl(int code, long n)
{
em_opr_t op;

	op.type = EM_OP_DIR;
	op.val.vtype = EM_VTYPE_STRLBL;
	op.val.v.n = n;
	em_op(code,&op);
}

/*
* Emissione di un'istruzione nella forma "opcode registro"
*/

static void _co_op_1reg(int opcode, int reg)
{
em_opr_t op;

	op.type = EM_OP_REG;
	op.r1 = reg;
	em_op(opcode,&op);
}

/*
* Emissione di un'istruzione nella forma "opcode registro,registro"
*/

static void _co_op_2reg(int opcode, int r1, int r2)
{
em_opr_t op1,op2;

	op1.type = EM_OP_REG;
	op1.r1 = r1;
	op2.type = EM_OP_REG;
	op2.r1 = r2;
	em_op(opcode,&op1,&op2);
}

/*
* Emissione di un'istruzione nella forma "opcode $dato,registro"
*/

static void _co_op_imm_reg(int opcode, long v, int r)
{
em_opr_t op1,op2;

	op1.type = EM_OP_IMM;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = v;
	op2.type = EM_OP_REG;
	op2.r1 = r;
	em_op(opcode,&op1,&op2);
}

/*
* Somma di una costante ad un registro. Qualche ottimizzazione.
*/

static void _co_op_add_imm_reg(long v, int r)
{
	switch ((int)v) {
	case 0:
		break;
	case 1:
		_co_op_1reg(AOP_INCL,r);
		break;
	case -1:
		_co_op_1reg(AOP_DECL,r);
		break;
	default:
		_co_op_imm_reg(AOP_ADDL, v, r);
		break;
	}
}

/*
* Sottrazione di una costante ad un registro. Qualche ottimizzazione.
*/

static void _co_op_sub_imm_reg(long v, int r)
{
	switch ((int)v) {
	case 0:
		break;
	case -1:
		_co_op_1reg(AOP_INCL,r);
		break;
	case 1:
		_co_op_1reg(AOP_DECL,r);
		break;
	default:
		_co_op_imm_reg(AOP_SUBL, v, r);
		break;
	}
}


/*
* Emissione della direttiva "space dimensione, carattere di fill".
*/

static void _co_space(int size, int fill)
{
em_opr_t op1,op2;

	op1.type = EM_OP_DIR;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = size;
	op2.type = EM_OP_DIR;
	op2.val.vtype = EM_VTYPE_NUM;
	op2.val.v.n = fill;
	em_op(AOP_SPACE,&op1,&op2);
}

/*
* Emissione di un valore come parte di una lista di dati di inizializzazione.
* Il valore e` rappresentato da un puntatore ad un valore di tipo "co_data_t".
* Deve essere specificata la dimensione in byte del valore, ed il modo
* di interpretarlo (float o int).
*/

typedef union {
	double d;
	long l;
} co_data_t;

static void _co_emit_dataval(co_data_t *pd, int fl, int sz)
{
float f;

	if (fl) {
		if (sz == sizeof(float)) {
			f = (float)(pd -> d);
			em_add_datalist((char *)&f);
		}
		else {
			em_add_datalist((char *)&(pd -> d));
		}
	}
	else {
		em_add_datalist((char *)&(pd -> l));
	}
}

/*
* Riserva-emette un'etichetta temporanea, richiamabile per numero.
*/

static int _co_get_tmplabel(void)
{
	return ++co_local_label_count;
}

static int _co_get_db_tmplabel(void)
{
	return ++co_local_db_label_count;
}

static int _co_get_stringlabel(void)
{
	return ++co_string_label_count;
}

static int _co_def_tmplabel(int n)
{
	_co_op_1tlbl(AOP_LABEL,n);
	return n;
}

static int _co_def_stringlabel(int n)
{
	_co_op_1strbl(AOP_LABEL,n);
	return n;
}

/*
* Definizione e creazione di un'etichetta temporanea.
*/

static int _co_tmplabel(void)
{
	return _co_def_tmplabel(_co_get_tmplabel());
}

/*
* Moltiplica un registro per 7 (indispensabile per i segnali di scambio).
*/
static void _co_mult7(int reg)
{
em_opr_t op1,op2;

	op1.type = EM_OP_IMM;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = 7;
	op2.type = EM_OP_REG;
	op2.r1 = reg;
	em_op(AOP_IMULL,&op1,&op2);
}

/*****************************************\
* GENERAZIONE DI DEFINIZIONI DI VARIABILI *
\*****************************************/

/*
* Selezione della sezione di codice (text o data).
*/

void co_set_data(void)
{
	if (co_curr_section != CO_SECT_DATA) {
		em_op(AOP_DATA);
		co_curr_section = CO_SECT_DATA;
	}
}

void co_set_text(void)
{
	if (co_curr_section != CO_SECT_TEXT) {
		em_op(AOP_TEXT);
		co_curr_section = CO_SECT_TEXT;
	}
}

/*
* Definizione di un simbolo come "extern"
*/

void co_def_extern(symbol_t *sym)
{
	_co_op_1name(AOP_EXTERN,sym -> name);
}

/*
* Procedura di calcolo della posizione di una variabile
* locale ad un blocco funzionale. La procedura
* e` composta da due funzioni, una principale, ed una "di servizio".
* Ques'ultima puo` chiamare a sua volta la principale, in caso di
* variabili di tipo "blocco funzionale".
*/

/* Funzione di servizio. */

static int _co_assign_offset(context_t *context, symbol_t *sym, void *user)
{
	if (sym -> class != SYMCL_VAR)
		return 1;

	switch (sym -> type -> code) {
	case TYPE_PROGRAM:
	case TYPE_FUNCTION:
	case TYPE_FUNCTION_BLOCK:
		co_assign_offset(sym -> type);
		break;
	case TYPE_NONE:
	case TYPE_UNDEF:
		err_error(LA_E_NOTYPE,sym -> type -> d.undef_name);
		return 0;
	default:
		break;
	}

/* Il parametro "user" punta al campo "size" del tipo. */

	sym -> var -> location = *(int *)user;
	*((int *)user) += sym -> type -> size;

	return 1;
}

/* Funzione principale. */

void co_assign_offset(type_t *type)
{
	if (! type -> size) {
		st_explore_context(type -> d.fb.context,
		                   _co_assign_offset,
		                   (void *) &(type -> size));
	}
}

/*
* Generazione di codice di definizione ed inizializzazione di
* variabili.
* La procedura e` composta dalle seguenti funzioni :
*
*	_co_make_lvar_space : funzione di servizio per l'esplorazione
*	                      del contesto associato a tipi funzione
*	_co_make_typed_data : nucleo della procedura, che produce
*	                      i dati di inizializzazione. E` ricorsiva,
*                             attraverso la _co_make_lvar_space, per la
*	                      possibilita` di avere variabili locali di tipo
*	                      funzione.
*	_co_def_named_data  : funzione principale. Produce l'etichetta
*	                      e lo spazio inizializzato del simbolo dato.
*/


static void _co_make_typed_data(symbol_t *sym);

static int _co_make_lvar_space(context_t * context, symbol_t *sym, void *user)
{
	if (sym -> class == SYMCL_VAR)
		_co_make_typed_data(sym);
	return 1;
}

static void _co_make_typed_data(symbol_t *sym)
{
inival_t *p;
int fl;
type_t *type;
int i,n;
em_val_t v;

	type = sym -> type;

	switch (type -> code) {
	case TYPE_ARRAY:

	/* Per i vettori, si esplora la lista di inizializzazione,
	 descritta da un puntatore ad "inival_t". */

		n = st_get_array_size(type);
		type = type -> d.a.type;
		em_open_datalist(type -> size);
		if (type -> code == TYPE_STRING) {
			for (p = sym -> var -> init.a; p; p = p -> next) {
				for (i = 0; i < p -> count; ++i) {
					v.vtype = EM_VTYPE_STRLBL;
					if ((v.v.n = ((co_data_t*)&(p->data))->l)==0)
						v.vtype = EM_VTYPE_NUM;
					em_addv_datalist(&v,0);
				}
				n -= p -> count;
			}
		}
		else {
			fl = st_is_float_type(type);
			for (p = sym -> var -> init.a; p; p = p -> next) {
				for (i = 0; i < p -> count; ++i) {
					_co_emit_dataval((co_data_t *) &(p -> data),
							 fl, type -> size);
				}
				n -= p -> count;
			}
		}
		em_close_datalist();

	/* Riserva l'eventuale spazio non ricoperto dall'inizializzatore. */

		if (n > 0)
			_co_space(n * type -> size, 0);
		break;

	case TYPE_PROGRAM:
	case TYPE_FUNCTION:
	case TYPE_FUNCTION_BLOCK:

	/* Funzione, programma, blocco funzionale. E` un'ottima occasione
	 per calcolare gli offset delle variabili locali. */

		co_assign_offset(sym -> type);

	/* Elemento per elemento, si definiscono tutti gli spazi locali. */

		st_explore_context(sym -> type -> d.fb.context,
		                   _co_make_lvar_space, NULL);
		break;

	case TYPE_NONE:
	case TYPE_UNDEF:

	/* Questo e` evidentemente il risultato di una mancata definizione
	 di blocco funzionale, od un'erronea indicazione di tipo. */

		err_error(LA_E_NOTYPE,type -> d.undef_name);
		break;

	default:

	/* Tipi "normali". Si procede alla definizione degli spazi,
	 inizializzati o no che siano. */

		if (sym -> var -> flags & VAR_F_INIT) {
			em_open_datalist(type -> size);
			if (type -> code == TYPE_STRING) {
				v.vtype = EM_VTYPE_STRLBL;
				if ( (v.v.n = ((co_data_t *)
				          &(sym -> var -> init)) -> l) == 0) 
					v.vtype = EM_VTYPE_NUM;
				em_addv_datalist(&v,0);
			}
			else {
				_co_emit_dataval((co_data_t *) &(sym -> var -> init),
						 st_is_float_type(type),
						 type -> size);
			}
			em_close_datalist();
		}
		else {
			_co_space(type -> size, 0);
		}

		break;
	}
}

static void _co_def_named_data(symbol_t *sym)
{

/* Emette l'etichetta associata al simbolo. */

	_co_op_1name(AOP_LABEL,sym -> name);

/* Crea lo spazio necessario. */

	_co_make_typed_data(sym);
}

/*
* Definizione di una variabile globale.
*/

void _co_def_global_var_core(symbol_t *sym)
{
	co_set_data();
	_co_op_1name(AOP_GLOBAL,sym -> name);
	_co_def_named_data(sym);
}

void co_def_global_var(symbol_t *sym)
{
type_t *save_type;
int save_flags;

/* Le variabili condivise (simboli con associazione "%M")
 necessitano di un trattamento molto particolare :
  Se l'associazione e` a chiave variabile ("%M*"), il compilatore
 definisce il simbolo e gli riserva uno spazio fisso di 4 byte;
 il caricatore dinamico del PLC assegnera` a quest'area il valore
 della chiave di accesso alla variabile.
  Se l'associazione e` a chiave fissa, non e`
 necessaria alcuna generazione di codice, perche` il compilatore
 utilizza direttamente la chiave, nota a priori. */

	if ((sym -> var -> flags & VAR_F_CONNECTION_M)
	 == VAR_F_CONNECTION_MEMORY) {
		if (sym -> var -> at.direct_io[3] == '*') {
		/* "Bara" sul tipo per creare una globale di tipo "long"
		 con lo stesso nome. */
			save_type = sym -> type;
			sym -> type = st_create_type(TYPE_DINT);
		/* Bara anche sul flag di variabile inizializzata, per
		 assicurare che sia generato un valore iniziale NULL. */
			save_flags = sym -> var -> flags;
			sym -> var -> flags &= ~VAR_F_INIT;
		/* Definisce il simbolo. */
			_co_def_global_var_core(sym);
		/* Rimette a posto le cose. */
			sym -> type = save_type;
			sym -> var -> flags = save_flags;
		}
	}
	else {
		_co_def_global_var_core(sym);
	}
}

/*
* Definizione di una variabile locale.
*/

void co_def_local_var(symbol_t *sym)
{
	co_set_data();
	_co_def_named_data(sym);
}

/********************************************************************\
* CONFIGURAZIONI, RISORSE, ISTANZE DI PROGRAMMI, ASSOCIAZIONI DI I/O *
\********************************************************************/

/*
* Comincia qui una delle parti piu` rognose del compilatore : la
* gestione dei costrutti "CONFIGURATION" e derivati, e quella delle
* associazioni di I/O e variabili condivise.
*
* Il compilatore costruisce all'inizio di ogni oggetto generato un
* descrittore di oggetto IL, che contiene un puntatore al descrittore
* successivo, un puntatore ad una lista di associazione di I/Q/M per le
* variabili globali definite nell'oggetto stesso, ed una lista di puntatori
* ai descrittori di configurazione trovati nell'oggetto.
* In fase di link, tutti gli oggetti generati dal compilatore dovranno essere
* raggruppati; in testa al gruppo dovra` comparire l'oggetto di libreria
* "ilc0.o", ed in coda l'oggetto "ilcn.o". Questi due oggetti, i sorgenti
* dei quali si trovano in "../runtm/", contengono il codice di inizio
* e fine della catena dei descrittori di oggetto IL, e la definizione
* del descrittore radice, associata ad un simbolo che sara` riconosciuto dal
* caricatore dinamico dell'esecutore PLC. Senza di essi, il caricamento del
* programma non puo` avvenire. Oggetti IL non racchiusi tra "ilc0.o" ed
* "ilcn.o" non potranno definire associazioni di I/Q/M, segnali di scambio,
* o variabili retentive, ed avranno ottime probabilita` di schiantarsi.
*
* La struttura esatta dei descrittori di configurazione, risorsa,
* istanza del programma ed associazione I/Q/M si trova nel file
* "../runtm/ilcrtm.h". Si tratta di una formalizzazione delle strutture
* coinvolte, senza pretesa di utilizzo diretto
* per scrivere codice di accesso ai dati descritti.
*/

/*
* Definizione di una configurazione.
*/

static char * co_config_name = (char *) 0;
static int co_config_name_label = 0;
static int co_config_resources_list[MAX_RESOURCE_COUNT] = { 0 };
static int co_resource_count = 0;

void co_begin_def_configuration(char *name)
{
	co_set_text();
/* Produce la stringa contenente il nome, e ne registra l'etichetta
 di riferimento. */
	co_config_name_label = _co_tmplabel();
	co_config_name = name;
	_co_op_1name(AOP_ASCIZ,name);
/* Azzera il contatore di risorse. */
	co_resource_count = 0;
}

void co_end_def_configuration(void)
{
em_val_t v;
int i;
int reslist;

	co_set_text();

/* Produce la lista dei puntatori a risorsa. */

/* Etichetta di inizio lista. */
	reslist = _co_tmplabel();

/* Lista dei puntatori. */
	em_open_datalist(sizeof(void *));
	v.vtype = EM_VTYPE_TLBL;
	for (i = 0; i < co_resource_count; ++i) {
		v.v.n = co_config_resources_list[i];
		em_addv_datalist(&v,0);
	}
/* NULL finale. */
	v.vtype = EM_VTYPE_NUM;
	v.v.n = 0;
	em_addv_datalist(&v,1);
	em_close_datalist();

/* Produce il descrittore di configurazione. */

/* Nome della configurazione (etichetta locale). */
	_co_op_1name(AOP_LABEL,co_config_name);

	em_open_datalist(sizeof(void *));
/* Puntatore al nome della configurazione. */
	v.vtype = EM_VTYPE_TLBL;
	v.v.n = co_config_name_label;
	em_addv_datalist(&v,0);
/* Puntatore alla lista delle risorse. */
	v.v.n = reslist;
	em_addv_datalist(&v,0);
	em_close_datalist();
}

/*
* Definizione di una risorsa.
*/

static int co_resource_name_label = 0;
static int co_resource_on_label = 0;
static int co_resource_program_list[MAX_PROGRAM_COUNT] = { 0 };
static int co_program_count = 0;

void co_begin_def_resource(char *name, char *on)
{
	co_set_text();
/* Produce la stringa contenente il nome, e ne registra l'etichetta
 di riferimento. */
	co_resource_name_label = _co_tmplabel();
	_co_op_1name(AOP_ASCIZ,name);
/* Produce la stringa contenente il nome della CPU, e ne registra l'etichetta
 di riferimento. */
	co_resource_on_label = _co_tmplabel();
	_co_op_1name(AOP_ASCIZ,on);
/* Azzera il contatore di programmi facenti parte della risorsa. */
	co_program_count = 0;
}

void co_end_def_resource(void)
{
em_val_t v;
int i;
int proglist;

	co_set_text();

/* Produce la lista dei descrittori di istanza di programma. */

/* Etichetta di inizio lista. */
	proglist = _co_tmplabel();

/* Lista dei puntatori. */
	em_open_datalist(sizeof(void *));
	v.vtype = EM_VTYPE_TLBL;
	for (i = 0; i < co_program_count; ++i) {
		v.v.n = co_resource_program_list[i];
		em_addv_datalist(&v,0);
	}
/* NULL finale. */
	v.vtype = EM_VTYPE_NUM;
	v.v.n = 0;
	em_addv_datalist(&v,1);
	em_close_datalist();

/* Definisce l'etichetta per il descrittore di risorsa, e la aggiunge
 alla lista delle risorse nella configurazione. */

	if (co_resource_count < MAX_RESOURCE_COUNT) {
		co_config_resources_list[co_resource_count] = _co_tmplabel();
		++co_resource_count;
	}
	else {
		err_error(LA_E_TOORES,NULL);
	}

	em_open_datalist(sizeof(long));
/* Puntatre al nome della risorsa. */
	v.vtype = EM_VTYPE_TLBL;
	v.v.n = co_resource_name_label;
/* Puntatre al nome della CPU. */
	em_addv_datalist(&v,0);
	v.v.n = co_resource_on_label;
	em_addv_datalist(&v,0);
/* Puntatre alla lista dei programmi. */
	v.v.n = proglist;
	em_addv_datalist(&v,0);
	em_close_datalist();
}

/*
* Definizione di un'istanza di programma.
*/

static symbol_t * co_program_sym = (symbol_t *) 0;
static symbol_t * co_program_procedure_sym = (symbol_t *) 0;

void co_def_program_instance(symbol_t *sym, symbol_t *proc)
{
em_val_t v;

/* Memorizza il simbolo che descrive l'istanza del programma. */

	co_program_sym = sym;

/* Memorizza il simbolo che descrive il prototipo del programma. */

	co_program_procedure_sym = proc;

	co_set_text();

/* Produce l'etichetta per il descrittore di istanza, e la aggiunge
 alla lista di programmi della risorsa corrente. */

	if (co_resource_count < MAX_PROGRAM_COUNT) {
		co_resource_program_list[co_program_count] = _co_tmplabel();
		++co_program_count;
	}
	else {
		err_error(LA_E_TOORES,NULL);
	}

	em_open_datalist(sizeof(long));
/* Puntatore all'area delle variabili dell'istanza del programma. */
	v.vtype = EM_VTYPE_STR;
	v.v.s = co_program_sym -> name;
	em_addv_datalist(&v,0);
/* Puntatore al corpo del programma. */
	v.v.s = co_program_procedure_sym -> name;
	em_addv_datalist(&v,0);
	em_close_datalist();
/* Seguiranno i descrittori di associazione I/Q/M del programma. */
}

void co_end_def_program_instance(void)
{
em_val_t v;

	co_set_text();

/* Terminatore di lista MSD : NULL + ILC_END. */
	em_open_datalist(sizeof(long));
	v.vtype = EM_VTYPE_NUM;
	v.v.n = 0;
	em_addv_datalist(&v,1);
	v.v.n = ILC_END;
	em_addv_datalist(&v,1);
	em_close_datalist();
}

/*
* Produzione di un descrittore di associazione I/Q/M.
*
* Il nucleo della procedura e` costituito dalla funzione _co_make_msd_entry,
* che e` chiamata dalle interfacce co_add_msd... e co_emit_global_msd
*/

enum { IOMODE_IN, IOMODE_OUT, IOMODE_CONST, IOMODE_MEM };

static void _co_make_msd_entry(symbol_t *sym, symbol_t *prog, char *io, int mode)
{
em_val_t v;
int eflags,modef,memf,sz;
VAL cop;
type_t *t;
#ifdef FFSS
int dim1,dim2;
#else
int dim1 = 1;
int dim2 = 1;
#endif

	co_set_text();

	if (! sym)
		return;

/* Flag di variabile M con chiave fissata. */

	eflags = memf = 0;

/* Memorizza il tipo base della variabile. */

	t = sym -> type;
/* Le associazioni M ammettono target vettoriali. */
	if (st_is_array_type(t))
		t = t -> d.a.type;

/* Costruzione della prima parte del descrittore :
 - per associazioni %I/%Q, indirizzo della variabile "target", che
   puo` essere una globale o un parametro di programma (prog != NULL),
 - per associazioni %M*, l'indirizzo della variabile che conterra`
   la chiave assegnata dinamicamente,
 - per associazioni %Mn, la chiave statica. */

	if (prog) {

	/* Associazione I/Q di parametro di programma. Produce
	 il puntatore alla variabile nella forma BASE+offset. */

		em_open_datalist(sizeof(long));
		v.vtype = EM_VTYPE_STROFF;
		v.v.s = prog -> name;
		v.v.n = sym -> var -> location;
		em_addv_datalist(&v,0);
		em_close_datalist();
	}
	else if (mode == IOMODE_MEM) {

	/* Associazione M */

		if (io[3] != '*') {
		/* Per i segnali di scambio con chiave definita, il
		 campo "target" contiene la chiave. */
			em_open_datalist(sizeof(long));
			v.vtype = EM_VTYPE_NUM;
			v.v.n = atoi(io + 3);
			em_addv_datalist(&v,1);
			memf = ILC_MSD_MEMVAR_FIXED;
			em_close_datalist();
		}
		else {
		/* Associazione %M*. Bisogna produrre una variabile
		 con lo stesso nome del target e dimensione 4 (sara` fatto
		 quando si elaborera` "VAR_GLOBAL", non ora). Essa
		 contetta` la chiave dinamica calcolata dal caricatore
		 del kernel PLC. Il campo "target" conterra` il puntatore
		 a questa variabile. */
			em_open_datalist(sizeof(long));
			v.vtype = EM_VTYPE_STR;
			v.v.s = sym -> name;
			em_addv_datalist(&v,0);
			em_close_datalist();
		}
	/* Aggiunge alle proprieta` della variabile l'informazione
	 riguardante il formato del dato. */
		if (st_is_float_type(t))
			memf |= ILC_MSD_MEMVAR_FLOAT;
	/* I segnali di scambio inizializzati e quelli RETAIN hanno
	 bisogno dell'estensione di proprieta`. */
		if (sym -> var -> flags & (VAR_F_RETAIN | VAR_F_INIT)) {
			memf |= ILC_MSD_EXTENSION;
		/* Le variabili RETAIN sono inizializzate in ogni caso. */
			eflags |= ILC_MSD_EXT_INIT;
		/* Aggiunge la proprieta` "RETAIN", se richiesto. */
			if (sym -> var -> flags & VAR_F_RETAIN)
				eflags |= ILC_MSD_EXT_RETAIN;
		}
	}
	else {

	/* Associazione I/Q di variabile globale. Produce
	 il puntatore alla variabile. */

		em_open_datalist(sizeof(long));
	/* Puntatore alla variabile da associare. */
		v.vtype = EM_VTYPE_STR;
		v.v.s = sym -> name;
		em_addv_datalist(&v,0);
		em_close_datalist();
	}

/* Produce il byte di tipo di associazione, contenente le informazioni
 sulla dimensione, il formato, e la modalita` di associazione. */

	/* Modalita` di associazione. */
	switch (mode) {
	case IOMODE_IN:	modef = ILC_MSD_BIND_IN; break;
	case IOMODE_OUT:	modef = ILC_MSD_BIND_OUT; break;
	case IOMODE_MEM:	modef = ILC_MSD_BIND_MEM; break;
	default:	modef = ILC_MSD_BIND_NONE; break;
	}

	/* Dimensione della variabile. */
	switch (t -> size) {
	case 1: if (t -> code == TYPE_BOOL)
			sz = ILC_MSD_SIZE_1;
		else
			sz = ILC_MSD_SIZE_8;
		break;
	case 2: sz = ILC_MSD_SIZE_16; break;
	case 4: sz = ILC_MSD_SIZE_32; break;
	case 8: sz = ILC_MSD_SIZE_64; break;
	default: sz = 0; break;
	}
	modef |= sz | memf;

/* Emette il byte di tipo/proprieta`. */

	em_open_datalist(sizeof(unsigned char));
	v.vtype = EM_VTYPE_NUM;
	v.v.n = modef;
	em_addv_datalist(&v,1);

/* Produce il campo dei dati e informazioni estese, di lunghezza variabile,
 preceduto da un byte di lunghezza. */

	if (mode == IOMODE_CONST) {

	/* Pseudo-associazione di parametro di programma. La variabile
	 non e` associata ad un input diretto, ma le e` assegnato un valore
	 costante. Buono in fase di debug. */

	/* La dimensione e` quella della variabile. */
		v.v.n = t -> size;
		em_addv_datalist(&v,1);
		em_close_datalist();

	/* Emette il valore che si trova sullo stack del valutatore
	 di espressioni costanti. */

		em_open_datalist(t -> size);
		ce_pop(&cop);
		_co_emit_dataval((co_data_t *) &cop.data,
				 st_is_float_type(t),
				 t -> size);
		em_close_datalist();
	}
	else if (mode == IOMODE_MEM) {

	/* Associazione M. I dati consistono in due interi contenenti
	 le dimensioni dell'oggetto (eventualmente 1,1), seguiti dal
	 nome della variabile. */

		/* Lunghezza. */
		v.v.n = strlen(sym -> name) + 1 + 2*sizeof(int);
		em_addv_datalist(&v,1);
		em_close_datalist();
		em_open_datalist(sizeof(long));
		/* Dimensioni. */
		dim1 = 1;
		dim2 = 1;
		if (st_is_array_type(sym -> type)) {
			dim1 = sym -> type -> d.a.dim[0];
			if (sym -> type -> d.a.n_dim == 2) {
				dim2 = sym -> type -> d.a.dim[1];
			}
		}
		v.v.n = dim1;
		em_addv_datalist(&v,1);
		v.v.n = dim2;
		em_addv_datalist(&v,1);
		em_close_datalist();
		/* Nome. */
		_co_op_1name(AOP_ASCIZ,sym -> name);
	}
	else {

	/* Associazioni I/Q. Il campo dati conterra` la stringa che rappresenta
	 il segnale, privata dei primi tre caratteri (%I? o %Q?). */
		/* Lunghezza. */
		v.v.n = strlen(io) - 3 + 1;
		em_addv_datalist(&v,1);
		em_close_datalist();
		/* Stringa. */
		_co_op_1name(AOP_ASCIZ,io + 3);
	}

/* Emette l'entry aggiuntiva, se e` stato acceso il flag di estensione. */

	if (modef & ILC_MSD_EXTENSION) {

	/* L'unica estensione gestita attualmente e` la proprieta` "RETAIN",
	 eventualmente seguita dal valore iniziale della variabile. */

		em_open_datalist(sizeof(long));
		v.vtype = EM_VTYPE_NUM;
		v.v.n = 0;
		em_addv_datalist(&v,1);
		em_close_datalist();
		em_open_datalist(sizeof(unsigned char));
		em_addv_datalist(&v,1);
	/* L'unico campo significativo e` "data"
	 (e "data_size", ovviamente). */
		v.v.n = 1;
		if (eflags & ILC_MSD_EXT_INIT) {
			v.v.n += dim1 * dim2 * t -> size;
		}
		em_addv_datalist(&v,1);
		v.v.n = eflags;
		em_addv_datalist(&v,1);
		em_close_datalist();
		if (eflags & ILC_MSD_EXT_INIT) {
			_co_make_typed_data(sym);
		}
	}
}
void co_add_msd_in(symbol_t *sym, symbol_t *prog, char *io)
{
	_co_make_msd_entry(sym,prog,io,IOMODE_IN);
}

void co_add_msd_out(symbol_t *sym, symbol_t *prog, char *io)
{
	_co_make_msd_entry(sym,prog,io,IOMODE_OUT);
}

void co_add_msd_const(symbol_t *sym, symbol_t *prog)
{
	_co_make_msd_entry(sym,prog,NULL,IOMODE_CONST);
}

static int co_end_text_label = 0;

static int _co_emit_configs(context_t * context, symbol_t *sym, void *user)
{
em_val_t v;

	if (sym -> class == SYMCL_TYPE
	 && sym -> type -> code == TYPE_CONFIGURATION) {
		v.vtype = EM_VTYPE_STR;
		v.v.s = sym -> name;
		em_addv_datalist(&v,0);
	}
	return 1;
}

/*
* Questo gruppo di funzioni produce il codice relativo alle configurazioni
* globali contenute in un sorgente. Inoltre produce il codice di inizio e
* fine oggetto.
*/

static void _co_lineinfo(void)
{
em_opr_t op1,op2,op3;

/* Informazioni di debug. */

	op1.type = op2.type = EM_OP_DIRD;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = pScCtl -> n_line;
	op2.val.vtype = EM_VTYPE_STR;
	op2.val.v.s = co_curr_fb -> name;
	op3.type = EM_OP_DIRD;
	op3.val.vtype = EM_VTYPE_NUM;
	if (main_status.ggdb) {
		op3.val.v.n = _co_get_db_tmplabel();
		em_op(AOP_LINEINFO,&op1,&op2,&op3);
	}
	if (main_status.gil) {
		op3.val.v.n = co_curr_acc;
		if (main_status.begin_lineinfo)
			em_op(AOP_LINEINFO2,&op1,&op2,&op3);
		if (main_status.end_lineinfo) {
			--op1.val.v.n;
			em_op(AOP_ELINEINFO2,&op1,&op2,&op3);
			++op1.val.v.n;
		}
	}
}

/*
* Emissine di un elemento della lista di associazione globale. Questa
* funzione e` chiamata per ogni simbolo presente nel contesto del file
* corrente.
*/

static int _co_emit_global_msd(context_t * context, symbol_t *sym, void *user)
{
int mode;

	if (sym -> class == SYMCL_VAR
	 && (mode = (sym -> var -> flags & VAR_F_CONNECTION_M))
	     != VAR_F_CONNECTION_NONE) {
		if (mode == VAR_F_CONNECTION_IN)
			mode = IOMODE_IN;
		else if (mode == VAR_F_CONNECTION_OUT)
			mode = IOMODE_OUT;
		else
			mode = IOMODE_MEM;
		_co_make_msd_entry(sym, (symbol_t *) 0,
		                   sym -> var -> at.direct_io, mode);
	}
	return 1;
}

static char * co_curr_base_file_name;

void co_object_header(void)
{
int global_msd;
em_val_t v;
char *s,*q,c;

/* Riserva l'etichetta che segnera` l'inizio dell'oggetto successivo. */

	co_end_text_label = _co_get_tmplabel();

/* Riserva l'etichetta per la lista MSD globale. */

	global_msd = _co_get_tmplabel();

/* Intestazione ed "etichetta magica". */

	_co_op_1name(AOP_FILE,curr_file_context -> name);
	co_set_text();
	em_op(AOP_FIRSTLABEL);
	q = curr_file_context -> name;
	s = strrchr(q,'/');
	if (! s) {
		s = q;
		q = "./";
		c = '\0';
	}
	else {
		c = *s;
		*(s++) = '\0';
	}
	if (main_status.ggdb)
		_co_op_1name(AOP_DIRINFO,q);
	if (c)
		*(s - 1) = c;
	co_curr_base_file_name = s;
	if (main_status.ggdb)
		_co_op_1name(AOP_FILEINFO,s);
	if (main_status.gil)
		_co_op_1hexname(AOP_FILEINFO2,s);
	if (main_status.ggdb)
		em_op(AOP_TYPEINFO);

	em_open_datalist(sizeof(long));
/* Puntatore al prossimo record di inizio oggetto. */
	v.vtype = EM_VTYPE_TLBL;
	v.v.n = co_end_text_label;
	em_addv_datalist(&v,0);
/* Puntatore alla lista MSD globale. */
	v.v.n = global_msd;
	em_addv_datalist(&v,0);
/* Puntatori ai recordi di configurazione presenti nel file. */
	st_explore_context(curr_file_context, _co_emit_configs, (void *) 0);
/* NULL finale */
	v.vtype = EM_VTYPE_NUM;
	v.v.n = 0;
	em_addv_datalist(&v,1);
	em_close_datalist();

/* Creazione della lista MSD globale. */
	_co_def_tmplabel(global_msd);
	st_explore_context(curr_file_context, _co_emit_global_msd, (void *) 0);
/* Terminatore di lista MSD. */
	em_open_datalist(sizeof(long));
	v.vtype = EM_VTYPE_NUM;
	v.v.n = 0;
	em_addv_datalist(&v,1);
	v.v.n = ILC_END;
	em_addv_datalist(&v,1);
	em_close_datalist();
}

static int _co_define_string(context_t *context, symbol_t *sym, void *u)
{
char c, *p;
em_val_t v;

/* Definisce l'etichetta associata alla stringa. */

	if (sym -> class == SYMCL_STRING) {
		_co_def_stringlabel((int)(sym -> type));
		em_open_datalist(1);
		v.vtype = EM_VTYPE_NUM;
		p = sym -> name;
		do {
			v.v.n = (unsigned char) (c = *(p++));
			em_addv_datalist(&v,1);
		} while (c);
		em_close_datalist();
	}
	return 1;
}

void co_object_end(void)
{

/* Codice di fine oggetto. */

/* Si definiscono tutte le stringhe utilizzate nell'oggetto. */

	co_set_text();

	st_explore_context(string_context,_co_define_string,(void *) 0);

/* Infine, Si deve definire un'etichetta, l'indirizzo
 della quale coincidera` con quello di inizio del codice dell'oggetto
 successivo. */

	co_set_text();
	em_op(AOP_LALIGN);
	_co_def_tmplabel(co_end_text_label);

	if (main_status.gil)
		_co_op_1hexname(AOP_EFILEINFO2, co_curr_base_file_name);
}

/*************************************************\
* FUNZIONI DI AMMINISTRAZIONE DEI REGISTRI DI CPU *
\*************************************************/

/* Struttura di stato dei registri. */

static reg_status_t co_regs[MAX_REG_32];

/* Numero di "push", alias livello di stack corrente. */

static int co_stack_level = 0;

/*
* Inizializza il gestore dei registri.
*/

static void _co_reg_reset(void)
{
int i;

	for (i = 0; i < sizeof(co_regs)/sizeof(co_regs[0]); ++i) {
		co_regs[i].flags = 0;
		co_regs[i].level = 0;
		co_regs[i].count = 0;
	}
	co_stack_level = 0;
}

/*
* Blocca un registro, in modo che sia considerato "speciale" e non
* venga mai utilizzato se non su richiesta esplicita.
*/

static void _co_reg_lock(int reg)
{
	co_regs[reg].flags |= RSTS_LOCKED;
}

/*
* Sblocca un registro.
*/
static void _co_reg_unlock(int reg)
{
	co_regs[reg].flags &= ~RSTS_LOCKED;
}

/*
* Salva un registro, rendendolo di nuovo disponibile.
*/

static void _co_reg_transparent_push(int reg)
{
	_co_op_1reg(AOP_PUSHL,reg);
	co_regs[reg].flags &= ~RSTS_USED;
}

static void _co_reg_push(int reg)
{
	_co_reg_transparent_push(reg);
	co_regs[reg].level = ++co_stack_level;
	++co_regs[reg].count;
}

/*
* Recupera un registro dallo stack, rendendolo di nuovo occupato.
*/

static void _co_reg_transparent_pop(int reg)
{
	_co_op_1reg(AOP_POPL,reg);
	co_regs[reg].flags |= RSTS_USED;
	co_regs[reg].flags &= ~RSTS_CACHE;
}

static void _co_reg_pop(int reg)
{
	_co_reg_transparent_pop(reg);
	co_regs[reg].level = --co_stack_level;
	--co_regs[reg].count;
}

/*
* Richiede un registro.
*/

static int _co_reg_request(int reg)
{
int inf,sup,r,lev, nosave;

	if (reg == R_NULL)
		reg = R_ANY;
	nosave = reg & R_F_NOSAVE;
	reg &= ~R_F_NOSAVE;
	if (reg < MAX_REG_32) {

	/* Richiesta specifica di registro. Salva se necessario, ed
	 ignora il flag LOCKED. */

		if (co_regs[reg].flags & RSTS_USED)
			_co_reg_push(reg);
	}
	else {

	/* Richieste generiche. Cerca un registro libero nell'insieme
	 specificato. */

		if (reg == R_ANY) {
		/* Un registro qualsiasi. */
			inf = R_0;
			sup = R_7;
		}
		else if (reg == R_ANY_INDEX) {
		/* Un registro indice qualsiasi, Uscira` quasi
		 sicuramente %edi. */
			inf = R_4;
			sup = R_5;
		}
		else {
		/* Un registro generale qualsiasi. */
			inf = R_0;
			sup = R_3;
		}

	/* Cerca nell'insieme il primo registro non utilizzato. */

		for (reg = inf; reg <= sup; ++reg)
			if (!(co_regs[reg].flags & (RSTS_USED | RSTS_LOCKED)))
				break;

		if (reg > sup) {

			if (nosave)
				return R_NULL;

		/* Se non ci sono registri liberi, cerca il registro
		 non bloccato con il livello di stack piu` basso, e lo salva. */

			lev = co_regs[inf].level;
			for (r = reg = inf; r <= sup; ++r) {
				if (!(co_regs[r].flags & RSTS_LOCKED)
				 && (lev > co_regs[r].level)) {
					lev = co_regs[r].level;
					reg = r;
				}
			}
			_co_reg_push(reg);
		}
	}

/* Marca "usato" il registro, e cancella il flag di cache. */

	co_regs[reg].flags |= RSTS_USED;
	co_regs[reg].flags &= ~RSTS_CACHE;

	return reg;
}

/*
* Libera un registro. Se un registro e` bloccato, non viene estratto
* mai dallo stack.
*/

static void _co_reg_free(int reg)
{
	if (reg == R_NULL)
		return;
	if (co_regs[reg].count && !(co_regs[reg].flags & RSTS_LOCKED))
		_co_reg_pop(reg);
	else
		co_regs[reg].flags &= ~RSTS_USED;
}

/*
* Data un coppia di registri (che si suppongono occupati) ne sceglie
* uno adatto ad essere "riciclato". L'altro dovrebbe essere liberato
* dal chiamante. Quale dei due registri
* liberare e` deciso in base a queste regole :
*
* - I registri LOCKED non vengono mai scelti. Percio`, se entrambi
*   i registri sono LOCKED, il risultato sara` R_NULL.
* - Viene sempre liberato il registro salvato sullo stack per ultimo.
* - A parita` di livello di stack, viene sempre liberato il registro
*   con indice piu` alto.
*/

static int _co_reg_reuse(int r1, int r2)
{
unsigned long t1,t2;

	if (r1 == R_NULL) {
		r1 = r2;
		r2 = R_NULL;
	}
	if (r2 == R_NULL) {
		if (r1 == R_NULL || co_regs[r1].flags & RSTS_LOCKED)
			return R_NULL;
		else
			return r1;
	}
	t1 = r1;
	t2 = r2;

/* Truccaccio maledetto. Si sintetizza in due long il "peso" da dare
 alle varie condizioni. La condizione prioritaria e` il flag "LOCKED",
 segue il livello di stack, infine il codice di registro. */
	if (co_regs[r1].flags & RSTS_LOCKED)
		t1 += 0x8000;	/* Si spera che 0x8000/MAX_REG_32 sia sempre
				 maggiore del massimo livello di stack
				 raggiungibile. */
	if (co_regs[r2].flags & RSTS_LOCKED) {
		t2 += 0x8000;
	/* Due registri bloccati. Ciccia. */
		if (co_regs[r1].flags & RSTS_LOCKED)
			return R_NULL;
	}
	t1 += co_regs[r1].count * MAX_REG_32;
	t2 += co_regs[r2].count * MAX_REG_32;

	if (t1 > t2) {
		return r2;
	}
	else {
		return r1;
	}
}

/*
* Traduzione di un valore "cval_t" in un dato "em_opr_t".
*/

static void _co_cv_to_em(cval_t *cv, em_opr_t *em)
{
	if (cv -> flags & CO_CEX_SCALE)
		em -> scale = cv -> scale;
	else
		em -> scale = 1;

	if ((cv -> flags & CO_CEX_REG1)
	 || (cv -> flags & CO_CEX_REG2)) {
		em -> type = EM_OP_IND;
		if (cv -> flags & CO_CEX_REG1)
			em -> r1 = cv -> r1;
		else
			em -> r1 = R_NULL;
		if (cv -> flags & CO_CEX_REG2)
			em -> r2 = cv -> r2;
		else
			em -> r2 = R_NULL;
	}
	else {
		em -> type = EM_OP_DIR;
		em -> r1 = R_NULL;
		em -> r2 = R_NULL;
	}

	if ((cv -> flags & CO_CEX_NUMOFF)
	 && (cv -> flags & CO_CEX_SYMOFF)) {
		em -> val.vtype = EM_VTYPE_STROFF;
		em -> val.v.s = cv -> symoff;
		em -> val.v.n = cv -> numoff;
	}
	else if (cv -> flags & CO_CEX_SYMOFF) {
		em -> val.vtype = EM_VTYPE_STR;
		em -> val.v.s = cv -> symoff;
	}
	else if (cv -> flags & CO_CEX_NUMOFF) {
		em -> val.vtype = EM_VTYPE_NUM;
		em -> val.v.n = cv -> numoff;
	}
	else {
		em -> val.vtype = EM_VTYPE_NONE;
	}
}

/*
* Questa funzione applica l'operazione di macchina "op", che prevede
* due operandi, al valore "cv", di tipo "cval_t", che descrive uno
* dei possibili indirizzamenti disponibili. L'indirizzamento e`
* utilizzato come primo operando, mentre il secondo operando e` il registro
* indicato dal terzo parametro della funzione. Se vale "R_NULL",
* il registro e` scelto tra quelli coinvolti nell'indirizzamento, se possibile,
* oppure ne e` allocato uno nuovo.
* Il valore di ritorno e` l'indice del registro destinazione.
*/

static int co_last_locallabel = 0;

static int _co_reg_apply(int op, cval_t *cv, int dest)
{
em_opr_t op1,op2;
int rd;

/* Traduce in forma comprensibile dall'emettitore il set di indirizzamento. */

	_co_cv_to_em(cv, &op1);

/* Se non e` specificata la destinazione, si sceglie un registro tra
 quelli facenti parte del set di indirizzamento. Se non ce ne sono,
 si richiede un nuovo registro. */

	if (dest == R_NULL) {
	/* Tenta di preservare i registri "cache", se possibile. */
		if (op1.r1 != R_NULL && (co_regs[(int)(op1.r1)].flags & RSTS_CACHE)) {
			if (op1.r2 != R_NULL && (co_regs[(int)(op1.r2)].flags&RSTS_CACHE)){
			/* ARRGH ! sono entrambi cache ! Tentiamo
			 di richiederne uno diverso, se c'e`, bloccando
			 temporaneamente i due. */
				_co_reg_lock(op1.r1);
				_co_reg_lock(op1.r2);
				rd = _co_reg_request(R_ANY|R_F_NOSAVE);
				_co_reg_unlock(op1.r1);
				_co_reg_unlock(op1.r2);
				if (rd == R_NULL)
					rd = _co_reg_reuse(op1.r1,op1.r2);
			}
			else {
				rd = _co_reg_reuse(R_NULL,op1.r2);
				if (rd == R_NULL) {
					_co_reg_lock(op1.r1);
					rd = _co_reg_request(R_ANY|R_F_NOSAVE);
					_co_reg_unlock(op1.r1);
					if (rd == R_NULL)
						rd=_co_reg_reuse(op1.r1,op1.r2);
				}
			}
		}
		else if(op1.r2!=R_NULL&&(co_regs[(int)(op1.r2)].flags & RSTS_CACHE)) {
			rd = _co_reg_reuse(op1.r1,R_NULL);
			if (rd == R_NULL) {
				_co_reg_lock(op1.r2);
				rd = _co_reg_request(R_ANY|R_F_NOSAVE);
				_co_reg_unlock(op1.r2);
				if (rd == R_NULL)
					rd=_co_reg_reuse(op1.r1,op1.r2);
			}
		}
		else {
			rd = _co_reg_reuse(op1.r1,op1.r2);
		}
		if (rd == R_NULL)
			rd = _co_reg_request(R_NULL);
		else
			co_regs[rd].flags &= ~RSTS_CACHE;
	}
	else {
		rd = dest;
	}

/* Applica l'operazione. */

	op2.type = EM_OP_REG;
	op2.r1 = rd;
/* Registra il punto di intervento del debugger. */
	if (main_status.gil) {
		co_last_locallabel = _co_tmplabel();
	}

	em_op(op,&op1,&op2);

/* Libera i due (eventuali) registri, se non sono stati riciclati. */

	if (op1.r1 != rd)
		_co_reg_free(op1.r1);
	if (op1.r2 != rd)
		_co_reg_free(op1.r2);

	return rd;
}

/*
* Emette le informazioni di debug relative all'ultima operazione prodotta.
*/

static void _co_emit_v_debinfo(co_arg_t *p)
{
cval_t *cv;
em_opr_t op1,op2,op3,op4,op5;
char tmp[5];
#ifdef FFSS
int scale,r1,r2,off;
#endif /* FFSS */

	cv = &p -> val.cexpr;

	if (main_status.gil) {
		op1.type = op2.type = op3.type = op4.type = EM_OP_DIRD;
		op1.val.vtype = EM_VTYPE_NUM;
		op1.val.v.n = pScCtl -> n_line;
		if (cv -> flags & CO_CEX_SCALE)
			tmp[2] = cv -> scale + '0';
		else
			tmp[2] = '1';
		tmp[0] = tmp[1] = R_NULL + '0';
		if (cv -> flags & CO_CEX_REG1)
			tmp[0] = cv -> r1 + '0';
		if (cv -> flags & CO_CEX_REG2)
			tmp[1] = cv -> r2 + '0';
		else if (tmp[2] != '1') {
			tmp[1] = tmp[0];
			tmp[0] = R_NULL + '0';
		}
		switch (p -> format) {
		case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
		case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
			tmp[3] = '1';
			break;
		case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
		case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
			tmp[3] = '2';
			break;
		case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_S:
		case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U:
		default:
			tmp[3] = '4';
			break;
		}
		tmp[4] = '\0';
		op2.val.vtype = EM_VTYPE_STR;
		op2.val.v.s = tmp;
		op3.val.vtype = EM_VTYPE_NUM;
		if (cv -> flags & CO_CEX_NUMOFF)
			op3.val.v.n = cv -> numoff;
		else
			op3.val.v.n = 0;
		op4.val.vtype = EM_VTYPE_STR;
		if (cv -> flags & CO_CEX_SYMOFF)
			op4.val.v.s = cv -> symoff;
		else
			op4.val.v.s = "";
		op5.type = EM_OP_DIR;
		op5.val.vtype = EM_VTYPE_TLBL;
		op5.val.v.n = co_last_locallabel;

		em_op(AOP_VALINFO2,&op1,&op2,&op3,&op4,&op5);
	}
}

static void _co_emit_i_debinfo(int r, int ind)
{
em_opr_t op1,op2,op3;
#ifdef FFSS
char tmp[5];
#endif /* FFSS */

	if (main_status.gil) {
		op1.type = EM_OP_DIRD;
		op1.val.vtype = EM_VTYPE_NUM;
		op1.val.v.n = pScCtl -> n_line;
		op2 = op1;
		op3 = op1;
		op2.val.v.n = r;
		op3.val.v.n = ind;
		em_op(AOP_INDINFO2,&op1,&op2,&op3);
	}
}

/*
* Copia un registro su un altro, senza liberare il primo.
*/

static int _co_reg_copy(int src, int dest)
{
	dest = _co_reg_request(dest);
	_co_op_2reg(AOP_MOVL,src,dest);
	return dest;
}

/*
* Copia un registro su un altro, lo libera, e marca occupato il secondo.
*/

static int _co_reg_load(int src, int dest)
{
	_co_reg_free(src);
	return _co_reg_copy(src,dest);
}

/*
* Questa funzione carica alla locazione indicata la "cv" il registro
* "reg", eseguendo un caricamento a "sz" byte.
*/


static int _co_reg_store_core(int reg, cval_t *cv, int sz)
{
em_opr_t op1,op2;
int r;
int ex;

	ex = 0;

	switch (sz) {
	case 4:
		op1.type = EM_OP_REG;
		op1.r1 = reg;
	/* Traduce in forma comprensibile dall'emettitore. */
		_co_cv_to_em(cv, &op2);
		em_op(AOP_MOVL,&op1,&op2);
		break;
	case 2:
	case 1:
		if (reg > R_3) {
		/* Un casino. */
			r = _co_reg_request(R_ANY_GENERIC | R_F_NOSAVE);
			if (r != R_NULL) {
		/* Alloca un registro generale libero, se c'e`, trasferisce
		 il dato e libera il vecchio registro. */
				_co_reg_load(reg,r);
				reg = r;
			}
			else {
		/* Se non c'e` un registro generale libero, son cazzi. */
		/* Scambia con EAX. Non c'e` perdita di informazioni. */
				_co_op_2reg(AOP_XCHGL,R_0,reg);
		/* Controlla se, tra i registri in "cv", c'e` EAX. Se c'e`
		 mette al suo posto il registro "reg", e "reg" diventa EAX. */
				if ((cv -> flags & CO_CEX_REG1)
				    && (cv -> r1 == R_0)) {
					cv -> r1 = reg;
					reg = R_0;
					ex = 1;
				}
				else if ((cv -> flags & CO_CEX_REG2)
				         && (cv -> r2 == R_0)) {
					cv -> r2 = reg;
					reg = R_0;
					ex = 2;
				}
				else {
					ex = 3;
				}
			}
		}
	/* Traduce in forma comprensibile dall'emettitore. */
		_co_cv_to_em(cv, &op2);
		op1.type = EM_OP_REG;
		if (sz == 1) {
			op1.r1 = em_lowb(reg);
			em_op(AOP_MOVB,&op1,&op2);
		}
		else {
			op1.r1 = em_loww(reg);
			em_op(AOP_MOVW,&op1,&op2);
		}
	/* Rimette a posto EAX (truccaccio). */
		if (ex) {
			_co_op_2reg(AOP_XCHGL,R_0,reg);
			switch (ex) {
			case 1:
				reg = cv -> r1;
				cv -> r1 = R_0;
				break;
			case 2:
				reg = cv -> r2;
				cv -> r2 = R_0;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return reg;
}

static int _co_reg_store(int reg, cval_t *cv, int sz)
{
em_opr_t op1;
cval_t cv2;

	if (cv -> flags & CO_CEX_NVRAM && sz == 4) {
/* Poveri noi ! Ci sono problemi hardware a causa dei quali le scritture
 a 32 bit alla RAM non volatile devono essere spezzate. */
		op1.type = EM_OP_REG;
		op1.r1 = reg;
		cv2 = *cv;
		if (! (cv2.flags & CO_CEX_NUMOFF)) {
			cv2.flags |= CO_CEX_NUMOFF;
			cv2.numoff = 0;
		}
		cv2.numoff += 2;
		em_op(AOP_SWAP16,&op1);
		_co_reg_store_core(reg, &cv2, 2);
		em_op(AOP_SWAP16,&op1);
		reg = _co_reg_store_core(reg, cv, 2);
	}
	else {
		reg = _co_reg_store_core(reg, cv, sz);
	}
/* Libera i registri che compongono l'indirizzo destinazione. */
	if (cv -> flags & CO_CEX_REG2)
		_co_reg_free(cv -> r2);
	if (cv -> flags & CO_CEX_REG1)
		_co_reg_free(cv -> r1);
	return reg;
}

/*
* Questa funzione carica alla locazione indicata la "cv" la costante
* "val", eseguendo un caricamento a "sz" byte.
*/

static void _co_const_store_core(long val, cval_t *cv, int sz)
{
em_opr_t op1,op2;
int aop = -1;

/* Costruisce il primo operando. */
	op1.type = EM_OP_IMM;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = val;

/* Traduce in forma comprensibile dall'emettitore. */
	_co_cv_to_em(cv, &op2);

/* Sceglie l'istruzione in base al formato. */
	switch (sz) {
	case 4:
		aop = AOP_MOVL;
		break;
	case 2:
		aop = AOP_MOVW;
		break;
	case 1:
		aop = AOP_MOVB;
	default:
		break;

	}

/* Emette l'istruzione. */
	em_op(aop,&op1,&op2);
}

static void _co_const_store(long val, cval_t *cv, int sz)
{
cval_t cv2;

	if (cv -> flags & CO_CEX_NVRAM && sz == 4) {
		cv2 = *cv;
		if (! (cv2.flags & CO_CEX_NUMOFF))
			cv2.numoff = 0;
		cv2.numoff += 2;
		cv2.flags |= CO_CEX_NUMOFF;
		_co_const_store_core((val >> 16) & 0xFFFF, &cv2, 2);
		_co_const_store_core(val & 0xFFFF, cv, 2);
	}
	else {
		_co_const_store_core(val, cv, sz);
	}

/* Libera i registri che compongono l'indirizzo destinazione. */
	if (cv -> flags & CO_CEX_REG2)
		_co_reg_free(cv -> r2);
	if (cv -> flags & CO_CEX_REG1)
		_co_reg_free(cv -> r1);
}

/*
* Gestione dei "rami di codice", cioe` di quelle sequenze di
* istruzioni che il debugger raccoglie in una sola lista di break.
*/
static void _co_start_branch(void)
{
	co_branch_instr_count = 0;
}

static void _co_inc_branch(void)
{
	++co_branch_instr_count;
}

static int _co_branch_count(void)
{
	return co_branch_instr_count;
}

/*************************************************\
* GENERAZIONE DI CODICE DI INIZIO E FINE FUNZIONE *
\*************************************************/

/*
* Esegue un indirizzamento indicizzato, e marca occupato il registro
* destinazione.
*/

static void _co_loadind(int off, int ind, int dest)
{
em_opr_t op1,op2;

	op1.type = EM_OP_IND;
	op1.r1 = ind;
	op1.r2 = 0;
	op1.scale = 1;
	op1.val.vtype = EM_VTYPE_NUM;
	op1.val.v.n = off;
	op2.type = EM_OP_REG;
	op2.r1 = dest;
	em_op(AOP_MOVL,&op1,&op2);
}

/*
* Carica un indirizzo costante in un registro.
*/

static void _co_loadaddr(char * src, int dest)
{
em_opr_t op1,op2;

	op2.type = EM_OP_REG;
	op2.r1 = dest;
	op1.type = EM_OP_IMM;
	op1.val.vtype = EM_VTYPE_STR;
	op1.val.v.s = src;
	em_op(AOP_MOVL,&op1,&op2);
	co_regs[dest].flags |= RSTS_USED;
	co_regs[dest].flags &= ~RSTS_CACHE;
}

/*
* Carica un parametro in un registro secondo le convenzioni usuali
* dei compilatori per i386.
*/

static void _co_loadparam(int src, int dest)
{
	_co_loadind((int)((src + 2) * sizeof(long)), R_BP, dest);
	co_regs[dest].flags |= RSTS_USED;
	co_regs[dest].flags &= ~RSTS_CACHE;
}

/*
* Produzione del codice di inizio e fine funzione.
*
* _co_begin_code  produzione delle istruzioni di inizio.
* co_begin_fb     funzione principale per l'inizio (etichetta+codice).
* _co_end_code    produzione delle istruzioni di fine.
* co_end_fb       funzione principale per la fine.
*/

static void _co_begin_code(void)
{
em_opr_t op1,op2;

	co_curr_acc = R_NULL;
	_co_reset_cache();
	_co_reg_reset();

/* Informazioni di debug. */

	op1.type = op2.type = EM_OP_DIRD;
	op1.val.vtype = EM_VTYPE_STR;
	op1.val.v.s = co_curr_fb -> name;
	op2.val.vtype = EM_VTYPE_NUM;
	op2.val.v.n = pScCtl -> n_line;
	if (main_status.ggdb)
		em_op(AOP_FUNCINFO,&op1,&op2);
	if (main_status.gil)
		em_op(AOP_FUNCINFO2,&op1,&op2);
	_co_lineinfo();

	_co_reg_push(R_BP);
	_co_reg_load(R_SP,R_BP);
	_co_reg_push(R_PST);
	if (co_curr_fb -> type -> code == TYPE_FUNCTION) {
		_co_loadaddr(co_curr_fb -> type -> d.fb.statics -> name, R_PST);
	}
	else {
		_co_loadparam(0,R_PST);
	}
/*
	_co_reg_lock(R_ACC);
*/
	_co_reg_lock(R_PST);
	_co_reg_lock(R_SP);
	_co_reg_lock(R_BP);
}

/*
* Generazione dei "buchi" di codice necessari per ospitare
* le istruzioni aggiunte al volo dal debugger (call brk_hook).
*/

static void _co_add_debug_filler(void)
{
int i;

	for (i = 0; i < 5; ++i)
		em_op(AOP_NOP);
}

/* Con i salti e le etichette si deve produrre un "buco" di 5 byte
 prima del punto di debug comunicato al debugger, in modo che le
 eventuali liste precedenti non vadano a ricoprire il codice corrente. */
 
static void _co_fixdebug(int opr)
{
em_opr_t op1,op2;

	if (main_status.gil) {
		op1.type = op2.type = EM_OP_DIRD;
		op1.val.vtype = op2.val.vtype = EM_VTYPE_NUM;
		op1.val.v.n = pScCtl -> n_line;
		op2.val.v.n = co_curr_acc;
		em_op(opr,&op1,&op2);
	}
}

static void _co_fixdebug_jmp(void)
{
	_co_add_debug_filler();
	_co_fixdebug(AOP_JMPINFO2);
/* Oltre allo spazio di fine ramo, ci vogliono due spazi di sicurezza
 per il caso sommamente sfigato di lista di break di un solo elemento
 contenente un JMP o RET. */
	_co_add_debug_filler();
	_co_add_debug_filler();
}

static void _co_fixdebug_lbl(void)
{
/* Per il caso peggiore (un'istruzione LD o ST seguita da un'etichetta)
 il debugger ha bisogno di dieci byte per immagazzinare due punti di break
 a distanza di sicurezza. */
	_co_add_debug_filler();
	_co_add_debug_filler();
	_co_fixdebug(AOP_LBLINFO2);
}

/*
* Emette il codice di fine funzione.
*/

static int co_end_label_index = 0;

static void _co_end_code(void)
{
	_co_fixdebug_lbl();
	_co_def_tmplabel(co_end_label_index);
	_co_loadind(-(int)sizeof(long), R_BP, R_PST);
/*
	_co_resume(R_PST);
*/
	_co_reg_load(R_BP,R_SP);
	_co_reg_pop(R_BP);
	em_op(AOP_RET);
}

void co_begin_fb(symbol_t *fb)
{
	co_curr_fb = fb;
	co_set_text();
	_co_op_1name(AOP_GLOBAL,fb -> name);
	_co_op_1name(AOP_LABEL,fb -> name);
	_co_begin_code();
	_co_start_branch();
	++co_fb_count;
	co_end_label_index = _co_get_tmplabel();
}

void co_end_fb(symbol_t *fb)
{
	_co_end_code();
}

void co_label(void)
{
	_co_reset_cache();
	_co_start_branch();
}

/*
* Applicazione dell'operatore "op" all'etichetta associata al simbolo,
* ed assegnamento dell'indice di locazione, se necessario.
*/

static void _co_op_label(int op, symbol_t *sym)
{
	if (! sym)
		return;
	if (sym -> var -> location == 0)
		sym -> var -> location = _co_get_tmplabel();
	_co_op_1tlbl(op,(long)(sym -> var -> location));
}

/*
* Definizione di un'etichetta IL, con assegnamento dell'indice di
* locazione, se necessario.
*/

void co_def_label(symbol_t *sym)
{
/* Cinque NOP prima di ogni etichetta scongiurano il problema
 dei breakpoint a cavallo di un punto di salto. */
	_co_fixdebug_lbl();
	_co_op_label(AOP_LABEL,sym);
}

/******************************************************************\
* VALUTAZIONE E COMPILAZIONE DI RIFERIMENTI A VARIABILI E COSTANTI *
\******************************************************************/

/*
* Inserimento di un elemento nello stack.
*/

static void _co_expr_push(co_arg_t * p)
{
	if (co_expr_sp >= MAX_CO_REG_ARGS)
		util_fatal(2030);
	co_stack[co_expr_sp++] = p;
}

/*
* Estrazione di un elemento dallo stack.
*/

static co_arg_t *_co_expr_pop(void)
{
	if (co_expr_sp == 0)
		util_fatal(2032);
	return co_stack[--co_expr_sp];
}


/*
* Funzione per l'allocazione di un argomento.
*/

static co_arg_t * _co_expr_new(void)
{
	if (co_expr_argp >= MAX_CO_REG_ARGS)
		util_fatal(2031);
	co_args[co_expr_argp].val.expr.nargs = 0;
	return &co_args[co_expr_argp++];
}

/*
* Caricamento di un valore come costante, estratto dallo stack del
* valutatore di espressioni costanti.
*/

co_arg_t * co_expr_push_num(void)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_NUM;
	ce_pop(&p -> val.num);
	_co_expr_push(p);
	return p;
}

/*
* Caricamento di un valore come costante stringa, estratto dall'apposita
* variabile statica.
*/

co_arg_t * co_expr_push_string(symbol_t *sym)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_STRING;
	p -> val.sym = sym;
	_co_expr_push(p);
	return p;
}

/*
* Caricamento di un valore come simbolo.
*/

co_arg_t * co_expr_push_sym(symbol_t *sym)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_SYM;
	p -> val.sym = sym;
	_co_expr_push(p);
	return p;
}

/*
* Caricamento di un valore come CVAL.
*/

co_arg_t * co_expr_push_cval(cval_t *v, int fmt)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_CVAL;
	p -> val.cexpr = *v;
	_co_expr_push(p);
	p -> format = fmt;
	return p;
}

/*
* Crea un elemento di tipo espressione con 0 parametri.
*/

void co_expr_push_expr(enum _co_exprop_t code)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_EXPR;
	p -> val.expr.code = code;
	p -> val.expr.nargs = 0;
	_co_expr_push(p);
}

/*
* Applica il valore corrente alla funzione...sottostante.
*/

void co_expr_addarg(void)
{
co_arg_t *p,*q;

	p = _co_expr_pop();
	q = _co_expr_pop();
	if (q -> val.expr.nargs >= MAX_CO_EXPR_ARGS)
		util_fatal(2034);
	q -> val.expr.args[ (q -> val.expr.nargs)++ ] = p;
	_co_expr_push(q);
}

/*
* Inizia la generzione di un'espressione di tipo "index", creando
* l'espressione "(CO_EOP_INDEX op1)".
*/

void co_expr_index_begin(void)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_EXPR;
	p -> val.expr.code = CO_EOP_INDEX;
	p -> val.expr.nargs = 1;
	p -> val.expr.args[0] = _co_expr_pop();
	_co_expr_push(p);
}

/*
* Registra un nuovo indice nell'espressione di tipo "index", corrente.
* L'espressione passa da "(CO_EOP_INDEX op1)" (per esempio) a
* "(CO_EOP_INDEX op1 op2)"
*/

void co_expr_index_register(void)
{
co_arg_t *p,*q;

	p = _co_expr_pop();
	q = _co_expr_pop();
	if (q -> val.expr.nargs >= MAX_CO_EXPR_ARGS)
		util_fatal(2034);
	q -> val.expr.args[ (q -> val.expr.nargs)++ ] = p;
	_co_expr_push(q);
}

/*
* Costruisce l'espressione "(CO_EOP_EXTRACT op1 op2)"
*/

void co_expr_extract(void)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_EXPR;
	p -> val.expr.code = CO_EOP_EXTRACT;
	p -> val.expr.nargs = 2;
	p -> val.expr.args[1] = _co_expr_pop();
	p -> val.expr.args[0] = _co_expr_pop();
	_co_expr_push(p);
}

/*
* Valuta l'espressione che si trova in cima allo stack
* (in pratica, la compila), reinizializza lo stack e libera tutti
* gli argomenti.
*/

#ifdef DEBUG
static void _co_dump_expr(co_arg_t *p)
{
int i;

	switch (p -> type) {
	case CO_ARG_STRING:
		printf("'%s'",p -> val.sym -> name);
		break;
	case CO_ARG_SYM:
		printf("%s",p -> val.sym -> name);
		break;
	case CO_ARG_NUM:
		db_putval(&p ->val.num);
		break;
	case CO_ARG_EXPR:
		switch (p -> val.expr.code) {
		case CO_EOP_EXTRACT:
			printf("extract");
			break;
		case CO_EOP_INDEX:
			printf("index");
			break;
		case CO_EOP_EVAL:
			printf("eval");
			break;
		case CO_EOP_ADD:
			printf("add");
			break;
		case CO_EOP_MUL:
			printf("mul");
			break;
		default:
			printf("?");
			break;
		}
		printf("(");
		for (i = 0; i < p -> val.expr.nargs; ++i) {
			if (i)
				printf(",");
			_co_dump_expr(p -> val.expr.args[i]);
		}
		printf(")");
		break;
	case CO_ARG_CVAL:
		if (p -> val.cexpr.flags & CO_CEX_SYMOFF) {
			printf("%s",p -> val.cexpr.symoff);
		}
		if (p -> val.cexpr.flags & CO_CEX_NUMOFF) {
			if (p -> val.cexpr.flags & CO_CEX_SYMOFF) {
				printf("+");
			}
			printf("%ld",p -> val.cexpr.numoff);
		}
		if (p -> val.cexpr.flags & CO_CEX_REG1) {
			printf("(%s",em_regs[(int)(p -> val.cexpr.r1)]);
		}
		if (p -> val.cexpr.flags & CO_CEX_REG2) {
			printf(",%s",em_regs[(int)(p -> val.cexpr.r1)]);
		}
		if (p -> val.cexpr.flags & CO_CEX_SCALE) {
			printf(",%d",p -> val.cexpr.scale);
		}
		if (p -> val.cexpr.flags & CO_CEX_REG1) {
			printf(")");
		}
		break;
	default:
		printf("?");
		break;
	}
}
#endif

static int _co_make_format(symbol_t *sym)
{
int format;
type_t *t;
/* Decide il formato. */
	t = sym -> type;
	if (st_is_array_type(sym -> type)) {
		t = sym -> type -> d.a.type;
	}
	switch (t -> size) {
	case 1:	format = CO_CEX_F_DIM_1; break;
	case 2:	format = CO_CEX_F_DIM_2; break;
	case 4:	format = CO_CEX_F_DIM_4; break;
	case 8:	format = CO_CEX_F_DIM_8; break;
	default: format = CO_CEX_F_DIM_1; break;
	}
	if (t -> code >= TYPE_PROGRAM) {
/* Maldestro tentativo di rappezzare la mancanza di informazioni "a valle"
 memorizzando nel formato che l'oggetto non e` una variabile ordinaria. */
		format |= CO_CEX_F_SPC_F;
	}
	if (st_is_float_type(t)) {
		format |= CO_CEX_F_FMT_F;
	}
	else if (st_is_unsigned_type(t)) {
		format |= CO_CEX_F_FMT_U;
	}
	else {
		format |= CO_CEX_F_FMT_S;
	}
	return format;
}

static void _co_reset_cache(void)
{
	co_regs[R_4].flags &= ~RSTS_CACHE;
	co_regs[R_2].flags &= ~RSTS_CACHE;
}

static void _co_cache_pKeyTab(void)
{
	co_regs[R_4].flags |= RSTS_CACHE;
}

static int _co_pKeyTab_cached(void)
{
	return co_regs[R_4].flags & RSTS_CACHE;
}

static void _co_cache_pchMem(void)
{
	co_regs[R_2].flags |= RSTS_CACHE;
}

static int _co_pchMem_cached(void)
{
	return co_regs[R_2].flags & RSTS_CACHE;
}

static int _co_get_cache(void)
{
	return (_co_pchMem_cached() != 0) | ((_co_pKeyTab_cached() != 0) << 1);
}

static void _co_set_cache(int v)
{
	if (v & 0x01)
		_co_cache_pchMem();
	else
		co_regs[R_2].flags &= ~RSTS_CACHE;
	if (v & 0x02)
		_co_cache_pKeyTab();
	else
		co_regs[R_4].flags &= ~RSTS_CACHE;
}

static co_arg_t * _co_load_pKeyTab(void)
{
cval_t cv;
co_arg_t *p;

	if (_co_pKeyTab_cached()) {
		co_regs[R_4].flags |= RSTS_USED;
		cv.flags = CO_CEX_REG1;
		cv.r1 = R_4;
		co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
		p = _co_expr_pop();
	}
	else {
		cv.flags = CO_CEX_SYMOFF;
		cv.symoff = "pKeyTab";
		co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
		_co_expr_eval_core();
		p = _co_expr_pop();
/* Trattamento speciale del registro %edi. Si tenta di utilizzarlo come
 cache per pKeyTab. */
		if (p -> val.cexpr.r1 != R_4) {
			if (!(co_regs[R_4].flags & RSTS_USED)) {
				_co_reg_copy(p -> val.cexpr.r1,R_4);
				_co_reg_free(R_4);
				_co_cache_pKeyTab();
			}
		}
		else {
			_co_cache_pKeyTab();
		}
	}
	return p;
}

static co_arg_t * _co_load_pchMem(void)
{
cval_t cv;
co_arg_t *p;

	if (_co_pchMem_cached()) {
		co_regs[R_2].flags |= RSTS_USED;
		cv.flags = CO_CEX_REG1;
		cv.r1 = R_2;
		co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
		p = _co_expr_pop();
	}
	else {
		cv.flags = CO_CEX_SYMOFF;
		cv.symoff = "pchMem";
		co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
		_co_expr_eval_core();
		p = _co_expr_pop();
/* Trattamento speciale del registro %ecx. Si tenta di utilizzarlo come
 cache per pchMem. */
		if (p -> val.cexpr.r1 != R_2) {
			if (!(co_regs[R_2].flags & RSTS_USED)) {
				_co_reg_copy(p -> val.cexpr.r1,R_2);
				_co_reg_free(R_2);
				_co_cache_pchMem();
			}
		}
		else {
			_co_cache_pchMem();
		}
	}
	return p;
}

static co_arg_t * _co_load_NVRAM(void)
{
cval_t cv;
co_arg_t *p;

	cv.flags = CO_CEX_SYMOFF;
	cv.symoff = "NVRAM_pub";
	co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
	_co_expr_eval_core();
	p = _co_expr_pop();
	return p;
}

static void _co_combine_cv_with_base_addr(cval_t *cv,symbol_t *sym)
{
co_arg_t *p,*q;

	co_expr_push_cval(cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
	_co_expr_eval_core();
	p = _co_expr_pop();
	if (sym -> var -> flags & VAR_F_RETAIN)
		q = _co_load_NVRAM();
	else
		q = _co_load_pchMem();
	*cv = p -> val.cexpr;
	if (sym -> var -> flags & VAR_F_RETAIN)
		cv -> flags |= CO_CEX_REG2 | CO_CEX_NVRAM;
	else
		cv -> flags |= CO_CEX_REG2;
	cv -> r2 = q -> val.cexpr.r1;
}

static void _co_compile_sym(symbol_t *sym)
{
#ifdef FFSS
cval_t cv,cv2;
int acc;
type_t *t;
#else
cval_t cv;
#endif /* FFSS */
co_arg_t *p,*q;

	if (sym -> class != SYMCL_VAR) {
/* Non e` una variabile. Brutto segno. */
		err_error(LA_E_NOTAVAR,sym -> name);
		cv.flags = CO_CEX_NUMOFF;
		cv.numoff = 0;
	}
/* Marca "usato" il simbolo. */
	sym -> var -> flags |= VAR_F_USED;
/* A seconda del tipo di variabile... */
	if ((sym -> var -> flags & VAR_F_CONNECTION_M)
	    == VAR_F_CONNECTION_MEMORY) {
/* E` un segnale di scambio. Che palle ! */
		if (sym -> var -> at.direct_io[3] == '*') {
		/* Lo sa il cazzo. */
			cv.flags = CO_CEX_SYMOFF;
			cv.symoff = sym -> name;
			co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
			_co_expr_eval_core();
			q = _co_expr_pop();
			_co_mult7(q -> val.cexpr.r1);
			p = _co_load_pKeyTab();
			cv = p -> val.cexpr;
			cv.flags |= CO_CEX_REG2;
			cv.r2 = q -> val.cexpr.r1;
		}
		else {
			p = _co_load_pKeyTab();
			cv = p -> val.cexpr;
			cv.flags |= CO_CEX_NUMOFF;
			cv.numoff = 7*atoi(&sym -> var -> at.direct_io[3]);
		}
		_co_combine_cv_with_base_addr(&cv,sym);
	}
	else if (sym -> var -> flags & VAR_F_LOCAL_FB) {
/* E` una variabile interna ad un blocco. Nome della funzione + offset, oppure
 puntatore alla zona locale + offset */
	/*
		if (co_curr_fb -> type -> code == TYPE_FUNCTION) {
			cv.flags = CO_CEX_SYMOFF | CO_CEX_NUMOFF;
			cv.symoff = co_curr_fb -> type -> d.fb.statics -> name;
			cv.numoff = sym -> var -> location;
		}
		else
	*/
		{
			cv.flags = CO_CEX_REG1 | CO_CEX_NUMOFF;
			cv.r1 = R_PST;
			cv.numoff = sym -> var -> location;
		}
	}
	else {
/* E` una statica. Basta il nome (forse). */
		cv.flags = CO_CEX_SYMOFF;
		cv.symoff = sym -> name;
	}
	co_expr_push_cval(&cv,_co_make_format(sym));
}

static void _co_compile_and_apply_cval(int cop,co_arg_t *p)
{
cval_t *cv;
int off;
int r;

	cv = &p -> val.cexpr;

	switch (p -> format) {
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
		off = 0;
		r = em_lowb(co_curr_acc);
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
		off = 1;
		r = em_loww(co_curr_acc);
		break;
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_S:
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U:
	default:
		off = 2;
		r = co_curr_acc;
		break;
	}
	_co_reg_apply(cop + off,cv,r);
}

static co_arg_t co_last_complex_address;

static void _co_compile_cval(co_arg_t *p)
{
cval_t *cv;
int r;

	co_last_complex_address = *p;

	cv = &p -> val.cexpr;

	switch (p -> format) {
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
		r = _co_reg_apply(AOP_MOVZBL,cv,R_NULL);
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
		r = _co_reg_apply(AOP_MOVZWL,cv,R_NULL);
		break;
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
		r = _co_reg_apply(AOP_MOVSBL,cv,R_NULL);
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
		r = _co_reg_apply(AOP_MOVSWL,cv,R_NULL);
		break;
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_S:
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U:
	default:
		r = _co_reg_apply(AOP_MOVL,cv,R_NULL);
		break;
	}

/* Il valore e` stato estratto. Ora risiede su un registro di CPU, percio`
 ha una dimensione di 32 bit (per ora i valori "double" non sono gestiti). */

	cv -> flags = CO_CEX_REG1;
	cv -> r1 = r;

	co_expr_push_cval(cv,CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U);
}

static void _co_compile_extract(co_arg_t *e)
{
#ifdef FFSS
int acc,r;
co_arg_t *base,*idx1,*idx2,*p,*q;
em_opr_t op1,op2;
cval_t *pcv;
int cindexf,i,n,shvar,tmp;
int cindex = 0;
#else
co_arg_t *base,*p;
#endif /* FFSS */
symbol_t *sym;
type_t *t;

	base = e -> val.expr.args[0];
	t = base -> val.sym -> type;
	_co_expr_push(base);
	_co_expr_eval_core();
	p = _co_expr_pop();
	sym = (e -> val.expr.args[1]) -> val.sym;
	if (! (p -> val.cexpr.flags & CO_CEX_NUMOFF)) {
		p -> val.cexpr.numoff = 0;
		p -> val.cexpr.flags |= CO_CEX_NUMOFF;
	}
	p -> val.cexpr.numoff += sym -> var -> location;
	co_expr_push_cval(&p -> val.cexpr,_co_make_format(sym));
}

static void _co_bound_chk(long base, int dim, int r)
{
	_co_op_sub_imm_reg(base, r);
	if (main_status.bound_check) {
		_co_op_imm_reg(AOP_CMPL, dim, r);
		if (main_status.bound_check == 1) {
			em_op(AOP_ERRBOUND);
		}
		else {
			em_op(AOP_ERRBOUND2);
		}
	}
}

static void _co_compile_index(co_arg_t *e)
{
int acc,r = -1;
co_arg_t *base,*idx1,*idx2,*p,*q;
cval_t cv,*pcv;
#ifdef FFSS
em_opr_t op1,op2;
int cindex,cindexf,i,n,shvar,tmp;
#else
int cindex = 0;
int cindexf,n,shvar,tmp;
#endif /* FFSS */
symbol_t *sym;
type_t *t;

	n = e -> val.expr.nargs;
	base = e -> val.expr.args[0];
	if (base -> type == CO_ARG_EXPR) {
	/* Si fa l'ipotesi che l'espressione sia un'estrazione di campo. */
		sym = base -> val.expr.args[1] -> val.sym;
	}
	else {
		sym = base -> val.sym;
	}
	t = sym -> type;
	if (t -> d.a.n_dim != --n) {
		err_error(LA_E_INCIND,NULL);
		return;
	}
	_co_expr_push(e -> val.expr.args[n]);
	_co_expr_eval_core();
	idx1 = _co_expr_pop();
	if (idx1 -> type == CO_ARG_NUM) {
		cindex = idx1 -> val.num.data.l - t -> d.a.idx_base[n - 1];
		cindexf = 1;
		if (cindex < 0 || cindex >= t -> d.a.dim[n - 1]) {
			err_error(LA_E_INDOUT,NULL);
		}
	}
	else {
		cindexf = 0;
		_co_expr_push(idx1);
		acc = co_curr_acc;
		_co_expr_eval_ld();
		co_curr_acc = acc;
		idx1 = _co_expr_pop();
		r = idx1 -> val.cexpr.r1;
		_co_emit_i_debinfo(r, 0);
		_co_bound_chk((long) t -> d.a.idx_base[n - 1],
			      t -> d.a.dim[n - 1], r);
	}
	if (n == 2) {
		idx2 = idx1;
		_co_expr_push(e -> val.expr.args[--n]);
		_co_expr_eval_core();
		idx1 = _co_expr_pop();
		if (idx1 -> type == CO_ARG_NUM) {
			tmp = idx1 -> val.num.data.l - t -> d.a.idx_base[0];
			if (tmp < 0 || tmp >= t -> d.a.dim[0]) {
				err_error(LA_E_INDOUT,NULL);
			}
			if (cindexf) {
				cindex += tmp * t -> d.a.dim[1];
				cindexf = 1;
			}
			else {
				_co_op_imm_reg(AOP_IMULL, t -> d.a.dim[1],r);
				_co_op_add_imm_reg(tmp,r);
				idx1 = idx2;
				cindexf = 0;
			}
		}
		else {
			_co_expr_push(idx1);
			acc = co_curr_acc;
			_co_expr_eval_ld();
			co_curr_acc = acc;
			idx1 = _co_expr_pop();
			r = idx1 -> val.cexpr.r1;
			_co_emit_i_debinfo(r, 1);
			_co_bound_chk((long) t -> d.a.idx_base[n - 1],
			              t -> d.a.dim[n - 1], r);
			_co_op_imm_reg(AOP_IMULL,t -> d.a.dim[1],r);
			if (cindexf) {
				_co_op_add_imm_reg((long)cindex,r);
			}
			else {
				if (idx2 -> val.cexpr.r1 > r) {
					_co_op_2reg(AOP_ADDL,
						    idx2 -> val.cexpr.r1,
						    r);
					_co_reg_free(idx2 -> val.cexpr.r1);
				}
				else {
					_co_op_2reg(AOP_ADDL,
						    r,
						    idx2 -> val.cexpr.r1);
					_co_reg_free(r);
					r = idx2 -> val.cexpr.r1;
					idx1 -> val.cexpr.r1 = r;
				}
			}
			cindexf = 0;
		}
	}
	if ((sym -> var -> flags & VAR_F_CONNECTION_M)
	    == VAR_F_CONNECTION_MEMORY) {
		shvar = 1;
		if (sym -> var -> at.direct_io[3] == '*') {
			cv.flags = CO_CEX_SYMOFF;
			cv.symoff = sym -> name;
			co_expr_push_cval(&cv,CO_CEX_F_FMT_U | CO_CEX_F_DIM_4);
			_co_expr_eval_core();
			q = _co_expr_pop();
			if (cindexf) {
				_co_op_add_imm_reg( (long)cindex,
					    q -> val.cexpr.r1);
			}
			else {
				if (idx1 -> val.cexpr.r1 > q -> val.cexpr.r1) {
					_co_op_2reg(AOP_ADDL,
						    idx1 -> val.cexpr.r1,
						    q -> val.cexpr.r1);
					_co_reg_free(idx1 -> val.cexpr.r1);
				}
				else {
					_co_op_2reg(AOP_ADDL,
						    q -> val.cexpr.r1,
						    idx1 -> val.cexpr.r1);
					_co_reg_free(q -> val.cexpr.r1);
					q -> val.cexpr.r1=idx1 -> val.cexpr.r1;
				}
			}
			_co_mult7(q -> val.cexpr.r1);
			p = _co_load_pKeyTab();
			cv = p -> val.cexpr;
			cv.flags |= CO_CEX_REG2;
			cv.r2 = q -> val.cexpr.r1;
		}
		else {
			if (! cindexf) {
				_co_op_add_imm_reg( (long)atoi(&sym->var->at.direct_io[3]),
					       idx1 -> val.cexpr.r1);
				_co_mult7(idx1 -> val.cexpr.r1);
			}
			p = _co_load_pKeyTab();
			cv = p -> val.cexpr;
			if (cindexf) {
				cv.flags |= CO_CEX_NUMOFF;
				cv.numoff = (cindex + atoi(&sym->var->at.direct_io[3])) * 7;
			}
			else {
				cv.flags |= CO_CEX_REG2;
				cv.r2 = idx1 -> val.cexpr.r1;
			}
		}
		_co_combine_cv_with_base_addr(&cv,sym);
		co_expr_push_cval(&cv,_co_make_format(sym));
	}
	else {
		shvar = 0;
		_co_expr_push(base);
		_co_expr_eval_core();
	}
	base = _co_expr_pop();
	pcv = &base -> val.cexpr;
	switch (base -> format) {
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
		pcv -> scale = 1;
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
		pcv -> flags |= CO_CEX_SCALE;
		pcv -> scale = 2;
		break;
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_S:
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_4 | CO_CEX_F_FMT_F:
		pcv -> flags |= CO_CEX_SCALE;
		pcv -> scale = 4;
		break;
	case CO_CEX_F_DIM_8 | CO_CEX_F_FMT_F:
	case CO_CEX_F_DIM_8 | CO_CEX_F_FMT_S:
	case CO_CEX_F_DIM_8 | CO_CEX_F_FMT_U:
		pcv -> flags |= CO_CEX_SCALE;
		pcv -> scale = 8;
		break;
	default: break;
	}
	if (shvar) {
		pcv -> flags &= ~CO_CEX_SCALE;
	}
	else if (cindexf) {
		if (pcv -> flags & CO_CEX_SCALE)
			cindex *= pcv -> scale;
		if (pcv -> flags & CO_CEX_NUMOFF)
			pcv -> numoff += cindex;
		else
			pcv -> numoff = cindex;
		pcv -> flags &= ~CO_CEX_SCALE;
		pcv -> flags |= CO_CEX_NUMOFF;
	}
	else {
		if (pcv -> flags & CO_CEX_REG1) {
			pcv -> flags |= CO_CEX_REG2;
			pcv -> r2 = r;
		}
		else {
			pcv -> flags |= CO_CEX_REG1;
			pcv -> r1 = r;
		}
	}
	co_expr_push_cval(pcv,base -> format);
}

static void _co_compile_add(void)
{
}

static void _co_compile_mul(void)
{
}

static void _co_expr_eval_core(void)
{
#ifdef FFSS
int i;
int r;
#endif /* FFSS */
co_arg_t *p;

	p = _co_expr_pop();

#ifdef DEBUG
	if (main_status.debug & 0x40) {
		printf("  > ");
		_co_dump_expr(p);
		printf("\n");
	}
#endif
	switch (p -> type) {
	case CO_ARG_ACC:
	case CO_ARG_STRING:
	case CO_ARG_NUM:
		_co_expr_push(p);
		break;
	case CO_ARG_SYM:
		_co_compile_sym(p -> val.sym);
		break;
	case CO_ARG_CVAL:
		_co_compile_cval(p);
		break;
	case CO_ARG_EXPR:
		switch (p -> val.expr.code) {
		case CO_EOP_EXTRACT:
			_co_compile_extract(p);
			break;
		case CO_EOP_INDEX:
			_co_compile_index(p);
			break;
		case CO_EOP_EVAL:
			_co_expr_push(p -> val.expr.args[0]);
			_co_expr_eval_core();
			_co_expr_eval_core();
			break;
		case CO_EOP_ADD:
			_co_compile_add();
			break;
		case CO_EOP_MUL:
			_co_compile_mul();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void co_expr_eval(void)
{
#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif

	_co_expr_eval_core();
#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif
}

/*
* Istruzione LD.
*/

static void _co_expr_eval_ld(void)
{
co_arg_t *p;
em_opr_t op1,op2;
int r;

	p = _co_expr_pop();
	_co_expr_push(p);

/* Controllo sulle proprieta` dell'argomento.
  Serve ad evitare di utilizzare un nome di blocco funzionale
 come solo argomento. */

	if (p -> format & CO_CEX_F_SPC_F) {
		err_error(LA_E_TYPEMIS, NULL);
	}

	_co_expr_eval_core();

	p = _co_expr_pop();
	_co_expr_push(p);

	switch (p -> type) {
	case CO_ARG_ACC:
		r = _co_reg_copy(co_curr_acc, R_ANY);
		_co_reg_transparent_pop(co_curr_acc);
		p -> type = CO_ARG_CVAL;
		p -> val.cexpr.flags = CO_CEX_REG1;
		p -> val.cexpr.r1 = r;
		break;
	case CO_ARG_SYM:
	case CO_ARG_STRING:
	case CO_ARG_NUM:
/* Dato immediato in un registro. */
		co_curr_acc = _co_reg_request(R_ANY);
		op2.type = EM_OP_REG;
		op2.r1 = co_curr_acc;
		op1.type = EM_OP_IMM;
		if (p -> type == CO_ARG_SYM) {
			op1.val.vtype = EM_VTYPE_STR;
			op1.val.v.s = p -> val.sym -> name;
		}
		else if (p -> type == CO_ARG_STRING) {
			op1.val.vtype = EM_VTYPE_STRLBL;
			op1.val.v.n = (long)(p -> val.sym -> type);
		}
		else {
			op1.val.vtype = EM_VTYPE_NUM;
			if (p -> val.num.type == TY_INT) {
				op1.val.v.n = p -> val.num.data.l;
			}
			else {
#ifdef FFSS
			float r;

				r = (float)(p -> val.num.data.d);
				op1.val.v.n = *(long *)(&r);
#else
			float fR;

				fR = (float)(p -> val.num.data.d);
				op1.val.v.n = *(long *)(&fR);
#endif
			}
		}
/* Il caricamento avviene a 32 bit (per ora). */
		em_op(AOP_MOVL,&op1,&op2);
/* Il valore diventa CVAL. */
		p -> type = CO_ARG_CVAL;
		p -> val.cexpr.flags = CO_CEX_REG1;
		p -> val.cexpr.r1 = co_curr_acc;
		break;
	case CO_ARG_CVAL:
/* Dato disponibile in un registro. A questo punto dell'elaborazione,
 l'unico caso possibile di valore "cexpr" e` rappresentato da un singolo
 registro. Non dovrebbero presentarsi altri casi. */
		co_curr_acc = p -> val.cexpr.r1;
		break;
	case CO_ARG_EXPR:
/* Non dovrebbe accadere... */
		break;
	default:
		break;
	}
}

void co_expr_eval_ld(void)
{
/* L'accunulatore corrente e` reso di nuovo disponibile. */

	_co_reg_free(co_curr_acc);

#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif

	_co_expr_eval_core();

#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif

	_co_expr_eval_ld();

#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif

	co_expr_sp = 0;
	co_expr_argp = 0;
}

/*
* Scrittura su un indirizzo complesso.
*/

static void _co_store_cval(co_arg_t *p)
{
cval_t *cv;
int r;

	cv = &p -> val.cexpr;

	switch (p -> format) {
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
		r = _co_reg_store(co_curr_acc,cv,1);
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
		r = _co_reg_store(co_curr_acc,cv,2);
		break;
	default:
		r = _co_reg_store(co_curr_acc,cv,4);
		break;
	}
	co_curr_acc = r;
}

/*
* Scrittura di una costante ad un indirizzo complesso.
*/

static void _co_store_const_cval(long val, co_arg_t *p)
{
cval_t *cv;

	cv = &p -> val.cexpr;

	switch (p -> format) {
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_1 | CO_CEX_F_FMT_S:
		_co_const_store(val,cv,1);
		break;
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_U:
	case CO_CEX_F_DIM_2 | CO_CEX_F_FMT_S:
		_co_const_store(val,cv,2);
		break;
	default:
		_co_const_store(val,cv,4);
		break;
	}
}

/* Controlla se e` definito il valore dell'accumulatore. */

static void _co_check_acc(void)
{
	if (co_curr_acc == R_NULL) {
		err_warning(LA_W_UNACC,NULL);
		co_curr_acc = _co_reg_request(R_ANY);
		_co_op_2reg(AOP_XORL,co_curr_acc,co_curr_acc);
	}
}

void co_expr_store(void)
{
co_arg_t *p;

	_co_check_acc();

	p = _co_expr_pop();

/* Controllo sulle proprieta` dell'argomento.
  Serve ad evitare di utilizzare un nome di blocco funzionale
 come solo argomento. */

	if (p -> format & CO_CEX_F_SPC_F) {
		err_error(LA_E_TYPEMIS, NULL);
	}

#ifdef DEBUG
	if (main_status.debug & 0x40) {
		printf("  > ");
		_co_dump_expr(p);
		printf("\n");
	}
#endif
	if (p -> type == CO_ARG_CVAL) {
		_co_store_cval(p);
	}
	co_expr_sp = 0;
	co_expr_argp = 0;
}

/*
* Caricamento di una costante ad un indirizzo complesso.
*/

static void _co_expr_store_const(long val)
{
co_arg_t *p;

	p = _co_expr_pop();

/* Controllo sulle proprieta` dell'argomento.
  Serve ad evitare di utilizzare un nome di blocco funzionale
 come solo argomento. */

	if (p -> format & CO_CEX_F_SPC_F) {
		err_error(LA_E_TYPEMIS, NULL);
	}

#ifdef DEBUG
	if (main_status.debug & 0x40) {
		printf("  > ");
		_co_dump_expr(p);
		printf("\n");
	}
#endif
	switch (p -> type) {
	case CO_ARG_CVAL:
		_co_store_const_cval(val,p);
		break;
	default:
		break;
	}
	co_expr_sp = 0;
	co_expr_argp = 0;
}

/* SET e RESET */

void _co_expr_sr(int sr)
{
int lbl;
int rcache;

	_co_check_acc();

	rcache = _co_get_cache();
	lbl = _co_get_tmplabel();
	if (main_status.optsr) {
	/* Compilazione ottimizzata: calcola l'indirizzo solo
	 se necessario. Non puo` generare informazioni di debug,
	 perche` l'indirizzo puo` non essere disponibile. */
		_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
		_co_op_1tlbl(AOP_JE,lbl);
		co_expr_eval();
	}
	else {
	/* Compilazione di debug: calcola sempre l'indirizzo, in modo
	 che il debugger sia in condizione di trovarlo. */
		co_expr_eval();
		if (main_status.gil) {
		co_arg_t *p;
			p = _co_expr_pop();
			co_last_locallabel = lbl;
			_co_emit_v_debinfo(p);
			_co_expr_push(p);
		}
		_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
		_co_op_1tlbl(AOP_JE,lbl);
	}
	_co_expr_store_const(sr);
	_co_def_tmplabel(lbl);
	_co_set_cache(rcache & _co_get_cache());
}
void co_expr_set(void)
{
	_co_expr_sr(1);
}
void co_expr_reset(void)
{
	_co_expr_sr(0);
}

/*
* Negazione (suffisso "N").
*/

static void _co_not(int reg,int bo)
{
int r;

	if (bo) {
	/* Booleana. */
		_co_op_2reg(AOP_TESTL,reg,reg);
		if (reg <= R_3) {
			_co_op_1reg(AOP_SETE,em_lowb(reg));
		}
		else {
			r = _co_reg_request(R_ANY_GENERIC);
			_co_reg_load(reg,R_ANY_GENERIC);
			_co_op_1reg(AOP_SETE,em_lowb(r));
			_co_reg_load(r,reg);
		}
	}
	else {
	/* A bit. */
		_co_op_1reg(AOP_NOTL,reg);
	}
}
void co_not(int bo)
{
	_co_check_acc();

	_co_not(co_curr_acc,bo);
}

/*
* Salvataggio con negazione.
*/

void co_expr_nstore(int bo)
{
int r,acc;

	_co_check_acc();

	r = _co_reg_copy(co_curr_acc,R_ANY);
	_co_not(r,bo);
	acc = co_curr_acc;
	co_curr_acc = r;
	co_expr_store();
	_co_reg_free(co_curr_acc);
	co_curr_acc = acc;
}

/*
* Operazioni bit a bit. (non solo. Anche confronti, nel qual caso
* "neg" non indica negazione, ma confronto senza segno).
*/

void co_expr_bitop(enum _cop_t cop, int neg, int bo)
{
co_arg_t *p;
int reg,acc,aop,cmp, uns, opt;
em_opr_t op1,op2;

	_co_check_acc();

	_co_expr_eval_core();

#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif
	p = _co_expr_pop();

/* Controllo sulle proprieta` dell'argomento.
  Serve ad evitare di utilizzare un nome di blocco funzionale
 come solo argomento. */

	if (p -> format & CO_CEX_F_SPC_F) {
		err_error(LA_E_TYPEMIS, NULL);
	}

	opt = uns = cmp = 0;

	switch (cop) {
	case CO_O_XOR: aop = AOP_XORB; break;
	case CO_O_AND: aop = AOP_ANDB; break;
	case CO_O_OR: aop = AOP_ORB; break;
	case CO_O_ADD: aop = AOP_ADDB; opt = 1; break;
	case CO_O_SUB: aop = AOP_SUBB; opt = 1; break;
	case CO_O_GT:
	case CO_O_GE:
	case CO_O_LT:
	case CO_O_LE:
	case CO_O_EQ:
	case CO_O_NE: aop = AOP_CMPB; cmp = 1; uns = neg; neg = 0; break;
	default: aop = 0; break;
	}

	if ((p -> type == CO_ARG_SYM && !neg) || p -> type == CO_ARG_NUM) {
/* Dato immediato. */
		op2.type = EM_OP_REG;
		op2.r1 = co_curr_acc;
		op1.type = EM_OP_IMM;
		if (p -> type == CO_ARG_SYM) {
			op1.val.vtype = EM_VTYPE_STR;
			op1.val.v.s = p -> val.sym -> name;
		}
		else {
			op1.val.vtype = EM_VTYPE_NUM;
			if (p -> val.num.type == TY_INT) {
				op1.val.v.n = p -> val.num.data.l;
			}
			else {
			float r;

				r = (float)(p -> val.num.data.d);
				op1.val.v.n = *(long *)(&r);
			}
			if (neg)
				op1.val.v.n = ~op1.val.v.n;
		}
/* Ottimizzazioni particolari. */
		if (opt) {
			switch (aop) {
			case AOP_ADDB:
				_co_op_add_imm_reg((long)op1.val.v.n,op2.r1);
				break;
			case AOP_SUBB:
				_co_op_sub_imm_reg((long)op1.val.v.n,op2.r1);
				break;
			}
		}
		else {
		/* Il caricamento avviene a 32 bit (per ora). */
			em_op(aop + 2,&op1,&op2);
		}
	}
	else if (! neg && p -> type != CO_ARG_ACC) {

		_co_compile_and_apply_cval(aop,p);
		_co_emit_v_debinfo(p);

	}
	else {
		_co_expr_push(p);
		acc = co_curr_acc;
		_co_expr_eval_ld();
		co_curr_acc = acc;
	#ifdef DEBUG
		if (main_status.debug & 0x20) {
			printf("--> ");
			_co_dump_expr(co_stack[co_expr_sp - 1]);
			printf("\n");
		}
	#endif

		p = _co_expr_pop();

		_co_emit_v_debinfo(&co_last_complex_address);

		reg = p -> val.cexpr.r1;

		if (neg)
			_co_not(reg,bo);

		_co_op_2reg(aop + 2,reg,co_curr_acc);

	/* Libera il registro. */

		_co_reg_free(reg);
	}

	if (cmp) {
		switch (cop) {
		case CO_O_GT: aop = uns ? AOP_SETA : AOP_SETG; break;
		case CO_O_GE: aop = uns ? AOP_SETAE : AOP_SETGE; break;
		case CO_O_LT: aop = uns ? AOP_SETB : AOP_SETL; break;
		case CO_O_LE: aop = uns ? AOP_SETBE : AOP_SETLE; break;
		case CO_O_EQ: aop = AOP_SETE; break;
		case CO_O_NE: aop = AOP_SETNE; break;
		default: aop = 0; break;
		}
		_co_op_1reg(aop,em_lowb(co_curr_acc));
		_co_op_2reg(AOP_MOVZBL,em_lowb(co_curr_acc),co_curr_acc);
	}

	co_expr_sp = 0;
	co_expr_argp = 0;
}

/*
* Moltiplicazioni, divisioni, ...
*/

void co_expr_arop(enum _cop_t cop, int uns)
{
co_arg_t *p;
#ifdef FFSS
em_opr_t op1,op2;
int reg,acc,aop,cmp, tacc, trem, isconst;
#else
int reg,acc,aop,isconst;
#endif /* FFSS */

	_co_check_acc();

	_co_expr_eval_core();

#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif
	p = _co_expr_pop();

	isconst = (p -> type == CO_ARG_SYM || p -> type == CO_ARG_NUM);

	_co_expr_push(p);
	acc = co_curr_acc;
	_co_expr_eval_ld();
	co_curr_acc = acc;
#ifdef DEBUG
	if (main_status.debug & 0x20) {
		printf("--> ");
		_co_dump_expr(co_stack[co_expr_sp - 1]);
		printf("\n");
	}
#endif

	p = _co_expr_pop();

	if (! isconst)
		_co_emit_v_debinfo(&co_last_complex_address);

	reg = p -> val.cexpr.r1;

	if (cop == CO_O_MUL) {
		_co_op_2reg(AOP_IMULL,reg,co_curr_acc);
	}
	else {

	/* "reg" non dovrebbe mai essere "%eax". O no ? */

	/* Occupa %eax e %edx */

		if (co_curr_acc != R_0) {
			_co_reg_load(co_curr_acc,R_0);
		}

	/* Povero me ! non funziona il trucco "divl %edx,%edx:%eax" ! */
		if (reg == R_3) {
			reg = _co_reg_load(reg,R_ANY);
		}

		if (uns) {
		/* Divisione senza segno: si azzerano i 32 bit alti. */
			_co_op_2reg(AOP_XORL,R_3,R_3);
			aop = AOP_EDIVL;
		}
		else {
		/* Divisione con segno: si estende l'operando a 64 bit. */
			em_op(AOP_CLTD);
			aop = AOP_EIDIVL;
		}
		_co_op_2reg(aop,reg,co_curr_acc);

		if (cop == CO_O_MOD) {
			_co_op_2reg(AOP_MOVL,R_3,R_0);
		}
		_co_reg_free(R_3);
		if (co_curr_acc != R_0) {
			_co_reg_load(R_0,co_curr_acc);
		}

	}


/* Libera il registro. */

	_co_reg_free(reg);
	co_expr_sp = 0;
	co_expr_argp = 0;
}

/*
* Espressione IL
*/

void co_il_expr_begin(void)
{
	_co_reg_transparent_push(co_curr_acc);
}

void co_il_expr_end(void)
{
co_arg_t *p;

	p = _co_expr_new();
	p -> type = CO_ARG_ACC;
	_co_expr_push(p);
}

/****************************\
* SALTI, RETURN, CHIAMATE... *
\****************************/

/*
* Salto incondizionato.
*/

void co_jmp(symbol_t *sym)
{
	_co_fixdebug_jmp();
	_co_op_label(AOP_JMP,sym);
	_co_start_branch();
}

/*
* Salto se zero.
*/

void co_jmpnc(symbol_t *sym)
{
	_co_fixdebug_jmp();
	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	_co_op_label(AOP_JE,sym);
	_co_start_branch();
}

/*
* Salto se non zero.
*/

void co_jmpc(symbol_t *sym)
{
	_co_fixdebug_jmp();
	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	_co_op_label(AOP_JNE,sym);
	_co_start_branch();
}

/*
* Return incondizionato.
*/

void co_ret(void)
{
	_co_fixdebug_jmp();
	_co_op_1tlbl(AOP_JMP,co_end_label_index);
	_co_start_branch();
}

/*
* Return se zero.
*/

void co_retnc(void)
{
	_co_fixdebug_jmp();
	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	_co_op_1tlbl(AOP_JE,co_end_label_index);
	_co_start_branch();
}

/*
* Return se non zero.
*/

void co_retc(void)
{
	_co_fixdebug_jmp();
	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	_co_op_1tlbl(AOP_JNE,co_end_label_index);
	_co_start_branch();
}

/*
* Nucleo della chiamata di blocco funzionale.
*/

/* Passa l'indirizzo calcolato alla funzione. Puo` usare un registro,
 l'indice del quale e` dato come valore di ritorno. Se non sono utilizzati
 registri, vale R_NULL. */
static int _co_pass_addr(void)
{
co_arg_t *p;
cval_t *pcv;
em_opr_t op;
int r;

	p = _co_expr_pop();
	p -> format = CO_CEX_F_DIM_4 | CO_CEX_F_FMT_U;
	pcv = &p -> val.cexpr;
	if (pcv -> flags & (CO_CEX_REG1 | CO_CEX_REG2 | CO_CEX_SCALE)) {
		r = _co_reg_apply(AOP_LEAL, pcv, R_NULL);
		_co_op_1reg(AOP_PUSHL,r);
	}
	else {
		op.type = EM_OP_IMM;
		if (pcv -> flags & CO_CEX_NUMOFF) {
			op.val.vtype = EM_VTYPE_STROFF;
			op.val.v.n = pcv -> numoff;
		}
		else {
			op.val.vtype = EM_VTYPE_STR;
		}
		op.val.v.s = pcv -> symoff;
		em_op(AOP_PUSHL,&op);
		r = R_NULL;
	}
	return r;
}

static void _co_call(symbol_t *sym)
{
int r;

	if (! sym)
		return;

/* Valuta il simbolo e ne carica l'indirizzo sullo stack. */

	co_expr_push_sym(sym);
	_co_expr_eval_core();
	r = _co_pass_addr();

/* Chiama la funzione. */

	_co_op_1name(AOP_CALL,sym -> type -> d.fb.sym -> name);
	_co_reset_cache();

/* Rimette a posto lo stack. */

	_co_op_imm_reg(AOP_ADDL,4L,R_SP);

/* Libera solo l'eventuale registro occupato, perche` cio` potrebbe comportare
 l'emissione di "popl". */

	_co_reg_free(r);

	co_expr_sp = 0;
	co_expr_argp = 0;
}

/*
* Chiamata incondizionata.
*/

void co_cal(symbol_t *sym)
{
	_co_call(sym);
}

/*
* Chiamata se zero.
*/

void co_calcn(symbol_t *sym)
{
int lbl;

	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	lbl = _co_get_tmplabel();
	_co_op_1tlbl(AOP_JNE,lbl);
	_co_call(sym);
	_co_def_tmplabel(lbl);
}

/*
* Chiamata se non zero.
*/

void co_calc(symbol_t *sym)
{
int lbl;

	_co_op_2reg(AOP_TESTL,co_curr_acc,co_curr_acc);
	lbl = _co_get_tmplabel();
	_co_op_1tlbl(AOP_JE,lbl);
	_co_call(sym);
	_co_def_tmplabel(lbl);
}

/*
* Chiamata di funzione.
*/

static paramlist_t *co_fn_paramlist;
static symbol_t *co_curr_fn;

static void _co_fn_storearg(void)
{
cval_t cv;
symbol_t *param;

	param = co_fn_paramlist -> sym;
	cv.flags = CO_CEX_SYMOFF | CO_CEX_NUMOFF;
	cv.symoff = co_curr_fn -> type -> d.fb.statics -> name;
	cv.numoff = param -> var -> location;
	co_expr_push_cval(&cv,_co_make_format(param));
	co_expr_store();
	co_fn_paramlist = co_fn_paramlist -> next;
}

void co_calfn(symbol_t *sym)
{
	co_curr_fn = sym;

	if (! sym)
		return;

	co_assign_offset(sym -> type);

	co_fn_paramlist = sym -> type -> d.fb.input;
	if (co_fn_paramlist) {
		_co_fn_storearg();
	}
}

void co_fnarg(void)
{
	if (co_fn_paramlist) {
		co_expr_eval_ld();
		_co_fn_storearg();
	}
	else {
	/* Troppi parametri. */
		err_error(LA_E_TOOPARAM,NULL);
	}
}

void co_do_fncall(void)
{
	if (co_curr_fn) {

		_co_op_1name(AOP_CALL,co_curr_fn -> name);
		_co_reset_cache();

		while (co_fn_paramlist) {

		/* Troppo pochi parametri. Sara` interessante sapere
		 quali parametri siano rimasti fuori. */

			err_warning(LA_W_UNASGN, co_fn_paramlist->sym->name);

			co_fn_paramlist = co_fn_paramlist -> next;
		}
		co_curr_fn = (symbol_t *) 0;

/* Libera e riassegna l'accumulatore, che conterra` il risultato della
 funzione. Si tratta di una semplificazione,
 basata sull'ipotesi che, al momento della chiamata, non ci siano
 registri generali occupati. Se ce ne fossero, andrebbero salvati... */

		_co_reg_free(co_curr_acc);
		co_curr_acc = _co_reg_request(R_ANY);
	}
}

/*
* Funzione utile per debug (e per generare informazioni di debug !).
*/

void co_il_newline(void)
{
	_co_op_1name(AOP_COMMENT,rdf.line);
	_co_lineinfo();
}

/*
* Funzione chiamata per riservare il descrittore e l'etichetta di
* una nuova stringa.
*/

void co_new_string(symbol_t *sym)
{
	if (! sym -> type) {
		sym -> type = (type_t *) _co_get_stringlabel();
	}
}

void co_add_string(symbol_t *sym)
{
	co_new_string(sym);
	co_expr_push_string(sym);
}

void co_begin_instruction(void)
{
/* All'inizio della seconda istruzione di ogni ramo
 si aggiungono cinque NOP per evitare che il debugger generi liste di break
 troppo corte. Il risutato e` un allungamento artificiale dell'istruzione,
 che assicura una distanza di almeno cinque byte tra inizio e fine.
 C'e` un caso particolare che qui non e` ricoperto: si tratta delle
 istruzioni JMP* e RET*, che necessitano di una gestione particolare
 (dieci byte di buco all'interno dell'istruzione - vedere le funzioni
 corrispondenti). Si veda inoltre il commento alla funzione seguente
 per una trattazione dettagliata. */
	if (_co_branch_count() == 1) {
		_co_add_debug_filler();
	}
/* Incrementa il contatore di linee nel ramo di codice corrente. */
	_co_inc_branch();
}

void co_end_instruction(void)
{
/* Alla fine della prima istruzione di ogni ramo
 si aggiungono dieci NOP per evitare che il debugger generi liste di break
 troppo corte. Questo spazio si somma a quello generato all'inizio
 della seconda istruzione del ramo, o alla fine del ramo (per JMP, etichetta,
 RET o fine funzione). Cinque dei dieci NOP sono emessi piu` avanti,
 all'inizio dell'istruzione successiva, in modo che l'istruzione
 risulti allungata di soli cinque byte. Si hanno infatti i seguenti casi
 potenzialmente pericolosi:

 1) Istruzione LD o ST all'inizio del ramo. In questo caso, il debugger
 produce un solo punto di break alla fine dell'istruzione. L'eventuale
 istruzione successiva apparira` distanziata di altri cinque byte: il
 debugger infatti utilizza le etichette di fine istruzione, oppure quelle
 contenenti le informazioni per il reperimento del parametro, oppure,
 ancora, le etichette speciali di salto. Tutte queste etichette sono
 associate ad indirizzi successivi al secondo blocco di cinque NOP.
 2) Operatori ordinari, funzioni o blocchi funzionali. In questo caso
 il debugger produce due punti di break: uno per estrarre il valore
 dell'operando (etichette $VA...), e uno per ottenere il valore
 finale dell'accumulatore. I cinque byte aggiunti qui assicurano
 la distanza di sicurezza tra i due punti.
 3) Istruzioni JMP o RET. Questo caso non e` risolvibile qui, a causa
 della particolarita` di queste istruzioni (alterazione del flusso di
 esecuzione). Il debugger produce un solo punto di break per acquisire
 il valore dell'accumulatore. Il punto generato e` necessariamente l'ultimo
 (o l'unico) della lista. Per risolvere il caso peggiore (lista di un solo
 elemento), vengono aggiunti dieci NOP nel corpo del codice dell'istruzione.
 Si veda il codice di compilazione dei salti e dei RET per un esame del
 codice.
 4) Etichetta. Come i salti e i RET, questa identifica la fine di una
 lista e l'inizio di un'altra. Sono prodotti dieci byte prima del punto
 di salto, per poter teminare in sicurezza la lista precedente.
*/
	if (_co_branch_count() == 1) {
		_co_add_debug_filler();
	}
}

