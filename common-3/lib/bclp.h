/* //*/
/* //								bclp.h*/
/* //*/
/* //	Costanti per i messaggi della stampante di BARCODE*/
/* //*/
#ifndef _BCLP_H
#define _BCLP_H

/* //	Formato dei messaggi ricevuti via seriale*/
enum
{
	LP_REC_GS,
	LP_REC_US,
	LP_REC_EM,
	LP_REC_DC3,
	LP_REC_DC4,
	LP_REC_FS,
	LP_REC_DC1,
	LP_REC_DC2,
	LP_REC_HT,
	LP_REC_SOH,
	LP_REC_RS,
	LP_REC_NAK,
	LP_REC_ERRCODE,

	/*	Altri codici */
	LP_FRAME_ERROR,		/*	Frame error ricevuto dalla stampante di BARCODE */
	LP_TO_WITHIN,		/*	Timeout within ricevuto dalla stampante di BARCODE */

/* //	Codici usati nella comunicazione di messaggi*/
	LP_RESET,
	LP_LABEL,
	LP_DATA,
	LP_ERRCODE,
	LP_STAT,
};

/* //	Formato dei messaggi trasmessi via seriale*/
enum
{
	LP_SND_RESET,
	LP_SND_DATA,
	LP_SND_ERRCODE,
};

/* //	Codici di errore ritornati dal modulo gestore della stampante di BARCODE*/
enum
{
	LP_STAT_OK,			/*	Tutto OK. Ricevuto ACK */
	LP_STAT_TIMING,		/*	Trasmissione in corso */
	LP_STAT_NORESP,		/*	Stampante di BARCODE non risponde */
	LP_STAT_TIMEOUT,	/*	Timeout in ricezione risposta */
	LP_STAT_TIMEIN,		/*	Timeout within */
	LP_STAT_ERROR,		/*	Errore generico (ripetuti frame errors) */
	LP_STAT_PRINT,		/*	Stampata etichetta */
};

#endif
