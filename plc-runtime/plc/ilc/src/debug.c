/*
* @(#) debug.c 1.0 Fri Jul 12 18:03:23 MET DST 1996
*
*  Questo file contiene funzioni utili a "capirci qualcosa".
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "conf.h"
#include "sysdep.h"
#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "cexpr.h"
#include "util.h"
#include "ilc.h"
#include "pass1.h"
#include "debug.h"

#ifdef DEBUG

void db_printf(const char *format,...)
{
va_list ap;

	if (main_status.debug & 0x02) {
		va_start(ap,format);
		vprintf( (char *)format,ap);
		va_end(ap);
	}
}

void db_putval(LPVAL op)
{
	if (!op)
		printf("(no op)");
	else if ((op) -> type == TY_INT)
		printf("(long) %ld = 0x%08lx",
		       ( op) -> data.l,
		       ( op) -> data.l);
	else if ((op) -> type == TY_NAME)
		printf("\"%s\"",(op) -> data.str);
	else
		printf("(double) %f",(op) -> data.d);
}

void db_printval(LPVAL op)
{
	if (!( main_status.debug & 0x02))
		return;
	db_putval(op);
	printf("\n");
}

static const char *db_tp_names[] = {
"NONE",
"BOOL",
"SINT",
"USINT",
"BYTE",
"INT",
"UINT",
"WORD",
"DINT",
"UDINT",
"DWORD",
"LINT",
"ULINT",
"LWORD",
"REAL",
"LREAL",
"TIME",
"DATE",
"TOD",
"DT",
};

void db_dump_arglist(type_t * tp)
{
paramlist_t *pl;

	printf(" (\n\t(* input *)\n");
	for (pl = tp -> d.fb.input; pl; pl = pl -> next) {
		printf("\t\t");
		db_dump_var(pl -> sym);
	}
	if (tp -> code != TYPE_FUNCTION) {
		printf("\t(* output *)\n");
		for (pl = tp -> d.fb.output; pl; pl = pl -> next) {
			printf("\t\t");
			db_dump_var(pl -> sym);
		}
	}
	printf("\t)");
}

void db_dump_type(type_t * tp)
{
int code,i;

	if (! tp) {
		printf("(null)");
		return;
	}

	while (tp) {
		code = tp -> code;
		if (code >= TYPE_NONE && code <= TYPE_DT) {
			printf("%s",db_tp_names[code]);
			break;
		}
		switch (code) {
		case TYPE_STRING:
			printf("STRING[%d]",tp -> d.str_len);
			break;
		case TYPE_USER:
			printf("(user)");
			break;
		case TYPE_STRUCT:
			printf("STRUCT (...)");
			break;
		case TYPE_PROGRAM:
			printf("PROGRAM");
			db_dump_arglist(tp);
			break;
		case TYPE_FUNCTION_BLOCK:
			printf("FUNCTION_BLOCK");
			db_dump_arglist(tp);
			break;
		case TYPE_FUNCTION:
			printf("FUNCTION");
			db_dump_arglist(tp);
			printf(" ");
			tp = tp -> d.fb.rv_type;
			continue;
		case TYPE_CONFIGURATION:
			printf("CONFIGURATION");
			break;
		case TYPE_RESOURCE:
			printf("RESOURCE");
			break;
		case TYPE_ARRAY:
			printf("ARRAY");
			for (i = 0; i < tp -> d.a.n_dim; ++i)
				printf("%c%d..%d",
				       i == 0 ? '[' : ',',
				       tp -> d.a.idx_base[i],
				       tp -> d.a.idx_base[i]
				        + tp -> d.a.dim[i] - 1);
			printf("] OF ");
			tp = tp -> d.a.type;
			continue;
		default:
			printf("(undefined)");
			break;
		}
		tp = (type_t *) 0;
	}
}

void db_dump_types(symbol_t * sym)
{
	if (! sym) {
		printf("(null)\n");
		return;
	}

	if (sym -> class != SYMCL_TYPE)
		return;

	printf(" %s ",sym -> name);

	db_dump_type(sym -> type);

	printf(" ;\n");
}

void db_dump_var(symbol_t * sym)
{
inival_t *ip;
int re,bi,un;

	if (! sym) {
		printf("(null)\n");
		return;
	}

	if (sym -> class != SYMCL_VAR)
		return;

	printf(" %s ",sym -> name);
	switch (sym -> var -> flags & VAR_F_CONNECTION_M) {
	case VAR_F_CONNECTION_IN:
	case VAR_F_CONNECTION_OUT:
	case VAR_F_CONNECTION_MEMORY:
		printf("AT %s", sym -> var -> at.direct_io);
		break;
	default:
		break;
	}
	printf(" : ");

	db_dump_type(sym -> type);

	if (sym -> var -> flags & VAR_F_INIT) {
		printf(" := ");
		if (st_is_array_type(sym -> type)) {
			re = st_is_float_type(sym -> type -> d.a.type);
			bi = st_is_binary_type(sym -> type -> d.a.type);
			un = st_is_unsigned_type(sym -> type -> d.a.type);
			printf("[\n");
			for (ip = sym -> var -> init.a; ip; ip = ip -> next) {
				printf("\t\t");
				if (ip -> count != 1) {
					printf("%d(",ip -> count);
				}
				if (re)
					printf("%f",ip -> data.d);
				else if (bi)
					printf("0x%lx",ip -> data.l);
				else if (un)
					printf("%lu",ip -> data.l);
				else
					printf("%ld",ip -> data.l);
				if (ip -> count != 1) {
					printf(")");
				}
				printf("\n");
			}
			printf("\t]");
		}
		else {
			re = st_is_float_type(sym -> type);
			bi = st_is_binary_type(sym -> type);
			un = st_is_unsigned_type(sym -> type);
			if (re)
				printf("%f",sym -> var -> init.d);
			else if (bi)
				printf("0x%lx",sym -> var -> init.l);
			else if (un)
				printf("%lu",sym -> var -> init.l);
			else
				printf("%ld",sym -> var -> init.l);
		}
	}
	if (!(sym -> var -> flags & VAR_F_DEFINED))
		printf(" (* EXTERN *)");
	if (!(sym -> var -> flags & VAR_F_USED))
		printf(" (* UNUSED *)");
	printf(" ;\n");
}

static int _db_dump_var(context_t * context, symbol_t *sym, void *user)
{
	db_dump_var(sym);
	return 1;
}

static int _db_dump_types(context_t * context, symbol_t *sym, void *user)
{
	db_dump_types(sym);
	return 1;
}

void db_dump_context(context_t * context)
{
	printf("(* CONTEXT \"%s\" *)\n", context -> name);
	printf("(* Variables *)\n");
	st_explore_context(context,_db_dump_var,NULL);
	printf("(* Types and procedures *)\n");
	st_explore_context(context,_db_dump_types,NULL);
	printf("(* END OF CONTEXT \"%s\" *)\n", context -> name);
}

void db_dump_context_chain(void)
{
filescope_t *fs;

	if (main_status.debug & 16) {
		for (fs = main_status.scope_list; fs; fs = fs -> next) {
			printf("\nFILE \"%s\"\n- GLOBAL CONTEXT -\n\n",
			       fs -> name);
			db_dump_context(fs -> global);
			printf("\n- LOCAL CONTEXT -\n\n");
			db_dump_context(fs -> local);
		}
	}
}

#endif

