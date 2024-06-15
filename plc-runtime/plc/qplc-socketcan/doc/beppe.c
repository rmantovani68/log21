


/*********************************************************\
* Esempio di creazione di segnali di scambio da programma *
\*********************************************************/

/*
* Questo frammento di programma genera un segnale di scambio,
* o ne ottiene le caratteristiche se il segnale era gia` definito.
* Si suppone che questo codice sia aseguito all'interno di qplc o
* di qualche suo modulo caricato dinamicamente (es. hatsh.oo o prog.oo).
*/

#include <plcapi.h>

void esempio(void)
{
shv_t segnale;
int cambiato,chiave;

/* Nome della variabile. */

	strcpy(segnale.name,"MIO_SEGNALE");

/* Tipo e caratteristiche. sono ammessi i seguenti tipi : BOOL, CHAR, UCHAR,
 LONG, ULONG, FLOAT, DOUBLE (mai collaudato seriamente, quest'ultimo). */

	segnale.mode = SHV_MODE_TYPE_BOOL 
/* E` possibile richiedere che la variabile sia "a rilevemento di fronte".
 Nel caso interessi, togliere il commento qui. */
	             /* | SHV_MODE_EDGE */
/* E` possibile richiedere che la variabile risieda nella RAM non volatile.
 Nel caso interessi, togliere il commento qui. */
	             /* | SHV_MODE_RETAIN */
	             ;

/* Dimensioni della variabile. In questo caso, si sta definendo un vettore
 di dieci elementi. */
	segnale.dim1 = 10;
	segnale.dim2 = 1;

/* "Chiave" del segnale di scambio. Questo numero ha la stessa natura
 delle costanti che compaiono in "plcsym.h". Se si indica "-1", sara`
 il PLC stesso a scegliere la prima chiave libera disponibile. */

	segnale.key = -1;

/* Creazione dell'oggetto. Se l'oggetto esiste gia`, ed ha caratteristiche
 compatibili con quelle richieste, "chiave" conterra` la chiave da utilizzare
 per accedervi. */

	chiave = shvCreate(&segnale,&cambiato);

	if (chiave == -1) {
	/* ERRORE. E` successo qualcosa di grave. La cosa piu` probabile
	 e` che la tabella dei simboli sia piena. Puo` anche darsi che non
	 ci sia piu` memoria, o che i dati in "segnale" siano sbagliati. */

		/* ... */
	}
	else if (cambiato) {

	/* ERRORE. L'oggetto non ha esattamente le caratteristiche richieste.
	 Probabilmente e` stato definito precedentemente con tipo o dimensioni
	 diverse. Verificare. La struttura "scambio" contiene comunque le
	 proprieta` correnti dell'oggetto. */

		/* ... */
	}
	else {

	/* Ora la variabile "chiave" contiene il codice caratteristico
	 dell'oggetto. Conservare in luogo sicuro. */

	}
}


/*
* Il frammento di codice seguente mostra come si possano acquisire
* informazioni per accedere ad un segnale di scambio di cui si conosca solo
* il nome. Si suppone che questo codice sia eseguito all'interno di
* un'applicazione diversa da qplc (es. all_smis).
*/

#include <plclink.h>

void esempio(void)
{
struct qplc_shv_t * caratteristiche;
int chiave;

/*
*  "dbGetQplcVarDescr" trova l'indirizzo ed il descrittore di una variabile
* di QPLC dato il nome. L'indirizzo del descrittore e` copiato nel
* puntato del secondo parametro.
*  Se il nome non e` presente, o gli indici contenuti nel nome
* sono fuori dai limiti previsti per la variabile, riporta NULL. Questa
* funzione, infatti, interpreta i nomi del tipo "<nome>_<numero>..."
* come accessi indicizzati alla variabile "<nome>"; per esempio, la
* stringa "ABC_1_2" e` interpretata come ABC[1,2].
*/

	if (dbGetQplcVarDescr("MIO_SEGNALE",&caratteristiche) == NULL) {

	/* Mmh... questo nome non esiste, oppure c'e` un gran casino... */

		/* ... */
	}
	else {

	/* Trovata la variabile. La caratteristica che in genere interessa
	 e` la chiave di accesso. */

		chiave = caratteristiche -> key;

	/* Ora la variabile "chiave" contiene il codice caratteristico
	 dell'oggetto. Conservare in luogo sicuro. */

	}
}

