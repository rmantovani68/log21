/* lplc017.c */
#include "lplc.h"


/*
* Funzione "dbGetVar"
* -------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table, e fornisce
* nella struttura puntata da "res" le caratteristiche della variabile.
*  Se "res" e` NULL, la funzione alloca dinamicamente la struttura, che
* dovra` essere liberata dall'utente dopo l'utilizzo con una "free()".
*  Se il nome non e` presente, riporta NULL.
*/

/* Tipi di QPLC */
#define SHV_MODE_TYPE 0x70 /* Maschera. */
#define SHV_MODE_TYPE_BOOL 0x00
#define SHV_MODE_TYPE_CHAR 0x10
#define SHV_MODE_TYPE_UCHAR 0x20
#define SHV_MODE_TYPE_LONG 0x30
#define SHV_MODE_TYPE_ULONG 0x40
#define SHV_MODE_TYPE_FLOAT 0x50
#define SHV_MODE_TYPE_DOUBLE 0x60

#if _NO_PROTO
struct plcvar_t * dbGetVar(name,res) char *name; struct plcvar_t *res;
#else
struct plcvar_t * dbGetVar(char *name, struct plcvar_t *res)
#endif
{
struct symtab_t *p;
char *line,*q;
int scope,len;
struct qplc_shv_t * qsym;
int row,col;

	if (dbQPLC()) {

	/* Cerca la variabile. */

		q = dbGetQplcVarDescr(name,&qsym);

		if (! q)
			return (struct plcvar_t *) 0;

		if (! res) {
			res = (struct plcvar_t *) malloc(sizeof(*res));
			if (! res)
				return (struct plcvar_t *) 0;
		}

		res -> name = qsym -> name;
		res -> pval = q;
		res -> size = qsym -> size;
		res -> type = ISAVAR_T_UNKN;
		res -> format[0] = "";
		res -> format[1] = "";
		res -> attr = ISAVAR_A_INTERNAL;
		res -> scope = 0;

		switch (qsym -> mode & SHV_MODE_TYPE) {
		case SHV_MODE_TYPE_BOOL:
			res -> type = ISAVAR_T_BOOL;
			res -> format[0] = "FALSE";
			res -> format[1] = "TRUE";
			break;
		case SHV_MODE_TYPE_CHAR:
		case SHV_MODE_TYPE_UCHAR:
		case SHV_MODE_TYPE_LONG:
		case SHV_MODE_TYPE_ULONG:
			res -> type = ISAVAR_T_ANA_I;
			res -> format[0] = "I";
			break;
		case SHV_MODE_TYPE_FLOAT:
		case SHV_MODE_TYPE_DOUBLE:
			res -> type = ISAVAR_T_ANA_F;
			res -> format[0] = "F";
			break;
		default:
			break;
		}
	}
	else {

	/* Cerca la variabile. */

		p = dbFindIsaVar(name);

		if (! p)
			return (struct plcvar_t *) 0;

		if (! res) {
			res = (struct plcvar_t *) malloc(sizeof(*res));
			if (! res)
				return (struct plcvar_t *) 0;
		}

	/* Traduce i puntatori secondo lo spazio virtuale del processo. */

		res -> name = line = MTRANS(p -> name);
		res -> pval = MTRANS(p -> pval);
		res -> size = p -> len;
		res -> type = ISAVAR_T_UNKN;
		res -> attr = ISAVAR_A_UNKN;
		res -> format[0] = "";
		res -> format[1] = "";
		res -> scope = 0;

	/* Estrae il tipo di variabile dallo spazio dei simboli. */

		switch (*(line - 5)) {
		case '1': res -> type = ISAVAR_T_BOOL; break;
		case '2': res -> type = ISAVAR_T_ANA_I; break;
		case '3': res -> type = ISAVAR_T_TIMER; break;
		case '4': res -> type = ISAVAR_T_MSG; break;
		default:  res -> type = ISAVAR_T_UNKN; return res;
		}

	/* Cerca la fine del nome. */

#ifdef NEW_strcmp
		while (*++line && *line != ',') {
		}
#else
		while (*++line) {
		}
#endif

	/* Estrae l'attributo. */

		if (res -> type != ISAVAR_T_TIMER) {
			line += 2;
			switch (*line) {
			case 'I': res -> attr = ISAVAR_A_INPUT; break;
			case 'O': res -> attr = ISAVAR_A_OUTPUT; break;
			case 'X': res -> attr = ISAVAR_A_INTERNAL; break;
			default:  res -> attr = ISAVAR_A_UNKN; return res;
			}

		/* Passa al campo successivo. */

			++line;
		}
		else {

		/* I timer non hanno attributo. */

			res -> attr = ISAVAR_A_INTERNAL;
		}

	/* Estrae lo "scope". */

		line += 2;

		scope = 0;

		while (isxdigit(*line)) {
			scope *= 16;
			scope += *line <= '9' ? *line - '0'
			                      : toupper(*line) - 'A' + 10;
			++line;
		}

		res -> scope = scope;

	/* Estrae (o costruisce) il formato. */

		switch (res -> type) {
		case ISAVAR_T_BOOL:
		case ISAVAR_T_ANA_I:

		/* "TRUE" o formato numerico. */

			q = ++line;
			while (*line != ',' && *line != '\0') {
				if (*line == '\n')
					return res;
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[0] = q;

		/* "FALSE" o unit string. */

			q = ++line;
			while (*line != '\r'&&*line != '\n'&&*line != '\0') {
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[1] = q;

		/* Se la variabile e` analogica e il formato e` "F", allora si
		 tratta di un "float". */

			if (res -> type == ISAVAR_T_ANA_I
			    && res -> format[0][0] == 'F')
				res -> type = ISAVAR_T_ANA_F;

			break;

		case ISAVAR_T_TIMER:
			res -> format[0] = "I";
			res -> format[1] = "";
			break;

		case ISAVAR_T_MSG:
			res -> format[0] = "S";

		/* I messaggi contengono la lunghezza al posto del formato. */

			q = ++line;
			while (*line!='\r' && *line!='\n' && *line!='\0') {
				++line;
			}

		/* Conclude la stringa alterando il database dei simboli. */
			*line = '\0';

			res -> format[1] = q;

			break;
		default:
			break;
		}
	}

	return res;

}

