
/*
* @(#) hsort.h GG, CNI Thu Aug 05 15:23:13 ita 1993
*
* Algoritmo di heap-sort.
*/


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

void hsort(char *heap, unsigned int n, unsigned int width, int (*compar)(void *, void *), char *t);
