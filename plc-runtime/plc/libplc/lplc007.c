/* lplc007.c */
#include "lplc.h"

/*
* Funzione "plcSafeCopyVar"
* -------------------------
*
*  Questa funzione trasferisce "size" byte dal segnale di scambio
* di chiave base "var", con indici di riga e di colonna "row" e "col",
* nel buffer "dest". Sono eseguiti controlli di congruenza dei dati
* della funzione per assicurare che si acceda solo ad indirizzi di
* memoria validi. Se tali controlli rilevano errori, la funzione
* evita di leggere dall'area dei segnali di scambio, azzera la
* destinazione ed esce con valore di ritorno 0. Se non sono rilevati errori,
* la funzione copia i byte richiesti in "dest", e termina con valore di
* ritorno 1.
*  Se "dest" e` il puntatore NULL, la funzione si limita a convalidare
* i dati e riportare il risultato del controllo nel valore di ritorno.
*
* NOTA 1 : Questa funzione non e` in grado di rilevare tutti i possibili
* accessi illegali. In particolare, non e` possibile verificare se un
* indice di riga superi la dimensione del vettore "var", ne` e` possibile
* controllare che la dimensione della destinazione sia compatibile con quella
* della variabile da leggere. Per l'indice di riga, e` possibile solo
* trovare un limite superiore teorico, corrispondente alla dimensione della
* tabella delle variabili. Per quanto riguarda "size", la funzine ammette
* un massimo di 8 byte per la destinazione. Non esistono infatti variabili
* di dimensione maggiore. Cio` che la funzione comunque assicura e` che,
* dati certi indici, non si rischi di accedere ad un indirizzo di memoria
* non valido, cioe` non facente parte dello spazio di indirizzamento del
* processo. Parte del controllo sulla sensatezza logica dell'operazione
* resta a carico del chiamante.
*
* NOTA 2 : L'uso incondizionato di due indici ha senso per tutti i segnali
* di scambio, anche se scalari o vettori monodimensionali. I primi infatti
* possono essere considerati matrici 1x1 (gli indici saranno quindi 0,0),
* i secondi sono matrici Nx1 (il secondo indice sara` quindi 0).
*/

int plcSafeCopyVar(int var,int row,int col,char * dest,int size)
{
register int off,lim;

	lim =  spDir[SPC_TBL].size / sizeof(*pKeyTab);

	if (
	/* Se e` definita la tabella */
	       pKeyTab
	/* e il PLC e` in GO */
	    && pStatus[PLCSTS_GO]
	/* e la chiave ha un valore sensato */
	    && var >= 0
	    && var < lim
	/* e la dimensione ha un valore ammissibile */
	    && size > 0
	    && size <= sizeof(double)
	/* e l'indice di riga ha un valore credibile */
	    && row >= 0
	    && row < lim
	/* e l'indice di colonna e` dentro i limiti */
	    && col >= 0
	    && col < pKeyTab[var].dim
	/* e la chiave finale e` minore del massimo assoluto */
	    && (var += col + pKeyTab[var].dim * row) < lim
	/* e la variabile esiste */
	    && (off = pKeyTab[var].off)) {

	/* si puo` procedere con la copia, */

		if (dest)
			memcpy(dest, pchMem + off, size);
		return 1;
	}
	else {

	/* altrimenti, niente. */

		if (dest)
			memset(dest,0,size);
		return 0;
	}
}

