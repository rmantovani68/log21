/* //*/
/* //								bcreader.h*/
/* //*/
/* //	Costanti per i messaggi del lettore BARCODE*/
/* //*/
#ifndef _BCREADER_H
#define _BCREADER_H

/* //	Formato dei messaggi ricevuti via seriale*/
enum
{
	BC_REC_DATA,

/* //	Altri codici*/
	BC_FRAME_ERROR,		/*	Frame error ricevuto dal BARCODE */
	BC_TO_WITHIN,		/*	Timeout within ricevuto dal BARCODE */

/* //	Codici usati nella comunicazione di messaggi*/
	BC_SETUP,
	BC_IN_CODE,
	BC_STAT,
};

/* //	Formato dei messaggi trasmessi via seriale*/
enum
{
	BC_SND_DATA,
};

/* //	Codici di errore ritornati dal modulo gestore del BARCODE*/
enum
{
	BC_STAT_OK,			/*	Tutto OK. Ricevuto ACK */
	BC_STAT_TIMING,		/*	Trasmissione in corso */
	BC_STAT_NORESP,		/*	BARCODE non risponde */
	BC_STAT_TIMEOUT,	/*	Timeout in ricezione risposta */
	BC_STAT_TIMEIN,		/*	Timeout within */
	BC_STAT_ERROR,		/*	Errore generico (ripetuti frame errors) */
};

#endif
