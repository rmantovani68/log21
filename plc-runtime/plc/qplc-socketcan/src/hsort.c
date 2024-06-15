/*
* @(#) hsort.c GG, CNI Thu Aug 05 15:23:13 ita 1993
*
* Algoritmo di heap-sort.
*/

#include <string.h>
#include "hsort.h"

/*
* Funzione di appoggio per la "hsort()". Realizza un passo dell'algoritmo.
* Il passo consiste nel ripristino della gerarchia violata dall'elemento
* "k" rispetto ai propri inferiori.
*/

static void hsort_step( char *heap, unsigned int k, unsigned int n, unsigned int width, int (*compar)(void *, void *), char *t)
{
unsigned int n2,n1;
register unsigned int j;
register char *p;

	memcpy(t,heap + k*width,width);
	for (n2 = n / 2, n1 = n - 1; k < n2; k = j) {
		p = heap + width*(j = 2*k + 1);
		if (j < n1 && (*compar)(p,p + width) < 0) {
			++j;
			p += width;
		}
		if ((*compar)(t,p) >= 0)
			break;
		memcpy(heap + k*width,p,width);
	}
	memcpy(heap + k*width,t,width);
}

/*
* Funzione "hsort"
* ----------------
*
*  Questa funzione ordina in senso crescente un vettore di indirizzo "heap"
* composto da "n" elementi di dimensione "width". Il confronto tra gli
* elementi del vettore e` realizzato dalla funzione "(*compar)(&x,&y)", di tipo
* "int", che vuole come parametri i due indirizzi degli elementi "x,y" da con-
* frontare, ed ha un valore positivo se x > y, negativo se x < y.
* La funzione necessita di un'area di appoggio "t" di dimensione "width".
*/

#if _NO_PROTO
void hsort(heap,n,width,compar,t)
char *heap;
unsigned int n;
unsigned int width;
int (*compar)();
char *t;
#else
void hsort(
	char *heap,
	unsigned int n,
	unsigned int width,
	int (*compar)(void *, void *),
	char *t)
#endif
{
register unsigned int i;
register char *q;

	if (n < 2)
		return;
	for (i = n / 2; i;)
		hsort_step(heap,--i,n,width,compar,t);
	for (i = n; --i;) {
		memcpy(t,heap,width);
		memcpy(heap,(q = heap + i*width),width);
		memcpy(q,t,width);
		hsort_step(heap,0,i,width,compar,t);
	}
}

