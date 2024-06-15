
/*
* @(#) emitter.c 1.8 Mon Sep 22 10:05:45 MET DST 1997
*
* Livello basso del processo di generazione del codice.
*
* 16/07/96 GG 1.0 Prima stesura.
* 27/09/96 GG 1.1 Aggiunta la possibilita` di espansioni ricorsive nella
*          "em_op", tarmite il costrutto "%(n)", dove n e` l'indice del
*          codice operativo da espandere, espresso come stringa decimale.
* 16/10/96 GG 1.2 Condizionata a "gencode" la creazione del file d'uscita.
* 28/11/96 GG 1.3 Facilitata l'emissione di numeri decimali.
* 29/11/96 GG 1.4 Modifiche per la lettura di costanti stringa.
* 03/12/96 GG 1.5 Portato a 9 il numero di parametri di em_op.
* 05/12/96 GG 1.6 Portato a 9 il numero di parametri di em_op (sul serio,
*             stavolta !).
* 13/01/97 GG 1.7 Aggiunto il tipo "EM_VTYPE_HEXSTR", che serve ad emettere
*             una stringa come sequenza di cifre esadecimali.
* 22/09/97 GG 1.8 Un piccolo aggiustamento per compilazione su NT.
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
#include "pass2.h"
#include "cexpr.h"
#include "debug.h"
#include "errs.h"
#include "rdfile.h"
#include "emitter.h"

/*
* File contenente le definizioni delle parole chiave dell'assembler.
*/

#include "asmop.c"

/*
* VARIABILI LOCALI
*/

static FILE * em_file_handle = (FILE *) 0;
static int em_last_char = '\0';

/* FUNZIONI */

/****************************************************\
* INIZIALIZZAZIONE, APERTURA E CHIUSURA DELL'OGGETTO *
\****************************************************/

void em_init(void)
{
	em_last_char = '\0';
}

int em_start(file_action_t *act)
{
	if (act -> objformat != OF_ASM) {
		err_error(PA_E_UNSUPP,NULL);
		return 0;
	}

	em_last_char = '\0';

	if (main_status.gencode) {

		em_file_handle = fopen(act -> object,"w");

		if (! em_file_handle) {
			err_error(LA_E_NOFILE,act -> object);
			return 0;
		}
	}
	else {
		em_file_handle = (FILE *) 0;
	}

	return 1;
}

void em_end(void)
{
	if (em_file_handle) {
		fclose(em_file_handle);
		em_file_handle = (FILE *) 0;
	}
}

/****************************\
* FUNZIONI BASE DI SCRITTURA *
\****************************/

static void _em_emitch(int c)
{
	if (em_file_handle) {
		fputc(c,em_file_handle);
#ifdef DEBUG
		fflush(em_file_handle);
#endif
		em_last_char = c;
	}
}

static void _em_emit(char *s)
{
	while (*s) {
		_em_emitch(*s);
		++s;
	}
}

static void _em_emitxs(char *s)
{
static const char hstr[] = "0123456789ABCDEF";

	while (*s) {
		_em_emitch(hstr[(unsigned char)(*s) / 16]);
		_em_emitch(hstr[(unsigned char)(*s) % 16]);
		++s;
	}
}

#ifdef FFSS /* DEFINITA_MA_INUTILIZZATA */
static void _em_emitln(char *s)
{
	_em_emit(s);
	_em_emitch('\n');
}

static void _em_emittab(char *s)
{
	_em_emitch('\t');
	_em_emit(s);
}
#endif /* DEFINITA_MA_INUTILIZZATA */

static void _em_nl(void)
{
	if (em_last_char != '\n')
		_em_emitch('\n');
}

#ifdef FFSS /* DEFINITA_MA_INUTILIZZATA */
static void _em_emittln(char *s)
{
	_em_emitch('\t');
	_em_emit(s);
	_em_emitch('\n');
}
#endif /* DEFINITA_MA_INUTILIZZATA */

static void _em_emiti(long val)
{
char num[20];

	sprintf(num,"%ld",val);
	_em_emit(num);
}

static void _em_emith(long val, int sz)
{
char num[20];
char fmt[10];

	strcpy(fmt,"0x%0*lx");
	fmt[4] = '0' + sz;
	sprintf(num,fmt,val);
	_em_emit(num);
}

#ifdef FFSS /* DEFINITA_MA_INUTILIZZATA */
static void _em_emittabi(long val)
{
	_em_emitch('\t');
	_em_emiti(val);
}

static void _em_emitlni(long val)
{
	_em_emiti(val);
	_em_emitch('\n');
}

static void _em_emittlni(long val)
{
	_em_emitch('\t');
	_em_emiti(val);
	_em_emitch('\n');
}
#endif /* DEFINITA_MA_INUTILIZZATA */

/**************************************************\
* EMISSIONE DI ETICHETTE, SEZIONI, VALORI INIZIALI *
\**************************************************/

static int em_curr_dl_size = 0;
static int em_curr_dl_count = 0;
static int em_curr_dl_limit = 0;

void em_open_datalist(int size)
{
	em_curr_dl_size = size;
	em_curr_dl_count = 0;
	em_curr_dl_limit = MAX_DATA_PER_ASM_LINE / size;
	if (size == 1)
		em_curr_dl_limit /= 2;
}

void em_close_datalist(void)
{
	if (em_curr_dl_count)
		_em_nl();
	em_curr_dl_size = 0;
	em_curr_dl_count = 0;
}

static void _em_emitval(char *val)
{
	switch (em_curr_dl_size) {
	case 1:
		_em_emith(*(unsigned char *)val, 2);
		break;
	case 2:
		_em_emith(*(unsigned short *)val, 4);
		break;
	case 8:
		_em_emith( (long)(((unsigned long *)val)[0]), 8);
		_em_emitch(',');
		_em_emith( (long)(((unsigned long *)val)[1]), 8);
		break;
	case 4:
		_em_emith( (long)(*(unsigned long *)val), 8);
		break;
	default:
		break;
	}
}

static void _em_emitv(em_val_t *val, int hex)
{
em_opr_t tmp;
em_opr_t tmp2;
char tmpstr[20];

	switch (val -> vtype) {
	case EM_VTYPE_TLBL:
		tmp.type = EM_OP_DIR;
		tmp.val.vtype = EM_VTYPE_STR;
		sprintf(tmpstr,"%ld",val -> v.n);
		tmp.val.v.s = tmpstr;
		em_op(AOP_TMPID,&tmp);
		break;
	case EM_VTYPE_STRLBL:
		tmp.type = EM_OP_DIR;
		tmp.val.vtype = EM_VTYPE_STR;
		sprintf(tmpstr,"%ld",val -> v.n);
		tmp.val.v.s = tmpstr;
		em_op(AOP_STRID,&tmp);
		break;
	case EM_VTYPE_STR:
		_em_emit(val -> v.s);
		break;
	case EM_VTYPE_HEXSTR:
		_em_emitxs(val -> v.s);
		break;
	case EM_VTYPE_STROFF:
		tmp.type = EM_OP_DIR;
		tmp.val.v.s = val -> v.s;
		tmp.val.vtype = EM_VTYPE_STR;
		tmp2.type = EM_OP_DIR;
		tmp2.val.v.n = val -> v.n;
		tmp2.val.vtype = EM_VTYPE_NUM;
		em_op(AOP_SYMPLOFF,&tmp,&tmp2);
		break;
	case EM_VTYPE_NUM:
		if (hex)
			_em_emith(val -> v.n, 1);
		else
			_em_emiti(val -> v.n);
		break;
	default:
		break;
	}
}

static void _em_add_datalist_core(char *val, int hex)
{
	if (em_curr_dl_count == 0) {
		switch (em_curr_dl_size) {
		case 1:
			em_op(AOP_BYTE);
			break;
		case 2:
			em_op(AOP_SHORT);
			break;
		case 4:
		case 8:
			em_op(AOP_LONG);
			break;
		default:
			break;
		}
	}
	else {
		_em_emitch(',');
	}
	if (hex == -1)
		_em_emitval(val);
	else
		_em_emitv((em_val_t *)val,hex);
	++em_curr_dl_count;
	if (em_curr_dl_count >= em_curr_dl_limit) {
		_em_nl();
		em_curr_dl_count = 0;
	}
}

void em_add_datalist(char *val)
{
	_em_add_datalist_core(val,-1);
}


void em_addv_datalist(em_val_t *val, int hex)
{
	_em_add_datalist_core((char *)val,hex != 0);
}

/*************************\
* EMISSIONE DI OPERAZIONI *
\*************************/

const char *em_regs[] = {
"?",
"%eax", "%ebx", "%ecx", "%edx", "%edi", "%esi", "%ebp", "%esp",
"%ax", "%bx", "%cx", "%dx",
"%al", "%bl", "%cl", "%dl",
"%ah", "%bh", "%ch", "%dh",
"%cs", "%ds", "%es", "%ss", "%fs", "%gs",
};
static void _em_mkopr(em_opr_t *op)
{
	switch (op -> type) {
	case EM_OP_NULL:
		break;
	case EM_OP_IMM:
		_em_emitch('$');
		_em_emitv(&op -> val,1);
		break;
	case EM_OP_IMMD:
		_em_emitch('$');
		_em_emitv(&op -> val,0);
		break;
	case EM_OP_REG:
		_em_emit(em_regs[(int)(op -> r1)]);
		break;
	case EM_OP_DIR:
		_em_emitv(&op -> val,1);
		break;
	case EM_OP_DIRD:
		_em_emitv(&op -> val,0);
		break;
	case EM_OP_IND:
		if (op -> val.vtype != EM_VTYPE_NONE
		 && !(op -> val.vtype == EM_VTYPE_NUM && op -> val.v.n == 0))
			_em_emitv(&op -> val,0);
		_em_emitch('(');
		if (op -> scale != 1 && ! op -> r2) {
			_em_emitch(',');
		}
		if (op -> r1) {
			_em_emit(em_regs[(int)(op -> r1)]);
		}
		if (op -> r2) {
			_em_emitch(',');
			_em_emit(em_regs[(int)(op -> r2)]);
		}
		if (op -> scale != 1 || (! op -> r1 && ! op -> r2)) {
			_em_emitch(',');
			_em_emiti(op -> scale);
		}
		_em_emitch(')');
		break;
	default:
		break;
	}
}

static void _em_op_core(int op,va_list ap)
{
em_opr_t *opr[9];
char *s, *p;
char c;
int tab,nl,emop,arg,argp;

	s = asmop_list[op];

	tab = 1;
	nl = 1;
	emop = 0;
	arg = 0;
	argp = 0;
	for (p = s; (c = *p); ++p) {
		if (c == '%') {
			c = *++p;
			switch (c) {
			case 'b':
				tab = 0;
				c = '\0';
				break;
			case 't':
				c = ' ';
				break;
			case 'c':
				nl = 0;
				c = '\0';
				break;
			case 's':
				c = '\001';
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				arg = c - '1';
				emop = 1;
				c = '\0';
				break;
			case '(':
				_em_op_core(atoi(++p),ap);
				p = strchr(p,')');
				c = '\0';
				break;
			default:
				break;
			}
		}
		if (tab) {
			_em_emitch('\t');
			tab = 0;
		}
		if (emop) {
			for (; argp <= arg; ++argp) {
				opr[argp] = va_arg(ap,em_opr_t *);
			}
			emop = 0;
			_em_mkopr(opr[arg]);
		}
		else {
			if (c) {
				if (c == '\001')
					c = ' ';
				else if (c == ' ')
					c = '\t';
				_em_emitch(c);
			}
		}
	}
	if (nl)
		_em_nl();
}

void em_op(int op,...)
{
va_list ap;

	va_start(ap,op);
	_em_op_core(op,ap);
	va_end(ap);
}

