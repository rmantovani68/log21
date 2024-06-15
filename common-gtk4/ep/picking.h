/*
* picking.h
* Easy Picking General Include file 
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#ifndef	_PICKING_H_
#define	_PICKING_H_

#include <glib.h>
#include <gio/gio.h>

#include "linklist.h"

/* application class */
#define APP_CLASS "easy-picking"

/* generic resources location */
#define APP_GENERIC "generic"

/* utilities resources location */
#define APP_UTILITIES "utilities"

/* use postgres for database backend */
#define USE_POSTGRES
/* use mysql for database backend */
/* #define USE_MYSQL */

/* Trace files max size : 10 MB */
#define	PICKING_TRACE_FILE_MAXSIZE	(10*1024*1024)

/* 
* gestisco al massimo 1024 righe d'ordine 
*/
#define	MAX_RIGHE_ORDINE	1024

#define MAX_UBICAZIONI    10000
#define MAX_CEDOLE        32

#define MAX_ISOLE       32
#define MAX_SETTORI    100
#define MAX_DISPLAYS   100

#define MAX_PKL	         4	/* numero max di moduli PKL presenti */
#define MAX_RIGHE_PKL		64
#define MAX_COLONNE_PKL	64

/*
* gestisco 128 imballi diversi
*/
#define MAX_IMBALLI       128

#define LOW_PRIORITY  1000

#ifndef MIN_MAX_DEFINED
#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))
#endif

#ifndef BYTE_TYPE_DEFINED
typedef unsigned char BYTE;
#define BYTE_TYPE_DEFINED
#endif

typedef enum {
	DOPO_IL_LANCIO,
	INIZIO_PRELIEVO
} STAMPA_RAC;

typedef enum {
	TOLLERANZA_ASSOLUTA,
	TOLLERANZA_PERCENTUALE,
	TOLLERANZA_PERCENTUALE_SOGLIA_MASSIMA,
	TOLLERANZA_CEDOLA
} TOLLERANZA;

typedef enum {
	PESATURA_ORDINE,       /* Il fine linea effettua il controllo peso su un intero ordine di produzione */ 
	PESATURA_COLLO         /* Il fine linea effettua il controllo peso su ogni collo */
} GESTIONE_PESATURA;

typedef enum {
	EQUILIBRATURA_PEZZI,
	EQUILIBRATURA_RIGHE,
} EQUILIBRATURA;

typedef enum {
	PRINT_ASCII,
	PRINT_A2PS,
	PRINT_HTML2PS
} TIPO_STAMPA_FILE;

typedef struct TagSemaforo {
	ep_bool_t bFree;
} SEMAFORO, *PSEMAFORO;

typedef struct TagOrdine {
	LINK_ITEM link;
	char szOrdProg[80]; /* Progressivo della Spedizione */
	char szCodCli[80];  /* Codice Cliente */
	char szCodVet[80];  /* Codice Vettore */
	char szDatiVettore[80];  /* Dati Vettore */
	char szOrdProv[80]; /* Provenienza Ordine */
	char szOrdTipo[80]; /* Tipo Ordine */
	char szROCDLNA[80]; /* Codice Linea */
	char szROTRMAR[80]; /* Terminal Arrivo */
	char szROZNCNS[80]; /* Codice Linea */
	char cFlag;         /* Flag ordine originale */
	char cStato;        /* Stato ordine originale */
	char cStatoOrdineElaborato;
	char cStatoColloElaborato;
	char cStatoRigaElaborata;
	int nIsola;
	int nColliAutomatici;
	int nColli;
	int nRighe;
	int nPrese;
	int nCopie;
	int nPesoTotale;
	int nVolumeTotale;
	int nPesoUsato;
	int nVolumeUsato;
	int nProgressivoLancio;
} ORDINE, *PORDINE;

typedef struct TagCollo {
	char szOrdProg[80];
	char szCodiceImballo[80];
	char szBarcode[80];
	char szBarcodeVettore[80];
	int nCategoria;
	int nIndiceImballo;
	int nProgressivoRiga;
	int nSwLinea;
	int nVolume;
	int nCollo;
	int nCopie;
	int nRighe;
	int nPeso;
} COLLO, *PCOLLO;

typedef struct TagRigaOrdine{
	char szOrdProg[80];
	int nCollo;
	int nProgressivoRiga;
	int nProgRigaRicevuta;
	char szCodiceProdotto[80];
	char nPrezzo;
	char szUbicazione[80];
	int nPriorita;
	int nLength;
	int nWidth;
	int nHeight;
	int nDim[3];
	int nCopieOrdinate;
	int nCopieCaricate;
	int nPesoCopia;
	int nPesoTotale;
	int nVolumeTotale;
	int nVolumeCopia;
	int nSwFF;
	int nCategoriaFF;
	int nSwPCF;
	int nCopiePCF;
	int nCopiePCP;
	int nCopieFF;
	char szFCPCF[80];
	char cFlag;
} EP_RIGA, *PEP_RIGA;

typedef struct TagImballo {
	char szCodice[40];
	int nDim[3];
	int nHeight;
	int nWidth;
	int nLength;
	int nVolume;
	int nVolumeUtile;
	int nTara;
	int nPesoMax;
	int nVolPerc;
	int nSwLinea;
	int nCategoria;
} IMBALLO, *PIMBALLO;


typedef struct TagImballi {
	int nImballi;
	int nImballiAutomatici;
	IMBALLO Imballo[MAX_IMBALLI];
} IMBALLI, *PIMBALLI;

typedef struct TagUbicazione {
	char szCodProd[40];
	char szCodUbi[40];
	char szLocazione[40];
	int  nSettore;
	int  nIsola;
	int  nDisplay;
	int nIOS;
	int nCPU;
	int nModulo;
	int nRowIndex;
	int nColIndex;
	int nPriorita;
	int nIndex;
} UBICAZIONE, *PUBICAZIONE;

typedef struct TagUbicazioni {
	int nUbicazioni;
	UBICAZIONE Ubicazione[MAX_UBICAZIONI];
} UBICAZIONI, *PUBICAZIONI;


typedef struct tagEvento {
	int nSettore;
	int nIsola;
	int nEvento;
	int nModulo;
	int nIOS;
	int nCPU;
	int nRiga;
	int nColonna;
	PUBICAZIONE pUbicazione;
} EVENTO, *PEVENTO;

typedef struct {
	int nCedola;
	long lLastOrdine;
	int nOrdini;
	int nRighe;
	int nCopie;
	int nStartSettore;
	int nEndSettore;
} CEDOLASTRUCT, *PCEDOLASTRUCT;



/*
* Strutture di definizione Righe di prelievo
*/
typedef struct TagRigaPrelievo {
	char szCodSped[40];
	char szCodProd[40];
	char szCodUbi[40];
	int nProgressivo;
	int nNumCollo;
	int nNextCollo;
	int nNumCopie;
	int nModulo;
	int nRiga;
	int nColonna;
	int nStato;
	int nIOS;
	int nCPU;
	int nDisplay;
	int nIsola;
} RIGA_PRELIEVO, *PRIGA_PRELIEVO;


/*
* Strutture di definizione Imballi in prelievo
*/
typedef struct TagImballoPrelievo {
	char szOrdProg[40];
	char szFormato[40];
	int nNumCollo;
	int nIOS;
	int nCPU;
	int nModulo;
	int nRiga;
	int nColonna;
	int nDisplay;
} IMBALLO_PRELIEVO, *PIMBALLO_PRELIEVO;

typedef struct tagSettore {
	RIGA_PRELIEVO RigaOrdine[MAX_RIGHE_ORDINE]; /* righe d'ordine relative a questo settore */
	IMBALLO_PRELIEVO Imballo[MAX_RIGHE_ORDINE]; /* Imballi da prelevare */
	int  nIndiceImballo;                        /* puntatore all'imballo da prelevare */
	int  nNumeroImballi;                        /* numero degli imballi da prelevare */
	int  nIndiceRigaOrdine;                     /* puntatore alla riga d'ordine da prelevare */
	int  nNumeroRigheOrdine;                    /* numero delle righe d'ordine da prelevare */
	char szRiga_1_Display[10];                  /* Riga 1 del display */
	char szRiga_2_Display[10];                  /* Riga 2 del display */
	int nStatoRiga1;                            /* Stato della riga 1 del display (BLINKING / NORMALE) */
	int nStatoRiga2;                            /* Stato della riga 2 del display (BLINKING / NORMALE) */
	int nSettore;                               /* Numero progressivo del settore (offset 0) */
	int nIsola;                                 /* Codice Isola di appartenenza */
	int nStatoSettore;                          /* Stato attuale del settore */
	int nTipoSettore;                           /* Tipo attuale del settore */
	int nFlagSettore;                           /* Flag di abilitato/disabilitato */
	int nCedola;                                /* Cedola associata */
	int nFirstMod;                              /* Modulo  primo pulsante settore (priorita') */
	int nFirstRow;                              /* Riga    primo pulsante settore (priorita') */
	int nFirstColumn;                           /* Colonna primo pulsante settore (priorita') */
	int nFirstIOS;                              /* IOS primo pulsante settore (priorita') */
	int nFirstCPU;                              /* CPU primo pulsante settore (priorita') */
	int nFirstDisplay;                          /* Display primo pulsante settore (priorita') */
	int nIOS;                                   /* IOS  Tasto Premuto / da premere */
	int nCPU;                                   /* CPU  Tasto Premuto / da premere */
	int nModulo;                                /* Modulo  Tasto Premuto / da premere */
	int nRiga;                                  /* Riga    Tasto Premuto / da premere */
	int nColonna;                               /* Colonna Tasto Premuto / da premere */
	int nDisplay;                               /* DIsplay Attivo */
	int nNumCopie;                              /* Numero Copie */
	int nNumCollo;                              /* Numero Collo */
	int nStatoLampada;                          /* Stato Lampada */
	char szCodSped[40];                         /* Codice Ordine Attivo */
	char szCodProd[40];                         /* Codice Prodotto da prelevare/prelevato*/
	char szCodUbi[40];                          /* Codice Ubicazione del prodotto da prelevare/prelevato / in test */
	char szTitolo[80];                          /* Titolo del prodotto da prelevare/prelevato*/
	char szCodOperatore[40];                    /* Codice Operatore addetto al prelievo */
	char szNomeOperatore[40];                   /* Nome Operatore addetto al prelievo */
	char szCognomeOperatore[40];                /* Cognome Operatore addetto al prelievo */
	int nOpBlinking;                            /* Blinking del campo operazione in display */
} SETTORE, *PSETTORE;


/*
* Strutture comuni a tutti i moduli componenti il progetto
*/
typedef struct TagLineaStruct {
	unsigned int nStatoLinea;
	ep_bool_t bAnalisi;
	ep_bool_t bStampaRAC;
	ep_bool_t bStampa;
	ep_bool_t bRxData;
	ep_bool_t bTxData;
	ep_bool_t bRxActive;
	ep_bool_t bTxActive;
	ep_bool_t bPLCStatus;
} LINEA_STRUCT, *PLINEA_STRUCT;



typedef struct tagDisplayStruct{
	char szRiga_1_Display[10];                  /* Riga 1 del display */
	char szRiga_2_Display[10];                  /* Riga 2 del display */
	int nStatoRiga1;                            /* Stato della riga 1 del display (BLINKING / NORMALE) */
	int nStatoRiga2;                            /* Stato della riga 2 del display (BLINKING / NORMALE) */
	int nIOS;                                   /* numero della scheda ios associata al display */
	int nCPU;                                   /* numero della CPU ios associata al display */
	int nModulo;                                /* indirizzo del modulo associato al display */
	int nIsola;                                 /* Isola */
	int nSettore;                               /* Settore */
	int nDisplay;                               /* Display */
} DISPLAYSTRUCT, *PDISPLAYSTRUCT;


typedef enum {
	ATTESA_PRELIEVO,
	PRELEVATA
} StatoRiga;

typedef enum tagModiDisplay {
	NORMAL,
	BLINK,
} ModiDisplay;

typedef enum tagRigaDisplay {
	TUTTO,
	RIGA_1,
	RIGA_2,
} RigaDisplay;

typedef enum tagStatiLinea {
	LINEA_GENERAL_ERROR,

	LINEA_IN_START,
	LINEA_IN_STOP,
	LINEA_IN_TEST,

	NUM_STATI_LINEA,
} StatiLinea;


typedef enum tagStatiSettore {
    SETTORE_GENERAL_ERROR, 

	ATTESA,
	PRELIEVO_IMBALLI,
	PRELIEVO_COPIE,
	PASSA_SETT_SUCC,
	CHIUSURA_COLLO,
	FINE_ORDINE,
	DISABILITATO,
	TEST,

	NUM_STATI_SETTORE,
} StatiSettore;

typedef enum tagTipiSettore {
	SETTORE_START,
	SETTORE_INTERMEDIO,
	SETTORE_END,
} TipiSettore;

typedef enum tagStatiOrdProd {
	EVASO,
	DA_EVADERE,
	IN_EVASIONE,
} StatiOrdProd;

typedef enum tagTipiOrdine {
	ORDINE_AUTOMATICO,
	ORDINE_MANUALE,

	NUM_TIPI_ORDINE,
} TipiOrdine;

typedef enum tag_tipi_task {
	COD_TASK_LANCIO_CREA_FILE_CORRIERE,
	COD_TASK_GENERICO,
} tipi_task;

typedef enum {
	MAIL_FILE_CORRIERE,
	MAIL_XAB_PDF,
	MAIL_DIST_PDF,
} TIPO_INVIO_MAIL;

/*
* Messaggi di stampa - in comune con common-new
*/
typedef enum tagTipiStampa {
	COD_STAMPA_FRONTESPIZIO_BANCALE,
	COD_STAMPA_DISTINTA_LINEA,
	COD_STAMPA_SPEDIZIONE,
} TipiStampa;

#define LEN_UBICAZIONE 12

#define QUADRATURA_OK               'O'
#define QUADRATURA_KO               'K'

/*
* stati dei records archivi operativi 
*/
#define CEDOLA_RICEVUTA               'R'
#define CEDOLA_TERMINATA              'T'


#define ORDINE_ATTESA_RICEZIONE       'Z'
#define ORDINE_RICEVUTO               ' '
#define ORDINE_CONTROLLO_QUADRATURA   'Q'
#define ORDINE_SPEDITO                'S'
#define ORDINE_SPEDITO_DA_EVADERE     'K'
#define ORDINE_IN_ANALISI             'I'
#define ORDINE_ELABORATO              'A'
#define ORDINE_STAMPATA_RAC           'R'
#define ORDINE_IN_PRELIEVO            'P'
#define ORDINE_PRELEVATO              'T'
#define ORDINE_EVASO                  'E'
#define ORDINE_STAMPATA_XAB           'X'
#define ORDINE_STAMPATA_DISTINTA      'D'
#define ORDINE_FILE_INVIATO           'F'
#define ORDINE_SPEDITO_HOST           'H'
#define ORDINE_ERRATO                 '*'

/*
* Stati del campo rofliem (ric_ord flag invio e-mail)
*	
*/
#define EMAIL_VETTORE_NON_SPEDITA        ' '
#define EMAIL_VETTORE_SPEDITA_AUTOMATICA 'A'
#define EMAIL_VETTORE_SPEDITA_MANUALE    'M'
#define EMAIL_VETTORE_DA_NON_SPEDIRE     'N'
//#define EMAIL_VETTORE_IN_SPEDIZIONE      'A'
#define EMAIL_VETTORE_IN_CODA            'Q'
#define EMAIL_VETTORE_ERRORE             'X'

#define FLUSSO_INVIATO                 'S'
#define FLUSSO_NON_INVIATO             'N'

/* sistema generico di spedizione mail */
#define EMAIL_IN_CODA            'Q'
#define EMAIL_ERRORE             'E'
#define EMAIL_SPEDITA            'S'
#define EMAIL_STORNATA           'X'

/* priorita' mail */
#define EMAIL_PRIORITA_MOLTO_BASSA   50	
#define EMAIL_PRIORITA_BASSA         40
#define EMAIL_PRIORITA_MEDIA         30
#define EMAIL_PRIORITA_ALTA          20
#define EMAIL_PRIORITA_MOLTO_ALTA    10

#define EMAIL_SPEDITA_DISTINTA      'D'
#define EMAIL_NON_SPEDITA_DISTINTA  'N'
#define EMAIL_SPEDITA_XAB           'X'
#define EMAIL_NON_SPEDITA_XAB       'N'

/*
* Stati del campo mvflaut (mail_vettori flag nvio iautomatico)
*	
*/
#define EMAIL_VETTORE_AUTOMATICA      'A'
#define EMAIL_VETTORE_MANUALE         'M'

#define COLLO_ELABORATO               'A'
#define COLLO_PRELIEVO                'P'
#define COLLO_CHIUSO                  'C'
#define COLLO_EVASO                   'E'
#define COLLO_SPEDITO_HOST            'H'
#define COLLO_ELIMINATO               'D'
#define COLLO_SCARTATO                'S'
#define COLLO_FUORI_LINEA             'F'
#define COLLO_INIZIATO                'I'   /* per i resi */

#define CONTENITORE_ATTIVO            'A'
#define CONTENITORE_SCARICATO         'S'
#define CONTENITORE_UBICATO           'U'
#define CONTENITORE_BANCALATO         'B'

#define BANCALE_CREATO                'C'
#define BANCALE_SPEDITO               'S'

#define CARICO_CONFERMATO             'C'
#define CARICO_SPEDITO                'S'

#define RIGA_ELABORATA                'A'
#define RIGA_PRELEVATA                'P'
#define RIGA_EVASA                    'E'
#define RIGA_SPEDITA_HOST             'H'
#define RIGA_SCARTATA                 'S'

#define RIFORNIMENTO_RICHIESTO        'S'
#define RIFORNIMENTO_EFFETTUATO       'E'
#define RIFORNIMENTO_CANCELLATO       'D'


/* ordine predestinato allo scarto in zona bilancia */
#define ORDINE_DA_SCARTARE            'X'

/* campo ubcdflg in ubicazioni */
#define UBICAZIONE_AUTOMATICA         'A'
#define UBICAZIONE_MANUALE            'M'

/* campo ubtpubi in ubicazioni */
#define UBICAZIONE_PALLET             'P'
#define UBICAZIONE_SCAFFALE           'S'

/* campo ubitipo in ubicazioni */
#define UBICAZIONE_CATALOGO           'C'
#define UBICAZIONE_LANCIO             'N'
#define UBICAZIONE_RESO               'R'

/*
* Flags dei records archivi operativi 
*/
#define ORDINE_RICEVUTO               ' '
#define ORDINE_SPEDITO                'S'
#define ORDINE_ELABORATO              'A'
#define ORDINE_STAMPATA_RAC           'R'
#define ORDINE_IN_PRELIEVO            'P'
#define ORDINE_COLLO_EVASO            'C'
#define ORDINE_PRELEVATO              'T'
#define ORDINE_EVASO                  'E'
#define ORDINE_STAMPATA_XAB           'X'
#define ORDINE_STAMPATA_DIST          'D'
#define ORDINE_SPEDITO_HOST           'H'

#define ORDINE_BLOCCATO               'B'
#define ORDINE_FORZATO                'F'
#define COLLO_RETTIFICATO             'R'
#define COLLO_CONTROLLATO             'C'

#define RIGA_BLOCCATA                 'B'
#define RIGA_TAGLIATA                 'T'
#define RIGA_RETTIFICATA              'R'
#define RIGA_IMPEGNATA                'I'
#define RIGA_NON_UBICATA              'U'


/*
* Tipi di collo particolari (cptpfor)
*/
#define COLLO_FUORI_FORMATO   'F'
#define COLLO_PALLET          'P'
#define COLLO_PRECONFEZIONATO 'C'

#define COLLO_AUTOMATICO       0
#define COLLO_MANUALE          1

/*
* Tipi di ordine (ORDTIPO :catalogo/lancio novita')
*/
#define ORDINE_CATALOGO               'C'
#define ORDINE_LANCIO                 'N'

/*
* Tipi di buttata ordine (rotpbut)
* Solo le G vanno spedite nel consuntivo
*/
#define ORDINE_TIPO_BUTTATA_G         'G'
#define ORDINE_TIPO_BUTTATA_C         'C'
/*
* Flag Nota (RIFLNOT)
*/
#define NOTA_CORRIERE_SETTATA         'S'
#define NOTA_CORRIERE_NON_SETTATA     'N'

/*
* FLAG ricezione ordine (ROFLRIC su import_ric_ord)
*/
#define IMPORT_ORDINE_OK              ' '
#define IMPORT_ORDINE_DOPPIO          'D'
#define IMPORT_ORDINE_DUPLICATO       'N'
#define IMPORT_ORDINE_SENZA_ARTICOLI  'A'


/*
* ROFLIMC (flag importazione in carichi) 
* Serve a tenere traccia degli ordini gia' importati nel programma
* di gestione carichi.
*/
#define CARICHI_ORDINE_DA_IMPORTARE   ' '
#define CARICHI_ORDINE_IMPORTATO      'I'

/*
* FLAG ricezione articolo (RAFLRIC su import_ric_art)
*/
#define IMPORT_ARTICOLO_OK                 ' '
#define IMPORT_ARTICOLO_DOPPIO             'D'
#define IMPORT_ARTICOLO_DUPLICATO          'N'
#define IMPORT_ARTICOLO_SENZA_ORDINE       'A'
#define IMPORT_ARTICOLO_CON_ORDINE_DOPPIO  'Z'


/*
* FLAG ricezione catalogo (PRFLRIC su import_ric_catalogo)
*/
#define IMPORT_CATALOGO_OK                 ' '
#define IMPORT_CATALOGO_DOPPIO             'D'
#define IMPORT_CATALOGO_DUPLICATO          'N'
#define IMPORT_CATALOGO_SENZA_ORDINE       'A'
#define IMPORT_CATALOGO_CON_ORDINE_DOPPIO  'Z'


/*
* ROFLIMC (flag importazione in carichi) 
* Serve a tenere traccia degli ordini gia' importati nel programma
* di gestione carichi.
*/
#define CARICHI_ORDINE_DA_IMPORTARE   ' '
#define CARICHI_ORDINE_IMPORTATO      'I'

/*
* Usati per l'invio del flusso udc restituiti a mondadori
*/

#define DISTINTA_NON_SPEDITA          ' '
#define DISTINTA_IN_SPEDIZIONE        'I'
#define DISTINTA_SPEDITA              'D'
#define DISTINTA_SPEDITA_STAMPATA     'S'

/*
* FLAG ricezione catalogo (PRFLRIC su import_ric_catalogo)
*/
#define IMPORT_CATALOGO_OK                 ' '
#define IMPORT_CATALOGO_DOPPIO             'D'
#define IMPORT_CATALOGO_DUPLICATO          'N'
#define IMPORT_CATALOGO_SENZA_ORDINE       'A'
#define IMPORT_CATALOGO_CON_ORDINE_DOPPIO  'Z'

#define ORDINE_PALLET	1
#define ORDINE_COLLI 	0

/*
* CARICHI
*/
#define PRODOTTO_EVASO                 'E'
#define PRODOTTO_RESTITUITO            'R'
#define PRODOTTO_CREATO                ' '


#define UDC_EVASO                 'E'
#define UDC_RESTITUITO            'R'
#define UDC_CREATO                ' '

#define DISTINTA_ERRATA                'X'
#define DISTINTA_EVASA                 'E'
#define DISTINTA_RESTITUITA            'R'
#define DISTINTA_CREATA                ' '

#define DISTINTA_NON_SPEDITA           ' '
#define DISTINTA_IN_SPEDIZIONE         'I'
#define DISTINTA_SPEDITA               'D'
#define DISTINTA_SPEDITA_STAMPATA      'S'


/*
* ROFLIMC (flag importazione in carichi) 
* Serve a tenere traccia degli ordini gia' importati nel programma
* di gestione carichi.
*/
#define CARICHI_ORDINE_DA_IMPORTARE           ' '
#define CARICHI_ORDINE_IMPORTATO              'I'

#define CARICHI_PRODUCT_CODE_LENGTH	14

#define CARICHI_IMPORT_CARICO_DA_IMPORTARE    ' '
#define CARICHI_IMPORT_CARICO_OK              'I'
#define CARICHI_IMPORT_CARICO_MOVIMENTATO     'M'

#define CARICHI_RIMANENZA_DA_SPEDIRE          ' '
#define CARICHI_RIMANENZA_SPEDITA             'S'


#endif	/* _PICKING_H_ */
