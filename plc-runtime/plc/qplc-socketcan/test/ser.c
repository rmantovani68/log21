
/*
* @(#) ser.c 1.0 Fri Jul 18 17:53:24 MET DST 1997
*
* Piccolo programma di prova, che esemplifica l'uso delle
* primitive di qplc per la gestione di linee seriali.
*
* 18/07/97 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>
#include <plcsym.h>

/* Le seguenti due strutture sono necessarie per il caricamento dinamico. */
char module_name[] = "U_CODE";
/* Versione. */
char ver_user[] = "seriali-da-plc 1.0 Fri Jul 18 17:54:46 MET DST 1997";

/*
* Questa variabile conterra` il codice che individua la linea
* seriale. E` formalmente una variabile di input, ma il suo contenuto
* e` definito una volta per tutte quando si realizza il collegamento con
* la linea seriale con "ioAttach".
*/

static long ser_handle = 0;

int UserStart(int * code)
{
char *dev;

/* Micro-MSD. */

	dev = "ser.com1.19200n81";

/* Associazione tra la variabile di programma "ser_handle" ed il
 dispositivo seriale. Se non ci sono errori, la funzione "ioAttach"
 assegna alla variabile un valore da utilizzare nelle successive
 "ioExtendedOp" per trasmettere o ricevere dati tramite il dispositivo
 associato. Poiche` "ioAttach" altera il contenuto della variabile,
 l'associazione deve essere considerata di INPUT. */

	if (! ioAttach(dev,&ser_handle,
	               sizeof(ser_handle)*8,DRIVER_MODE_INPUT)) {

	/* Un errore, se qualcosa va storto. E "0" come valore di
	 ritorno, per impedire l'entrata in GO del PLC. */

		plcError(5,"%s",dev);
		return 0;
	}

	return 1;
}

void UserStop(void)
{
}

/*
* Queste due strutture sono utilizzate rispettivamente per le trasmissioni
* e per le ricezioni da seriale. Si noti che devono essere oggetti statici
* (almeno rx, in questo esempio), perche` tengono traccia dell'evolvere
* delle operazioni di trasmissione e ricezione. In ricezione, in particolare, 
* il campo "msg_curr_pos" contiene il numero di byte ricevuti finora.
* Anche i buffer di trasmissione e ricezione dovrebbero essere statici,
* salvo che quello di trasmissione puo` essere dinamico se si e` certi che
* sia sempre piu` piccolo della parte libera della coda di trasmissione
* interna del PLC (normalmente 512 byte). Se questo non e` assicurato,
* la trasmissione puo` richiedere piu` di un ciclo di PLC per essere
* eseguita, ed il buffer deve pertanto essere statico. Il campo "msg_curr_pos"
* conterra` il numero di byte gia` trasferiti nella coda di trasmissione.
*/
static stream_rxtx_t tx,rx;
/* Buffer di trasmissione e ricezione. */
static char tmsg[80];
static char rmsg[80];

static unsigned long timer;

int UserMain(int start)
{
unsigned long t;

	t = plcTime();
	if (start) {
		timer = t;

/* Pone il flag di stato del trasmettitore a "trasmissione completata",
 vale a dire "nessuna trasmissione in corso".  */
		tx.status = DREXTOP_STREAM_F_DONE;

/* Azzera il numero di byte ricevuti. */
		rx.msg_curr_pos = 0;
	}

/*
* TRASMETTITORE
*
* Il messaggio e` normalmente piu` corto della parte libera della
* coda di trasmissione del PLC (lunga 512 byte), quindi sara` trasferito
* tutto in una volta. Se questo non fosse vero (coda piu` corta per
* diversa configurazione, velocita` di trasmissione piu` bassa,
* periodo di trasmissione piu` rapido...) si dovrebbe controllare
* il flag DREXTOP_STREAM_F_DONE per accertarsi che i dati siano
* stati trasferiti nella coda. Se il flag e` falso, bisogna richiamare
* la primitiva di trasmissione al ciclo successivo finche` non torna vero.
* La macro "TRASMISSIONE_LENTA" va posta ad 1 se si desidera che il programma
* esegua il controllo anziche` confidare nel fatto che ci sia sempre
* spazio nella coda di trasmissione. Nella maggior parte dei casi non
* e` necessario.
*/

#define TRASMISSIONE_LENTA 0

#if TRASMISSIONE_LENTA
/* Se la trasmissione iniziata al ciclo precedente non e` stata completata,
 la ritenta (la struttura "tx" ricorda il numero di byte gia` trasmessi). */
	if (! (tx.status & DREXTOP_STREAM_F_DONE))
		ioExtendedOp(ser_handle,DREXTOP_STREAM_OP_SEND,&tx);
#endif

/*
* Ogni due secondi e` emesso il messaggio
* "Sono le NNNN e tutto va bene"
* sulla linea seriale.
*/

	if (t - timer >= 2000) {

		timer = t;

#if TRASMISSIONE_LENTA
	/* Se due secondi non sono bastati a trasmettere un messaggio,
	 non resta che rassegnarsi all'errore. */
		if (! (tx.status & DREXTOP_STREAM_F_DONE))
			plcError(9999,"overrun");
#endif

	/* Preparazione del messaggio. */
		util_safe_sprintf(tmsg,sizeof(tmsg),
		                  "Sono le %d e tutto va bene.\n",t);

	/* Inizializzazione della struttura di comunicazione.
	 Si noti l'azzeramento del contatore di byte trasmessi. */
		tx.msg_curr_pos = 0;
		tx.message = tmsg;
		tx.msg_size = strlen(tmsg);

	/* Trasmissione. */
		ioExtendedOp(ser_handle,DREXTOP_STREAM_OP_SEND,&tx);
	}

/*
* RICEVITORE
*
* E` emesso il messaggio 100 ogni volta che si riceve un gruppo di 10
* byte dalla seriale.
*/

/* Preparazione della struttura di controllo (lo si potrebbe anche fare 
 volta per tutte). */
	rx.msg_size = 10;
	rx.message = rmsg;

/* Ricezione. */
	ioExtendedOp(ser_handle,DREXTOP_STREAM_OP_RECV,&rx);

/* Se la ricezione e` stata completata, cioe` sono arrivati tutti i
 byte richiesti, si emette un messaggio, e si prepara la struttura
 di stato per la ricezione di un nuovo pacchetto. */

	if (rx.status & DREXTOP_STREAM_F_DONE) {

	/* Stampa del pacchetto di dati ricevuto. */
		rmsg[rx.msg_size] = '\0';
		{
		char m[200];
		char *p,*q;

			for (p = rmsg,q = m; *p; ++p) {
				if (*p < 0x20 || *p >= 0x7F) {
					util_safe_sprintf(q,6,"\\x%x",
					             (unsigned char)*p);
					q += 3;
				}
				else {
					*q = *p;
				}
				++q;
			}
			*q = '\0';
			plcMessage(100,"Ricevuto \"%s\"",m);
		}

	/* Azzera il contatore di byte ricevuti. Con questo, il
	 ricevitore si prepara ad accogliere un nuovo pacchetto. */
		rx.msg_curr_pos = 0;
	}

	return 0;
}

