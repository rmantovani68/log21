/* QUESTO FILE E` STATO GENERATO IN MODO AUTOMATICO A PARTIRE DA parser_c.h */
/* NON APPORTARE MODIFICHE QUI, MA SOLO IN parser_c.h */


/*
* @(#) parser.h 2.4 Mon Nov 20 20:54:46 MET 2000
* @(#) Rif. Guerrini
*
*  Parser per una sintassi simil-C.
*
* --/--/-- 2.0 Prima stesura.
* 02/07/96 2.1 Eliminati i parametri di tutte le azioni.
* 11/10/96 GG 2.2 Aggiunto il riconoscimento delle costanti stringa, per ora
*             solo come parametro di funzioni.
* 16/10/96 GG 2.3 Aggiunte callback di fine istruzione IL.
* 20/11/00 GG 2.4 Aggiunta callback di inizio istruzione IL.
*/


#ifndef _PARSER_H

#define _PARSER_H

#include "sysdep.h"

/* Operazioni possibili sugli operandi. */

enum op_t {
/* Per aggiungere nuovi codici di callback, rispettare le spaziature ! */
/* LISTA DEI CODICI DI CALLBACK */
	OP_ADD ,
	OP_ADD_DIM_ARRAY ,
	OP_AND ,
	OP_ARRAY_INIT_MULTI ,
	OP_ARRAY_INIT_SINGLE ,
	OP_BODY ,
	OP_BOOL ,
	OP_BYTE ,
	OP_CONFIGURATION ,
	OP_DATE ,
	OP_DATE_AND_TIME ,
	OP_DINT ,
	OP_DIV ,
	OP_DWORD ,
	OP_ENDEXPR ,
	OP_END_ARRAY_INIT ,
	OP_END_CONFIGURATION ,
	OP_END_DIM_ARRAY ,
	OP_END_PROGRAM_INSTANCE ,
	OP_END_RESOURCE ,
	OP_END_TYPE ,
	OP_END_VAR_DECL ,
	OP_END_VAR_EXTERNAL ,
	OP_END_VAR_GLOBAL ,
	OP_END_VAR_INPUT ,
	OP_END_VAR_LOCAL ,
	OP_END_VAR_OUTPUT ,
	OP_EQ ,
	OP_ERROR ,
	OP_FUNCTION_BLOCK_END ,
	OP_FUNCTION_BLOCK_HEADER ,
	OP_FUNCTION_END ,
	OP_FUNCTION_HEADER ,
	OP_GE ,
	OP_GT ,
	OP_IL_ADD ,
	OP_IL_AND ,
	OP_IL_ANDN ,
	OP_IL_BEGIN ,
	OP_IL_CAL ,
	OP_IL_CALC ,
	OP_IL_CALCN ,
	OP_IL_DIV ,
	OP_IL_END ,
	OP_IL_ENDLD ,
	OP_IL_ENDNULL ,
	OP_IL_ENDOPR ,
	OP_IL_STARTOP ,
	OP_IL_EQ ,
	OP_IL_EXPR_BEGIN ,
	OP_IL_EXPR_END ,
	OP_IL_FBCALL ,
	OP_IL_FCALL ,
	OP_IL_FNCALL ,
	OP_IL_FORMLIST_ARGREF ,
	OP_IL_FORMLIST_ARGVAL ,
	OP_IL_FORMLIST_BEGIN ,
	OP_IL_FORMLIST_END ,
	OP_IL_GE ,
	OP_IL_GT ,
	OP_IL_INDEX ,
	OP_IL_INDEX_BEGIN ,
	OP_IL_INDEX_END ,
	OP_IL_JMP ,
	OP_IL_JMPC ,
	OP_IL_JMPCN ,
	OP_IL_LABEL ,
	OP_IL_LD ,
	OP_IL_LDN ,
	OP_IL_LE ,
	OP_IL_LT ,
	OP_IL_LVAL_CREATE ,
	OP_IL_LVAL_EXTRACT ,
	OP_IL_MOD ,
	OP_IL_MUL ,
	OP_IL_NE ,
	OP_IL_NEWLINE ,
	OP_IL_NONFLIST_ARG ,
	OP_IL_NONFLIST_BEGIN ,
	OP_IL_NONFLIST_END ,
	OP_IL_NOT ,
	OP_IL_OR ,
	OP_IL_ORN ,
	OP_IL_R ,
	OP_IL_RET ,
	OP_IL_RETC ,
	OP_IL_RETCN ,
	OP_IL_S ,
	OP_IL_ST ,
	OP_IL_STN ,
	OP_IL_SUB ,
	OP_IL_XOR ,
	OP_IL_XORN ,
	OP_INIT_DIM_VAR ,
	OP_INIT_SCALAR_VAR ,
	OP_INT ,
	OP_LE ,
	OP_LINT ,
	OP_LREAL ,
	OP_LT ,
	OP_LWORD ,
	OP_MAKE_SUBRANGE ,
	OP_MINUS ,
	OP_MOD ,
	OP_NE ,
	OP_NOT ,
	OP_OR ,
	OP_POW ,
	OP_PROGRAM_CLASS ,
	OP_PROGRAM_CONSTCONF ,
	OP_PROGRAM_END ,
	OP_PROGRAM_ENDIOCONF ,
	OP_PROGRAM_HEADER ,
	OP_PROGRAM_INCONF ,
	OP_PROGRAM_INSTANCE ,
	OP_PROGRAM_OUTCONF ,
	OP_RDFALSE ,
	OP_RDFLOAT ,
	OP_RDINT ,
	OP_RDSTRING ,
	OP_RDTIME ,
	OP_RDTRUE ,
	OP_REAL ,
	OP_RESOURCE ,
	OP_RESOURCE_ON ,
	OP_SINT ,
	OP_STARTEXPR ,
	OP_START_ARRAY_INIT ,
	OP_START_DIM_ARRAY ,
	OP_STRING ,
	OP_STRINGEXPR ,
	OP_SUB ,
	OP_TIME ,
	OP_TIMES ,
	OP_TIME_OF_DAY ,
	OP_TYPE ,
	OP_TYPE_ARRAY ,
	OP_TYPE_FB ,
	OP_TYPE_SCALAR ,
	OP_UDINT ,
	OP_UINT ,
	OP_ULINT ,
	OP_USINT ,
	OP_VAR_DECL ,
	OP_VAR_DECL_AT ,
	OP_VAR_EXTERNAL ,
	OP_VAR_GLOBAL ,
	OP_VAR_INPUT ,
	OP_VAR_LOCAL ,
	OP_VAR_OUTPUT ,
	OP_VAR_START_DECL_RETAIN ,
	OP_WORD ,
	OP_XOR ,
/* FINE DELLA LISTA DEI CODICI DI CALLBACK */
/* Non aggiungere mai niente dopo questa : */
	MAXOP ,
};

/* Flag che infulenzano il funzionamento del parser/interprete. */

#define PA_F_PARSE	0x0001
#define PA_F_EVAL	0x0002
#define PA_F_QUIT	0x0004

/* Struttura di controllo del parser. */

typedef struct pactl_t {

/* Codice del primo errore rilevato. */
	int error;

/* Puntatore all'area temporanea il cui il parser scrive l'ultimo
 identificatore o parola chiave incontrati. La dimensione dell'area
 deve almeno superare di uno la lunghezza massima degli identificatori
 impostata nell'apposito campo della struttura di controllo dello scanner
 (campo "maxidlen"). */

	LPCHAR symstr;

/* Puntatore all'area temporanea in cui il parser scrive l'ultima
 stringa numerica (float) incontrata. La dimensione dell'area
 deve essere almeno "maxnumlen*3+5", dove "maxnumlen" e` la lunghezza
 massima di una stringa intera senza segno impostata nella struttura di
 controllo dello scanner. */

	LPCHAR numstr;

/* Puntatore all'area temporanea in cui il parser scrive l'ultima costante
 stringa incontrata. */

	LPCHAR stringstr;

/* Puntatore alla lunghezza dell'ultima costante stringa incontrata. */

	int *lenstr;

/* Vettore delle operazioni ammesse. */
	void (PTR_ op[MAXOP])(void);

} PACTL, PTR_ LPPACTL;

/* VARIABILI */

/* Puntatore GLOBALE alla struttura di controllo. */

extern LPPACTL pPaCtl;

/* FUNZIONI */

#endif

int paError(int n);
LPPACTL paInit(void);
int paIEC1131(void);
