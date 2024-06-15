/*
* @(#) pass2.c 1.9 Mon Nov 20 19:50:48 MET 2000
* @(#) Rif. Guerrini
*
* 15/07/96 1.0 Prima stesura.
* 07/10/96 1.1 Aggiunti "S" ed "R" (in realta`, questo file varia
*          in continuazione).
* 15/10/96 GG 1.2 Corretto un buco clamoroso nell'uscita da un'espressione
*             tra parentesi.
* 16/10/96 GG 1.3 La gestione del flag "gencode" e` stata spostata al livello
*             dell'emettitore di caratteri, per fare si` che le elaborazioni
*             caratteristiche del passo 2 siano eseguite su tutti i file.
* 16/10/96 GG 1.4 Aggiunte callback di fine istruzione IL.
* 17/10/96 GG 1.5 Modifiche per ottimizzazione del codice generato.
* 29/11/96 GG 1.6 Modifiche per la lettura di costanti stringa.
* 22/01/97 GG 1.7 Migliorata la diagnostica nel caso di RETAIN+AT.
* 22/01/97 GG 1.8 Cambiata la sintassi delle dichiarazioni RETAIN. La
*             parola chiave RETAIN deve seguire immediatamente VAR,
*             VAR_GLOBAL e VAR_OUTPUT, e si intende applicata a tutte
*             le variabili dichiarate fino ad END_VAR.
* 20/11/00 GG 1.9 Aggiunta (o, meglio, utilizzata) la callback di fine
*             istruzione, per migliorare il codice di debug.
*             Aggiunta la callback di fine istruzione IL, con lo stesso scopo.
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

/* FUNZIONI */

/*******************************\
* ----------------------------- *
* AZIONI SPECIFICHE DEL PASSO 2 *
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

#ifdef FFSS
static type_t PTR_ p2_curr_op_type = (type_t *) 0;
static symbol_t PTR_ p2_curr_conf_sym = (symbol_t *) 0;
static inival_t PTR_ p2_array_inival = (inival_t *) 0;
static inival_t PTR_ p2_array_inival_tail = (inival_t *) 0;
static int p2_array_inival_count = 0;
static int p2_param_in = 0;
static int p2_param_out = 0;
static paramlist_t PTR_ p2_param_in_list = (paramlist_t *) 0;
static paramlist_t PTR_ p2_param_out_list = (paramlist_t *) 0;
static paramlist_t PTR_ p2_param_in_list_tail = (paramlist_t *) 0;
static paramlist_t PTR_ p2_param_out_list_tail = (paramlist_t *) 0;
#endif /* FFSS */

static int p2_scope = SCOPE_NULL;
static int p2_retain = 0;
static symbol_t PTR_ p2_curr_sym = (symbol_t *) 0;
static symbol_t PTR_ p2_curr_fb = (symbol_t *) 0;
static type_t PTR_ p2_pred_type = (type_t *) 0;
static type_t PTR_ p2_curr_type = (type_t *) 0;
static symbol_t PTR_ p2_curr_array_sym = (symbol_t *) 0;
static void (*p2_cexpr_endexpr)(void);
static int p2_in_body = 0;
static int p2_il_expr_level = 0;
static type_t * p2_il_expr_lv_stack[MAX_IL_EXPR_LEVEL + 1];
static char *p2_curr_conf_name = (char *) 0;
static char *p2_curr_res_name = (char *) 0;
static char *p2_curr_cpu_name = (char *) 0;
static symbol_t * p2_curr_prog_instance = (symbol_t *) 0;

static void p2_VAR_EXTERNAL(void)
{
	p2_scope = SCOPE_EXTERN;
}
static void p2_VAR_GLOBAL(void)
{
	p2_scope = SCOPE_GLOBAL;
}
static void p2_END_VAR_EXTERNAL(void)
{
	p2_scope = SCOPE_NULL;
	p2_retain = 0;
}
static void p2_END_VAR_GLOBAL(void)
{
	p2_scope = SCOPE_NULL;
	p2_retain = 0;
}
static void p2_VAR_LOCAL(void)
{
	switch (p2_scope) {
	case SCOPE_NULL: p2_scope = SCOPE_LOCAL; break;
	case SCOPE_PROGRAM: p2_scope = SCOPE_PROG_VAR; break;
	case SCOPE_FUNCTION: p2_scope = SCOPE_FN_VAR; break;
	case SCOPE_FUNCTION_BLOCK: p2_scope = SCOPE_FB_VAR;  break;
	default: util_fatal(2005);
	}
}
static void p2_resume_scope(void)
{
	switch (p2_scope) {
	case SCOPE_PROG_VARIN:
	case SCOPE_PROG_VAROUT:
	case SCOPE_PROG_VAR: p2_scope = SCOPE_PROGRAM; break;
	case SCOPE_FN_VARIN:
	case SCOPE_FN_VAR: p2_scope = SCOPE_FUNCTION; break;
	case SCOPE_FB_VARIN:
	case SCOPE_FB_VAROUT:
	case SCOPE_FB_VAR: p2_scope = SCOPE_FUNCTION_BLOCK; break;
	default: p2_scope = SCOPE_NULL; break;
	}
}
static void p2_END_VAR_LOCAL(void)
{
	p2_resume_scope();
	p2_retain = 0;
}

static symbol_t * p2_name_decl(char *name)
{
symbol_position_t *pos;
symbol_t *sym;

	switch (p2_scope) {
	case SCOPE_EXTERN:
		pos = st_register_name(name,global_context);
		sym = pos -> node;
		co_def_extern(sym);
		break;
	case SCOPE_GLOBAL:
		pos = st_register_name(name,global_context);
		sym = pos -> node;
		co_def_global_var(sym);
		break;
	case SCOPE_LOCAL:
		pos = st_register_name(name,local_context);
		sym = pos -> node;
		co_def_local_var(sym);
		break;
	case SCOPE_PROGRAM:
	case SCOPE_FUNCTION:
	case SCOPE_FUNCTION_BLOCK:
	/* Non puo` che essere un'etichetta. */
		pos = st_register_name(name,fb_context);
		sym = pos -> node;
		if (sym -> class != SYMCL_LABEL)
			util_fatal(2020);
	/* Memorizza il tipo corrente utilizzato dal programma. */
		sym -> type = p2_curr_type;
		co_def_label(sym);
		break;
	default:
		sym = (symbol_t *) 0;
		break;
	}
	return sym;
}
static symbol_t * p2_var_decl(void)
{
	return p2_name_decl(rdf.identifier);
}
static void p2_VAR_DECL(void)
{
	p2_var_decl();
}
static void p2_END_VAR_DECL(void)
{
}
static void _p2_chk_msd_val(symbol_t *sym, char *io)
{
#ifdef FFSS
int size,sz;
#else
int size;
int sz = 0;
#endif
type_t *t;

	if (! sym || ! io)
		return;

	t = sym -> type;
	if (! st_is_scalar_type(t)) {
		if (io[1] != 'M') {
			err_error(LA_E_NOTARRAY,NULL);
			return;
		}
		t = t -> d.a.type;
	}

	size = t -> size;

	switch (io[2]) {
	case 'X':
		if (t -> code != TYPE_BOOL)
			err_warning(LA_W_SIZE,NULL);
		return;
	case 'B': sz = 1; break;
	case 'W': sz = 2; break;
	case 'D': sz = 4; break;
	case 'L': sz = 8; break;
	default: util_fatal(2060);
	}
	if (size != sz)
		err_warning(LA_W_SIZE,NULL);
}

static void p2_VAR_DECL_AT(void)
{
symbol_t *sym;

	sym = p2_var_decl();
	_p2_chk_msd_val(sym,rdf.number);
}
static void p2_VAR_START_DECL_RETAIN(void)
{
	p2_retain = 1;
}

static void p2_VAR_INPUT(void)
{
	switch (p2_scope) {
	case SCOPE_PROGRAM: p2_scope = SCOPE_PROG_VARIN; break;
	case SCOPE_FUNCTION: p2_scope = SCOPE_FN_VARIN; break;
	case SCOPE_FUNCTION_BLOCK: p2_scope = SCOPE_FB_VARIN;  break;
	default: util_fatal(2007);
	}
}
static void p2_VAR_OUTPUT(void)
{
	switch (p2_scope) {
	case SCOPE_PROGRAM: p2_scope = SCOPE_PROG_VAROUT; break;
	case SCOPE_FUNCTION_BLOCK: p2_scope = SCOPE_FB_VAROUT;  break;
	default: util_fatal(2008);
	}
}
static void p2_END_VAR_INPUT(void)
{
	p2_resume_scope();
	p2_retain = 0;
}
static void p2_END_VAR_OUTPUT(void)
{
	p2_resume_scope();
	p2_retain = 0;
}

static void p2_define_fb(int code)
{
char *name;
symbol_position_t *pos;
symbol_t *node;

	name = rdf.identifier;
	pos = st_register_name(name,global_context);
	node = pos -> node;
	p2_curr_sym = node;
	p2_curr_fb = node;
	fb_context = node -> type -> d.fb.context;
	st_push_context(fb_context);
}

/* Funzione di fine blocco. */
static void p2_end_fb(void)
{
	co_end_fb(p2_curr_fb);
	st_pop_context();
	fb_context = (context_t *) 0;
	p2_scope = SCOPE_NULL;
	p2_in_body = 0;
}
static void p2_PROGRAM_HEADER(void)
{
	p2_define_fb(TYPE_PROGRAM);
	p2_scope = SCOPE_PROGRAM;
	co_begin_fb(p2_curr_fb);
}
static void p2_PROGRAM_END(void)
{
	p2_end_fb();
}

static void p2_FUNCTION_BLOCK_HEADER(void)
{
	p2_define_fb(TYPE_FUNCTION_BLOCK);
	p2_scope = SCOPE_FUNCTION_BLOCK;
	co_begin_fb(p2_curr_fb);
}
static void p2_FUNCTION_BLOCK_END(void)
{
	p2_end_fb();
}
static void p2_FUNCTION_HEADER(void)
{
	p2_define_fb(TYPE_FUNCTION);
	p2_scope = SCOPE_GLOBAL;
	co_def_global_var(p2_curr_fb -> type -> d.fb.statics);
	p2_resume_scope();
	p2_scope = SCOPE_FUNCTION;
	co_begin_fb(p2_curr_fb);
}
static void p2_FUNCTION_END(void)
{
	p2_end_fb();
}
static void p2_BODY(void)
{
/* Al passo 2 e` possibile assegnare la locazione agli elementi
 del contesto. */
	co_assign_offset(p2_curr_fb -> type);
/* Inizializza il tipo corrente a "NIENTE". */
	p2_pred_type = p2_curr_type = st_create_type(TYPE_NONE);
/* Segnala che si inizia a compilare una blocco di codice eseguibile. */
	p2_in_body = 1;
/* Azzera lo stack dei tipi. */
	p2_il_expr_level = 0;
}
static void p2_IL_LABEL(void)
{
	p2_var_decl();
	co_label();
}

static symbol_t * p2_find_sym(void)
{
char *name;
symbol_position_t *pos;

	name = rdf.identifier;

	pos = st_register_name(name,(context_t *) 0);
	if (! pos -> node)
		err_error(LA_E_UNDEFSYM, name);
	return pos -> node;
}

static void p2_IL_LVAL_CREATE(void)
{
symbol_t *sym;

	sym = p2_find_sym();
	if (! sym)
		return;
	p2_curr_sym = sym;
	p2_curr_type = sym -> type;
	co_expr_push_sym(sym);
	if (p2_curr_sym -> class != SYMCL_VAR) {
		err_error(LA_E_NOTAVAR, sym -> name);
	}
}

static symbol_t * p2_find_field(void)
{
char *name;
symbol_position_t *pos;

	name = rdf.identifier;

	if (!p2_curr_sym
	    || p2_curr_sym -> type -> code != TYPE_FUNCTION_BLOCK) {
		err_error(LA_E_NOTAFB, p2_curr_sym -> name);
		return (symbol_t *) 0;
	}
	pos = st_register_name(name,p2_curr_sym -> type -> d.fb.context);
	if (! pos -> node)
		err_error(LA_E_UNDEFSYM, name);
	return pos -> node;
}

static void p2_IL_LVAL_EXTRACT(void)
{
symbol_t *sym;

	sym = p2_find_field();
	if (! sym)
		return;
	p2_curr_sym = sym;
	p2_curr_type = sym -> type;
	co_expr_push_sym(sym);
	co_expr_extract();
}
static void p2_IL_INDEX(void)
{
	co_expr_index_register();
}
static void p2_IL_INDEX_BEGIN(void)
{
	p2_curr_array_sym = p2_curr_sym;
	co_expr_index_begin();
}
static void p2_IL_INDEX_END(void)
{
	p2_curr_type = p2_curr_array_sym -> type -> d.a.type;
}

static void p2_IL_LD(void)
{
	co_expr_eval_ld();
}
static void p2_IL_LDN(void)
{
	co_expr_eval_ld();
	co_not(p2_curr_type -> code == TYPE_BOOL);
}
static void p2_IL_ST(void)
{
	co_expr_eval();
	co_expr_store();
}
static void p2_IL_STN(void)
{
	co_expr_eval();
	co_expr_nstore(p2_curr_type -> code == TYPE_BOOL);
}
static void p2_IL_S(void)
{
	co_expr_set();
}
static void p2_IL_R(void)
{
	co_expr_reset();
}

static void p2_ENDEXPR(void)
{
	(*p2_cexpr_endexpr)();
	if (p2_in_body)
		co_expr_push_num();
}

static void p2_IL_AND(void){ co_expr_bitop(CO_O_AND,0,0); }
static void p2_IL_ANDN(void){ co_expr_bitop(CO_O_AND,1,p2_curr_type -> code == TYPE_BOOL); }
static void p2_IL_OR(void){ co_expr_bitop(CO_O_OR,0,0); }
static void p2_IL_ORN(void){ co_expr_bitop(CO_O_OR,1,p2_curr_type -> code == TYPE_BOOL); }
static void p2_IL_XOR(void){ co_expr_bitop(CO_O_XOR,0,0); }
static void p2_IL_XORN(void){ co_expr_bitop(CO_O_XOR,1,p2_curr_type -> code == TYPE_BOOL); }
static void p2_IL_NOT(void)
{
	co_not(p2_curr_type -> code == TYPE_BOOL);
}
static void p2_IL_ADD(void){ co_expr_bitop(CO_O_ADD,0,0); }
static void p2_IL_SUB(void){ co_expr_bitop(CO_O_SUB,0,0); }
static void p2_IL_MUL(void){ co_expr_arop(CO_O_MUL,st_is_unsigned_type(p2_curr_type)); }
static void p2_IL_DIV(void){ co_expr_arop(CO_O_DIV,st_is_unsigned_type(p2_curr_type)); }
static void p2_IL_MOD(void){ co_expr_arop(CO_O_MOD,st_is_unsigned_type(p2_curr_type)); }
static void p2_IL_GT(void){ co_expr_bitop(CO_O_GT,st_is_unsigned_type(p2_curr_type),0); }
static void p2_IL_GE(void){ co_expr_bitop(CO_O_GE,st_is_unsigned_type(p2_curr_type),0); }
static void p2_IL_EQ(void){ co_expr_bitop(CO_O_EQ,st_is_unsigned_type(p2_curr_type),0); }
static void p2_IL_NE(void){ co_expr_bitop(CO_O_NE,st_is_unsigned_type(p2_curr_type),0); }
static void p2_IL_LE(void){ co_expr_bitop(CO_O_LE,st_is_unsigned_type(p2_curr_type),0); }
static void p2_IL_LT(void){ co_expr_bitop(CO_O_LT,st_is_unsigned_type(p2_curr_type),0); }

static void p2_check_jmplabel(void)
{
symbol_position_t *pos;

	pos = st_register_name(rdf.identifier,fb_context);
	p2_curr_sym = pos -> node;
	if (! p2_curr_sym
	 || ! (p2_curr_sym -> var -> flags & VAR_F_DEFINED)
	 || p2_curr_sym -> class != SYMCL_LABEL) {
		err_error(LA_E_UNDEFSYM,rdf.identifier);
	}
}
static void p2_IL_JMP(void)
{
	p2_check_jmplabel();
	co_jmp(p2_curr_sym);
}
static void p2_IL_JMPC(void)
{
	p2_check_jmplabel();
	co_jmpc(p2_curr_sym);
}
static void p2_IL_JMPCN(void)
{
	p2_check_jmplabel();
	co_jmpnc(p2_curr_sym);
}

static void p2_check_calfb(void)
{
symbol_position_t *pos;

	pos = st_register_name(rdf.identifier,(context_t *)0);
	p2_curr_sym = pos -> node;
	if (! p2_curr_sym)
		err_error(LA_E_UNDEFSYM,rdf.identifier);
	else if ((p2_curr_sym -> class != SYMCL_VAR)
	        || (p2_curr_sym -> type -> code != TYPE_FUNCTION_BLOCK)) {
		err_error(LA_E_NOTAFB,rdf.identifier);
		p2_curr_sym = (symbol_t *) 0;
	}
	else if (! (p2_curr_sym -> var -> flags & VAR_F_DEFINED)) {
		err_error(LA_E_UNDEFSYM,rdf.identifier);
		p2_curr_sym = (symbol_t *) 0;
	}
}
static void p2_IL_CAL(void)
{
	p2_check_calfb();
	co_cal(p2_curr_sym);
}
static void p2_IL_CALC(void)
{
	p2_check_calfb();
	co_calc(p2_curr_sym);
}
static void p2_IL_CALCN(void)
{
	p2_check_calfb();
	co_calcn(p2_curr_sym);
}

static symbol_t *p2_curr_fn = (symbol_t *) 0;

static void p2_check_calfn(void)
{
symbol_position_t *pos;

	pos = st_register_name(rdf.identifier,(context_t *)0);
	p2_curr_sym = p2_curr_fn = pos -> node;
	if (! p2_curr_sym)
		err_error(LA_E_UNDEFSYM,rdf.identifier);
	else if ((p2_curr_sym -> class != SYMCL_TYPE)
	        || (p2_curr_sym -> type -> code != TYPE_FUNCTION)) {
		err_error(LA_E_NOTAFB,rdf.identifier);
		p2_curr_sym = (symbol_t *) 0;
	}
/*
	else if (! (p2_curr_sym -> var -> flags & VAR_F_DEFINED)) {
		err_error(LA_E_UNDEFSYM,rdf.identifier);
		p2_curr_sym = (symbol_t *) 0;
	}
*/
}
static void p2_IL_FNCALL(void)
{
	p2_check_calfn();
	co_calfn(p2_curr_sym);
}
static void p2_IL_NONFLIST_ARG(void)
{
	co_fnarg();
}
static void p2_IL_NONFLIST_BEGIN(void)
{
}
static void p2_IL_NONFLIST_END(void)
{
/* Chiama la funzione. */
	co_do_fncall();
/* Registra il tipo del valore di ritorno come tipo corrente. */
	if (p2_curr_fn)
		p2_curr_type = p2_curr_fn -> type -> d.fb.rv_type;
}

static void p2_IL_RET(void)
{
	co_ret();
}
static void p2_IL_RETC(void)
{
	co_retc();
}
static void p2_IL_RETCN(void)
{
	co_retnc();
}

static void p2_IL_EXPR_BEGIN(void)
{
	p2_il_expr_lv_stack[p2_il_expr_level++] = p2_pred_type;
	co_il_expr_begin();
}
static void p2_IL_EXPR_END(void)
{
	p2_pred_type = p2_il_expr_lv_stack[--p2_il_expr_level];
	co_il_expr_end();
}

static void p2_IL_BEGIN(void){}
static void p2_IL_END(void){}

static void p2_CONFIGURATION(void)
{
	p2_curr_conf_name = util_strdup(rdf.identifier);
	co_begin_def_configuration(p2_curr_conf_name);
}
static void p2_END_CONFIGURATION(void)
{
	co_end_def_configuration();
}
static void p2_RESOURCE(void)
{
	p2_curr_res_name = util_strdup(rdf.identifier);
}
static void p2_RESOURCE_ON(void)
{
	p2_curr_cpu_name = util_strdup(rdf.identifier);
	co_begin_def_resource(p2_curr_res_name, p2_curr_cpu_name);
}
static void p2_END_RESOURCE(void)
{
	co_end_def_resource();
}
static void p2_PROGRAM_INSTANCE(void)
{
symbol_position_t *pos;

	pos = st_register_name(rdf.identifier,local_context);
	p2_curr_prog_instance = pos -> node;
	p2_scope = SCOPE_LOCAL;
	p2_var_decl();
}
static int _p2_deref_iovar(context_t *c, symbol_t *sym, void *u)
{
	if (sym -> class == SYMCL_VAR
	 && ((sym -> var -> flags & VAR_F_PARAM_M) != VAR_F_PARAM_NONE))
		sym -> var -> flags &= ~VAR_F_ASSIGNED;
	return 1;
}
static void p2_PROGRAM_CLASS(void)
{
symbol_position_t *pos;
char *name;

	name = rdf.identifier;
	pos = st_register_name(name,global_context);
	if (pos -> node -> class != SYMCL_TYPE
	 || pos -> node -> type -> code != TYPE_PROGRAM) {
		err_error(LA_E_NOTAPROG, name);
	}
	else {
	/* Questo servira`. */
		fb_context = pos -> node -> type -> d.fb.context;
		co_assign_offset(pos -> node -> type);
	/* Azzera il flag "ASSIGNED" di tutti gli elementi delle
	 liste di input e di output. */
		st_explore_context(fb_context,
		                   _p2_deref_iovar,
		                   (void *) 0);
	}
	co_def_program_instance(p2_curr_prog_instance,pos -> node);
}
static int _p2_chk_ref_iovar(context_t *c, symbol_t *sym, void * u)
{
	if (sym -> class == SYMCL_VAR
	 && ((sym -> var -> flags & VAR_F_PARAM_M) != VAR_F_PARAM_NONE)
	 && !(sym -> var -> flags & VAR_F_ASSIGNED)) {
		err_warning(LA_W_UNASGN,sym -> name);
	}
	return 1;
}
static void p2_END_PROGRAM_INSTANCE(void)
{
	co_end_def_program_instance();
	st_explore_context(fb_context,
			   _p2_chk_ref_iovar,
			   (void *) 0);
	p2_resume_scope();
	fb_context = (context_t *) 0;
}
static symbol_t * _p2_chk_msd(int mode)
{
symbol_position_t *pos;
symbol_t *sym ;

	if (! fb_context)
		return (symbol_t *) 0;
	pos = st_register_name(rdf.identifier,fb_context);
	sym = pos -> node;
	if (! sym) {
		err_error(LA_E_UNDEFSYM,rdf.identifier);
		return sym;
	}
	if (sym -> class != SYMCL_VAR
	 || (sym -> var -> flags & VAR_F_PARAM_M) != mode) {
		err_error(LA_E_NOTIO,sym -> name);
		return sym;
	}
	if (sym -> var -> flags & VAR_F_ASSIGNED) {
		err_error(LA_E_DUPASGN,sym -> name);
	}
	else {
		sym -> var -> flags |= VAR_F_ASSIGNED;
	}
	return sym;
}
static void p2_PROGRAM_INCONF(void)
{
symbol_t *sym;

	sym = _p2_chk_msd(VAR_F_PARAM_IN);
	_p2_chk_msd_val(sym,rdf.number);
	co_add_msd_in(sym,p2_curr_prog_instance,rdf.number);
}
static void p2_PROGRAM_CONSTCONF(void)
{
symbol_t *sym;

	sym = _p2_chk_msd(VAR_F_PARAM_IN);
	co_add_msd_const(sym,p2_curr_prog_instance);
}
static void p2_PROGRAM_OUTCONF(void)
{
symbol_t *sym;

	sym = _p2_chk_msd(VAR_F_PARAM_OUT);
	_p2_chk_msd_val(sym,rdf.number);
	co_add_msd_out(sym,p2_curr_prog_instance,rdf.number);
}
static void p2_PROGRAM_ENDIOCONF(void)
{
}

static void p2_IL_NEWLINE(void)
{
	co_il_newline();
}

static symbol_t *p2_curr_string = (symbol_t *) 0;

static void p2_RDSTRING(void)
{
symbol_position_t *pos;

	pos = st_register_name(util_strdup(rdf.string),string_context);
	p2_curr_string = st_add_registered_name_as_string();
	p2_curr_type = st_create_type(TYPE_STRING);
	co_add_string(p2_curr_string);
}

static void p2_STRINGEXPR(void)
{
}

static void p2_IL_ENDLD(void)
{
	co_end_instruction();
}

static void p2_IL_ENDNULL(void)
{
	co_end_instruction();
}

static void p2_IL_ENDOPR(void)
{
	co_end_instruction();
}

static void p2_IL_STARTOP(void)
{
	co_begin_instruction();
}

/*******************************\
* +---------------------------+ *
* | AZIONI NON ANCORA GESTITE | *
* +---------------------------+ *
\*******************************/

static void p2_null(void){}

/*
#define p2_PROGRAM_INSTANCE p2_null
#define p2_PROGRAM_CLASS p2_null
#define p2_END_PROGRAM_INSTANCE p2_null
#define p2_PROGRAM_INCONF p2_null
#define p2_PROGRAM_CONSTCONF p2_null
#define p2_PROGRAM_OUTCONF p2_null
#define p2_PROGRAM_ENDIOCONF p2_null
*/
#define p2_BOOL p2_null
#define p2_SINT p2_null
#define p2_INT p2_null
#define p2_DINT p2_null
#define p2_LINT p2_null
#define p2_USINT p2_null
#define p2_UINT p2_null
#define p2_UDINT p2_null
#define p2_ULINT p2_null
#define p2_REAL p2_null
#define p2_LREAL p2_null
#define p2_TIME p2_null
#define p2_TIME_OF_DAY p2_null
#define p2_DATE p2_null
#define p2_DATE_AND_TIME p2_null
#define p2_STRING p2_null
/*
#define p2_STRINGEXPR p2_null
*/
#define p2_BYTE p2_null
#define p2_WORD p2_null
#define p2_DWORD p2_null
#define p2_LWORD p2_null
#define p2_TYPE p2_null
#define p2_END_TYPE p2_null
/*
#define p2_PROGRAM_HEADER p2_null
#define p2_PROGRAM_END p2_null
#define p2_FUNCTION_BLOCK_HEADER p2_null
#define p2_FUNCTION_BLOCK_END p2_null
#define p2_FUNCTION_HEADER p2_null
#define p2_FUNCTION_END p2_null
#define p2_IL_LVAL_CREATE p2_null
#define p2_IL_LVAL_EXTRACT p2_null
#define p2_IL_INDEX p2_null
#define p2_IL_INDEX_BEGIN p2_null
#define p2_IL_INDEX_END p2_null
#define p2_IL_LABEL p2_null
#define p2_IL_LD p2_null
#define p2_IL_LDN p2_null
#define p2_IL_ST p2_null
#define p2_IL_STN p2_null
#define p2_IL_S p2_null
#define p2_IL_R p2_null
#define p2_IL_AND p2_null
#define p2_IL_ANDN p2_null
#define p2_IL_OR p2_null
#define p2_IL_ORN p2_null
#define p2_IL_XOR p2_null
#define p2_IL_XORN p2_null
#define p2_IL_NOT p2_null
#define p2_IL_ADD p2_null
#define p2_IL_SUB p2_null
#define p2_IL_MUL p2_null
#define p2_IL_DIV p2_null
#define p2_IL_MOD p2_null
#define p2_IL_GT p2_null
#define p2_IL_GE p2_null
#define p2_IL_EQ p2_null
#define p2_IL_NE p2_null
#define p2_IL_LE p2_null
#define p2_IL_LT p2_null
#define p2_IL_JMP p2_null
#define p2_IL_JMPC p2_null
#define p2_IL_JMPCN p2_null
#define p2_IL_CAL p2_null
#define p2_IL_CALC p2_null
#define p2_IL_CALCN p2_null
#define p2_IL_RET p2_null
#define p2_IL_RETC p2_null
#define p2_IL_RETCN p2_null
#define p2_IL_EXPR_BEGIN p2_null
#define p2_IL_EXPR_END p2_null
#define p2_IL_BEGIN p2_null
#define p2_IL_END p2_null
#define p2_IL_FNCALL p2_null
#define p2_IL_NONFLIST_ARG p2_null
#define p2_IL_NONFLIST_BEGIN p2_null
#define p2_IL_NONFLIST_END p2_null
*/
/*
#define p2_IL_ENDLD p2_null
#define p2_IL_ENDNULL p2_null
#define p2_IL_ENDOPR p2_null
#define p2_IL_STARTOP p2_null
*/
#define p2_IL_FBCALL p2_null
#define p2_IL_FCALL p2_null
#define p2_IL_FORMLIST_ARGREF p2_null
#define p2_IL_FORMLIST_ARGVAL p2_null
#define p2_IL_FORMLIST_BEGIN p2_null
#define p2_IL_FORMLIST_END p2_null
#define p2_START_DIM_ARRAY p2_null
#define p2_MAKE_SUBRANGE p2_null
#define p2_ADD_DIM_ARRAY p2_null
#define p2_END_DIM_ARRAY p2_null
#define p2_TYPE_SCALAR p2_null
#define p2_TYPE_FB p2_null
#define p2_TYPE_ARRAY p2_null
#define p2_START_ARRAY_INIT p2_null
#define p2_END_ARRAY_INIT p2_null
#define p2_ARRAY_INIT_MULTI p2_null
#define p2_ARRAY_INIT_SINGLE p2_null
#define p2_INIT_DIM_VAR p2_null
#define p2_INIT_SCALAR_VAR p2_null
/*
#define p2_BODY p2_null
#define p2_ADD_DIM_ARRAY p2_null
#define p2_END_DIM_ARRAY p2_null
#define p2_VAR_EXTERNAL p2_null
#define p2_VAR_GLOBAL p2_null
#define p2_END_VAR_EXTERNAL p2_null
#define p2_END_VAR_GLOBAL p2_null
#define p2_VAR_LOCAL p2_null
#define p2_VAR_INPUT p2_null
#define p2_VAR_OUTPUT p2_null
#define p2_END_VAR_INPUT p2_null
#define p2_END_VAR_OUTPUT p2_null
#define p2_END_VAR_LOCAL p2_null
#define p2_VAR_DECL p2_null
#define p2_END_VAR_DECL p2_null
#define p2_VAR_DECL_AT p2_null
#define p2_VAR_START_DECL_RETAIN p2_null
#define p2_TYPE_ARRAY p2_null
#define p2_TYPE_FB p2_null
#define p2_TYPE_SCALAR p2_null
*/

/*
* -------------------------
* CONTROLLO DEL COMPILATORE
* -------------------------
*/

static void p2_restart(void)
{

/* Inizializza alcune variabili di stato. */

	p2_in_body = 0;
	p2_scope = SCOPE_NULL;
	p2_retain = 0;

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

	/* pPaCtl -> op[OP_ADD] = p2_ADD; */
	pPaCtl -> op[OP_ADD_DIM_ARRAY] = p2_ADD_DIM_ARRAY;
	/* pPaCtl -> op[OP_AND] = p2_AND; */
	pPaCtl -> op[OP_ARRAY_INIT_MULTI] = p2_ARRAY_INIT_MULTI;
	pPaCtl -> op[OP_ARRAY_INIT_SINGLE] = p2_ARRAY_INIT_SINGLE;
	pPaCtl -> op[OP_BODY] = p2_BODY;
	pPaCtl -> op[OP_BOOL] = p2_BOOL;
	pPaCtl -> op[OP_BYTE] = p2_BYTE;
	pPaCtl -> op[OP_CONFIGURATION] = p2_CONFIGURATION;
	pPaCtl -> op[OP_DATE] = p2_DATE;
	pPaCtl -> op[OP_DATE_AND_TIME] = p2_DATE_AND_TIME;
	pPaCtl -> op[OP_DINT] = p2_DINT;
	/* pPaCtl -> op[OP_DIV] = p2_DIV; */
	pPaCtl -> op[OP_DWORD] = p2_DWORD;
/* Aggancia la callback del valutatore di espressioni costanti,
 per poter eseguire alcune azioni aggiuntive. */
	p2_cexpr_endexpr = pPaCtl -> op[OP_ENDEXPR];
	pPaCtl -> op[OP_ENDEXPR] = p2_ENDEXPR;
	pPaCtl -> op[OP_END_ARRAY_INIT] = p2_END_ARRAY_INIT;
	pPaCtl -> op[OP_END_CONFIGURATION] = p2_END_CONFIGURATION;
	pPaCtl -> op[OP_END_DIM_ARRAY] = p2_END_DIM_ARRAY;
	pPaCtl -> op[OP_END_PROGRAM_INSTANCE] = p2_END_PROGRAM_INSTANCE;
	pPaCtl -> op[OP_END_RESOURCE] = p2_END_RESOURCE;
	pPaCtl -> op[OP_END_TYPE] = p2_END_TYPE;
	pPaCtl -> op[OP_END_VAR_DECL] = p2_END_VAR_DECL;
	pPaCtl -> op[OP_END_VAR_EXTERNAL] = p2_END_VAR_EXTERNAL;
	pPaCtl -> op[OP_END_VAR_GLOBAL] = p2_END_VAR_GLOBAL;
	pPaCtl -> op[OP_END_VAR_INPUT] = p2_END_VAR_INPUT;
	pPaCtl -> op[OP_END_VAR_LOCAL] = p2_END_VAR_LOCAL;
	pPaCtl -> op[OP_END_VAR_OUTPUT] = p2_END_VAR_OUTPUT;
	/* pPaCtl -> op[OP_EQ] = p2_EQ; */
	/* pPaCtl -> op[OP_ERROR] = p2_ERROR; */
	pPaCtl -> op[OP_FUNCTION_BLOCK_END] = p2_FUNCTION_BLOCK_END;
	pPaCtl -> op[OP_FUNCTION_BLOCK_HEADER] = p2_FUNCTION_BLOCK_HEADER;
	pPaCtl -> op[OP_FUNCTION_END] = p2_FUNCTION_END;
	pPaCtl -> op[OP_FUNCTION_HEADER] = p2_FUNCTION_HEADER;
	/* pPaCtl -> op[OP_GE] = p2_GE; */
	/* pPaCtl -> op[OP_GT] = p2_GT; */
	pPaCtl -> op[OP_IL_ADD] = p2_IL_ADD;
	pPaCtl -> op[OP_IL_AND] = p2_IL_AND;
	pPaCtl -> op[OP_IL_ANDN] = p2_IL_ANDN;
	pPaCtl -> op[OP_IL_BEGIN] = p2_IL_BEGIN;
	pPaCtl -> op[OP_IL_CAL] = p2_IL_CAL;
	pPaCtl -> op[OP_IL_CALC] = p2_IL_CALC;
	pPaCtl -> op[OP_IL_CALCN] = p2_IL_CALCN;
	pPaCtl -> op[OP_IL_DIV] = p2_IL_DIV;
	pPaCtl -> op[OP_IL_END] = p2_IL_END;
	pPaCtl -> op[OP_IL_ENDLD] = p2_IL_ENDLD;
	pPaCtl -> op[OP_IL_ENDNULL] = p2_IL_ENDNULL;
	pPaCtl -> op[OP_IL_ENDOPR] = p2_IL_ENDOPR;
	pPaCtl -> op[OP_IL_STARTOP] = p2_IL_STARTOP;
	pPaCtl -> op[OP_IL_EQ] = p2_IL_EQ;
	pPaCtl -> op[OP_IL_EXPR_BEGIN] = p2_IL_EXPR_BEGIN;
	pPaCtl -> op[OP_IL_EXPR_END] = p2_IL_EXPR_END;
	pPaCtl -> op[OP_IL_FBCALL] = p2_IL_FBCALL;
	pPaCtl -> op[OP_IL_FCALL] = p2_IL_FCALL;
	pPaCtl -> op[OP_IL_FNCALL] = p2_IL_FNCALL;
	pPaCtl -> op[OP_IL_FORMLIST_ARGREF] = p2_IL_FORMLIST_ARGREF;
	pPaCtl -> op[OP_IL_FORMLIST_ARGVAL] = p2_IL_FORMLIST_ARGVAL;
	pPaCtl -> op[OP_IL_FORMLIST_BEGIN] = p2_IL_FORMLIST_BEGIN;
	pPaCtl -> op[OP_IL_FORMLIST_END] = p2_IL_FORMLIST_END;
	pPaCtl -> op[OP_IL_GE] = p2_IL_GE;
	pPaCtl -> op[OP_IL_GT] = p2_IL_GT;
	pPaCtl -> op[OP_IL_INDEX] = p2_IL_INDEX;
	pPaCtl -> op[OP_IL_INDEX_BEGIN] = p2_IL_INDEX_BEGIN;
	pPaCtl -> op[OP_IL_INDEX_END] = p2_IL_INDEX_END;
	pPaCtl -> op[OP_IL_JMP] = p2_IL_JMP;
	pPaCtl -> op[OP_IL_JMPC] = p2_IL_JMPC;
	pPaCtl -> op[OP_IL_JMPCN] = p2_IL_JMPCN;
	pPaCtl -> op[OP_IL_LABEL] = p2_IL_LABEL;
	pPaCtl -> op[OP_IL_LD] = p2_IL_LD;
	pPaCtl -> op[OP_IL_LDN] = p2_IL_LDN;
	pPaCtl -> op[OP_IL_LE] = p2_IL_LE;
	pPaCtl -> op[OP_IL_LT] = p2_IL_LT;
	pPaCtl -> op[OP_IL_LVAL_CREATE] = p2_IL_LVAL_CREATE;
	pPaCtl -> op[OP_IL_LVAL_EXTRACT] = p2_IL_LVAL_EXTRACT;
	pPaCtl -> op[OP_IL_MOD] = p2_IL_MOD;
	pPaCtl -> op[OP_IL_MUL] = p2_IL_MUL;
	pPaCtl -> op[OP_IL_NE] = p2_IL_NE;
	pPaCtl -> op[OP_IL_NEWLINE] = p2_IL_NEWLINE;
	pPaCtl -> op[OP_IL_NONFLIST_ARG] = p2_IL_NONFLIST_ARG;
	pPaCtl -> op[OP_IL_NONFLIST_BEGIN] = p2_IL_NONFLIST_BEGIN;
	pPaCtl -> op[OP_IL_NONFLIST_END] = p2_IL_NONFLIST_END;
	pPaCtl -> op[OP_IL_NOT] = p2_IL_NOT;
	pPaCtl -> op[OP_IL_OR] = p2_IL_OR;
	pPaCtl -> op[OP_IL_ORN] = p2_IL_ORN;
	pPaCtl -> op[OP_IL_R] = p2_IL_R;
	pPaCtl -> op[OP_IL_RET] = p2_IL_RET;
	pPaCtl -> op[OP_IL_RETC] = p2_IL_RETC;
	pPaCtl -> op[OP_IL_RETCN] = p2_IL_RETCN;
	pPaCtl -> op[OP_IL_S] = p2_IL_S;
	pPaCtl -> op[OP_IL_ST] = p2_IL_ST;
	pPaCtl -> op[OP_IL_STN] = p2_IL_STN;
	pPaCtl -> op[OP_IL_SUB] = p2_IL_SUB;
	pPaCtl -> op[OP_IL_XOR] = p2_IL_XOR;
	pPaCtl -> op[OP_IL_XORN] = p2_IL_XORN;
	pPaCtl -> op[OP_INIT_DIM_VAR] = p2_INIT_DIM_VAR;
	pPaCtl -> op[OP_INIT_SCALAR_VAR] = p2_INIT_SCALAR_VAR;
	pPaCtl -> op[OP_INT] = p2_INT;
	/* pPaCtl -> op[OP_LE] = p2_LE; */
	pPaCtl -> op[OP_LINT] = p2_LINT;
	pPaCtl -> op[OP_LREAL] = p2_LREAL;
	/* pPaCtl -> op[OP_LT] = p2_LT; */
	pPaCtl -> op[OP_LWORD] = p2_LWORD;
	pPaCtl -> op[OP_MAKE_SUBRANGE] = p2_MAKE_SUBRANGE;
	/* pPaCtl -> op[OP_MINUS] = p2_MINUS; */
	/* pPaCtl -> op[OP_MOD] = p2_MOD; */
	/* pPaCtl -> op[OP_NE] = p2_NE; */
	/* pPaCtl -> op[OP_NOT] = p2_NOT; */
	/* pPaCtl -> op[OP_OR] = p2_OR; */
	/* pPaCtl -> op[OP_POW] = p2_POW; */
	pPaCtl -> op[OP_PROGRAM_CLASS] = p2_PROGRAM_CLASS;
	pPaCtl -> op[OP_PROGRAM_CONSTCONF] = p2_PROGRAM_CONSTCONF;
	pPaCtl -> op[OP_PROGRAM_END] = p2_PROGRAM_END;
	pPaCtl -> op[OP_PROGRAM_ENDIOCONF] = p2_PROGRAM_ENDIOCONF;
	pPaCtl -> op[OP_PROGRAM_HEADER] = p2_PROGRAM_HEADER;
	pPaCtl -> op[OP_PROGRAM_INCONF] = p2_PROGRAM_INCONF;
	pPaCtl -> op[OP_PROGRAM_INSTANCE] = p2_PROGRAM_INSTANCE;
	pPaCtl -> op[OP_PROGRAM_OUTCONF] = p2_PROGRAM_OUTCONF;
	/* pPaCtl -> op[OP_RDFALSE] = p2_RDFALSE; */
	/* pPaCtl -> op[OP_RDFLOAT] = p2_RDFLOAT; */
	/* pPaCtl -> op[OP_RDINT] = p2_RDINT; */
	pPaCtl -> op[OP_RDSTRING] = p2_RDSTRING;
	/* pPaCtl -> op[OP_RDTRUE] = p2_RDTRUE; */
	/* pPaCtl -> op[OP_RDTIME] = p2_RDTIME; */
	pPaCtl -> op[OP_REAL] = p2_REAL;
	pPaCtl -> op[OP_RESOURCE] = p2_RESOURCE;
	pPaCtl -> op[OP_RESOURCE_ON] = p2_RESOURCE_ON;
	pPaCtl -> op[OP_SINT] = p2_SINT;
	/* pPaCtl -> op[OP_STARTEXPR] = p2_STARTEXPR; */
	pPaCtl -> op[OP_START_ARRAY_INIT] = p2_START_ARRAY_INIT;
	pPaCtl -> op[OP_START_DIM_ARRAY] = p2_START_DIM_ARRAY;
	pPaCtl -> op[OP_STRING] = p2_STRING;
	pPaCtl -> op[OP_STRINGEXPR] = p2_STRINGEXPR;
	/* pPaCtl -> op[OP_SUB] = p2_SUB; */
	pPaCtl -> op[OP_TIME] = p2_TIME;
	/* pPaCtl -> op[OP_TIMES] = p2_TIMES; */
	pPaCtl -> op[OP_TIME_OF_DAY] = p2_TIME_OF_DAY;
	pPaCtl -> op[OP_TYPE] = p2_TYPE;
	pPaCtl -> op[OP_TYPE_ARRAY] = p2_TYPE_ARRAY;
	pPaCtl -> op[OP_TYPE_FB] = p2_TYPE_FB;
	pPaCtl -> op[OP_TYPE_SCALAR] = p2_TYPE_SCALAR;
	pPaCtl -> op[OP_UDINT] = p2_UDINT;
	pPaCtl -> op[OP_UINT] = p2_UINT;
	pPaCtl -> op[OP_ULINT] = p2_ULINT;
	pPaCtl -> op[OP_USINT] = p2_USINT;
	pPaCtl -> op[OP_VAR_DECL] = p2_VAR_DECL;
	pPaCtl -> op[OP_VAR_DECL_AT] = p2_VAR_DECL_AT;
	pPaCtl -> op[OP_VAR_EXTERNAL] = p2_VAR_EXTERNAL;
	pPaCtl -> op[OP_VAR_GLOBAL] = p2_VAR_GLOBAL;
	pPaCtl -> op[OP_VAR_INPUT] = p2_VAR_INPUT;
	pPaCtl -> op[OP_VAR_LOCAL] = p2_VAR_LOCAL;
	pPaCtl -> op[OP_VAR_OUTPUT] = p2_VAR_OUTPUT;
	pPaCtl -> op[OP_VAR_START_DECL_RETAIN] = p2_VAR_START_DECL_RETAIN;
	pPaCtl -> op[OP_WORD] = p2_WORD;
	/* pPaCtl -> op[OP_XOR] = p2_XOR; */

	paInit();

}

static filescope_t *curr_scope;

void p2_init(void)
{
	curr_scope = main_status.scope_list;

	co_init();

	p2_restart();
}

int p2_start(file_action_t *act)
{
int rv;
char *file;

	st_reset_context(&string_context,"(strings)");

	p2_restart();

	file = act -> source;
	if (!rdf_open(file)) {
		err_error(LA_E_NOFILE,NULL);
		return 0;
	}

/* Inizializza il livello medio del generatore di codice. */

	if (! co_begin(act))
		return 0;

	curr_file_context = curr_scope -> global;
	local_context = curr_scope -> local;
	fb_context = (context_t *) 0;
	st_reset_context_stack();
	st_push_context(global_context);
	st_push_context(local_context);

	co_object_header();

	rv = paIEC1131();

	co_object_end();

	rdf_close();

	co_end();

	curr_scope = curr_scope -> next;

	return ! rdf.error;
}

