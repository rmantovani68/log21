/*
* cexpr.c 1.3 Wed Jun  4 10:39:18 MET DST 1997
*
* Questo file contiene il valutatore di espresisoni costanti, agganciato
* al parser ed utilizzato sia al passo 1 che al passo 2.
*
* 15/07/96 GG 1.0 Prima stesura.
* 29/08/96 GG 1.1 Aggiunta la lettura del tipo "TIME".
* 29/11/96 GG 1.2 Modifiche per la lettura di costanti stringa.
*             Rappezzato un "buco" della funzione "strtol", che non si
*             comporta bene con esadecimali maggiori di 0x7FFFFFFF.
* 04/06/96 GG 1.3 Aggiunta la lettura delle costanti "TIME" in forma
*             decimale.
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
#include "cexpr.h"
#include "debug.h"
#include "errs.h"
#include "rdfile.h"

/* Puntatore alla lista di operandi liberi. */

static LPVAL ce_op_free = (LPVAL) 0;

/* Puntatore allo stack degli operandi. */

LPVAL ce_op_stack = (LPVAL) 0;

/* Livello di annidamento raggiunto. */

static int ce_stack_level = 0;

/*
* Funzione agganciata agli errori.
*/

static void ce_ERROR(void)
{
	err_error(pPaCtl -> error,NULL);
}

/*****************************************************\
* Funzioni per la gestione dello stack di valutazione *
\*****************************************************/

/*
*  Porta in uno stato consistente lo stack.
*/

void ce_stack_init(void)
{
	if (ce_op_stack) {
		ce_op_stack -> next = ce_op_free;
		ce_op_free = ce_op_stack;
		ce_op_stack = (LPVAL) 0;
	}
	ce_stack_level = 0;
}

/*
*  Crea un nuovo oggetto di tipo "VAL" in cima allo stack.
*/

LPVAL ce_push(LPVAL op)
{
register LPVAL r;
int i;

	if (! ce_op_free) {
		ce_op_free = (LPVAL)util_malloc(sizeof(VAL)
		                                 * NUM_VAL_ALLOC_BLOCK);
		for (i = 0; i < NUM_VAL_ALLOC_BLOCK - 1; ++i) {
			ce_op_free[i].next = &ce_op_free[i + 1];
		}
		ce_op_free[NUM_VAL_ALLOC_BLOCK - 1].next = (LPVAL) 0;
	}

	r = ce_op_free;
	ce_op_free = r -> next;
	if (op)
		*r = *op;
	else
		r -> type = TY_NONE;
	r -> next = ce_op_stack;
	ce_op_stack = r;

	++ce_stack_level;

	return r;
}

/*
*  Rimuove e, se op != NULL, copia TOS.
*/

LPVAL ce_pop(LPVAL op)
{
register LPVAL r;

	r = ce_op_stack;
	if (! r) {
		if (op)
			op -> type = TY_NONE;
		return (LPVAL) 0;
	}
	if (op)
		*op = *r;
	ce_op_stack = r -> next;
	r -> next = ce_op_free;
	ce_op_free = r;

	--ce_stack_level;

	return r;
}


/************************************\
* VALUTATORE DI ESPRESSIONI COSTANTI *
\************************************/

/*
* Le funzioni raccolte in questa sezione costituiscono il valutatore
* di espressioni costanti.
*/

/*
*  Dereferenzia l'oggetto (insomma, legge la variabile codificata,
* se l'oggetto non e` gia` di tipo numerico).
*
*  Per ora non c'e` bisogno che faccia nulla di particolare (tutte le
* grandezze sono costanti). Ma se le cose si dovessero complicare un
* po'...
*/

static void ce_eval(LPVAL op)
{
	return;
}

/*
*  Dereferenzia gli oggetti (vedi ce_eval) e li promuove al tipo del
* piu` "alto" (cioe`, se uno dei due e` un double, entrambi lo diventano).
*/

static void ce_promote(void)
{
long l;

	ce_eval(NOS);
	ce_eval(TOS);

	if (NOS -> type == TY_DOUBLE
	    && TOS -> type != TY_DOUBLE) {

		l = TOS -> data.l;
		TOS -> data.d = (double) l;
		TOS -> type = TY_DOUBLE;
	}
	else if (NOS -> type != TY_DOUBLE
	    && TOS -> type == TY_DOUBLE) {

		l = NOS -> data.l;
		NOS -> data.d = (double) l;
		NOS -> type = TY_DOUBLE;
	}
}

/*
*  Dereferenzia gli oggetti (vedi ce_eval) e li converte in INT.
*/

static void ce_convert_to_int(void)
{
double d;

	ce_eval(NOS);
	ce_eval(TOS);

	if (NOS -> type == TY_DOUBLE) {
		d = NOS -> data.d;
		NOS -> data.l = (long) d;
		NOS -> type = TY_INT;
	}
	if (TOS -> type == TY_DOUBLE) {
		d = TOS -> data.d;
		TOS -> data.l = (long) d;
		TOS -> type = TY_INT;
	}
}

/*
*  Questa funzione e` chiamata ogni volta che il parser incontra
* l'inizio di un costrutto di tipo "Expression".
*/

static void ce_STARTEXPR(void)
{
/*
	ce_stack_init();
*/
	return;
}

/*
*  Questa funzione e` chiamata ogni volta che il parser incontra
* la fine di un costrutto di tipo "Expression".
*/

static void ce_ENDEXPR(void)
{
	db_printf("=== TOS = ");
	db_printval(TOS);
}

/*
*  Assegna al TOS il valore del float in "rdf.number".
*/

static void ce_RDFLOAT(void)
{
register LPVAL r;

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_DOUBLE;
	r -> data.d = atof(rdf.number);
}

/*
*  Assegna al TOS il valore dell'intero "rdf.number", prodotto dallo scanner.
*/

static void ce_RDINT(void)
{
register LPVAL r;
long l;
unsigned long v;
int base;
LPCHAR p;
char c;

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_INT;

/* Il primo byte contiene la base, calcolata dallo scanner. */

	base = rdf.number[0];
	p = &rdf.number[1];

	if (*p == '-') {
		l = strtol(p,(char **)0,base);
	}
	else {
		v = 0;
		while ( (c = *(p++)) ) {
			if (c > '9')
				c -= 'A' - 10;
			else
				c -= '0';
			v = v * base + c;
		}
		l = (long)v;
	}

	r -> data.l = l;

	return;
}

/*
*  Assegna al TOS il valore del TIME "rdf.number", prodotto dallo scanner.
*/

static void ce_RDTIME(void)
{
register LPVAL r;
long l,v;
int unit;
LPCHAR p,q;
float f;
static long tconv[] = { 24L*60*60*1000, 60L*60*1000, 60L*1000, 1000L, 1L };

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_INT;

	l = 0;
	unit = 0;
	q = rdf.number;

	do {
		f = 0.0;
		p = q;
		v = strtol(p,&q,10);
		if (*q == '.') {
			f = atof(q);
			p = ++q;
			strtol(p,&q,10);
		}
		l += (long)(((float)v + f)*tconv[unit]);
		++unit;
	} while (p != q);
	
	r -> data.l = l;

	return;
}

/*
*  Assegnano al TOS i valori FALSE e TRUE.
*/

static void ce_RDFALSE(void)
{
register LPVAL r;

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_INT;
	r -> data.l = 0;
	return;
}


static void ce_RDTRUE(void)
{
register LPVAL r;

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_INT;
	r -> data.l = 1;
	return;
}


/*
* Genera un valore di tipo stringa.
*/

void ce_STRINGEXPR(long n)
{
register LPVAL r;

	if (! ((r = ce_push((LPVAL) 0))) )
		return;
	r -> type = TY_STRING;
	r -> data.l = n;
}


/*
*  Operazione unaria "-". Poco o nulla da dire.
*/

static void ce_MINUS(void)
{
	ce_eval(TOS);
	if (TOS -> type == TY_INT)
		TOS -> data.l = - (TOS -> data.l);
	else
		TOS -> data.d = - (TOS -> data.d);
	return;
}

/*
*  Operazioni unarie. L'unica realizzata e` NOT.
*/

static void ce_NOT(void)
{
	ce_eval(TOS);
	if (TOS -> type == TY_INT)
		TOS -> data.l = ! (TOS -> data.l);
	else
		TOS -> data.d = (double)((TOS -> data.d) != 0.0);
	return;
}

/*
* Operatori binari "**", "+", "-", "*", "/", "MOD".
*/

static void ce_POW(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l =
		  (long)(0.5
		         + pow((double)NOS -> data.l, (double)TOS -> data.l));
	else
		NOS -> data.d = pow(NOS -> data.d, TOS -> data.d);
	ce_pop((LPVAL) 0);
}

static void ce_ADD(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l += (TOS -> data.l);
	else
		NOS -> data.d += (TOS -> data.d);
	ce_pop((LPVAL) 0);
}

static void ce_SUB(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l -= (TOS -> data.l);
	else
		NOS -> data.d -= (TOS -> data.d);
	ce_pop((LPVAL) 0);
}

static void ce_TIMES(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l *= (TOS -> data.l);
	else
		NOS -> data.d *= (TOS -> data.d);
	ce_pop((LPVAL) 0);
}

static void ce_DIV(void)
{
	ce_promote();
	if (NOS -> type == TY_INT) {
		if ((TOS -> data.l) == 0) {
			err_error(LA_E_DIV0,NULL);
			ce_pop((LPVAL) 0);
			return;
		}
		NOS -> data.l /= (TOS -> data.l);
	}
	else {
		if ((TOS -> data.d) == 0.0) {
			err_error(LA_E_DIV0,NULL);
			ce_pop((LPVAL) 0);
			return;
		}
		NOS -> data.d /= (TOS -> data.d);
	}
	ce_pop((LPVAL) 0);
}

static void ce_MOD(void)
{
	ce_promote();
	if (NOS -> type == TY_INT) {
		if ((TOS -> data.l) == 0) {
			err_error(LA_E_MOD0,NULL);
			ce_pop((LPVAL) 0);
			return;
		}
		NOS -> data.l %= TOS -> data.l;
	}
	else {
		err_error(LA_E_MODFLOAT,NULL);
	}
	ce_pop((LPVAL) 0);
}

/*
*  Operatori di relazione. Poco o nulla da dire.
*/

static void ce_EQ(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l == TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d == TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

static void ce_NE(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l != TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d != TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

static void ce_GT(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l > TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d > TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

static void ce_GE(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l >= TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d >= TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

static void ce_LT(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l < TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d < TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

static void ce_LE(void)
{
	ce_promote();
	if (NOS -> type == TY_INT)
		NOS -> data.l = NOS -> data.l <= TOS -> data.l;
	else
		NOS -> data.l = NOS -> data.d <= TOS -> data.d;
	NOS -> type = TY_INT;
	ce_pop((LPVAL) 0);
}

/*
* Operatori binari "AND", "OR", "XOR" (bit a bit).
*/

static void ce_AND(void)
{
	ce_convert_to_int();
	NOS -> data.l = NOS -> data.l & TOS -> data.l;
	ce_pop((LPVAL) 0);
}

static void ce_OR(void)
{
	ce_convert_to_int();
	NOS -> data.l = NOS -> data.l | TOS -> data.l;
	ce_pop((LPVAL) 0);
}

static void ce_XOR(void)
{
	ce_convert_to_int();
	NOS -> data.l = NOS -> data.l ^ TOS -> data.l;
	ce_pop((LPVAL) 0);
}

/*
*  Inizializzazione del valutatore di espressioni costanti.
*  Aggancia il valutatore alle callback del parser relative ai
* costrutti facenti capo alla regola "ConstantExpression",
* ed inizializza lo stack.
*/

void ce_init(void)
{
	ce_op_free = ce_op_stack = (LPVAL) 0;
	ce_stack_level = 0;

	pPaCtl -> op[OP_ADD] = ce_ADD;
	pPaCtl -> op[OP_AND] = ce_AND;
	pPaCtl -> op[OP_DIV] = ce_DIV;
	pPaCtl -> op[OP_ENDEXPR] = ce_ENDEXPR;
	pPaCtl -> op[OP_EQ] = ce_EQ;
	pPaCtl -> op[OP_ERROR] = ce_ERROR;
	pPaCtl -> op[OP_GE] = ce_GE;
	pPaCtl -> op[OP_GT] = ce_GT;
	pPaCtl -> op[OP_LE] = ce_LE;
	pPaCtl -> op[OP_LT] = ce_LT;
	pPaCtl -> op[OP_MINUS] = ce_MINUS;
	pPaCtl -> op[OP_MOD] = ce_MOD;
	pPaCtl -> op[OP_NE] = ce_NE;
	pPaCtl -> op[OP_NOT] = ce_NOT;
	pPaCtl -> op[OP_OR] = ce_OR;
	pPaCtl -> op[OP_POW] = ce_POW;
	pPaCtl -> op[OP_RDFALSE] = ce_RDFALSE;
	pPaCtl -> op[OP_RDFLOAT] = ce_RDFLOAT;
	pPaCtl -> op[OP_RDINT] = ce_RDINT;
	pPaCtl -> op[OP_RDTRUE] = ce_RDTRUE;
	pPaCtl -> op[OP_RDTIME] = ce_RDTIME;
	pPaCtl -> op[OP_STARTEXPR] = ce_STARTEXPR;
	pPaCtl -> op[OP_SUB] = ce_SUB;
	/* pPaCtl -> op[OP_STRINGEXPR] = ce_STRINGEXPR; */
	pPaCtl -> op[OP_TIMES] = ce_TIMES;
	pPaCtl -> op[OP_XOR] = ce_XOR;
}

