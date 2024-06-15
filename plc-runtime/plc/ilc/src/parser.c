/*
* @(#) parser.c 2.12 Wed Jan 22 17:48:59 MET 1997
* @(#) Rif. Guerrini
*
*  Parser per compilatore PLC a norme IEC1131-3.
*
* 26/06/96 GG 1.0 Prima stesura.
* 02/07/96 GG 2.0 Eliminati i parametri dalle azioni.
* 05/07/96 GG 2.1 Aggiunta la definizione di tipi (solo sintassi).
*                 Condizionato ad un flag il riconoscimento di espressioni
*                 costanti complesse. Aggiunta la possibilita` di indicare
*                 la parola chiave "CONSTANT" all'inizio di un'espressione
*                 costante complessa, per evitare ambiguita` nell'uso della
*                 parentesi tonda aperta iniziale in operandi del linguaggio
*                 IL.
* 10/07/96 GG 2.2 Aggiunta la possibilita` di definire variabili con
*                 scope limitato al file (estensione CNi).
*                 Spostata l'invocazione della callback "OP_END_DIM_ARRAY"
*                 prima della lettura del tipo base.
* 11/07/96 GG 2.3 Evitato il riconoscimento di "VAR_OUTPUT" all'interno di
*                 funzioni.
* 12/07/96 GG 2.4 Raccolta la chiamata di callback nella funzione interna
*                 "paCallback" per facilitare il debug.
* 15/07/96 GG 2.5 Raccolte altrove le definizioni degli errori.
* 23/09/96 GG 2.6 Disattivata (per ora) la possibilita` di indicare una lista
*                 formale di parametri per i blocchi funzionali.
* 11/10/96 GG 2.7 Aggiunto il riconoscimento delle costanti stringa, per ora
*                 solo come parametro di funzioni.
* 16/10/96 GG 2.8 Corretta la sintassi dell'istruzione IL "NOT", che non
*                 vuole operandi.
* 16/10/96 GG 2.9 Aggiunte callback per la fine delle istruzioni IL.
* 29/11/96 GG 2.10 Modifiche per permettere l'inizializzazione di variabili
*             di tipo stringa.
* 22/01/97 GG 2.11 Migliorata la diagnostica nel caso di RETAIN+AT. Peccato,
*             perche` alcuni controlli, pur possibili a livello sintattico,
*             sono stati trasferiti nella parte "semantica" (pass1.c e pass2.c).
* 22/01/97 GG 2.12 Cambiata la sintassi delle dichiarazioni RETAIN. La
*             parola chiave RETAIN deve seguire immediatamente VAR,
*             VAR_GLOBAL e VAR_OUTPUT, e si intende applicata a tutte
*             le variabili dichiarate fino ad END_VAR.
*/

#include <stdio.h>
#include <stdlib.h>

#include "conf.h"
#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "ilc.h"
#include "errs.h"

#include "parser_p.h"

/* VARIABILI */

/* Struttura di controllo del parser/interprete. */

static PACTL paCtl;

/* Puntatore GLOBALE alla struttura di controllo. */

LPPACTL pPaCtl = & paCtl;

/* Parole chiave. */
#include "keywords.c"
/* Stringhe contenenti i nomi delle callback, per debug. */

#ifdef DEBUG
#include "parser_op.c"
#endif

/* FUNZIONI */

/*
* Funzione paCallback
* -------------------
*
* Parametri :
*
*	code : codice della callback da chiamare.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Questa funzione chiama la callback indicata dal codice "code".
*/

static void paCallback(int code)
{
#ifdef DEBUG
	if (main_status.debug & 0x08) {
	char *p;
	int i;
		printf("%s + \"",pa_cb_names[code]);
		for (p = pScCtl -> curr, i = 0; *p && i < 30; ++p, ++i) {
			if (*p >= ' ')
				putchar(*p);
			else
				putchar(' ');
		}
		printf("%s\"\n",*p ? "..." : "");
	}
	else {
		if (main_status.debug & 0x04)
			printf("%s\n",pa_cb_names[code]);
	}
#endif

	(*pPaCtl -> op[code])();
}

/*
* Funzione paError
* ----------------
*
* Parametri :
*
*	int err : codice di errore prodotto dal parser.
*
* Valori calcolati :
*
*	Valore di ritorno : il risultato (inteso come valore booleano) della
*		funzione d'errore definita dall'utente.
*
* Descrizione :
*
*  Questa funzione Registra l'errore rilevato nell'apposito campo
* della struttura di controllo, e chiama il gestore d'errore definito
* esternamente.
*/

int paError(int n)
{
	pPaCtl -> error = n;
	paCallback(OP_ERROR);
	return 1;
}

/*
* Funzione paUndef
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
*  Questa funzione provoca l'errore "PA_E_UNSUPP", che indica che si
* e` tentato di eseguire un'operazione non gestita. La funzione "paInit"
* associa questa funzione ad ogni operazione il cui puntatore nella
* struttura di controllo sia trovato uguale a NULL.
*/

static void paUndef(void)
{
	paError(PA_E_UNSUPP);
}

/*
* Funzione paInit
* ---------------
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
*  Questa funzione inizializza la parte non indipendente della struttura
* di controllo del parser, e assegna valori di default ai campi indipendenti
* non inizializzati. Il programma chiamante, prima di chiamare questa funzione,
* dovrebbe riempire opportunamente i campi di interesse della struttura,
* accedendovi tramite il puntatore globale "pPaCtl". In particolare,
* dovrebbero essere forniti i puntatori alle funzioni associate
* agli operatori ed ai vari costrutti sintattici. "NULL" e` interpretato
* come "funzione non supportata", ed e` sostituito dall'indirizzo della
* funzione di errore "paUndef".
*/

LPPACTL paInit(void)
{
int i;

	for (i = 0; i < MAXOP; ++i) {
		if (! (pPaCtl -> op[i]))
			pPaCtl -> op[i] = paUndef;
	}

	pPaCtl -> error = PA_E_OK;

	pScCtl -> lf_blank = 1;

	scRegisterKeywords(kwAll,sizeof(kwAll)/sizeof(kwAll[0]));

	return (LPPACTL) 0;
}

#ifdef FFSS
static int paJunk(void)
{
	return scIdent(pPaCtl -> symstr)
	    || scKeyword() != -1
	    || scMatchSet(".+-*/;:'#$@!^&(){}[],?<>=\\|\n%")
	    || scInteger(pPaCtl -> numstr);
}
#endif /* FFSS */

/*
* Riconoscimento di costrutti IL
* ------------------------------
*/

static int paILIndex(void)
{ 
	if (scIdent(pPaCtl -> symstr)) {
		paCallback(OP_IL_LVAL_CREATE);
		return 1;
	}

	return paConstantExpression();
}

static int paILIndexList(void)
{ 
	if (! paILIndex())
		return 0;
	paCallback(OP_IL_INDEX);
	while (scMatchCh(',')) {
		if (! paILIndex()) {
			paError(PA_E_NINDEX);
			return 1;
		}
		paCallback(OP_IL_INDEX);
	}

	return 1;

}

static int paILLValue(void)
{ 
	if (! scIdent(pPaCtl -> symstr))
		return 0;

	paCallback(OP_IL_LVAL_CREATE);

	if (scMatchCh('.')) {
		if (! scIdent(pPaCtl -> symstr)) {
			paError(PA_E_NID);
			return 1;
		}
		paCallback(OP_IL_LVAL_EXTRACT);
	}
	if (scMatchCh('[')) {
		paCallback(OP_IL_INDEX_BEGIN);
		if (! paILIndexList()) {
			paError(PA_E_NINDEX);
		}
		paCallback(OP_IL_INDEX_END);
		if (! scMatchCh(']')) {
			paError(PA_E_NCSQ);
			return 1;
		}
	}

	return 1;

}

static int paILConstantExpression(void)
{
	if (scMatchKeyword(KT_CONSTANT)) {
		if (! paConstantExpression()) {
			paError(PA_E_NOPR);
		}
		return 1;
	}
	else {
		return paConstantExpression();
	}
}

#if 1
#define ALWAYS_LOAD_STRINGS
#endif

static int paILArgument(void)
{ 
#ifdef ALWAYS_LOAD_STRINGS
	return paString() || paILConstantExpression() || paILLValue();
#else
	return paILConstantExpression() || paILLValue();
#endif
}

static int paILExtendedArgument(void)
{ 
#ifdef ALWAYS_LOAD_STRINGS
	return paILArgument();
#else
	return paString() || paILArgument();
#endif
}

static int paILLabel(void)
{ 
	if (! scIdent(pPaCtl -> symstr))
		return 0;
	if (! scMatchCh(':')) {
		scBackToken();
		return 0;
	}

	paCallback(OP_IL_LABEL);

	return 1;
}

static int paILAssign(void)
{
	if (! paILLValue())
		return 0;

	if (! scMatch(":=")) {
		paError(PA_E_NASGN);
	}

	paCallback(OP_IL_FORMLIST_ARGREF);

	if (! paILExpr()) {
		paError(PA_E_NOPR);
	}

	paCallback(OP_IL_FORMLIST_ARGVAL);

	return 1;
}

static int paILFormalList(void)
{
	if (scMatchCh('(')) {
		paCallback(OP_IL_FORMLIST_BEGIN);
		if (! scMatchCh('\n')) {
			paError(PA_E_NEOL);
		}
		while (paILAssign()) {
			if (! scMatchCh('\n')) {
				paError(PA_E_NEOL);
				break;
			}
		}
		if (! scMatchCh(')')) {
			paError(PA_E_NCP);
		}
		paCallback(OP_IL_FORMLIST_END);
	}
	else {
		paCallback(OP_IL_NONFLIST_BEGIN);
		while (paILExtendedArgument()) {
			paCallback(OP_IL_NONFLIST_ARG);
			if (! scMatchCh(','))
				break;
		}
		paCallback(OP_IL_NONFLIST_END);
	}

	return 1;
}

static int paILExpr(void)
{
	if (scMatchCh('(')) {
		paCallback(OP_IL_EXPR_BEGIN);
		if (paILArgument()) {
			paCallback(OP_IL_LD);
		}
		if (! scMatchCh('\n')) {
			paError(PA_E_NEOL);
		}
		if (main_status.standard) {
			while (paILStatement()) {
			}
		}
		else {
			while (paILInstruction()) {
			}
		}
		if (! scMatchCh(')')) {
			paError(PA_E_NCP);
		}
		paCallback(OP_IL_EXPR_END);
	}
	else if (! paILArgument()) {
		return 0;
	}

	return 1;
}

static int paILStatement(void)
{
int k;
int op,o2;
enum { _OPR_ALL, _OPR_LVAL, _OPR_LABEL, _OPR_NONE, _OPR_NOEXPR,
       _OPR_CALL, _OPR_FCAL } opr;


	if (scMatchCh('\n'))
		return 1;

	k = scKeyword();

	opr = _OPR_ALL;
	o2 = OP_IL_ENDOPR;

	switch (k) {
	case KT_LD    : op = OP_IL_LD   ; o2 = OP_IL_ENDLD ; opr = _OPR_NOEXPR ; break;
	case KT_LDN   : op = OP_IL_LDN  ; o2 = OP_IL_ENDLD ; opr = _OPR_NOEXPR ; break;

	case KT_ST    : op = OP_IL_ST   ; o2 = OP_IL_ENDNULL ; opr = _OPR_LVAL ; break;
	case KT_STN   : op = OP_IL_STN  ; o2 = OP_IL_ENDNULL ; opr = _OPR_LVAL ; break;
	case KT_S     : op = OP_IL_S    ; o2 = OP_IL_ENDNULL ; opr = _OPR_LVAL ; break;
	case KT_R     : op = OP_IL_R    ; o2 = OP_IL_ENDNULL ; opr = _OPR_LVAL ; break;

	case KT_AND   : op = OP_IL_AND   ; break;
	case KT_ANDN  : op = OP_IL_ANDN  ; break;
	case KT_OR    : op = OP_IL_OR    ; break;
	case KT_ORN   : op = OP_IL_ORN   ; break;
	case KT_XOR   : op = OP_IL_XOR   ; break;
	case KT_XORN  : op = OP_IL_XORN  ; break;
	case KT_NOT   : op = OP_IL_NOT  ; o2 = OP_IL_ENDNULL ; opr = _OPR_NONE ; break;
	case KT_ADD   : op = OP_IL_ADD   ; break;
	case KT_SUB   : op = OP_IL_SUB   ; break;
	case KT_MUL   : op = OP_IL_MUL   ; break;
	case KT_DIV   : op = OP_IL_DIV   ; break;
	case KT_MOD   : op = OP_IL_MOD   ; break;
	case KT_GT    : op = OP_IL_GT    ; break;
	case KT_GE    : op = OP_IL_GE    ; break;
	case KT_EQ    : op = OP_IL_EQ    ; break;
	case KT_NE    : op = OP_IL_NE    ; break;
	case KT_LE    : op = OP_IL_LE    ; break;
	case KT_LT    : op = OP_IL_LT    ; break;

	case KT_JMP   : op = OP_IL_JMP  ; o2 = OP_IL_ENDNULL ; opr = _OPR_LABEL ; break;
	case KT_JMPC  : op = OP_IL_JMPC ; o2 = OP_IL_ENDNULL ; opr = _OPR_LABEL ; break;
	case KT_JMPCN : op = OP_IL_JMPCN; o2 = OP_IL_ENDNULL ; opr = _OPR_LABEL ; break;

	case KT_CAL   : op = OP_IL_CAL  ; o2 = OP_IL_ENDNULL ; opr = _OPR_CALL ; break;
	case KT_CALC  : op = OP_IL_CALC ; o2 = OP_IL_ENDNULL ; opr = _OPR_CALL ; break;
	case KT_CALCN : op = OP_IL_CALCN; o2 = OP_IL_ENDNULL ; opr = _OPR_CALL ; break;

	case KT_RET   : op = OP_IL_RET  ; o2 = OP_IL_ENDNULL ; opr = _OPR_NONE ; break;
	case KT_RETC  : op = OP_IL_RETC ; o2 = OP_IL_ENDNULL ; opr = _OPR_NONE ; break;
	case KT_RETCN : op = OP_IL_RETCN; o2 = OP_IL_ENDNULL ; opr = _OPR_NONE ; break;

	case -1       : op = OP_IL_FCALL; o2 = OP_IL_ENDNULL ; opr = _OPR_FCAL ; break;

	default:
		scBackKeyword(k);
		return 0;
	}


	/* Chiamata posticipata in questi due casi, perche` il controllo
	 sintattico non e` ancora finito. */
	if (opr != _OPR_CALL && opr != _OPR_FCAL)
		paCallback(OP_IL_STARTOP);

	switch (opr) {
	case _OPR_ALL:
		if (! paILExpr()) {
			paError(PA_E_NOPR);
		}
		break;
	case _OPR_NOEXPR:
		if (! paILArgument()) {
			paError(PA_E_NOPR);
		}
		break;
	case _OPR_LABEL:
		if (! scIdent(pPaCtl -> symstr)) {
			paError(PA_E_NID);
		}
		break;
	case _OPR_NONE:
		break;
	case _OPR_LVAL:
		if (! paILLValue()) {
			paError(PA_E_NOPR);
		}
		break;
	case _OPR_CALL:
		if (! scIdent(pPaCtl -> symstr)) {
			return 0;
		}
		paCallback(OP_IL_STARTOP);
	/*
		paCallback(OP_IL_FBCALL);
		paILFormalList();
	*/
		break;
	case _OPR_FCAL:
		if (! scIdent(pPaCtl -> symstr)) {
			return 0;
		}
		paCallback(OP_IL_STARTOP);
		paCallback(OP_IL_FNCALL);
		paILFormalList();
		break;
	}

	if (! scMatchCh('\n')) {
		paError(PA_E_NEOL);
	}

	paCallback(op);

	paCallback(o2);

	return 1;
}

static int paILInstruction(void)
{ 
/* Un trucco per forzare la lettura della nuova linea. */
	if (scNextCh('\n')) {
	}

	paCallback(OP_IL_NEWLINE);

	paILLabel();

	return paILStatement();
}

static int paIL(void)
{
int first = 1;

/* Il linguaggio IL non e` a formato libero : '\n' e` significativo. */

	pScCtl -> lf_blank = 0;

/* Anche un programma vuoto, purtroppo, e` un programma IL. Questo
 impedisce di riconoscere in modo pulito un costrutto IL. Comunque, per
 ora, il problema di gestire anche il linguaggio ST non si pone. */

	paCallback(OP_IL_BEGIN);

	while (paILInstruction()) {
		first = 0;
	}

	paCallback(OP_IL_END);

	pScCtl -> lf_blank = 1;

	return ! first;
}

/*
* Riconoscimento di costrutti ST
* ------------------------------
*/

static int paST(void)
{
	return 0;
}

/*
* Riconoscimento di un'espressione in stile ST
* --------------------------------------------
*
* Gli operandi sono, per ora, limitati alle costanti.
*
* L'algoritmo e` composto dalle funzioni paConstant, paPrimary, paHeir[0-8].
*/

/* Riconoscimento di una costante stringa. */

static int paString(void)
{
int n;

	n = scIECString(pPaCtl -> stringstr);
	if (n >= 0) {
		if (!scMatchCh('\'')) {
			paError(PA_E_NOQU);
		}
		*(pPaCtl -> lenstr) = n;
		paCallback(OP_RDSTRING);
		return 1;
	}
	return 0;
}

/* Riconoscimento di una costante. */

static int paConstant(void)
{
	if (scFloat(pPaCtl -> numstr))
		paCallback(OP_RDFLOAT);
	else if (scIECInteger(pPaCtl -> numstr))
		paCallback(OP_RDINT);
	else if (scMatchKeyword(KT_FALSE))
		paCallback(OP_RDFALSE);
	else if (scMatchKeyword(KT_TRUE))
		paCallback(OP_RDTRUE);
	else if (scIECTime(pPaCtl -> numstr))
		paCallback(OP_RDTIME);
	else
		return 0;

	return 1;

}

/* Costante o espressione tra parentesi. */

static int paPrimary(void)
{
	if (paConstant()) {
		return 1;
	}

	if (scMatchCh('(')) {
		if (! paHeir1()) {
			paError(PA_E_NOPR);
			return 1;
		}
		if (! scMatchCh(')')) {
			paError(PA_E_NCP);
			return 1;
		}
	}
	else {
		return 0;
	}

	return 1;

}

/* ** (elevamento a potenza) */

static int paHeir9(void)
{
	if (! paPrimary())
		return 0;

	while (scMatch("**")) {

		if (! paPrimary()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(OP_POW);
	}

	return 1;

}
/* - (unario), NOT */

static int paHeir8(void)
{
	if (scMatchCh('-')) {
		if (! paHeir8()) {
			paError(PA_E_NOPR);
			return 1;
		}
		paCallback(OP_MINUS);
	}
	else if (scMatchKeyword(KT_NOT)) {
		if (! paHeir8()) {
			paError(PA_E_NOPR);
			return 1;
		}
		paCallback(OP_NOT);
	}
	else {
		if (! paHeir9())
			return 0;
	}


	return 1;

}

/* *, /, MOD */

static int paHeir7(void)
{
int op;

	if (! paHeir8())
		return 0;

	for (;;) {

		if (scMatchCh('*'))
			op = OP_TIMES;
		else if (scMatchCh('/'))
			op = OP_DIV;
		else if (scMatchKeyword(KT_MOD))
			op = OP_MOD;
		else
			break;

		if (! paHeir8()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(op);
	}

	return 1;

}

/* +, - */

static int paHeir6(void)
{
int op;

	if (! paHeir7())
		return 0;

	for (;;) {

		if (scMatchCh('+'))
			op = OP_ADD;
		else if (scMatchCh('-'))
			op = OP_SUB;
		else
			break;

		if (! paHeir7()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(op);
	}

	return 1;
}

/* >, <, >=, <= */

static int paHeir5(void)
{
int op;

	if (! paHeir6())
		return 0;


	for (;;) {

		if (scMatch(">="))
			op = OP_GE;
		else if (scMatch("<="))
			op = OP_LE;
		else if (scMatch(">"))
			op = OP_GT;
		else if (scMatch("<>")) {
			scBack("<>");
			break;
		}
		else if (scMatch("<"))
			op = OP_LT;
		else
			break;

		if (! paHeir6()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(op);
	}

	return 1;
}

/* =, <> */

static int paHeir4(void)
{
int op;

	if (! paHeir5())
		return 0;

	for (;;) {

		if (scMatchCh('='))
			op = OP_EQ;
		else if (scMatch("<>"))
			op = OP_NE;
		else
			break;

		if (! paHeir5()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(op);
	}

	return 1;
}

/* AND, & */

static int paHeir3(void)
{
	if (! paHeir4())
		return 0;

	while (scMatchKeyword(KT_AND) || scMatchCh('&')) {

		if (! paHeir4()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(OP_AND);
	}

	return 1;
}

/* XOR */

static int paHeir2(void)
{
	if (! paHeir3())
		return 0;

	while (scMatchKeyword(KT_XOR)) {

		if (! paHeir3()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(OP_XOR);
	}

	return 1;

}

/* OR */

static int paHeir1(void)
{
	if (! paHeir2())
		return 0;

	while (scMatchKeyword(KT_OR)) {

		if (! paHeir2()) {
			paError(PA_E_NOPR);
			break;
		}

		paCallback(OP_OR);
	}

	return 1;
}

static int paConstantExpression(void)
{
int rv;

	paCallback(OP_STARTEXPR);
	if (main_status.standard)
		rv = paConstant();
	else
		rv = paHeir1();
	if (rv)
		paCallback(OP_ENDEXPR);
	return rv;
}

static int paInitString(void)
{
int rv;

	paCallback(OP_STARTEXPR);
	rv = paString();
	if (rv) {
		paCallback(OP_STRINGEXPR);
		paCallback(OP_ENDEXPR);
	}
	return rv;
}

static int paInitConstantExpression(void)
{
	return paConstantExpression() || paInitString();
}

static int paConstantIntegerExpression(void)
{
	return paConstantExpression();
}

/*
* Riconoscimento dei costrutti di configurazione e dichiarazione
* --------------------------------------------------------------
*/

/*
* Funzione paInputVarDeclaration
* ------------------------------
*
*  <InputVarDecl> ::= 'VAR_INPUT' <InputVarDeclList> 'END_VAR'
*/

static int paInputVarDeclaration(void)
{
	return paGenericVarDecl(KT_VAR_INPUT,OP_VAR_INPUT,OP_END_VAR_INPUT);
}

/*
* Funzione paOutputVarDeclaration
* -------------------------------
*
*  <OutputVarDecl> ::= 'VAR_OUTPUT' <OutputVarDeclList> 'END_VAR'
*/

static int paOutputVarDeclaration(void)
{
	return paGenericVarDecl(KT_VAR_OUTPUT,OP_VAR_OUTPUT,OP_END_VAR_OUTPUT);
}

/*
* Funzione paLocalVarDeclaration
* ------------------------------
*
*  <LocalVarDecl> ::= 'VAR' <LocalVarDeclList> 'END_VAR'
*/

static int paLocalVarDeclaration(void)
{
	return paGenericVarDecl(KT_VAR,OP_VAR_LOCAL,OP_END_VAR_LOCAL);
}

static int paBody(void)
{
	return paIL() || paST();
}

static int paGenericProcedure(int k_begin,int op_begin,int k_end,int op_end)
{
	if (! scMatchKeyword(k_begin)) {
		return 0;
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}

	paCallback(op_begin);

	if (op_begin == OP_FUNCTION_HEADER) {
		if (! scMatchCh(':')) {
			paError(PA_E_NSP);
		}
		if (! paType()) {
			paError(PA_E_NTYPE);
		}
	}

	while (paInputVarDeclaration()
	    || ((op_begin != OP_FUNCTION_HEADER) && paOutputVarDeclaration())
	    || paLocalVarDeclaration()) {
	}

	paCallback(OP_BODY);
	
	paBody();

	if (! scMatchKeyword(k_end)) {
		paError(PA_E_NENDPROC);
	}

	paCallback(op_end);

	return 1;

}

static int paProgram(void)
{
	return paGenericProcedure(KT_PROGRAM,OP_PROGRAM_HEADER,
	                          KT_END_PROGRAM,OP_PROGRAM_END);
}

static int paFunction(void)
{
	return paGenericProcedure(KT_FUNCTION,OP_FUNCTION_HEADER,
	                          KT_END_FUNCTION,OP_FUNCTION_END);
}

static int paFunctionBlock(void)
{
	return paGenericProcedure(KT_FUNCTION_BLOCK,OP_FUNCTION_BLOCK_HEADER,
	                          KT_END_FUNCTION_BLOCK,OP_FUNCTION_BLOCK_END);
}

static int paTypeDecl(void)
{
	return paGenericSymDecl(KT_TYPE,OP_TYPE,KT_END_TYPE,OP_END_TYPE);
}

/*
* Funzione paVarDecl
* ------------------
*
*  <VarDecl> ::= <GlobalVarDecl> | <ExternalVarDecl>
* Estensione CNi :
*  <VarDecl> ::= <GlobalVarDecl> | <ExternalVarDecl> | <LocalVarDecl>
*/

static int paVarDecl(void)
{
	return paGlobalVarDecl()
	    || paExternalVarDecl()
	    || (!main_status.standard && paLocalVarDeclaration());
}

/*
* Funzione paArrayInitElem
* ------------------------
*
*  <ArrayInitElem> ::= <ConstantExpression>
*                    | <ConstantExpression> '(' <ConstantExpression> ')'
*/

static int paArrayInitElem(void)
{
	if (! paInitConstantExpression())
		return 0;

	if (scMatchCh('(')) {
		if (! paInitConstantExpression()) {
			paError(PA_E_NCONST);
		}
		if (! scMatchCh(')')) {
			paError(PA_E_NCP);
		}
		paCallback(OP_ARRAY_INIT_MULTI);
	}
	else {
		paCallback(OP_ARRAY_INIT_SINGLE);
	}

	return 1;
}

/*
* Funzione paArrayInitList
* ------------------------
*
*  <ArrayInitList> ::= <ArrayInitElem> | <ArrayInitElem> "," <ArrayInitList>
*/

static int paArrayInitList(void)
{
	if (! paArrayInitElem())
		return 0;

	while (scMatchCh(',')) {
		if (! paArrayInitElem()) {
			paError(PA_E_NCONST);
			break;
		}
	}

	return 1;
}

/*
* Funzione paArrayInit
* --------------------
*
*  <ArrayInit> ::= "[" <ArrayInitList> "]"
*/

static int paArrayInit(void)
{
	if (! scMatchCh('['))
		return 0;

	paCallback(OP_START_ARRAY_INIT);

	if (! paArrayInitList()) {
		paError(PA_E_NINIT);
	}

	if (! scMatchCh(']')) {
		paError(PA_E_NCSQ);
	}

	paCallback(OP_END_ARRAY_INIT);

	return 1;
}

/*
* Funzione paSclarType
* --------------------
*
*  Legge un nome di tipo scalare.
*/

static int paScalarType(void)
{
int k,op;

	k = scKeyword();

	switch (k) {
	case KT_BOOL:
		op = OP_BOOL;
		break;
	case KT_SINT:
		op = OP_SINT;
		break;
	case KT_INT:
		op = OP_INT;
		break;
	case KT_DINT:
		op = OP_DINT;
		break;
	case KT_LINT:
		op = OP_LINT;
		break;
	case KT_USINT:
		op = OP_USINT;
		break;
	case KT_UINT:
		op = OP_UINT;
		break;
	case KT_UDINT:
		op = OP_UDINT;
		break;
	case KT_ULINT:
		op = OP_ULINT;
		break;
	case KT_REAL:
		op = OP_REAL;
		break;
	case KT_LREAL:
		op = OP_LREAL;
		break;
	case KT_TIME:
		op = OP_TIME;
		break;
	case KT_DATE:
		op = OP_DATE;
		break;
	case KT_TIME_OF_DAY:
	case KT_TOD:
		op = OP_TIME_OF_DAY;
		break;
	case KT_DATE_AND_TIME:
	case KT_DT:
		op = OP_DATE_AND_TIME;
		break;
	case KT_STRING:
		op = OP_STRING;
		break;
	case KT_BYTE:
		op = OP_BYTE;
		break;
	case KT_WORD:
		op = OP_WORD;
		break;
	case KT_DWORD:
		op = OP_DWORD;
		break;
	case KT_LWORD:
		op = OP_LWORD;
		break;
	case -1:
		return 0;
	default:
		scBackKeyword(k);
		return 0;
	}

	paCallback(op);
	return 1;
}

/*
* Funzione paSubrange
* -------------------
*
*  <Subrange> ::= <ConstantIntegerExpression> '..' <ConstantIntegerExpression>
*/

static int paSubrange(void)
{
	if (! paConstantIntegerExpression()) {
		return 0;
	}

	if (! scMatch("..")) {
		paError(PA_E_NDOTS);
	}

	if (! paConstantIntegerExpression()) {
		paError(PA_E_NCONST);
	}

	paCallback(OP_MAKE_SUBRANGE);

	return 1;

}

/*
* Funzione paArrayType
* --------------------
*
*/

static int paArrayType(void)
{
	if (! scMatchKeyword(KT_ARRAY)) {
		return 0;
	}

	paCallback(OP_START_DIM_ARRAY);

	if (! scMatchCh('[')) {
		paError(PA_E_NOSQ);
	}

	do {
		if (! paSubrange()) {
			paError(PA_E_NDIMS);
			break;
		}
		paCallback(OP_ADD_DIM_ARRAY);
	} while(scMatchCh(','));

	if (! scMatchCh(']')) {
		paError(PA_E_NCSQ);
	}

	paCallback(OP_END_DIM_ARRAY);

	if (! scMatchKeyword(KT_OF)) {
		paError(PA_E_NOF);
	}

	if (! paScalarType()) {
		paError(PA_E_NTYPE);
	}

	return 1;
}

/*
* Funzione paType
* ---------------
*
*  <Type> ::= <ScalarType>
*                  | <ArrayType>
*                  | <Identifier>
*/

static int paType(void)
{
	if (paScalarType()) {
		paCallback(OP_TYPE_SCALAR);
	}
	else if (paArrayType()) {
		paCallback(OP_TYPE_ARRAY);
	}
	else if (scIdent(pPaCtl->symstr)){
		paCallback(OP_TYPE_FB);
	}
	else {
		return 0;
	}

	return 1;
}

/*
* Funzione paTypeAndInit
* ----------------------
*
*  <TypeAndInit> ::= <Type>
*                  | <Type> := <ConstantExpression>
*                  | <Type> := <ArrayInit>
*                  | <Identifier>
*/

static int paTypeAndInit(void)
{
	if (! paType()) {
		return 0;
	}

	if (scMatch(":=")) {
		if (paInitConstantExpression()) {
			paCallback(OP_INIT_SCALAR_VAR);
		}
		else if (paArrayInit()) {
			paCallback(OP_INIT_DIM_VAR);
		}
		else {
			paError(PA_E_NINIT);
		}
	}

	return 1;
}

/*
* Funzione paVarDeclElement
* -------------------------------
*
*  <VarDeclElement> ::= <IdAndLoc> ':' <TypeAndInit>
*                     | 'RETAIN' <IdAndLoc> ':' <TypeAndInit>
*/

static int paVarDeclElement(void)
{
	if (! scIdent(pPaCtl -> symstr)) {
		return 0;
	}

	if (scMatchKeyword(KT_AT)) {
		if (scIECDirectVar(pPaCtl -> numstr, '\0')) {
			paCallback(OP_VAR_DECL_AT);
		}
		else {
			paError(PA_E_NASSOC_IO);
		}
	}
	else {
		paCallback(OP_VAR_DECL);
	}

	if (! scMatchCh(':')) {
		paError(PA_E_NSP);
	}

	if (! paTypeAndInit()) {
		paError(PA_E_NTYPE);
	}

	paCallback(OP_END_VAR_DECL);

	return 1;

}

/*
* Funzione paGenericSymDecl
* -------------------------
*/

static int paGenericSymDecl(int k_begin,int op_begin,int k_end,int op_end)
{
	if (! scMatchKeyword(k_begin)) {
		return 0;
	}

	paCallback(op_begin);

	if (k_begin == KT_VAR_GLOBAL /* E` una piccola violazione */
	                             /* delle specifiche IEC1131/3... */
	   || k_begin == KT_VAR
	   || k_begin == KT_VAR_OUTPUT) {
		if (scMatchKeyword(KT_RETAIN)) {
			paCallback(OP_VAR_START_DECL_RETAIN);
		}
	}

	while (paVarDeclElement()) {
		if (! scMatchCh(';')) {
			paError(PA_E_NSC);
		}
	}

	if (! scMatchKeyword(k_end)) {
		paError(PA_E_NEND_VAR);
	}

	paCallback(op_end);

	return 1;

}

/*
* Funzione paGenericVarDecl
* -------------------------
*
*  <GlobalVarDecl> ::= 'VAR_GLOBAL' <VarDeclList> 'END_VAR'
*/

static int paGenericVarDecl(int k_begin,int op_begin,int op_end)
{
	return paGenericSymDecl(k_begin,op_begin,KT_END_VAR,op_end);
}

/*
* Funzione paExternalVarDecl
* -------------------------
*
*  <ExternalVarDecl> ::= 'VAR_EXTERNAL' <VarDeclList> 'END_VAR'
*/

static int paExternalVarDecl(void)
{
	return paGenericVarDecl(KT_VAR_EXTERNAL,OP_VAR_EXTERNAL,OP_END_VAR_EXTERNAL);
}

/*
* Funzione paGlobalVarDecl
* -------------------------
*
*  <GlobalVarDecl> ::= 'VAR_GLOBAL' <VarDeclList> 'END_VAR'
*/

static int paGlobalVarDecl(void)
{
	return paGenericVarDecl(KT_VAR_GLOBAL,OP_VAR_GLOBAL,OP_END_VAR_GLOBAL);
}

/*
* Funzione paProgramIOConf
* -------------------------
*
*  <ProgramIOConf> ::= <identifier> ':=' <DirectInput>
*                    | <identifier> ':=' <ConstantExpression>
*                    | <identifier> '=>' <DirectOutput>
*/

static int paProgramIOConf(void)
{


	if (! scIdent(pPaCtl -> symstr)) {
		return 0;
	}
	if (scMatch(":=")) {
		if (scIECDirectVar(pPaCtl -> numstr, 'I')) {
			paCallback(OP_PROGRAM_INCONF);
		}
		else if (paInitConstantExpression()) {
			paCallback(OP_PROGRAM_CONSTCONF);
		}
		else {
			paError(PA_E_NASSOC_I);
		}
	}
	else if (scMatch("=>")) {
		if (! scIECDirectVar(pPaCtl -> numstr, 'Q')) {
			paError(PA_E_NASSOC_O);
		}
		paCallback(OP_PROGRAM_OUTCONF);
	}
	else {
		paError(PA_E_NASSOC_IO);
	}
	paCallback(OP_PROGRAM_ENDIOCONF);

	return 1;

}

/*
* Funzione paProgramInstance
* --------------------------
*
*  <ProgramInstance> ::= 'PROGRAM' <identifier> ':' <identifier> "(" ")" ";"
*                      | 'PROGRAM' <identifier> ':' <identifier> "("
*                        <msd_list> ")" ";"
*/

static int paProgramInstance(void)
{
	if (! scMatchKeyword(KT_PROGRAM)) {
		return 0;
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}
	paCallback(OP_PROGRAM_INSTANCE);
	if (! scMatchCh(':')) {
		paError(PA_E_NSP);
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}
	paCallback(OP_PROGRAM_CLASS);
	if (! scMatchCh('(')) {
		paError(PA_E_NOP);
	}

/* Cosi` "passa" anche una virgola finale, spuria, ma tanto comoda... */
	while (paProgramIOConf() && scMatchCh(',')) {
	}

	if (! scMatchCh(')')) {
		paError(PA_E_NCP);
	}
	paCallback(OP_END_PROGRAM_INSTANCE);

	return 1;

}

/*
* Funzione paResource
* -------------------
*
*  <Resource> ::= 'RESOURCE' <identifier> 'ON' <identifier>
*                 <ProgramList> 'END_RESOURCE'
*/

static int paResource(void)
{
	if (! scMatchKeyword(KT_RESOURCE)) {
		return 0;
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}
	paCallback(OP_RESOURCE);
	if (! scMatchKeyword(KT_ON)) {
		paError(PA_E_NON);
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}
	paCallback(OP_RESOURCE_ON);
	while (paProgramInstance()) {
		if (! scMatchCh(';')) {
			paError(PA_E_NSC);
		}
	}
	if (! scMatchKeyword(KT_END_RESOURCE)) {
		paError(PA_E_NEND_RES);
	}
	paCallback(OP_END_RESOURCE);

	return 1;

}

/*
* Funzione paConfiguration
* ------------------------
*
*  <Configuration> ::= 'CONFIGURATION' <identifier>
*                      <VarAndResourceList> 'END_CONFIGURATION'
*  <VarAndResourceList> ::= <GlobalVarDecl> | <Resource>
*                         | <GlobalVarDecl> <VarAndResourceList>
*                         | <Resource> <VarAndResourceList>
*/

static int paConfiguration(void)
{
	if (! scMatchKeyword(KT_CONFIGURATION)) {
		return 0;
	}
	if (! scIdent(pPaCtl -> symstr)) {
		paError(PA_E_NID);
	}
	paCallback(OP_CONFIGURATION);
	while (paGlobalVarDecl() || paResource()) {
	}
	if (! scMatchKeyword(KT_END_CONFIGURATION)) {
		paError(PA_E_NEND_CONF);
	}
	paCallback(OP_END_CONFIGURATION);

	return 1;

}

/*
* Funzione paIEC1131Construct
* ---------------------------
*
*  <IEC1131Construct> ::= <Configuration> | <Program> | <Function> |
*                         <FunctionBlock> | <VarDecl> | <TypeDecl>
*/

static int paIEC1131Construct(void)
{
	return paConfiguration()
	    || paProgram()
	    || paFunction()
	    || paFunctionBlock()
	    || paVarDecl()
	    || paTypeDecl();
}


/*
* Funzione paIEC1131
* ------------------
*
*  <IEC1131> ::= <IEC1131Construct> | <IEC1131Construct> <IEC1131>
*/

int paIEC1131(void)
{
	while (! scNextCh('\0')) {
		if (! paIEC1131Construct()) {
			paError(PA_E_ICH);
			return 0;
		}
	}

	return 1;
}

