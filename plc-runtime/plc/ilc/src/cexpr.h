/*
* cexpr.h 1.0 Mon Jul 15 15:26:55 MET DST 1996
*
* Questo file contiene il valutatore di espresisoni costanti, agganciato
* al parser ed utilizzato sia al passo 1 che al passo 2.
*
* 15/07/96 GG 1.0 Prima stesura.
*/

#ifndef _CEXPR_H_

/* Struttura descrivente un operando. */

typedef struct _val_t {
	int type;
	struct _val_t PTR_ next;
	union {
		long l;
		double d;
		LPCHAR s;
		struct {
			unsigned int low;
			unsigned int high;
		} sr;
		type_t PTR_ t;
		LPCHAR str;
	} data;
} VAL, PTR_ LPVAL;

/* Puntatore allo stack degli operandi. */

extern LPVAL ce_op_stack;

/* Macro per l'accesso ai primi due operandi memorizzati nello stack. */

#define TOS ce_op_stack
#define NOS (ce_op_stack -> next)

/* Tipi di operando. */

enum type_t { TY_NONE, TY_INT, TY_DOUBLE, TY_STRING, TY_SUBRANGE, TY_NAME,
TY_TYPE, TY_ARRAY };


/*
* Funzione ce_stack_init
* -----------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Porta in uno stato consistente lo stack.
*/

void ce_stack_init(void);

/*
* Funzione ce_push
* ----------------
*
* Parametri :
*
*	Eventuale indirizzo de cui copiare i dati.
*
* Valori calcolati :
*
*	L'indirizzo del nuovo elemento allocato.
*
* Descrizione :
*
*  Crea un nuovo oggetto di tipo "VAL".
*/

LPVAL ce_push(LPVAL op);

/*
* Funzione ce_pop
* ----------------
*
* Parametri :
*
*	VAL * op : indirizzo su cui copiare il contenuto di TOS.
*
* Valori calcolati :
*
*	L'indirizzo dell'elemento appena liberato.
*
* Descrizione :
*
*  Copia TOS e lo rimuove.
*/

LPVAL ce_pop(LPVAL op);

/*
* Funzione ce_init
* ----------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Aggancia il valutatore alle callback del parser, ed inizializza lo stack.
*/

void ce_init(void);

void ce_STRINGEXPR(long);

#define _CEXPR_H_

#endif

