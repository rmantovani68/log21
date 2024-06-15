/*
* @(#) conf.h 1.1 Fri Oct 11 17:52:10 MET 1996
*
*  Macro di configurazione del pacchetto ILC.
*/

#ifndef _CONF_H_

/********************************************/
/* Definizioni relative al file "symtab.c". */
/********************************************/

/* Dimensione della tabella di hash "larga" */
#define MAX_HASH_LEN 128
/* Soglia per l'allargamento della tabella di hash. */
#define HASH_THRESHOLD (MAX_HASH_LEN / 2)
/* Soglia per la riorganizzazione di un albero binario. */
#define MAX_TREE_UNB 7
/* Numero massimo do cintesti di ricerca attivi. */
#define MAX_ACTIVE_CONTEXT 8

/* Dimensioni (in numero di elemeti) dei blocchi di allocazione dinamica
 di vari descrittori utilizzati dal compilatore. */
#define LEN_CONTEXT_BLOCK 100
#define LEN_SYMBOL_BLOCK 600
#define LEN_TYPE_BLOCK 100
#define LEN_VAR_BLOCK 500
#define LEN_INIVAL_BLOCK 500
#define LEN_PARAMLIST_BLOCK 200
#define LEN_FILESCOPE_BLOCK 30
#define LEN_STRING_BLOCK 50

/*******************************************/
/* Definizioni relative al file "cexpr.c". */
/*******************************************/

/* Numero massimo di dimensioni ammesse per un array. */

#define MAX_ARRAY_DIM 2

#define MAXIDLEN 80
#define MAXNUMLEN 80
#define MAXSTRINGLEN 100
#define MAXLINE 1000
#define NUM_VAL_ALLOC_BLOCK 100

/**********************************************/
/* Definizioni relative al file "compiler.c". */
/**********************************************/

/* Numero massimo di celle descriventi un operando (cioe`, massima
 complessita` di un operando non costante). */

#define MAX_CO_REG_ARGS 64

/* Numero massimo di argomenti ammessi nella rappresentazione interna
 di un'espressione. */

#define MAX_CO_EXPR_ARGS 3

/* Numero massimo di costrutti "RESOURCE". */

#define MAX_RESOURCE_COUNT 10

/* Numero massimo di costrutti "PROGRAM". */

#define MAX_PROGRAM_COUNT 10

/*******************************************/
/* Definizioni relative al file "pass1.c". */
/*******************************************/

/* Massimo livello di annidamento delle espressioni IL. */

#define MAX_IL_EXPR_LEVEL 16

/* Prefisso per la generazione del simbolo globale associato alle
 variabili statiche di una funzione. Deve contenere lettere minuscole. */

#define FN_STATICS_PREFIX "_f_"

#define _CONF_H_

#endif

