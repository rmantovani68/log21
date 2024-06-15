
/*
* @(#) emitter.h 1.3  Mon Jan 13 19:04:08 MET 1997
*
* Livello basso del processo di generazione del codice.
*
* 16/07/96 GG 1.0 Prima stesura.
* 28/11/96 GG 1.1 Aggiunti due tipi di operandi per l'emissione di
*             numeri decimali.
* 29/11/96 GG 1.2 Modifiche per la lettura di costanti stringa.
* 13/01/97 GG 1.3 Aggiunto il tipo "EM_VTYPE_HEXSTR", che serve ad emettere
*             una stringa come sequenza di cifre esadecimali.
*/


#ifndef _EMITTER_H_

#include "asmop.h"

#define MAX_DATA_PER_ASM_LINE 16

extern const char *em_regs[];

/* Codici astratti per i registri di macchina. */

enum _reg_t {
	R_NULL = 0,	/* Nessun registro */
	R_0,	/* %eax */
	R_1,	/* %ebx */
	R_2,	/* %ecx */
	R_3,	/* %edx */
	R_4,	/* %edi */
	R_5,	/* %esi */
	R_6,	/* %ebp */
	R_7,	/* %esp */

	R_0W,	/* %ax */
	R_1W,	/* %bx */
	R_2W,	/* %cx */
	R_3W,	/* %dx */

	R_0L,	/* %al */
	R_1L,	/* %bl */
	R_2L,	/* %cl */
	R_3L,	/* %dl */

	R_0H,	/* %ah */
	R_1H,	/* %bh */
	R_2H,	/* %ch */
	R_3H,	/* %dh */

	R_0C,	/* %cs */
	R_1C,	/* %ds */
	R_2C,	/* %es */
	R_3C,	/* %ss */
	R_4C,	/* %fs */
	R_5C,	/* %gs */

	MAX_REG_INDEX,

	R_ANY,	/* Indica "qualsiasi registro" */
	R_ANY_GENERIC,	/* Indica "qualsiasi registro generico" */
	R_ANY_INDEX,	/* Indica "qualsiasi registro indice" */
};

#define R_F_NOSAVE 0x1000

#define em_lowb(x) ((x) + R_0L - R_0)
#define em_loww(x) ((x) + R_0W - R_0)

#define MAX_REG_32 (R_7 + 1)

#define R_ACC R_0	/* Accumulatore */
#define R_CNT R_2	/* Contatore */
#define R_EAC R_3	/* Estensione accumulatore */
#define R_PST R_5	/* Puntatore all'area statica. */
#define R_BP  R_6	/* Puntatore al contesto. */
#define R_SP  R_7	/* Puntatore allo stack. */

/*
* Codici di funzione i386
*/

/*
* Tipi di operandi e struttura descrivente un operando.
*/

enum _em_op_t {
	EM_OP_NULL,
	EM_OP_IMM,
	EM_OP_REG,
	EM_OP_IND,
	EM_OP_DIR,
	EM_OP_IMMD,
	EM_OP_DIRD,
};

enum _em_vtype_t {
	EM_VTYPE_NONE,
	EM_VTYPE_NUM,
	EM_VTYPE_STR,
	EM_VTYPE_STROFF,
	EM_VTYPE_TLBL,
	EM_VTYPE_STRLBL,
	EM_VTYPE_HEXSTR
};

typedef struct {
	enum _em_vtype_t vtype;
	struct {
		char *s;
		long n;
	} v;
} em_val_t;

typedef struct {
	enum _em_op_t type;
	char r1;
	char r2;
	char scale;
	em_val_t val;
} em_opr_t;

/* FUNZIONI */

void em_init(void);

int em_start(file_action_t *act);

void em_end(void);

/**************************************************\
* EMISSIONE DI ETICHETTE, SEZIONI, VALORI INIZIALI *
\**************************************************/

void em_open_datalist(int size);

void em_close_datalist(void);

void em_add_datalist(char *val);

void em_addv_datalist(em_val_t *val, int hex);

void em_op(int op,...);

#define _EMITTER_H_

#endif

