/*
* @(#) pass1.c 2.9 Mon Nov 20 20:58:12 MET 2000
* @(#) Rif. Guerrini
*
* --/--/-- 1.0 Prima stesura, sperimentale.
* 02/07/96 2.0 Eliminati i parametri di tutte le azioni.
* 15/07/96 2.1 Cambiato nome al file. Ridotto il codice. spostate
*              altrove le funzioni di debug (file "debug.c").
* 14/10/96 GG 2.2 Aggiunto il riconoscimento delle costanti stringa.
* 16/10/96 GG 2.3 Aggiunte callback di fine istruzione IL.
* 26/11/96 GG 2.4 Iniziati i lavori per le variabili "retain". Sono
*             segnali di scambio speciali.
* 29/11/96 GG 2.5 Modifiche per la lettura di costanti stringa.
* 22/01/97 GG 2.6 Migliorata la diagnostica nel caso di RETAIN+AT.
* 22/01/97 GG 2.7 Cambiata la sintassi delle dichiarazioni RETAIN. La
*             parola chiave RETAIN deve seguire immediatamente VAR,
*             VAR_GLOBAL e VAR_OUTPUT, e si intende applicata a tutte
*             le variabili dichiarate fino ad END_VAR.
* 22/09/97 GG 2.8 Un piccolo aggiustamento per compilazione su NT.
* 20/11/00 GG 2.9 Aggiunta la callback di fine istruzione IL (non usata qui).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
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
#include "cexpr.h"
#include "debug.h"
#include "errs.h"
#include "rdfile.h"
#include "compiler.h"

/*
* Contesti di lavoro.
*/

/* Contesto globale. Contiene tutti i simboli globali definiti nei
 sorgenti esaminati. */

context_t *global_context = (context_t *) 0;

/* Contesto globale ristretto al file corrente. Contiene tutti i simboli
 globali definiti nel sorgenti corrente. Serve a generare l'header
 corrispondente al sorgente. */

context_t *curr_file_context = (context_t *) 0;

/* Contesto locale. Contiene tutti i simboli locali
 definiti nel sorgenti corrente. Corrisponde allo scope delle variabili
 "static" del C. */

context_t *local_context = (context_t *) 0;

/* Contesto "funzione". Contiene tutti i simboli locali
 definiti all'interno di un blocco procedurale. Corrisponde allo scope
 di funzione del C. */

context_t *fb_context = (context_t *) 0;

/* Contesto "stringhe". E` utilizzato piu` che altro come serbatoio. */

context_t *string_context = (context_t *) 0;

/* FUNZIONI */

/*
* ------------------------------------------------------------------
* FUNZIONI ASSOCIATE AI COSTRUTTI SINTATTICI RICONOSCIUTI DAL PARSER
* ------------------------------------------------------------------
*/

/*******************************\
* ----------------------------- *
* AZIONI SPECIFICHE DEL PASSO 1 *
* ----------------------------- *
\*******************************/


/*
* Possibili ambienti in cui possono incontrarsi definizioni di variabili.
*/

enum {
SCOPE_NULL,
SCOPE_GLOBAL, SCOPE_EXTERN, SCOPE_LOCAL,
SCOPE_FUNCTION, SCOPE_FUNCTION_BLOCK, SCOPE_PROGRAM,
SCOPE_FN_VAR, SCOPE_FB_VAR, SCOPE_PROG_VAR,
SCOPE_FN_VARIN, SCOPE_FB_VARIN, SCOPE_PROG_VARIN,
SCOPE_FN_VAROUT, SCOPE_FB_VAROUT, SCOPE_PROG_VAROUT,
SCOPE_STRUCT,
};

static int p1_scope = SCOPE_NULL;
static int p1_retain = 0;
static symbol_t PTR_ p1_curr_sym = (symbol_t *) 0;
static symbol_t PTR_ p1_curr_fb = (symbol_t *) 0;
static type_t PTR_ p1_curr_type = (type_t *) 0;
static type_t PTR_ p1_curr_array_type = (type_t *) 0;
#ifdef FFSS
static symbol_t PTR_ p1_curr_conf_sym = (symbol_t *) 0;
#endif /* FFSS */
static inival_t PTR_ p1_array_inival = (inival_t *) 0;
static inival_t PTR_ p1_array_inival_tail = (inival_t *) 0;
static int p1_array_inival_count = 0;
static int p1_param_in = 0;
static int p1_param_out = 0;
static paramlist_t PTR_ p1_param_in_list = (paramlist_t *) 0;
static paramlist_t PTR_ p1_param_out_list = (paramlist_t *) 0;
static paramlist_t PTR_ p1_param_in_list_tail = (paramlist_t *) 0;
static paramlist_t PTR_ p1_param_out_list_tail = (paramlist_t *) 0;
static int p1_curr_n_index = 0;
static int p1_il_expr_level = 0;
static int p1_il_expr_lv_stack[MAX_IL_EXPR_LEVEL + 1];


static void p1_store_sym_in_file_context(void);


static void p1_CONFIGURATION(void)
{
char *name;
symbol_position_t *pos;
symbol_t *node,*second_node;

	name = util_strdup(rdf.identifier);

	pos = st_register_name(name,global_context);
	second_node = pos -> node;
	pos = st_register_name(name,local_context);
	node = pos -> node;
	if (node) {
		err_error(LA_E_REDEF,name);
	}
	if (second_node) {
		err_warning(LA_W_HIDES,name);
	}
	p1_curr_sym = st_add_registered_name_as_type();
	p1_curr_sym -> type -> code = TYPE_CONFIGURATION;
	p1_curr_sym -> type -> d.conf.file = rdf.file_name;
	p1_curr_sym -> type -> d.conf.line = pScCtl -> n_line;
	p1_store_sym_in_file_context();
}
static void p1_END_CONFIGURATION(void)
{
}
static void p1_RESOURCE(void)
{
}
static void p1_RESOURCE_ON(void)
{
}
static void p1_END_RESOURCE(void)
{
}

static void p1_compare_types(type_t * t1, type_t * t2)
{
int i, diff;

	if (! t2)
		return;

	diff = 0;

	while (t1 && !diff) {
		if (t1 -> code != t2 -> code) {
			diff = 1;
			break;
		}
		switch (t1 -> code) {
		case TYPE_STRING:
			diff = t1 -> d.str_len != t2 -> d.str_len;
			t1 = (type_t *) 0;
			break;
		case TYPE_USER:
		case TYPE_STRUCT:
		case TYPE_PROGRAM:
		case TYPE_FUNCTION:
		case TYPE_FUNCTION_BLOCK:
		case TYPE_CONFIGURATION:
		case TYPE_RESOURCE:
		case TYPE_UNDEF:
			diff = (t1 != t2);
			t1 = (type_t *) 0;
			break;
		case TYPE_ARRAY:
			if (t1 -> d.a.n_dim != t2 -> d.a.n_dim) {
				diff = 1;
				break;
			}
			for (i = 0; i < t1 -> d.a.n_dim; ++i) {
				if ((t1 -> d.a.idx_base[i]
				    != t2 -> d.a.idx_base[i])
				 || (t1 -> d.a.dim[i]
				    != t2 -> d.a.dim[i])) {
					diff = 1;
				}
			}
			t1 = t1 -> d.a.type;
			t2 = t2 -> d.a.type;
			break;
		default:
			t1 = (type_t *) 0;
			break;
		}
	}
	if (diff) {
		err_error(LA_E_DIFFTYPE,p1_curr_sym -> name);
	}
}

static void p1_make_predef_type(int type_code)
{
	p1_curr_type = st_create_type(type_code);
}
static void p1_BOOL(void){p1_make_predef_type(TYPE_BOOL);}
static void p1_SINT(void){p1_make_predef_type(TYPE_SINT);}
static void p1_INT(void){p1_make_predef_type(TYPE_INT);}
static void p1_DINT(void){p1_make_predef_type(TYPE_DINT);}
static void p1_LINT(void){p1_make_predef_type(TYPE_LINT);}
static void p1_USINT(void){p1_make_predef_type(TYPE_USINT);}
static void p1_UINT(void){p1_make_predef_type(TYPE_UINT);}
static void p1_UDINT(void){p1_make_predef_type(TYPE_UDINT);}
static void p1_ULINT(void){p1_make_predef_type(TYPE_ULINT);}
static void p1_REAL(void){p1_make_predef_type(TYPE_REAL);}
static void p1_LREAL(void){p1_make_predef_type(TYPE_LREAL);}
static void p1_TIME(void){p1_make_predef_type(TYPE_TIME);}
static void p1_TIME_OF_DAY(void){p1_make_predef_type(TYPE_TOD);}
static void p1_DATE(void){p1_make_predef_type(TYPE_DATE);}
static void p1_DATE_AND_TIME(void){p1_make_predef_type(TYPE_DT);}
static void p1_STRING(void){p1_make_predef_type(TYPE_STRING);}
static void p1_BYTE(void){p1_make_predef_type(TYPE_BYTE);}
static void p1_WORD(void){p1_make_predef_type(TYPE_WORD);}
static void p1_DWORD(void){p1_make_predef_type(TYPE_DWORD);}
static void p1_LWORD(void){p1_make_predef_type(TYPE_LWORD);}

static void p1_START_DIM_ARRAY(void)
{
	p1_curr_type = st_create_type(TYPE_ARRAY);
	p1_curr_type -> d.a.type = (type_t *) 0;
	p1_curr_type -> d.a.n_dim = 0;
}
static void p1_MAKE_SUBRANGE(void)
{
VAL op_inf,op_sup;
long l;

	ce_pop(&op_sup);
	ce_pop(&op_inf);
	if (op_sup.type != TY_INT || op_inf.type != TY_INT) {
		err_error(LA_E_NONINT_DIM,NULL);
		op_sup.data.l = op_inf.data.l = 0;
	}
	if (op_inf.data.l > op_sup.data.l) {
		err_error(LA_E_BADDIM,NULL);
		op_sup.data.l = op_inf.data.l;
	}
	l = op_inf.data.l;
	op_inf.data.sr.low = l;
	op_inf.data.sr.high = op_sup.data.l;
	op_inf.type = TY_SUBRANGE;
	ce_push(&op_inf);
}

static void p1_ADD_DIM_ARRAY(void)
{
VAL op;
int n;

	ce_pop(&op);
	if (p1_curr_type -> d.a.n_dim >= MAX_ARRAY_DIM) {
		err_error(LA_E_TOODIM,NULL);
	}
	else {
		n = (p1_curr_type -> d.a.n_dim)++;
		p1_curr_type -> d.a.idx_base[n] = op.data.sr.low;
		p1_curr_type -> d.a.dim[n]
		 = op.data.sr.high - op.data.sr.low + 1;
	}
}
static void p1_END_DIM_ARRAY(void)
{
/* Scrive comunque qualcosa di sensato. */

	if (p1_curr_type -> d.a.n_dim == 0) {
		p1_curr_type -> d.a.n_dim = 1;
		p1_curr_type -> d.a.idx_base[0] = 0;
		p1_curr_type -> d.a.dim[0] = 1;
	}
	p1_curr_array_type = p1_curr_type;
}
static void p1_type_scalar(void)
{
	if (p1_curr_sym -> type) {
		if (p1_curr_sym -> type -> code == TYPE_FUNCTION) {
		/*
			p1_curr_sym -> var -> at.rv_type = p1_curr_type;
		*/
			p1_curr_sym -> type -> d.fb.rv_type = p1_curr_type;
			return;
		}
		else {
			p1_compare_types(p1_curr_sym -> type,p1_curr_type);
		}
	}
	p1_curr_sym -> type = p1_curr_type;
}
static void p1_TYPE_SCALAR(void)
{
	p1_type_scalar();
}
static void p1_type_fb(int code)
{
char *name;
symbol_position_t *pos;
symbol_t *sym;

	name = util_strdup(rdf.identifier);
	pos = st_register_name(name,(context_t *) 0);
	sym = pos -> node;
	if (! sym) {
		sym = st_add_registered_name_as_type();
		sym -> type -> code = TYPE_UNDEF;
		sym -> type -> d.undef_name = sym -> name;
	}
	else {
		if (sym -> class != SYMCL_TYPE
		    || (sym -> type 
		        && sym -> type -> code != TYPE_UNDEF
		        && sym -> type -> code != TYPE_FUNCTION
		        && sym -> type -> code != TYPE_PROGRAM
		        && sym -> type -> code != TYPE_FUNCTION_BLOCK)) {
			err_error(LA_E_NOTATYPE,name);
		}
	}
	if (p1_curr_sym -> type
	 && p1_curr_sym -> type -> code == TYPE_FUNCTION) {
		err_error(LA_E_NOSCALFN,NULL);
	}
	p1_curr_type = sym -> type;
	p1_type_scalar();
}
static void p1_TYPE_FB(void)
{
	p1_type_fb(TYPE_FUNCTION_BLOCK);
}
static void p1_TYPE_ARRAY(void)
{
	p1_curr_array_type -> d.a.type = p1_curr_type;
	p1_curr_array_type -> size = p1_curr_type -> size;
	p1_curr_array_type -> size *= st_get_array_size(p1_curr_array_type);
	p1_curr_type = p1_curr_array_type;
	if (p1_curr_sym -> type
	 && p1_curr_sym -> type -> code == TYPE_FUNCTION) {
		err_error(LA_E_NOSCALFN,NULL);
	}
	p1_type_scalar();
}

static void p1_chk_init_type(LPVAL op, type_t * type)
{
long l,lh,ll;
double d;
int nb;

	switch (op -> type) {
	case TY_INT:
		if (st_is_float_type(type)) {
			err_warning(LA_W_CONVTOREAL,NULL);
			l = op -> data.l;
			op -> data.d = (double) l;
			op -> type = TY_DOUBLE;
		}
		else {
			if (type -> code == TYPE_BOOL)
				nb = 1;
			else
				nb = type -> size * 8;
		/* Per il momento, al massimo, si lavora sui long. */
			if (nb < sizeof(long) * 8) {
				if (st_is_unsigned_type(type)) {
					lh = (1 << nb) - 1;
					ll = 0;
				}
				else {
					lh = (1 << (nb - 1)) - 1;
					ll = -(lh + 1);
				}
				if (op -> data.l < ll || op -> data.l > lh)
					err_warning(LA_W_TRUNCINT,NULL);
			}
		}
		break;
	case TY_DOUBLE:
		if (!st_is_float_type(type)) {
			err_error(LA_E_TYPEMIS,NULL);
			d = op -> data.d;
			op -> data.d = (long) d;
			op -> type = TY_INT;
		}
		break;
	case TY_STRING:
		if (type -> code != TYPE_STRING) {
			err_error(LA_E_TYPEMIS,NULL);
		/* Per ora, poi si vedra`. */
			d = op -> data.d;
			op -> data.d = (long) d;
			op -> type = TY_STRING;
		}
		break;
	default:
		err_error(LA_E_TYPEMIS,NULL);
		break;
	}
}

static void p1_add_array_initval(LPVAL op, int count)
{
inival_t *v;

	v = st_inival_alloc();

	if (! p1_array_inival)
		p1_array_inival = v;
	else
		p1_array_inival_tail -> next = v;
	p1_array_inival_tail = v;
	v -> next = (inival_t *) 0;

	if (st_is_array_type(p1_curr_sym -> type))
		p1_chk_init_type(op,p1_curr_sym -> type -> d.a.type);

	switch (op -> type) {
	case TY_INT:
		v -> data.l = op -> data.l;
		break;
	case TY_DOUBLE:
		v -> data.d = op -> data.d;
		break;
	case TY_STRING:
		v -> data.l = op -> data.l;
		break;
	default:
		break;
	}

	v -> count = count;
	p1_array_inival_count += count;
}

static void p1_chk_var_init(void)
{
int par,conn;

	if (p1_scope == SCOPE_EXTERN) {
		err_error(LA_E_ININOMEM,NULL);
	}
	par = p1_curr_sym -> var -> flags & VAR_F_PARAM_M;
	conn = p1_curr_sym -> var -> flags & VAR_F_CONNECTION_M;
/*
	if (par != VAR_F_PARAM_NONE) {
		err_error(LA_E_ININOMEM,NULL);
	}
	if (conn != VAR_F_CONNECTION_NONE) {
		err_error(LA_E_INIDIRV,NULL);
	}
*/
	if (par == VAR_F_CONNECTION_IN || par == VAR_F_CONNECTION_MEMORY) {
		err_error(LA_E_INIDIRV2,NULL);
	}
}

static void p1_START_ARRAY_INIT(void)
{
	if (!st_is_array_type(p1_curr_sym -> type)) {
		err_error(LA_E_NOTARRAY,NULL);
	}
	else if (!st_is_scalar_type(p1_curr_sym -> type -> d.a.type)) {
		err_error(LA_E_ININONUM,NULL);
	}
	p1_array_inival = (inival_t *) 0;
	p1_array_inival_tail = (inival_t *) 0;
	p1_array_inival_count = 0;
}
static void p1_END_ARRAY_INIT(void)
{
int n_el;

	if (!st_is_array_type(p1_curr_sym -> type))
		return;
	n_el = st_get_array_size(p1_curr_sym -> type);
	if (n_el < p1_array_inival_count) {
		err_error(LA_E_TOOMINIT,NULL);
	}
}
static void p1_ARRAY_INIT_MULTI(void)
{
VAL op;
VAL counter;

	ce_pop(&op);
	ce_pop(&counter);

	if (counter.type != TY_INT || counter.data.l < 1) {
		err_error(LA_E_BADCOUNT,NULL);
		counter.type = TY_INT;
		counter.data.l = 1;
	}

	p1_add_array_initval(&op,counter.data.l);
}
static void p1_ARRAY_INIT_SINGLE(void)
{
VAL op;

	ce_pop(&op);

	p1_add_array_initval(&op,1);
}
static void p1_INIT_DIM_VAR(void)
{
	p1_curr_sym -> var -> init.a = p1_array_inival;
	p1_curr_sym -> var -> flags |= VAR_F_INIT;
}

static void p1_INIT_SCALAR_VAR(void)
{
VAL op;

	ce_pop(&op);

	if (!st_is_scalar_type(p1_curr_sym -> type)) {
		if (st_is_array_type(p1_curr_sym -> type))
			err_error(LA_E_NSQB,NULL);
		else
			err_error(LA_E_ININONUM,NULL);
	}

	p1_chk_var_init();

	p1_chk_init_type(&op,p1_curr_sym -> type);

	switch (op.type) {
	case TY_INT:
		p1_curr_sym -> var -> init.l = op.data.l;
		break;
	case TY_DOUBLE:
		p1_curr_sym -> var -> init.d = op.data.d;
		break;
	case TY_STRING:
		p1_curr_sym -> var -> init.l = op.data.l;
		break;
	default:
		break;
	}
	p1_curr_sym -> var -> flags |= VAR_F_INIT;
}

static void p1_VAR_EXTERNAL(void)
{
	p1_scope = SCOPE_EXTERN;
}
static void p1_VAR_GLOBAL(void)
{
	p1_scope = SCOPE_GLOBAL;
}
static void p1_END_VAR_EXTERNAL(void)
{
	p1_scope = SCOPE_NULL;
	p1_retain = 0;
}
static void p1_END_VAR_GLOBAL(void)
{
	p1_scope = SCOPE_NULL;
	p1_retain = 0;
}
static void p1_VAR_LOCAL(void)
{
	switch (p1_scope) {
	case SCOPE_NULL: p1_scope = SCOPE_LOCAL; break;
	case SCOPE_PROGRAM: p1_scope = SCOPE_PROG_VAR; break;
	case SCOPE_FUNCTION: p1_scope = SCOPE_FN_VAR; break;
	case SCOPE_FUNCTION_BLOCK: p1_scope = SCOPE_FB_VAR;  break;
	default: util_fatal(1005);
	}
}
static void p1_resume_scope(void)
{
	switch (p1_scope) {
	case SCOPE_LOCAL: p1_scope = SCOPE_NULL; break;
	case SCOPE_PROG_VARIN:
	case SCOPE_PROG_VAROUT:
	case SCOPE_PROG_VAR: p1_scope = SCOPE_PROGRAM; break;
	case SCOPE_FN_VARIN:
	case SCOPE_FN_VAR: p1_scope = SCOPE_FUNCTION; break;
	case SCOPE_FB_VARIN:
	case SCOPE_FB_VAROUT:
	case SCOPE_FB_VAR: p1_scope = SCOPE_FUNCTION_BLOCK; break;
	default: util_fatal(1006);
	}
}
static void p1_END_VAR_LOCAL(void)
{
	p1_resume_scope();
	p1_retain = 0;
}

static void p1_store_sym_in_file_context(void)
{
symbol_t *sym;

	st_register_name(p1_curr_sym -> name,curr_file_context);
	sym = st_add_registered_name();
	sym -> class = p1_curr_sym -> class;
	sym -> type = p1_curr_sym -> type;
	sym -> var = p1_curr_sym -> var;
}

static void p1_make_local_var(char *name,context_t *first, context_t *second)
{
symbol_position_t *pos;
symbol_t *node,*second_node;

	pos = st_register_name(name,global_context);
	second_node = pos -> node;
	if (!second_node && second) {
		pos = st_register_name(name,second);
		second_node = pos -> node;
	}
	pos = st_register_name(name,first);
	node = pos -> node;
	if (node) {
		if (node -> class != SYMCL_LABEL
		 || (node -> var -> flags & VAR_F_DEFINED))
			err_error(LA_E_REDEF,name);
		p1_curr_sym = node;
		return;
	}
	if (second_node) {
		err_warning(LA_W_HIDES,name);
	}
	p1_curr_sym = st_add_registered_name_as_var();
	p1_curr_sym -> var -> flags |= VAR_F_DEFINED;
	p1_curr_sym -> var -> file = rdf.file_name;  
	p1_curr_sym -> var -> line = pScCtl -> n_line;
}

static void p1_var_decl(void)
{
char *name;
symbol_position_t *pos;
symbol_t *node,*second_node;
paramlist_t *pl;

	name = util_strdup(rdf.identifier);

	switch (p1_scope) {
	case SCOPE_GLOBAL:
	case SCOPE_EXTERN:
		pos = st_register_name(name,local_context);
		second_node = pos -> node;
		pos = st_register_name(name,global_context);
		node = pos -> node;
		if (node) {
			if (p1_scope == SCOPE_GLOBAL
			 && ((! node -> var)
		             || node -> var -> flags & VAR_F_DEFINED)) {
				err_error(LA_E_REDEF,name);
			}
			else {
			/* Bisognerebbe confrontare i tipi, ma
			 a questo punto non ci sono ancora tutte le
			 informazioni necessarie. */
			}
		}
		if (second_node) {
			err_warning(LA_W_HIDES,name);
		}
		p1_curr_sym = st_add_registered_name_as_var();
		if (p1_scope == SCOPE_GLOBAL) {
			p1_curr_sym -> var -> flags |= VAR_F_DEFINED;
			p1_curr_sym -> var -> file = rdf.file_name;  
			p1_curr_sym -> var -> line = pScCtl -> n_line;
		}
		break;
	case SCOPE_LOCAL:
		p1_make_local_var(name,local_context,(context_t *) 0);
		p1_curr_sym -> var -> flags |= VAR_F_LOCAL_FL;
		break;
	case SCOPE_PROG_VAR:
	case SCOPE_FN_VAR:
	case SCOPE_FB_VAR:
		p1_make_local_var(name,fb_context, local_context);
		p1_curr_sym -> var -> flags |= VAR_F_LOCAL_FB;
		break;
	case SCOPE_PROG_VARIN:
	case SCOPE_FN_VARIN:
	case SCOPE_FB_VARIN:
		p1_make_local_var(name,fb_context, local_context);
		p1_curr_sym -> var -> at.param_num = p1_param_in++;
		p1_curr_sym -> var -> flags |= VAR_F_PARAM_IN | VAR_F_LOCAL_FB;
		pl = st_paramlist_alloc();
		pl -> sym = p1_curr_sym;
		pl -> next = (paramlist_t *) 0;
		if (! p1_param_in_list_tail)
			p1_param_in_list = pl;
		else
			p1_param_in_list_tail -> next = pl;
		p1_param_in_list_tail = pl;
		break;
	case SCOPE_PROG_VAROUT:
	case SCOPE_FB_VAROUT:
		p1_make_local_var(name,fb_context, local_context);
		p1_curr_sym -> var -> at.param_num = p1_param_out++;
		p1_curr_sym -> var -> flags |= VAR_F_PARAM_OUT | VAR_F_LOCAL_FB;
		pl = st_paramlist_alloc();
		pl -> sym = p1_curr_sym;
		pl -> next = (paramlist_t *) 0;
		if (! p1_param_out_list_tail)
			p1_param_out_list = pl;
		else
			p1_param_out_list_tail -> next = pl;
		p1_param_out_list_tail = pl;
		break;
	case SCOPE_PROGRAM:
	case SCOPE_FUNCTION:
	case SCOPE_FUNCTION_BLOCK:
	/* Puo` essere solo un'etichetta. */
		p1_make_local_var(name,fb_context, (context_t *) 0);
		p1_curr_sym -> class = SYMCL_LABEL;
		p1_curr_sym -> var -> flags |= VAR_F_LOCAL_FB;
		p1_curr_sym -> var -> at.ilexpr_level.base = p1_il_expr_level;
		p1_curr_sym -> var -> at.ilexpr_level.serial
		 = p1_il_expr_lv_stack[p1_il_expr_level];
		break;
	case SCOPE_STRUCT:
	default:
		util_fatal(1002);
	}
}
static void p1_VAR_DECL_AT(void)
{
	p1_var_decl();

	if (p1_retain) {
		err_error(LA_E_RETAIN_AT,NULL);
		return;
	}

	if (p1_scope != SCOPE_GLOBAL) {
		err_error(LA_E_LOCAL_AT,NULL);
	}
	switch (rdf.number[1]) {
	case 'M':
		p1_curr_sym->var->at.direct_io = util_strdup(rdf.number);
		p1_curr_sym -> var -> flags |= VAR_F_CONNECTION_MEMORY;
		break;
	case 'I':
		p1_curr_sym->var->at.direct_io = util_strdup(rdf.number);
		p1_curr_sym -> var -> flags |= VAR_F_CONNECTION_IN;
		break;
	case 'Q':
		p1_curr_sym->var->at.direct_io = util_strdup(rdf.number);
		p1_curr_sym -> var -> flags |= VAR_F_CONNECTION_OUT;
		break;
	default:
		util_fatal(1003);
	}
}
static void p1_var_decl_retain(void)
{
	p1_var_decl();
	if (p1_scope != SCOPE_GLOBAL) {
		err_error(LA_E_LOCAL_RETAIN,NULL);
	}
/* Le variabili "RETAIN" sono segnali di scambio... particolari. */
	p1_curr_sym->var->at.direct_io = "%R**";
	p1_curr_sym -> var -> flags |= VAR_F_CONNECTION_MEMORY | VAR_F_RETAIN;
}
static void p1_VAR_START_DECL_RETAIN(void)
{
	p1_retain = 1;
}
static void p1_VAR_DECL(void)
{
	if (p1_retain) {
		p1_var_decl_retain();
	}
	else {
		p1_var_decl();
	}
}
static void p1_END_VAR_DECL(void)
{
	if (p1_scope == SCOPE_GLOBAL) {
		p1_store_sym_in_file_context();
	}
}
static void p1_PROGRAM_INSTANCE(void)
{
	p1_scope = SCOPE_LOCAL;
	p1_var_decl();
}
static void p1_PROGRAM_CLASS(void)
{
	p1_type_fb(TYPE_PROGRAM);
}
static void p1_END_PROGRAM_INSTANCE(void)
{
	p1_resume_scope();
}

static void p1_VAR_INPUT(void)
{
	switch (p1_scope) {
	case SCOPE_PROGRAM: p1_scope = SCOPE_PROG_VARIN; break;
	case SCOPE_FUNCTION: p1_scope = SCOPE_FN_VARIN; break;
	case SCOPE_FUNCTION_BLOCK: p1_scope = SCOPE_FB_VARIN;  break;
	default: util_fatal(1007);
	}
}
static void p1_VAR_OUTPUT(void)
{
	switch (p1_scope) {
	case SCOPE_PROGRAM: p1_scope = SCOPE_PROG_VAROUT; break;
	case SCOPE_FUNCTION_BLOCK: p1_scope = SCOPE_FB_VAROUT;  break;
	default: util_fatal(1008);
	}
}
static void p1_END_VAR_INPUT(void)
{
	p1_resume_scope();
	p1_retain = 0;
}
static void p1_END_VAR_OUTPUT(void)
{
	p1_resume_scope();
	p1_retain = 0;
}

static void p1_TYPE(void)
{
	err_error(PA_E_UNSUPP,NULL);
}
static void p1_END_TYPE(void)
{
}

static void p1_define_fb(int code)
{
char *name;
symbol_position_t *pos;
symbol_t *node,*second_node;

	name = util_strdup(rdf.identifier);

	p1_param_in = 0;
	p1_param_out = 0;
	p1_param_in_list = (paramlist_t *) 0;
	p1_param_out_list = (paramlist_t *) 0;
	p1_param_in_list_tail = (paramlist_t *) 0;
	p1_param_out_list_tail = (paramlist_t *) 0;

	pos = st_register_name(name,local_context);
	second_node = pos -> node;
	pos = st_register_name(name,global_context);
	node = pos -> node;
	if (node) {
		if (node -> class != SYMCL_NULL) {
			if (node -> class != SYMCL_TYPE
			    || (node -> type
			        && node -> type -> code != TYPE_UNDEF
				&& (node -> type -> code != code
				    || node -> type -> d.fb.context))) {
				err_error(LA_E_REDEF,name);
				return;
			}
		}
	}
	if (second_node) {
		err_warning(LA_W_HIDES,name);
	}
	p1_curr_sym = st_add_registered_name_as_type();
	p1_curr_sym -> type -> code = code;
	p1_curr_sym -> type -> size = 0;
	p1_curr_sym -> type -> d.fb.file = rdf.file_name;
	p1_curr_sym -> type -> d.fb.line = pScCtl -> n_line;
	p1_curr_sym -> type -> d.fb.context = st_create_context(name);
	p1_curr_sym -> type -> d.fb.sym = p1_curr_sym;
	fb_context = p1_curr_sym -> type -> d.fb.context;
	p1_curr_fb = p1_curr_sym;
	p1_store_sym_in_file_context();
	st_push_context(fb_context);
	if (code == TYPE_FUNCTION) {
	/* Genera una variabile globale del tipo "_f_FUNZIONE". */
		name = util_strcatdup(FN_STATICS_PREFIX,name);
		pos = st_register_name(name,global_context);
		node = st_add_registered_name_as_var();
		node -> var -> file = rdf.file_name;
		node -> var -> line = pScCtl -> n_line;
		node -> type = p1_curr_sym -> type;
		p1_curr_sym -> type -> d.fb.statics = node;
	}
}

static void p1_PROGRAM_HEADER(void)
{
	p1_define_fb(TYPE_PROGRAM);
	p1_scope = SCOPE_PROGRAM;
}
static void p1_leave_scope(void)
{
	st_pop_context();
	fb_context = (context_t *) 0;
	p1_scope = SCOPE_NULL;
}
static void p1_PROGRAM_END(void)
{
	p1_leave_scope();
}

static void p1_FUNCTION_BLOCK_HEADER(void)
{
	p1_define_fb(TYPE_FUNCTION_BLOCK);
	p1_scope = SCOPE_FUNCTION_BLOCK;
}
static void p1_FUNCTION_BLOCK_END(void)
{
	p1_leave_scope();
}
static void p1_FUNCTION_HEADER(void)
{
	p1_define_fb(TYPE_FUNCTION);
	p1_scope = SCOPE_FUNCTION;
}
static void p1_FUNCTION_END(void)
{
	p1_leave_scope();
}
static void p1_BODY(void)
{
	p1_curr_fb -> type -> d.fb.input = p1_param_in_list;
	p1_curr_fb -> type -> d.fb.output = p1_param_out_list;
}
static void p1_IL_INDEX_BEGIN(void)
{
	p1_curr_n_index = 0;
}
static void p1_IL_INDEX_END(void)
{
	p1_curr_n_index = 0;
}
static void p1_IL_INDEX(void)
{
	if (p1_curr_n_index < MAX_ARRAY_DIM)
		++p1_curr_n_index;
	else
		err_error(LA_E_TOODIM,NULL);
}

static void p1_IL_EXPR_BEGIN(void)
{
	if (p1_il_expr_level >= MAX_IL_EXPR_LEVEL) {
		err_error(LA_E_TOOILEXPR,NULL);
		return;
	}
	++p1_il_expr_level;
	++p1_il_expr_lv_stack[p1_il_expr_level];
}
static void p1_IL_EXPR_END(void)
{
	if (p1_il_expr_level < 0)
		return;
	--p1_il_expr_level;
}
static void p1_IL_BEGIN(void)
{
int i;

	p1_curr_n_index = 0;
	p1_il_expr_level = 0;
	for (i = 0; i < MAX_IL_EXPR_LEVEL + 1; ++i)
		p1_il_expr_lv_stack[i] = 0;
}
static void p1_IL_END(void)
{
	if (p1_il_expr_level)
		err_error(LA_E_UNTMLILEXPR,NULL);
	p1_curr_n_index = 0;
	p1_il_expr_level = 0;
}

static int p1_handle_label(void)
{
symbol_position_t *pos;

	pos = st_register_name(rdf.identifier,fb_context);
	p1_curr_sym = pos -> node;
	if (p1_curr_sym) {
/* Se il simbolo e` gia` definito, controlla
 che il livello di annidamento coincida. */
		if (p1_curr_sym -> var -> at.ilexpr_level.base
		    != p1_il_expr_level
		 || p1_curr_sym -> var -> at.ilexpr_level.serial
		    != p1_il_expr_lv_stack[p1_il_expr_level])
			err_error(LA_E_DIFFILLEV,NULL);
		return 0;
	}
	else {
/* Definisce l'etichetta. */
		p1_var_decl();
		return 1;
	}
}
static void p1_IL_LABEL(void)
{
	if (! p1_handle_label()) {
	/* Se il simbolo e` gia` definito, controlla se e` stato solo
	 referenziato da JMP o e` stato gia` incontrato come etichetta. */
		if (p1_curr_sym -> var -> flags & VAR_F_DEFINED)
			err_error(LA_E_REDEF,p1_curr_sym -> name);
	/* Marca comunque "definito". */
		p1_curr_sym -> var -> flags |= VAR_F_DEFINED;
	/* Registra il livello di annidamento "giusto". */
		p1_curr_sym -> var -> at.ilexpr_level.base = p1_il_expr_level;
		p1_curr_sym -> var -> at.ilexpr_level.serial
		 = p1_il_expr_lv_stack[p1_il_expr_level];
	}
}
static void p2_check_jmplabel(void)
{
	if (main_status.standard && p1_il_expr_level)
		err_error(LA_E_JMPOOC,NULL);
	if (p1_handle_label()) {
	/* Se l'etichetta non esisteva, la marca "indefinita". */
		p1_curr_sym -> var -> flags &= ~VAR_F_DEFINED;
	}
}
static void p1_IL_JMP(void)
{
	p2_check_jmplabel();
}
static void p1_IL_JMPC(void)
{
	p2_check_jmplabel();
}
static void p1_IL_JMPCN(void)
{
	p2_check_jmplabel();
}

static symbol_t *p1_curr_string = (symbol_t *) 0;

static void p1_RDSTRING(void)
{
symbol_position_t *pos;

	pos = st_register_name(util_strdup(rdf.string),string_context);
	p1_curr_string = st_add_registered_name_as_string();
	p1_curr_type = st_create_type(TYPE_STRING);
	co_new_string(p1_curr_string);
}

static void p1_STRINGEXPR(void)
{
	ce_STRINGEXPR((long)(p1_curr_string -> type));
}

/*******************************\
* +---------------------------+ *
* | AZIONI NON ANCORA GESTITE | *
* +---------------------------+ *
\*******************************/

static void p1_null(void){}

/*
#define p1_PROGRAM_INSTANCE p1_null
#define p1_PROGRAM_CLASS p1_null
#define p1_END_PROGRAM_INSTANCE p1_null
*/
#define p1_PROGRAM_INCONF p1_null
#define p1_PROGRAM_CONSTCONF p1_null
#define p1_PROGRAM_OUTCONF p1_null
#define p1_PROGRAM_ENDIOCONF p1_null
/*
#define p1_RDSTRING p1_null
#define p1_BOOL p1_null
#define p1_SINT p1_null
#define p1_INT p1_null
#define p1_DINT p1_null
#define p1_LINT p1_null
#define p1_USINT p1_null
#define p1_UINT p1_null
#define p1_UDINT p1_null
#define p1_ULINT p1_null
#define p1_REAL p1_null
#define p1_LREAL p1_null
#define p1_TIME p1_null
#define p1_TIME_OF_DAY p1_null
#define p1_DATE p1_null
#define p1_DATE_AND_TIME p1_null
#define p1_STRING p1_null
#define p1_STRINGEXPR p1_null
#define p1_BYTE p1_null
#define p1_WORD p1_null
#define p1_DWORD p1_null
#define p1_LWORD p1_null
#define p1_PROGRAM_HEADER p1_null
#define p1_PROGRAM_END p1_null
#define p1_FUNCTION_BLOCK_HEADER p1_null
#define p1_FUNCTION_BLOCK_END p1_null
#define p1_FUNCTION_HEADER p1_null
#define p1_FUNCTION_END p1_null
*/
#define p1_IL_LVAL_CREATE p1_null
#define p1_IL_LVAL_EXTRACT p1_null
/*
#define p1_IL_INDEX p1_null
#define p1_IL_INDEX_BEGIN p1_null
#define p1_IL_INDEX_END p1_null
#define p1_IL_LABEL p1_null
*/
#define p1_IL_LD p1_null
#define p1_IL_LDN p1_null
#define p1_IL_ST p1_null
#define p1_IL_STN p1_null
#define p1_IL_S p1_null
#define p1_IL_R p1_null
#define p1_IL_AND p1_null
#define p1_IL_ANDN p1_null
#define p1_IL_OR p1_null
#define p1_IL_ORN p1_null
#define p1_IL_XOR p1_null
#define p1_IL_XORN p1_null
#define p1_IL_NOT p1_null
#define p1_IL_ADD p1_null
#define p1_IL_SUB p1_null
#define p1_IL_MUL p1_null
#define p1_IL_DIV p1_null
#define p1_IL_MOD p1_null
#define p1_IL_GT p1_null
#define p1_IL_GE p1_null
#define p1_IL_EQ p1_null
#define p1_IL_NE p1_null
#define p1_IL_NEWLINE p1_null
#define p1_IL_LE p1_null
#define p1_IL_LT p1_null
/*
#define p1_IL_JMP p1_null
#define p1_IL_JMPC p1_null
#define p1_IL_JMPCN p1_null
*/
#define p1_IL_CAL p1_null
#define p1_IL_CALC p1_null
#define p1_IL_CALCN p1_null
#define p1_IL_RET p1_null
#define p1_IL_RETC p1_null
#define p1_IL_RETCN p1_null
/*
#define p1_IL_EXPR_BEGIN p1_null
#define p1_IL_EXPR_END p1_null
#define p1_IL_BEGIN p1_null
#define p1_IL_END p1_null
*/
#define p1_IL_ENDLD p1_null
#define p1_IL_ENDNULL p1_null
#define p1_IL_ENDOPR p1_null
#define p1_IL_STARTOP p1_null
#define p1_IL_FBCALL p1_null
#define p1_IL_FCALL p1_null
#define p1_IL_FNCALL p1_null
#define p1_IL_FORMLIST_ARGREF p1_null
#define p1_IL_FORMLIST_ARGVAL p1_null
#define p1_IL_FORMLIST_BEGIN p1_null
#define p1_IL_FORMLIST_END p1_null
#define p1_IL_NONFLIST_ARG p1_null
#define p1_IL_NONFLIST_BEGIN p1_null
#define p1_IL_NONFLIST_END p1_null
/*
#define p1_BODY p1_null
#define p1_ADD_DIM_ARRAY p1_null
#define p1_END_DIM_ARRAY p1_null
#define p1_VAR_EXTERNAL p1_null
#define p1_VAR_GLOBAL p1_null
#define p1_END_VAR_EXTERNAL p1_null
#define p1_END_VAR_GLOBAL p1_null
#define p1_VAR_LOCAL p1_null
#define p1_VAR_INPUT p1_null
#define p1_VAR_OUTPUT p1_null
#define p1_END_VAR_INPUT p1_null
#define p1_END_VAR_OUTPUT p1_null
#define p1_END_VAR_LOCAL p1_null
#define p1_VAR_DECL p1_null
#define p1_END_VAR_DECL p1_null
#define p1_VAR_DECL_AT p1_null
#define p1_VAR_START_DECL_RETAIN p1_null
#define p1_TYPE_ARRAY p1_null
#define p1_TYPE_FB p1_null
#define p1_TYPE_SCALAR p1_null
*/

/*
* -------------------------
* CONTROLLO DEL COMPILATORE
* -------------------------
*/

static void p1_restart(void)
{

/* Le consuete inizializzazioni. */

	p1_il_expr_level = 0;
	p1_scope = SCOPE_NULL;
	p1_il_expr_level = 0;
	p1_retain = 0;

/* Inizializza lo stack. */

	ce_stack_init();

/* Inizializza le primitive di lettura. */

	rdf_init();

/* Inizializza lo scanner. */

	pScCtl -> minus_into_sym = 0;
	pScCtl -> lf_blank = 1;
	scInit();

/* Inizializza il parser. */

	/* Inizializza la parte di conpetenza del valutatore di
	 espressioni costanti. */

	ce_init();

	/* Definizione delle azioni. */

	/* pPaCtl -> op[OP_ADD] = p1_ADD; */
	pPaCtl -> op[OP_ADD_DIM_ARRAY] = p1_ADD_DIM_ARRAY;
	/* pPaCtl -> op[OP_AND] = p1_AND; */
	pPaCtl -> op[OP_ARRAY_INIT_MULTI] = p1_ARRAY_INIT_MULTI;
	pPaCtl -> op[OP_ARRAY_INIT_SINGLE] = p1_ARRAY_INIT_SINGLE;
	pPaCtl -> op[OP_BODY] = p1_BODY;
	pPaCtl -> op[OP_BOOL] = p1_BOOL;
	pPaCtl -> op[OP_BYTE] = p1_BYTE;
	pPaCtl -> op[OP_CONFIGURATION] = p1_CONFIGURATION;
	pPaCtl -> op[OP_DATE] = p1_DATE;
	pPaCtl -> op[OP_DATE_AND_TIME] = p1_DATE_AND_TIME;
	pPaCtl -> op[OP_DINT] = p1_DINT;
	/* pPaCtl -> op[OP_DIV] = p1_DIV; */
	pPaCtl -> op[OP_DWORD] = p1_DWORD;
	/* pPaCtl -> op[OP_ENDEXPR] = p1_ENDEXPR; */
	pPaCtl -> op[OP_END_ARRAY_INIT] = p1_END_ARRAY_INIT;
	pPaCtl -> op[OP_END_CONFIGURATION] = p1_END_CONFIGURATION;
	pPaCtl -> op[OP_END_DIM_ARRAY] = p1_END_DIM_ARRAY;
	pPaCtl -> op[OP_END_PROGRAM_INSTANCE] = p1_END_PROGRAM_INSTANCE;
	pPaCtl -> op[OP_END_RESOURCE] = p1_END_RESOURCE;
	pPaCtl -> op[OP_END_TYPE] = p1_END_TYPE;
	pPaCtl -> op[OP_END_VAR_DECL] = p1_END_VAR_DECL;
	pPaCtl -> op[OP_END_VAR_EXTERNAL] = p1_END_VAR_EXTERNAL;
	pPaCtl -> op[OP_END_VAR_GLOBAL] = p1_END_VAR_GLOBAL;
	pPaCtl -> op[OP_END_VAR_INPUT] = p1_END_VAR_INPUT;
	pPaCtl -> op[OP_END_VAR_LOCAL] = p1_END_VAR_LOCAL;
	pPaCtl -> op[OP_END_VAR_OUTPUT] = p1_END_VAR_OUTPUT;
	/* pPaCtl -> op[OP_EQ] = p1_EQ; */
	/* pPaCtl -> op[OP_ERROR] = p1_ERROR; */
	pPaCtl -> op[OP_FUNCTION_BLOCK_END] = p1_FUNCTION_BLOCK_END;
	pPaCtl -> op[OP_FUNCTION_BLOCK_HEADER] = p1_FUNCTION_BLOCK_HEADER;
	pPaCtl -> op[OP_FUNCTION_END] = p1_FUNCTION_END;
	pPaCtl -> op[OP_FUNCTION_HEADER] = p1_FUNCTION_HEADER;
	/* pPaCtl -> op[OP_GE] = p1_GE; */
	/* pPaCtl -> op[OP_GT] = p1_GT; */
	pPaCtl -> op[OP_IL_ADD] = p1_IL_ADD;
	pPaCtl -> op[OP_IL_AND] = p1_IL_AND;
	pPaCtl -> op[OP_IL_ANDN] = p1_IL_ANDN;
	pPaCtl -> op[OP_IL_BEGIN] = p1_IL_BEGIN;
	pPaCtl -> op[OP_IL_CAL] = p1_IL_CAL;
	pPaCtl -> op[OP_IL_CALC] = p1_IL_CALC;
	pPaCtl -> op[OP_IL_CALCN] = p1_IL_CALCN;
	pPaCtl -> op[OP_IL_DIV] = p1_IL_DIV;
	pPaCtl -> op[OP_IL_END] = p1_IL_END;
	pPaCtl -> op[OP_IL_ENDLD] = p1_IL_ENDLD;
	pPaCtl -> op[OP_IL_ENDNULL] = p1_IL_ENDNULL;
	pPaCtl -> op[OP_IL_ENDOPR] = p1_IL_ENDOPR;
	pPaCtl -> op[OP_IL_STARTOP] = p1_IL_STARTOP;
	pPaCtl -> op[OP_IL_EQ] = p1_IL_EQ;
	pPaCtl -> op[OP_IL_EXPR_BEGIN] = p1_IL_EXPR_BEGIN;
	pPaCtl -> op[OP_IL_EXPR_END] = p1_IL_EXPR_END;
	pPaCtl -> op[OP_IL_FBCALL] = p1_IL_FBCALL;
	pPaCtl -> op[OP_IL_FCALL] = p1_IL_FCALL;
	pPaCtl -> op[OP_IL_FNCALL] = p1_IL_FNCALL;
	pPaCtl -> op[OP_IL_FORMLIST_ARGREF] = p1_IL_FORMLIST_ARGREF;
	pPaCtl -> op[OP_IL_FORMLIST_ARGVAL] = p1_IL_FORMLIST_ARGVAL;
	pPaCtl -> op[OP_IL_FORMLIST_BEGIN] = p1_IL_FORMLIST_BEGIN;
	pPaCtl -> op[OP_IL_FORMLIST_END] = p1_IL_FORMLIST_END;
	pPaCtl -> op[OP_IL_GE] = p1_IL_GE;
	pPaCtl -> op[OP_IL_GT] = p1_IL_GT;
	pPaCtl -> op[OP_IL_INDEX] = p1_IL_INDEX;
	pPaCtl -> op[OP_IL_INDEX_BEGIN] = p1_IL_INDEX_BEGIN;
	pPaCtl -> op[OP_IL_INDEX_END] = p1_IL_INDEX_END;
	pPaCtl -> op[OP_IL_JMP] = p1_IL_JMP;
	pPaCtl -> op[OP_IL_JMPC] = p1_IL_JMPC;
	pPaCtl -> op[OP_IL_JMPCN] = p1_IL_JMPCN;
	pPaCtl -> op[OP_IL_LABEL] = p1_IL_LABEL;
	pPaCtl -> op[OP_IL_LD] = p1_IL_LD;
	pPaCtl -> op[OP_IL_LDN] = p1_IL_LDN;
	pPaCtl -> op[OP_IL_LE] = p1_IL_LE;
	pPaCtl -> op[OP_IL_LT] = p1_IL_LT;
	pPaCtl -> op[OP_IL_LVAL_CREATE] = p1_IL_LVAL_CREATE;
	pPaCtl -> op[OP_IL_LVAL_EXTRACT] = p1_IL_LVAL_EXTRACT;
	pPaCtl -> op[OP_IL_MOD] = p1_IL_MOD;
	pPaCtl -> op[OP_IL_MUL] = p1_IL_MUL;
	pPaCtl -> op[OP_IL_NE] = p1_IL_NE;
	pPaCtl -> op[OP_IL_NEWLINE] = p1_IL_NEWLINE;
	pPaCtl -> op[OP_IL_NONFLIST_ARG] = p1_IL_NONFLIST_ARG;
	pPaCtl -> op[OP_IL_NONFLIST_BEGIN] = p1_IL_NONFLIST_BEGIN;
	pPaCtl -> op[OP_IL_NONFLIST_END] = p1_IL_NONFLIST_END;
	pPaCtl -> op[OP_IL_NOT] = p1_IL_NOT;
	pPaCtl -> op[OP_IL_OR] = p1_IL_OR;
	pPaCtl -> op[OP_IL_ORN] = p1_IL_ORN;
	pPaCtl -> op[OP_IL_R] = p1_IL_R;
	pPaCtl -> op[OP_IL_RET] = p1_IL_RET;
	pPaCtl -> op[OP_IL_RETC] = p1_IL_RETC;
	pPaCtl -> op[OP_IL_RETCN] = p1_IL_RETCN;
	pPaCtl -> op[OP_IL_S] = p1_IL_S;
	pPaCtl -> op[OP_IL_ST] = p1_IL_ST;
	pPaCtl -> op[OP_IL_STN] = p1_IL_STN;
	pPaCtl -> op[OP_IL_SUB] = p1_IL_SUB;
	pPaCtl -> op[OP_IL_XOR] = p1_IL_XOR;
	pPaCtl -> op[OP_IL_XORN] = p1_IL_XORN;
	pPaCtl -> op[OP_INIT_DIM_VAR] = p1_INIT_DIM_VAR;
	pPaCtl -> op[OP_INIT_SCALAR_VAR] = p1_INIT_SCALAR_VAR;
	pPaCtl -> op[OP_INT] = p1_INT;
	/* pPaCtl -> op[OP_LE] = p1_LE; */
	pPaCtl -> op[OP_LINT] = p1_LINT;
	pPaCtl -> op[OP_LREAL] = p1_LREAL;
	/* pPaCtl -> op[OP_LT] = p1_LT; */
	pPaCtl -> op[OP_LWORD] = p1_LWORD;
	pPaCtl -> op[OP_MAKE_SUBRANGE] = p1_MAKE_SUBRANGE;
	/* pPaCtl -> op[OP_MINUS] = p1_MINUS; */
	/* pPaCtl -> op[OP_MOD] = p1_MOD; */
	/* pPaCtl -> op[OP_NE] = p1_NE; */
	/* pPaCtl -> op[OP_NOT] = p1_NOT; */
	/* pPaCtl -> op[OP_OR] = p1_OR; */
	/* pPaCtl -> op[OP_POW] = p1_POW; */
	pPaCtl -> op[OP_PROGRAM_CLASS] = p1_PROGRAM_CLASS;
	pPaCtl -> op[OP_PROGRAM_CONSTCONF] = p1_PROGRAM_CONSTCONF;
	pPaCtl -> op[OP_PROGRAM_END] = p1_PROGRAM_END;
	pPaCtl -> op[OP_PROGRAM_ENDIOCONF] = p1_PROGRAM_ENDIOCONF;
	pPaCtl -> op[OP_PROGRAM_HEADER] = p1_PROGRAM_HEADER;
	pPaCtl -> op[OP_PROGRAM_INCONF] = p1_PROGRAM_INCONF;
	pPaCtl -> op[OP_PROGRAM_INSTANCE] = p1_PROGRAM_INSTANCE;
	pPaCtl -> op[OP_PROGRAM_OUTCONF] = p1_PROGRAM_OUTCONF;
	/* pPaCtl -> op[OP_RDFALSE] = p1_RDFALSE; */
	/* pPaCtl -> op[OP_RDFLOAT] = p1_RDFLOAT; */
	/* pPaCtl -> op[OP_RDINT] = p1_RDINT; */
	pPaCtl -> op[OP_RDSTRING] = p1_RDSTRING;
	/* pPaCtl -> op[OP_RDTRUE] = p1_RDTRUE; */
	/* pPaCtl -> op[OP_RDTIME] = p1_RDTIME; */
	pPaCtl -> op[OP_REAL] = p1_REAL;
	pPaCtl -> op[OP_RESOURCE] = p1_RESOURCE;
	pPaCtl -> op[OP_RESOURCE_ON] = p1_RESOURCE_ON;
	pPaCtl -> op[OP_SINT] = p1_SINT;
	/* pPaCtl -> op[OP_STARTEXPR] = p1_STARTEXPR; */
	pPaCtl -> op[OP_START_ARRAY_INIT] = p1_START_ARRAY_INIT;
	pPaCtl -> op[OP_START_DIM_ARRAY] = p1_START_DIM_ARRAY;
	pPaCtl -> op[OP_STRING] = p1_STRING;
	pPaCtl -> op[OP_STRINGEXPR] = p1_STRINGEXPR;
	/* pPaCtl -> op[OP_SUB] = p1_SUB; */
	pPaCtl -> op[OP_TIME] = p1_TIME;
	/* pPaCtl -> op[OP_TIMES] = p1_TIMES; */
	pPaCtl -> op[OP_TIME_OF_DAY] = p1_TIME_OF_DAY;
	pPaCtl -> op[OP_TYPE] = p1_TYPE;
	pPaCtl -> op[OP_TYPE_ARRAY] = p1_TYPE_ARRAY;
	pPaCtl -> op[OP_TYPE_FB] = p1_TYPE_FB;
	pPaCtl -> op[OP_TYPE_SCALAR] = p1_TYPE_SCALAR;
	pPaCtl -> op[OP_UDINT] = p1_UDINT;
	pPaCtl -> op[OP_UINT] = p1_UINT;
	pPaCtl -> op[OP_ULINT] = p1_ULINT;
	pPaCtl -> op[OP_USINT] = p1_USINT;
	pPaCtl -> op[OP_VAR_DECL] = p1_VAR_DECL;
	pPaCtl -> op[OP_VAR_DECL_AT] = p1_VAR_DECL_AT;
	pPaCtl -> op[OP_VAR_EXTERNAL] = p1_VAR_EXTERNAL;
	pPaCtl -> op[OP_VAR_GLOBAL] = p1_VAR_GLOBAL;
	pPaCtl -> op[OP_VAR_INPUT] = p1_VAR_INPUT;
	pPaCtl -> op[OP_VAR_LOCAL] = p1_VAR_LOCAL;
	pPaCtl -> op[OP_VAR_OUTPUT] = p1_VAR_OUTPUT;
	pPaCtl -> op[OP_VAR_START_DECL_RETAIN] = p1_VAR_START_DECL_RETAIN;
	pPaCtl -> op[OP_WORD] = p1_WORD;
	/* pPaCtl -> op[OP_XOR] = p1_XOR; */

	paInit();
}

void p1_init(void)
{
	p1_restart();

	st_reset_context(&global_context,"(globals)");

	string_context = (context_t *) 0;
}

int p1_start(file_action_t *act)
{
int rv;
filescope_t *filescope;
char *file;

	p1_restart();

	st_reset_context(&string_context,"(strings)");

	file = act -> source;

	if (!rdf_open(file)) {
		err_error(LA_E_NOFILE,NULL);
		return 0;
	}

	curr_file_context = st_create_context(rdf.file_name);
	local_context = st_create_context(rdf.file_name);
	filescope = st_filescope_alloc();
	filescope -> name = rdf.file_name;
	filescope -> global = curr_file_context;
	filescope -> local = local_context;
	filescope -> next = (filescope_t *) 0;
	if (! main_status.scope_list_tail)
		main_status.scope_list = filescope;
	else
		main_status.scope_list_tail -> next = filescope;
	main_status.scope_list_tail = filescope;
	fb_context = (context_t *) 0;
	st_reset_context_stack();
	st_push_context(global_context);
	st_push_context(local_context);

	rv = paIEC1131();

	rdf_close();
	return ! rdf.error;
}

