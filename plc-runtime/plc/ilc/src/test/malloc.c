
#define MINIFRAGM           1       /* 1=minimizza frammentazione, 0=lento */
#define STATISTICA          0       /* 0=disabilitata, 1=debug */
#define MALLOC_USE_START    1       /* 1=modalita veloce, 0=modalita lenta */
#define REALLOC_NONFAFREE   1       /* 1=modalita veloce, 0=modalita lenta */

/*
* @(#) malloc.c 1.0
*
*  Primitive di gestione dinamica della memoria.
*
*  L'algoritmo qui utilizzato considera l'heap come un array di elementi
* di tipo "HMEM", all'interno del quale si hanno blocchi di elementi "liberi"
* e blocchi "occupati". I blocchi liberi sono collegati in una lista
* concatenata che inizia sempre con l'elemento alla posizione "FREELIST".
*  Ogni blocco libero e` costituito da almeno due elementi, il primo dei
* quali contiene la lunghezza del blocco (espressa in numero di elementi,
* non in byte), il secondo l'indice del successivo blocco libero, oppure
* "FREELIST" se si tratta dell'ultimo elemento della lista. Sono definite
* le macro HMEM_SIZE(n) ed HMEM_NEXT(n) per accedere ai due campi di un
* blocco che inizia alla posizione "n" nell'array.
*  I blocchi occupati contengono la lunghezza nel primo elemento
* (si usi ancora HMEM_SIZE(n)), mentre i successivi sono disponibili per
* l'utilizzo da parte dell'applicazione chiamante.
*
*  L'obiettivo di questo pacchetto e` il superamento dei limiti mostrati
* dalla libreria standard Microsoft e Watcom nella dimensione dei blocchi
* allocabili. E` probabile che gli algoritmi qui utilizzati siano pero` piu`
* lenti.
*/

#include <stdio.h>       /* include libreria Microsoft */
#include <stdlib.h>
#include <string.h>
/*
#define CL_DEBUG   1
#define malloc          malloc2
#define free            free2
#define realloc         realloc2
#define _msize          _msize2
*/

#ifndef CL_DEBUG
#define CL_DEBUG 0
#endif

/* Dipendente dalla CPU e dal sistema operativo. Probabilmente
 esiste un simbolo  predefinito. Indagare. */

#define PAGE_SIZE 0x1000

#define MIN_SBRK_SIZE 0x10000

#define MIN_FRAGM_SIZE 4

#if defined(_MSC_VER) && !defined(__STDC__)
#define FAR_ far
#else
#define FAR_
#endif

typedef void FAR_ * LPVOID;
typedef char FAR_ * LPCHAR;

/* Questo e` il tipo del parametro che si aspetta la funzione "sbrk".
  Sospetto che dipenda fortemente dal sistema. */

typedef int SBRK_T;

extern LPVOID sbrk(SBRK_T);

typedef size_t HMEM, FAR_ * LPHMEM;

#define NULLHMEM ((LPHMEM) 0)
#define NULLPTR ((LPVOID) 0)

static LPHMEM _heap = NULLHMEM;
static LPHMEM _end_heap = NULLHMEM;
static LPHMEM _true_end_heap = NULLHMEM;

#define HMEM_T_SIZE (sizeof(HMEM))
#define FREELIST 0
#define HMEM_SIZE(n) (_heap[(n)])
#define HMEM_NEXT(n) (_heap[(n)+1])
#define HMEM_BUFF(n) (&HMEM_NEXT(n))

#ifndef STATISTICA
#define STATISTICA 0
#endif
#ifndef MALLOC_USE_START
#define MALLOC_USE_START 0
#endif


#if STATISTICA
static int n_calloc = 0;
static int n_malloc = 0;
static int n_free = 0;
static int n_realloc = 0;
static int n_memmove = 0;
static int n_alloc = 0;
static int n_sbrk = 0;
static int n_mbytes = 0;
#endif

#ifdef CL_DEBUG
void trappola(void) {}
#endif

#ifdef DEBUG
void h_write(long x)
{
char msg[8];
static char ht[] = "0123456789abcdef";
register int i;

	for (i = 8; i-- ;) {
		msg[i] = ht[x & 0x0F];
		x >>= 4;
	}
	write(2,msg,8);
}
void d_write(long x)
{
char msg[12];
register int i;

	i = 12;
	do {
		msg[--i] = x % 10 + '0';
		x /= 10;
	} while (x);
	write(2,msg + i,12 - i);
}
#endif

/*
* Funzione "_heap_alloc"
* ----------------------
*
*  Questa funzione espande l'heap in modo che possa ospitare "n" nuovi
* elementi di tipo HMEM. In caso di successo, fornisce la base della
* nuova area allocata, e sposta il puntatore di fine heap (_end_heap),
* altrimenti restituisce NULLHMEM.
*/

static LPHMEM _heap_alloc(size_t nn)
{
LPCHAR rv;
size_t n,nsbrk,residuo;

#if STATISTICA
	++n_alloc;
#endif

/* Numero di byte da richiedere al sistema. */

	n = nn * HMEM_T_SIZE;

/* Rappezziamo la sbrk() : la fetentona non accetta valori negativi
 (bug Watcom ?) ed alloca piu` memoria di quanto le si chieda (limite i386,
 ma potevano metterci una pezza !). */

	if (_true_end_heap) {
		residuo = (LPCHAR)_true_end_heap - (LPCHAR)_end_heap;
		if (residuo >= n) {
			rv = (LPCHAR) _end_heap;
			_end_heap += nn;
			return (LPHMEM) rv;
		}
		n -= residuo;
	}

	nsbrk = ((n + MIN_SBRK_SIZE - 1) / MIN_SBRK_SIZE) * MIN_SBRK_SIZE;

/* Combattimento corpo a corpo con la rappresentazione in complemento
 a due degli interi. La funzione "sbrk" considera valori con segno,
 i valori negativi essendo considerati decrementi dell'heap. Percio`
 allocazioni di aree di dimensioni superiori al massimo valore intero
 con segno rappresentabile devono essere spezzate in due allocazioni
 piu` piccole, per assicurare che il parametro della funzione sia sempre
 positivo.
  Si fa l'ipotesi che, segno a parte, la funzione "sbrk" si aspetti un
 parametro della stessa dimensione di HMEM. */

#if STATISTICA
	++n_sbrk;
#endif

	if ((SBRK_T) nsbrk < 0) {

	/* Binario morto, comunque... */

		rv = (LPCHAR) sbrk((SBRK_T)((unsigned long)(nsbrk + 1) / 2));
		if (rv == (LPCHAR) -1)
			return NULLHMEM;

		if ((LPCHAR)sbrk((SBRK_T)((unsigned long)(nsbrk) / 2)) == (LPCHAR) -1) {

		/* Se fallisce la seconda allocazione, restituisce il primo
		 blocco al sistema prima di uscire. */

			/* (Tanto non funziona...) */
			sbrk( - (SBRK_T)((unsigned long)(nsbrk + 1) / 2));
			return NULLHMEM;
		}
	}
	else {
		rv = (LPCHAR) sbrk((SBRK_T)nsbrk);
		if (rv == (LPCHAR) -1)
			return NULLHMEM;
	}

	_true_end_heap = (LPHMEM)(rv + nsbrk - PAGE_SIZE);
	if (! _end_heap)
		_end_heap = (LPHMEM)rv;

/* Aggiorna il puntatore alla fine dell'heap. */

	rv = (LPCHAR) _end_heap;
	_end_heap += nn;

	return (LPHMEM) rv;
}


/*
* Funzione "_msize"
* -----------------
*
* Si commenta da sola (si fa F1...).
*/

size_t _msize(LPVOID ptr)
{
HMEM ip;

	if (! ptr)
		return 0;

/* Faticosissimo calcolo dell'indirice del blocco.
  Ovviamente "ptr" deve essere generato da "malloc()". */

	ip = ((LPHMEM) ptr - _heap) - 1;
	return (size_t) (HMEM_SIZE(ip) * HMEM_T_SIZE);
}


/*
* Funzione "malloc"
* -----------------
*
*  Tutti sanno cosa fa.
*  L'algoritmo cerca il primo blocco libero abbastanza grande da soddifare
* la richiesta del chiamante, e, a seconda della sua dimensione, lo rimuove
* dalla lista dei blocchi liberi, oppure lo spezza in due blocchi, uno
* occupato, l'altro ancora libero. Se non esiste alcun blocco libero idoneo,
* l'algoritmo provvede ad espandere l'heap, crea un nuovo blocco libero
* della dimensione opportuna oppure, se un blocco libero termina esattamente
* alla fine dell'heap, lo espande.
*  Se l'algoritmo ha successo, la funzione restituisce in uscita l'indirizzo
* della parte non riservata del blocco occupato, cioe` l'indirizzo del
* secondo elemento del blocco, essendo il primo ancora la lunghezza.La macro
* HMEM_BUFF(n) calcola tale indirizzo.
*/

LPVOID malloc(size_t nb)
{
HMEM p,q,r;
size_t n,size;
LPVOID rv;

#if STATISTICA
	++n_malloc;
#endif

/* Caso particolare. */

	if (! nb) {
		rv = NULLPTR;
		goto EXIT_FUNC;
	}

/* Se non e` ancora stato allocato lo heap, provvede.
  I primi due elementi HMEM dello heap costituiscono un descrittore
 fittizio, usato come base della lista dei blocchi liberi. Tale descrittore
 non dovra` mai essere rimosso. */

	if (! _heap) {

		_heap = _heap_alloc(2);

		if (! _heap) {
			rv = NULLPTR;
			goto EXIT_FUNC;
		}

		HMEM_SIZE(FREELIST) = 0;
		HMEM_NEXT(FREELIST) = FREELIST;
	}

/* Calcola il numero di elementi HMEM da allocare. */

	n = (nb + HMEM_T_SIZE - 1) / HMEM_T_SIZE;

/* Ricerca di un blocco libero abbastanza grande. Il ciclo deve tenere
 traccia di due passi precedenti per poter aggiornare correttamente la
 lista dei blocchi liberi nel caso di rimozioni di elementi. */

	for (p = HMEM_NEXT(FREELIST), q = r = FREELIST;
		 p != FREELIST;
		 r = q, q = p, p = HMEM_NEXT(p)) {

		size = HMEM_SIZE(p);

		if (size >= n) {

		/* Trovato ! */

		/* Controlla la lunghezza della parte residua del blocco.
		  Il blocco residuo non puo` essere costituito da meno di due
		 elementi. */

#if MINIFRAGM
			if (size - n <= MIN_FRAGM_SIZE)
#else
			if (size == n || size == n + 1)
#endif
			{

			/* Giusto giusto. Rimuove il descrittore dalla lista
			 dei liberi. */

				HMEM_NEXT(q) = HMEM_NEXT(p);

			/* Calcola l'indirizzo della parte libera
			 del blocco e termina */

				rv = (LPVOID) HMEM_BUFF(p);
				goto EXIT_FUNC;
			}
			else {

#if MALLOC_USE_START
		/* In questa modalita` di funzionamento si utilizza la parte
		 iniziale del blocco libero. Questo dovrebbe facilitare la
		 riallocazione (ma dipende...). */

			/* Abbondante. Lo sposta e lo accorcia. */

				{
				register HMEM new;

					new = p + n + 1;
					HMEM_SIZE(new) = size - (n + 1);
					HMEM_NEXT(new) = HMEM_NEXT(p);
					HMEM_NEXT(q) = new;
				}

			/* Crea un blocco occupato con la parte iniziale del blocco
			 libero. */

				HMEM_SIZE(p) = n;

			/* Calcola l'indirizzo della parte libera
			 del blocco e termina */

				rv = (LPVOID) HMEM_BUFF(p);
				goto EXIT_FUNC;
#else
		/* In questa modalita` di funzionamento si utilizza la parte finale
		 del blocco libero. L'allocazione e` piu` veloce, ma la realloc
		 puo` diventare dispendiosa. */

			/* Abbondante. Lo accorcia. */

				HMEM_SIZE(p) = size = size - (n + 1);

			/* Crea un blocco occupato con la parte finale del blocco
			 libero. Riutilizza "p" come indice del nuovo blocco. */

				p += size + 1;

				HMEM_SIZE(p) = n;

			/* Calcola l'indirizzo della parte libera
			 del blocco e termina */

				rv = (LPVOID) HMEM_BUFF(p);
				goto EXIT_FUNC;
#endif
			}
		}
	}

/* Non e` stato trovato alcun blocco libero abbastanza grande.
  Si dovra` espandere l'heap (o "lo" heap). */

/* Controlla se l'ultimo blocco libero incontrato termina esattamente alla
 fine dell'heap, nel qual caso lo si puo` espandere aumentando l'heap
 del numero di elementi richiesto meno la lunghezza del blocco. Il blocco
 "FREELIST" non e` pero` da considerare. */

	size = HMEM_SIZE(q);

	if (q != FREELIST && HMEM_BUFF(q) + size == _end_heap) {

		if (_heap_alloc(n - size)) {

		/* Se l'espansione dell'heap ha successo, rimuove il blocco
		 libero dalla lista e lo trasforma in un blocco occupato di
		 lunghezza "n". */

			HMEM_NEXT(r) = HMEM_NEXT(q);
			HMEM_SIZE(q) = n;

		/* Calcola l'indirizzo della parte libera
		 del blocco e termina */

			rv = (LPVOID) HMEM_BUFF(q);
			goto EXIT_FUNC;
		}
		else {

		/* Sfiga... */

			rv = NULLPTR;
			goto EXIT_FUNC;

		}
	}
	else {

	register LPHMEM new;

	/* Espande l'heap richiedendo la memoria necessaria a contenere
	 un nuovo blocco di lunghezza "n". */

		new = _heap_alloc(n + 1);
		if (new) {

		/* Se l'espansione dell'heap ha successo, crea un blocco occupato
		 di lunghezza "n". "p" e` utilizzato come indice del nuovo blocco. */

			p = new - _heap;

			HMEM_SIZE(p) = n;

		/* Calcola l'indirizzo della parte libera
		 del blocco e termina */

			rv = (LPVOID) HMEM_BUFF(p);
			goto EXIT_FUNC;
		}
		else {

		/* Sfiga... */

			rv = NULLPTR;
			goto EXIT_FUNC;
		}
	}

EXIT_FUNC:
#if DEBUG
	{
		write(2,"malloc( ",8);
		d_write((long)nb);
		write(2," ) = ",5);
		h_write((long)rv);
		write(2,"\n",1);
	}
#endif
#if CL_DEBUG
	if (! _heap_check()) {
		write(2,"\nmalloc : internal error\n",strlen("\nmalloc : internal error\n"));
		trappola();
	}
#endif
	return rv;
}


/*
* Funzione "free"
* ---------------
*
*  Tutti sanno cosa fa.
*  L'algoritmo cerca nella lista dei blocchi liberi la posizione opportuna
* per il blocco da liberare, espandendo eventualmente blocchi gia` esistenti
* se il nuovo blocco libero confina con altri gia` liberati.
*/

void free(LPVOID ptr)
{
HMEM p,q,ip;
int expand;

#if STATISTICA
	++n_free;
#endif

/* Caso particolare, per simmetria con la "malloc(0)". */

	if (! ptr)
		goto EXIT_FUNC;

/* Questo flag indichera` che c'e` stata una ricongiunzione in testa al
 blocco da liberare. */

	expand = 0;

/* Faticosissimo calcolo dell'indirice del blocco da liberare.
  Caos assicurato se "ptr" non e` un indirizzo ottenuto con "malloc()". */

	ip = ((LPHMEM) ptr - _heap) - 1;

/* Controllo di congruenza. Leeennnntooo... */
#if CL_DEBUG
	if (_ptr_check(ip) == 0) {
		write(2,"free : bad address\n",strlen("free : bad address\n"));
		trappola();
		goto EXIT_FUNC;
	}
#endif

/* Ricerca della posizione nella lista dei blocchi liberi.
  Se l'indice del blocco libero e` maggiore di
 quello del blocco da liberare, oppure e FREELIST, la ricerca termina. */

	for (p = HMEM_NEXT(FREELIST), q = FREELIST;
		 p != FREELIST && p < ip;
		 q = p, p = HMEM_NEXT(p)) {
	}

#if DEBUG
	{
		write(2,"free( ",6);
		h_write((long)ptr);
		write(2," ) = ",5);
		d_write((long)HMEM_SIZE(ip)*HMEM_T_SIZE);
		write(2,"\n",1);
	}
#endif

/* Controlla se la fine del blocco libero precedente coincide con
 l'inizio del blocco da liberare ed esegue, se necessario, la
 ricongiunzione.
  Basta espandere il blocco libero in modo che comprenda il nuovo blocco. */

	if (q + HMEM_SIZE(q) + 1 == ip) {

	/* Coincidono. Segnala che ci sara` una ricongiunzione in testa.
	  Questo provochera` anche l'uscita dal ciclo. */

		expand = 1;

	/* Espande il blocco precedente. */

		HMEM_SIZE(q) += HMEM_SIZE(ip) + 1;
	}

 /* Controlla se la fine del blocco da liberare coincide con
  l'inizio del successivo blocco libero. */

	if (p == ip + HMEM_SIZE(ip) + 1) {

	/* Coincidono. Bisogna espandere il blocco da liberare. */

		if (expand) {

		/* Se c'e` stata una ricongiunzione in testa, e` sufficiente
		 espandere di nuovo il blocco precedente e rimuovere dalla lista
		 il successivo.
		  "q" non puo` valere FREELIST, perche` non si fanno ricongiunzioni
		 con il blocco FREELIST, percio` non e` necessario
		 controllarla. */

			HMEM_SIZE(q) += HMEM_SIZE(p) + 1;
			HMEM_NEXT(q) = HMEM_NEXT(p);
		}
		else {

		/* ...altrimenti i espande il nuovo blocco... */
			HMEM_SIZE(ip) += HMEM_SIZE(p) + 1;
		/* ...si rimuove il successivo... */
			HMEM_NEXT(ip) = HMEM_NEXT(p);
		/* ...e si inserisce il nuovo. */
			HMEM_NEXT(q) = ip;
		}
	}
	else {

	/* Non c'e` ricongiunzione con il blocco successivo. Se c'e`
	 stata una ricongiunzione con quello precedente, non e` necessaria
	 alcuna altra azione. Altrimenti, bisogna inserire nella lista il
	 nuovo blocco libero. */

		if (! expand) {
			HMEM_NEXT(ip) = p;
			HMEM_NEXT(q) = ip;
		}
	}

EXIT_FUNC:
#if CL_DEBUG
	if (! _heap_check()) {
		write(2,"\nfree : internal error\n",strlen("\nfree : internal error\n"));
		trappola();
	}
#endif
	return;
}


/*
* Funzione "realloc"
* ------------------
*
*  Tutti sanno cosa fa.
*  L'algoritmo, nel caso di espansione, controlla se c'e` un blocco libero
* di dimensione sufficiente dopo il blocco occupato. Se c'e`, lo accorcia
* o lo utilizza interamente, altrimenti alloca nuova memoria, eventualmente
* sfruttando l'ultimo blocco libero, se questo termina esattamente in fondo
* all'heap.
*  Nel caso di contrazione, il blocco viene, se possibile, spezzato, e la
* parte eliminata e` trasformata in un blocco occupato separato, che viene
* poi sottoposto alla funzione "free()".
*/

LPVOID realloc(LPVOID ptr, size_t nb)
{
HMEM p,q,r,ip;
size_t size,n;
LPVOID rv;
int ef,eb;

#if STATISTICA
	++n_realloc;
#endif

/* Casi particolari. */

	if (! ptr) {
		rv = malloc(nb);
		goto EXIT_FUNC;
	}

	if (! nb) {
		free(ptr);
		rv = NULLPTR;
		goto EXIT_FUNC;
	}

/* Calcola il numero di elementi HMEM da allocare. */

	n = (nb + HMEM_T_SIZE - 1) / HMEM_T_SIZE;

/* Faticosissimo calcolo dell'indirice del blocco da manipolare.
  Caos assicurato se "ptr" non e` un indirizzo ottenuto con "malloc()". */

	ip = ((LPHMEM) ptr - _heap) - 1;

/* Controllo di congruenza. Leeennnntooo... */
#if CL_DEBUG
	if (_ptr_check(ip) == 0) {
		write(2,"realloc : bad address\n",strlen("realloc : bad address\n"));
		trappola();
		rv = NULLPTR;
		goto EXIT_FUNC;
	}
#endif


/* Il blocco deve essere espanso o ridotto ? */

	size = HMEM_SIZE(ip);

	if (n == size) {

	/* OM Curioso caso di utilizzo: si vuole riallocare un buffer della
	 stessa dimensione. Risultato: NOP.
	 (GG : OK, Meriz !) */

		rv = ptr;
		goto EXIT_FUNC;
	}

	if (n < size) {

	/* Possibile contrazione. */

	/* Controlla se vi sia un blocco libero immediatamente dopo.
	  Se c'e`, si puo` contrarre incondizionatamente, ed espandere
	 il blocco libero successivo. Altrimenti, si puo` contrarre solo se
	 il residuo e` di almeno due elementi. In ogni caso serviranno
	 gli indici dei blocchi liberi precedente ("q") e successivo ("p"). */

		for (p = HMEM_NEXT(FREELIST), q = FREELIST;
			 p != FREELIST && p < ip;
			 q = p, p = HMEM_NEXT(p)) {
		}

		if (p == ip + size + 1) {

		/* C'e` un blocco immediatamente dopo. La contrazione
		 e` comunque fattibile (e conveniente). */

		register HMEM new;

		/* Accorcia il blocco occupato. */

			HMEM_SIZE(ip) = n;

		/* Calcola il nuovo indice del blocco successivo. */

			new = ip + n + 1;

		/* Ricostruisce l'intestazione del blocco libero alla
		 nuova posizione. */
		/* NOTA : L'ordine di queste due espressioni non deve
		 cambiare. */

			HMEM_SIZE(new) = HMEM_SIZE(p) + size - n;
			HMEM_NEXT(new) = HMEM_NEXT(p);

		/* Corregge il puntatore di lista nel blocco precedente. */

			HMEM_NEXT(q) = new;

		}
		else {

		/* Non c'e` alcun blocco immediatamente dopo. La contrazione
		 e` fattibile solo se la parte liberata consta di almeno due
		 elementi. */

#if MINIFRAGM
			if (size - n > MIN_FRAGM_SIZE)
#else
			if (size - n >= 2)
#endif
			{

			/* Si puo` creare un nuovo blocco libero. */

			register HMEM new;

			/* "new" : indice del nuovo blocco libero. */

				new = ip + n + 1;

			/* Crea il descrittore di blocco. */

				HMEM_SIZE(new) = size - n - 1;
				HMEM_NEXT(new) = p;

			/* Inserisce il blocco nella lista. */

				HMEM_NEXT(q) = new;

			/* Accorcia il blocco originario. */

				HMEM_SIZE(ip) = n;
			}
		}

	/* Fine del caso "contrazione".
	  L'indirizzo dell'area occupata non e` stato modificato. */

		rv = ptr;
		goto EXIT_FUNC;
	}
	else {

	/* Espansione. */

	/* Controlla se vi sia un blocco libero immediatamente dopo.
	  Se c'e`, ed e` di dimensione sufficiente, si puo` espandere
	 il blocco occupato. Altrimenti, si dovra` spostare altrove il
	 blocco (malloc() - memcpy() - free()). */

		for (p = HMEM_NEXT(FREELIST), q = r = FREELIST;
			 p != FREELIST && p < ip;
			 r = q, q = p, p = HMEM_NEXT(p)) {
		}

		ef = (p == ip + size + 1);
		eb = (q + HMEM_SIZE(q) + 1 == ip);

		if (ef && HMEM_SIZE(p) + size + 1 >= n) {

		/* C'e` un blocco abbastanza grande immediatamente dopo.
		  L'espansione e` fattibile (e conveniente). */

		register size_t res;

		/* Corregge il blocco libero successivo, o lo rimuove se
		 diventa troppo corto. */

			res = HMEM_SIZE(p) + 1 - (n - size);
#if MINIFRAGM
			if (res > MIN_FRAGM_SIZE)
#else
			if (res >= 2)
#endif
			{

			/* Il blocco libero puo` essere accorciato. */

			register HMEM new;

			/* "new" : nuovo indice del blocco libero. */

				new = ip + n + 1;

			/* Sposta il descrittore di blocco. */
			/* NOTA : L'ordine di queste due espressioni non deve
			 cambiare. */

				HMEM_NEXT(new) = HMEM_NEXT(p);
				HMEM_SIZE(new) = HMEM_SIZE(p) - (n - size);

			/* Corregge il puntatore di lista nel blocco libero
			 precedente. */

				HMEM_NEXT(q) = new;
			}
			else {

			/* Il blocco libero non puo` essere accorciato.
			  Sara` quindi rimosso, e tutti i suoi elementi
			 utilizzati per espandere quello occupato. */

			/* Tiene conto del primo elemento del blocco libero da
			 rimuovere. Si sfruttera` infatti anche questo per
			 l'espansione. */

#if MINIFRAGM
				n += res;
#else
				if (res == 1)        /* Potrebbe essere anche n += res ! */
					++n;
#endif

			/* Rimuove il blocco libero dalla lista. */

				HMEM_NEXT(q) = HMEM_NEXT(p);
			}

		/* Allunga il blocco originario. */

			HMEM_SIZE(ip) = n;

		/* Fine del caso "espansione senza spostamento".
		  L'indirizzo dell'area occupata non e` stato modificato. */

			rv = ptr;
			goto EXIT_FUNC;
		}

		if (eb) {

		/* Se l'inizio del blocco da riallocare coincide con la fine di
		 un blocco libero abbastanza grande, se ne utilizza lo spazio.
		  Eventualmente, espande l'heap della quantita` ancora mancante
		 se il blocco si trova esattamente alla fine. */

		/* Sposta il blocco e lascia libero il tratto residuo (se puo`
		 esistere). */

		size_t len;

			len = HMEM_SIZE(q) + size + 1;

			if (len < n && ip + size + 1 == _end_heap - _heap) {

			/* Gestione del caso espansione dell'heap se la fine del buffer
			 da ridimensionare coincide con la fine dell'heap. */

			/* Se l'espansione non dovesse avere successo,
			 si passera` oltre... */

				if (_heap_alloc(n - len)) {

				/* Se l'espansione dell'heap ha successo, aumento il blocco
				 occupato di lunghezza "n-size" */
				/* Se non ha successo, salta il prossimo test... */

					len = n;
				}
			 }

			 if (len >= n) {

			/* Rimuove l'intero blocco dalla lista. */

				HMEM_NEXT(r) = HMEM_NEXT(q);

			/* Sposta il contenuto dell'area occupata. */

#if STATISTICA
	++n_memmove;
	n_mbytes += size;
#endif

				memmove((LPVOID) HMEM_BUFF(q),
						(LPVOID) HMEM_BUFF(ip),
						size * HMEM_T_SIZE);

			/* Assegna la nuova dimensione al blocco e libera il residuo,
			 se esiste. */

#if MINIFRAGM
				if (len - n <= MIN_FRAGM_SIZE)
#else
				if (len - n < 2)
#endif
				{
					HMEM_SIZE(q) = len;
				}
				else {

				HMEM p2;

					HMEM_SIZE(q) = n;
#if REALLOC_NONFAFREE
				/* p2 = indice del blocco libero da inserire ed,
				 eventualmente, ricongiungere. */

					p2 = q + n + 1;
					HMEM_SIZE(p2) = len - n - 1;

				/* Controlla il confine superiore. */

					if (p2 + HMEM_SIZE(p2) + 1 == p) {

					/* Ricongiunzione. */

						HMEM_SIZE(p2) += HMEM_SIZE(p) + 1;
						HMEM_NEXT(p2) = HMEM_NEXT(p);
						HMEM_NEXT(r) = p2;
					}
					else {

					/* Inserimento. */

						HMEM_NEXT(p2) = p;
						HMEM_NEXT(r) = p2;
					}
#else

				/* Crea un blocco occupato con il residuo, e lo libera. */

					p2 = q + n + 1;
					HMEM_SIZE(p2) = len - n - 1;

				/* NOTA : Questa non e` la miglior soluzione possibile :
				 la "free" riesamina l'intera lista dei blocchi liberi,
				 benche` a questo punto si conosca assai bene la posizione
				 del blocco da liberare. Una versione ottimizzata di questa
				 parte dovrebbe modificare la lista e preoccuparsi di even-
				 tuali ricongiunzioni con aree libere preesistenti. */

					free((LPVOID) HMEM_BUFF(p2));
#endif
				}

			/* Restituisce il nuovo indirizzo. */

				rv = (LPVOID) HMEM_BUFF(q);
				goto EXIT_FUNC;
			}
		}

		if (eb && ef &&
			HMEM_SIZE(q) + 1 + HMEM_SIZE(p) + size + 1 >= n) {

		/* Se il blocco si trova tra due liberi abbastanza grandi... */

		/* Sposta il blocco e lascia libero il tratto residuo (se puo`
		 esistere). */

		size_t len;

		/* Rimuove i due blocchi liberi dalla lista. */

			HMEM_NEXT(r) = HMEM_NEXT(p);

		/* Sposta il contenuto dell'area occupata. */

#if STATISTICA
	++n_memmove;
	n_mbytes += size;
#endif

			memmove((LPVOID) HMEM_BUFF(q),
					(LPVOID) HMEM_BUFF(ip),
					size * HMEM_T_SIZE);

		/* Assegna la nuova dimensione al blocco e libera il residuo,
		 se esiste. */

			len = HMEM_SIZE(q) + 1 + HMEM_SIZE(p) + size + 1;

#if MINIFRAGM
			if (len - n <= MIN_FRAGM_SIZE)
#else
			if (len - n < 2)
#endif
			{
				HMEM_SIZE(q) = len;
			}
			else {
				HMEM_SIZE(q) = n;

			/* Crea un blocco occupato con il residuo, e lo libera.
			  E` sufficiente reinserire il blocco nella lista, poiche` e`
			 impossibile che termini su di un altro blocco libero. */

				p = q + n + 1;
				HMEM_SIZE(p) = len - n - 1;
				HMEM_NEXT(p) = HMEM_NEXT(r);
				HMEM_NEXT(r) = p;
			}

		/* Restituisce il nuovo indirizzo. */

			rv = (LPVOID) HMEM_BUFF(q);
			goto EXIT_FUNC;
		}

		{
		/* Non c'e` alcun blocco abbastanza grande nelle vicinanze.
		 L'espansione e` impossibile. Si ricorre alla riallocazione
		 con movimento di dati. */

		register LPVOID new;

			if (ip + size + 1 == _end_heap - _heap) {

			/* Gestione del caso espansione dell'heap se la fine del buffer
			 da ridimensionare coincide con la fine dell'heap. */

			/* Se l'espansione non dovesse avere successo, si prosegue
			 con "malloc()...". */

				new = _heap_alloc(n - size);
				if (new) {

				/* Se l'espansione dell'heap ha successo, aumento il blocco
				 occupato di lunghezza "n-size" */

					HMEM_SIZE(ip) += n - size;

				/* Il blocco non ha cambiato indirizzo. Termina */

					rv = ptr;
					goto EXIT_FUNC;
				}
			}

			new = malloc(nb);
			if (! new) {
				rv = NULLPTR;
				goto EXIT_FUNC;
			}

#if STATISTICA
	++n_memmove;
	n_mbytes += HMEM_SIZE(ip);
#endif

			memcpy(new,ptr,HMEM_SIZE(ip) * HMEM_T_SIZE);

			free(ptr);

			rv = new;
			goto EXIT_FUNC;
		}
	}

EXIT_FUNC:
#if DEBUG
	{
		write(2,"realloc( ",9);
		h_write((long)ptr);
		write(2," , ",3);
		d_write((long)nb);
		write(2," ) = ",5);
		h_write((long)rv);
		write(2,"\n",1);
	}
#endif
#if CL_DEBUG
	if (! _heap_check()) {
		write(2,"\nrealloc : internal error\n",strlen("\nrealloc : internal error\n"));
		trappola();
	}
#endif
	return rv;
}


/*
* Funzione "calloc"
* -----------------
*
*  Tutti sanno cosa fa.
*/

LPVOID calloc(size_t nb, size_t size)
{
LPVOID rv;

#if STATISTICA
	++n_calloc;
#endif

	rv = malloc(nb * size);
	if (! rv) {
		goto EXIT_FUNC;
	}
	memset(rv,0,_msize(rv));

EXIT_FUNC:
#if CL_DEBUG
	if (! _heap_check()) {
		write(2,"\ncalloc : internal error\n",strlen("\ncalloc : internal error\n"));
		trappola();
	}
#endif
	return rv;
}


/* ######################################################################## */
/* ######################################################################## */
/* ######################################################################## */

/*
* Funzione "_heap_view"
* ---------------------
*
*/

void _heap_view(void)
{
HMEM p,q;

	printf("heap size = %d\n",_end_heap - _heap);
	for (p = q = FREELIST; q < _end_heap - _heap;) {
		if (p == q) {
			if (p == FREELIST) {
				printf("FREELIST : %d, %d -> %d\n",p,HMEM_SIZE(p),HMEM_NEXT(p));
				p = HMEM_NEXT(p);
				q += 2;
			}
			else {
				printf("Free : %d, %d -> %d\n",p,HMEM_SIZE(p),HMEM_NEXT(p));
				p = HMEM_NEXT(p);
				q += HMEM_SIZE(q) + 1;
			}
		}
		else if (p != FREELIST && p < q) {
			printf("ERROR : Crak-si list (free = %d, busy = %d)\n",p,q);
			q += HMEM_SIZE(q) + 1;
		}
		else {
			printf("Busy : %d, %d\n",q,HMEM_SIZE(q));
			q += HMEM_SIZE(q) + 1;
		}
	}
}


/*
* Funzione "_ptr_check"
* ----------------------
*
*/

int _ptr_check(HMEM ip)
{
HMEM p,q;
int rv = 0;

	for (p = q = FREELIST; q < _end_heap - _heap;) {
		if (p == q) {
			if (q == ip) {
				rv = 0;
				break;
			}
			if (p == FREELIST) {
				p = HMEM_NEXT(p);
				q += 2;
			}
			else {
				p = HMEM_NEXT(p);
				q += HMEM_SIZE(q) + 1;
			}
		}
		else if (p != FREELIST && p < q) {
			printf("ERROR : Crak-si list (free = %d, busy = %d)\n",p,q);
			rv = 0;
			break;
		}
		else {
			if (q == ip) {
				rv = 1;
				break;
			}
			q += HMEM_SIZE(q) + 1;
		}
	}

	return rv;
}


/*
* Funzione "_heap_check"
* ----------------------
*
*/
HMEM ultimi[5];

int _heap_check(void)
{
HMEM p,q;

	ultimi[0] = ultimi[1] = ultimi[2] = ultimi[3] = ultimi[4] = 0;
	for (p = q = FREELIST; q < _end_heap - _heap;) {
		ultimi[0] = ultimi[1];
		ultimi[1] = ultimi[2];
		ultimi[2] = ultimi[3];
		ultimi[3] = ultimi[4];
		ultimi[4] = q;
		if (p == q) {
			if (p == FREELIST) {
				p = HMEM_NEXT(p);
				q += 2;
			}
			else {
				p = HMEM_NEXT(p);
				q += HMEM_SIZE(q) + 1;
			}
		}
		else if (p != FREELIST && p < q) {
			return 0;
		}
		else {
			q += HMEM_SIZE(q) + 1;
		}
	}

	return q == _end_heap - _heap;
}


#if STATISTICA
/*
* Funzione _mem_statis
* --------------------
*
*/

void _mem_statis(void)
{
	printf("\nmalloc=%d free=%d realloc=%d calloc=%d alloc=%d sbrk=%d memmove=%d\nBytes copied = %d\n",
			  n_malloc, n_free, n_realloc, n_calloc, n_alloc, n_sbrk, n_memmove, n_mbytes*HMEM_T_SIZE);
}
#endif


/*
* Funzione "_heap_free"
* ---------------------
*
*/

int _heap_free(void)
{
HMEM p,q;
int conta=0;

	for (p = q = FREELIST; q < _end_heap - _heap;) {
		if (p == q) {
			if (p == FREELIST) {
				p = HMEM_NEXT(p);
				q += 2;
			}
			else {
				conta++;
				printf("Free : %d, %d -> %d\n",p,HMEM_SIZE(p),HMEM_NEXT(p));
				p = HMEM_NEXT(p);
				q += HMEM_SIZE(q) + 1;
			}
		}
		else if (p != FREELIST && p < q) {
			printf("ERROR : Crak-si list (free = %d, busy = %d)\n",p,q);
			q += HMEM_SIZE(q) + 1;
		}
		else {
			q += HMEM_SIZE(q) + 1;
		}
	}

	return conta;
}
