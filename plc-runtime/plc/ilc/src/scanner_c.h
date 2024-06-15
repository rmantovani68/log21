
/*
* @(#) scanner.h 3.0 Fri Oct 11 14:32:17 MET 1996
* @(#) Rif. Guerrini
*
*  Primitive di lettura e riconoscimento degli elementi base di
* un linguaggio simil-C.
*
* 24/10/94 1.2 Versione stabilizzata, distribuita in CNI informatica.
* 26/06/96 2.0 Adattamenti per utilizzo nel compilatore PLC-IEC1131.
*          Aggiunto un flag di lettura "case-insensitive".
*          Modificato il default per i marcatori di commento.
* 07/10/96 2.1 Modificato il significato del descrittore di parole chiave.
*          Non si tratta piu` di un semplice insieme di nomi, ma contiene
*          anche informazioni riguardanti le proprieta` del nome.
* 11/10/96 GG 3.0 Aggiunta la gestione delle stringhe.
*/


#ifndef _SCANNER_H_

#define _SCANNER_H_

#include "sysdep.h"


/* Struttura descrivente una parola chiave. */

typedef struct {
	const LPCHAR name;
	char flags;
} SCNKW, PTR_ LPSCNKW;

/* Struttura di controllo dello scanner. Descrive la parte
 configurabile del pacchetto. */

typedef struct scnctl_t {

	/* Funzione di lettura di una linea. */
	int (PTR_ readln)(void);

	/* Puntatore al buffer di lettura. */
	LPCHAR line;

	/* Lunghezza del buffer di lettura. */
	int l_line;

	/* Posizione corrente di lettura. */
	LPCHAR curr;

	/* Posizione dell'ultimo identificatoreo stringa numerica. */
	LPCHAR pred;

	/* Numero di linea. */
	int n_line;

	/* Tabella delle parole chiave correntemente utilizzata. */
	LPSCNKW kwt;
	int kwt_len;

	/* "Cache" per le parole chiave. Registra l'ultimo indice di
	 parola chiave trovato. */

	int last_kw;

	/* Funzione di acquisizione dello stato corrente del lettore. */
	int (PTR_ getrdsts)(LPVOID);

	/* Funzione di impostazione dello stato corrente del lettore. */
	int (PTR_ setrdsts)(LPVOID);

	/* Dimensione dell'oggetto rappresentante lo stato del lettore. */
	int l_rdsts;

	/* Stringa di inizio commento. */
	LPCHAR start_co;

	/* Lunghezza della stringa di inizio commento. */
	int l_start_co;

	/* Stringa di fine commento. */
	LPCHAR end_co;

	/* Lunghezza della stringa di fine commento. */
	int l_end_co;

	/* Flag di commento aperto. */
	int incomment;

	/* Carattere utilizzato come inizio e fine stringa. */
	char start_str;

	/* Carattere utilizzato come escape nelle stringhe. */
	char esc_str;

	/* Flag che rende di tipo "blank" il carattere "\n". */
	int lf_blank;

	/* Flag di "case insensitive". */
	int case_insensitive;

	/* Flag che fa si` che il carattere "-" possa comparire all'interno
	 di un identificatore. */
	int minus_into_sym;

	/* Lunghezza massima di un identificatore. */
	int maxidlen;

	/* Lunghezza massima di un intero. La lunghezza massima di un float
	 e` invece 3*maxnumlen + 3. */
	int maxnumlen;

	/* Lunghezza massima di una costante stringa. */
	int maxstringlen;

} SCNCTL, PTR_ LPSCNCTL;

/* Struttura di stato dello scanner. Descrive (per l'applicazione chiamante)
 la posizione di lettura corrente. Contiene una parte dipendente dalla
 modalita` di acquisizione dei caratteri, la cui gestione e` a carico
 dell'applicazione. La dimensione effettiva della struttura, comprensiva
 della parte dipendente dal sistema, qui rappresentata da un campo di
 un byte, deve essere scritta nel campo "l_rdsts" all'atto della 
 chiamata della funzione "scInit()". */

typedef struct scnsts_t {

	/* Posizione di lettura corrente dello scanner (puntatore all'interno
	 del buffer di lettura). */
	LPCHAR curr;

	/* Posizione di lettura dell'ultimo identificatore o stringa numerica
	 (puntatore all'interno del buffer di lettura). */
	LPCHAR pred;

	/* Numero di linea. */
	int n_line;

	/* Tabella delle parole chiave correntemente utilizzata. */
	LPSCNKW kwt;
	int kwt_len;

	/* "Cache" per le parole chiave. Registra l'indice dell'ultima
	 parola chiave trovata. */

	int last_kw;

	/* Struttura dati codificante la posizione nello stream
	 di lettura (qualsiasi cosa esso sia) dell'ultima linea letta dalla
	 funzione di input definita dall'utente (campo "readln" della struttura
	 di controllo). La dimensione di questo campo dipende dall'applicazione
	 e dal sistema. Non e` quindi corretto calcolare la dimensione
	 effettiva di un istanza di questa struttura tramite "sizeof".
	  Un buon artificio per mantenere pulizia nel codice consiste nel
	 rinchiudere l'oggetto all'interno di una struttura :

			typedef struct mio_stato {
				SCNSTS sc;
					.... /-* Parte dipendente dal sistema, da non manipolare
							direttamente. *-/
			} MIO_STATO;

	 Il tipo MIO_STATO cosi` definito produce oggetti della dimensione
	adeguata. Le primitive del pacchetto potranno accedere al campo "sc"
	senza pericolo. */

	char rdsts[1];

} SCNSTS, PTR_ LPSCNSTS;

/* VARIABILI */

/* Puntatore associato alla struttura di controllo. */

extern LPSCNCTL pScCtl;

/* FUNZIONI */

#endif /* _SCANNER_H_ */

