/* lplc013.c */
#include "lplc.h"

/*
* Funzione "_dbST_Hash"
* ---------------------
*
*  Calcola una ragionevole funzione di hash per il nome "name".
*/

#if _NO_PROTO
static int _dbST_Hash(name) char *name;
#else
static int _dbST_Hash(char *name)
#endif
{
register char c,*p;
register unsigned long l;

	p = name;
	l = 0;
#ifdef NEW_strcmp
	while ((c = *p++) && (c != ','))
#else
	while ((c = *p++))
#endif
	{
		l += c * (l % 37 + 1); /* Si faccia avanti chi ha idee migliori. */
	}
	return (int)(l % (MAX_HASH_KEY + 1));
}

/*
* Funzione "_dbST_strcmp"
* -----------------------
*
*  Questa funzione realizza l'equivalente della "strcmp", ma considera
* come caratteri di fine stringa anche ','.
*/
 
#ifdef NEW_strcmp

#if _NO_PROTO
static int _dbST_strcmp(x,y) char *x; char *y;
#else
static int _dbST_strcmp(char *x, char *y)
#endif
{
register char cx,cy;
register int d;
 
        for (;;) {
                cx = *x;
                if (cx == ',')
                        cx = 0;
                cy = *y;
                if (cy == ',')
                        cy = 0;
				d = cx - cy;
                if (d)
                        return d;
                else if (! cx)
                        return 0;
                ++x;
                ++y;
        }
}
 
#endif


/*
* Funzione "_dbST_compare"
* ------------------------
*
*  Questa funzione confronta gli elementi "x" e "y" della symbol table,
* basandosi sul seguente criterio :
*
* 	se hash(x) > hash(y) -->  x > y
* 	se hash(x) < hash(y) -->  x < y
* 	se hash(x) == hash(y) -->  la parola passa alla "strcmp".
*
*/

#if _NO_PROTO
static int _dbST_compare(x,y) void *x; void *y;
#else
static int _dbST_compare(void *x, void *y)
#endif
{
char *s1,*s2;
int h1,h2;

/* s1, s2 : puntatori all'inizio del nome. */

	s1 = MTRANS(((struct symtab_t *)x) -> name);
	s2 = MTRANS(((struct symtab_t *)y) -> name);

/* Calcola le funzioni di hash per i due nomi. */

	h1 = _dbST_Hash(s1);
	h2 = _dbST_Hash(s2);

/* Confronto. */

	if (h1 == h2)
		return strcmp(s1,s2);
	else
		return h1 - h2;
}

/*
* Funzione "_dbST_srccomp"
* -----------------------
*
*  Questa funzione confronta alfabeticamente il nome di due
* entry della symbol table.
*/

#if _NO_PROTO
static int _dbST_srccomp(x,y) void *x; void *y;
#else
static int _dbST_srccomp(void *x, void *y)
#endif
{
	return strcmp(MTRANS(((struct symtab_t *)x) -> name),
					MTRANS(((struct symtab_t *)y) -> name));
}

/*
* Funzione "dbFindIsaVar"
* -----------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table (ISAGRAF).
*  Se il nome non e` presente, riporta NULL.
*/

#if _NO_PROTO
struct symtab_t * dbFindIsaVar(name) char *name;
#else
struct symtab_t * dbFindIsaVar(char *name)
#endif
{
int hkey;
struct symtab_t t;

/* Calcola l'indice di hash del nome. */

	hkey = _dbST_Hash(name);

	if (! pHashTab[hkey].len)
		return (struct symtab_t *) 0;

/* Cerca il simbolo nell'insieme di chiave "hkey". */

	t.name = MRTRANS(name);

	return (struct symtab_t *) bsearch(
	    (char *)&t,
	    (char *)MTRANS(pHashTab[hkey].set),
	    pHashTab[hkey].len,
	    sizeof(t),
	    _dbST_srccomp);
}

/*
* Funzione "dbFindQplcVar"
* ------------------------
*
*  Questa funzione trova il simbolo "name" nella symbol table (QPLC).
*  Se il nome non e` presente, riporta NULL.
*  In "row" e "col" sono riportati gli eventuali indici di riga e
* colonna ricavati dal nome secondo le regole di isaker.
*/

struct qplc_shv_t * dbFindQplcVar(char *name, int *row, int *col)
{
int hkey;
struct qplc_shv_t *p;
long offset;
char s[MAX_SHV_NAME*3];
char *q;
int i1,i2,f;

/* Trova le componenti. */

	strncpy(s,name,sizeof(s)-1);
	s[sizeof(s)-1] = '\0';
	i1 = i2 = 0;
	q = s;
	while ( (q = strchr(q,'_')) ) {
		++q;
		if (*q >= '0' && *q <= '9') {
			*(q - 1) = '\0';
			i1 = atoi(q);
			if ( (q = strchr(q,'_')) ) {
				i2 = atoi(++q);
			}
			break;
		}
	}

/* Calcola l'indice di hash del nome. */

	hkey = _dbST_Hash(s);

/* Ricerca del nome nella lista di hash. */

	for (offset = QplcHash[hkey]; offset; offset = p -> next_hash) {
		p = (struct qplc_shv_t *) (pchMem + offset);
		f = strcmp(p -> name, s);
		if (f >= 0) {
			if (f == 0) {
				*row = i1;
				*col = i2;
				return p;
			}
			break;
		}
	}

	return (struct qplc_shv_t *) 0;
}

