/*
	-----------------------------------------
     COSTANTI PER VALORI DEI TIMEOUT
	-----------------------------------------
*/

#define	TEMPOQ	100 				/* millisecondi timeout vis. debug variabili */
#define	TEMPOQFRONTE	100 		/* millisecondi timeout ricerca fronte trigger */
#define	TEMPOSTAT		500 		/* millisecondi timeout per la statistica */
#define	TEMPO_PLC_OPEN	1000 		/* millisecondi timeout per la apertura plc */

/* Dimensione della drawing area */
#define LARGX_DRA   700     /* dimensione x della drawing area */
#define LARGY_DRA   500     /* dimensione y della drawing area */
/* rm 25-07-2011 */
#define VAR_HEIGHT   12     /* altezza edit variabile PLC */

/* elementi definizione scatole illustrative moduli seriali */
/* scatola piccola */
#define ELARGXL 78              /* dimensione x esterna larghezza */
#define ELARGYL 15              /* dimensione y esterna altezza */
#define ILARGXL 76              /* dimensione x interna larghezza */
#define ILARGYL 14              /* dimensione y altezza */
#define ILARGXE	36				/* dimensione x interna del rettangolo di errore */

#define DIST_SX				20		/* distanza del rettangolo dal lato sinistro della drawing area */
#define	DIST_TOP			10		/* distanza dei rettangoli dall'alto */
#define	DIST_TOP_NUM		9		/* distanza dei numeri modulo dall'alto */
#define	DIST_TOP_ERR		22		/* distanza degli errori e delle scritte dei bit dall'alto */
#define DIST_STRING_SX		28		/* distanza della stringa dal lato sinistro della drawing area */
#define	DIST_ERR_SX_SPEC	657		/* distanza dell' errore dal lato sinistro della drawing area con moduli speciali*/
#define	DIST_ERR_SX_NO_SPEC	270		/* distanza dell' errore dal lato sinistro della drawing area senza moduli speciali*/
#define DISPX   			900     /* displacement x tra una fila da 16 e l'altra */

/* elementi definizione per la visualizzazione del trace */
#define	LARGXBOX	400		/* dimensione x del box di visualizzazione del trace */
#define	LARGYBOX	460		/* dimensione y del box di visualizzazione del trace */
#define	DISTXBOX	100		/* distanza del box di visualizzazione del trace dal bordo sx */
#define	DISTYBOX	20		/* distanza del box di visualizzazione del trace dal bordo alto */
#define	DISTYTRACE	28		/* distanza tra variabili da visualizzare nel trace */

/* elementi definizione per la visualizzazione della statistica */
#define	LARGXSTAT		100	/* dimensione x del box di visualizzazione della statistica */
#define	LARGYSTAT		100	/* dimensione y del box di visualizzazione della statistica */
#define	CAMPIONISTAT	50	/* numero di campioni della statistica */

/* equate per debug variabili */

#define XDEBVAR   130     /* displacement x tra nome e valore in debug variabili*/

#define	MAX_BUFFER_NUMBER	4	/* numero di buffer totali per il trace */

/*
	-----------------------------------------
	COSTANTI VALORI DI RITORNO DELLE FUNZIONI
	-----------------------------------------
*/
#define RETER	-1
#define RETOK	 0


/*
        ---------------------------------------------------------
        COSTANTI CHE DISCRIMINANO L'INIZIO E LA FINE ATTESA TRACE  
        ---------------------------------------------------------
*/
#define INIZIO_ATT      0
#define FINE_ATT        1

/*
    -----------------------------------------
        COSTANTI RELATIVE AL MENU FILE  
    -----------------------------------------
*/
#define FILE_OPEN 0
#define FILE_SAVE 1 
#define FILE_EXIT 2
#define LOAD_PLC  3
#define RESTORE_PLC  4

/*
    -----------------------------------------------
        COSTANTI DEL WIDGET DI  SELEZIONE FILE  
    -----------------------------------------------
*/
#define ID_DIR_VARP 0
#define ID_DIR_PPLC 1

#define ID_DIR_FILEVR	0		/*variabili x lettura*/
#define ID_DIR_FILEVW	1		/*variabili x scrittura*/
#define ID_DIR_FILEPR	2		/*programma  plc x lettura*/

/*
    ----------------------------------------------------------------------
        COSTANTI PER LA DEFINIZIONE DEL TIPO DI VARIABILE GLOBALE DEL PLC
    ----------------------------------------------------------------------
*/
#define CHAR_TYPE   	0    
#define LONG_TYPE 		1
#define FLOAT_TYPE 		2
#define PUNT_CHAR_TYPE  3    
#define INT_TYPE		4
#define UINT_TYPE 		5
#define	ULONG_TYPE		6
#define STRING_TYPE  	7    

/*
    ----------------------------------------------------------------------
        COSTANTI PER LA CHIAMATA DEI BOTTONI VERTICALI 
    ----------------------------------------------------------------------
*/
#define RUN_BUTTON			0
#define HALT_BUTTON			1
#define START_BUTTON		2
#define STOP_BUTTON			3
#define SAL_DIS_BUTTON		4
#define DEC_EX_BUTTON		5
#define TRACE_BUTTON		6
#define PRE_TRIPB_BUTTON	7

/*
    -----------------------------------------------------------------
        COSTANTI PER LA IDENTIFICAZIONE DELLE IOS E PER IL SUO STATO 
    -----------------------------------------------------------------
*/

#define IOS_MAX       4        	/*ipotizzo di avere un massimo di 4 ios*/
#define NO_DISP       0        	/*displacement della ios 1 di ciascuna board*/
#define SI_DISP  0x1000		/*displacement della ios 2 di ciascuna board*/  
#define IOS_OK					0		/* ios a posto */
#define COMUNICATION_ERROR		1		/* errore di comunicazione */
#define FAULT					2		/* modulo in fault */

/*
    --------------------------------------------------
      COSTANTI PER LA DEFINIZIONE BOTTONI DEL POPUP
    --------------------------------------------------
*/
#define	PDB_SRVZ_INT	1
#define	PDB_SRVZ_REAL	2
#define	PDB_SRVZ_TRACE	3

/*
   -----------------------------------------
        COSTANTI DEI CODICI ERRORE
    -----------------------------------------
*/

#define  NO_SERVER					100	/* non sono riuscito ad aprire il server dati macchina */
#define  NO_MAIN_SHELL				101	/* plc2 - FATAL: unable to create MAIN SHELL */
#define  NO_LINGUE					102	/* non sono riuscito a caricare le lingue */
#define  NO_TIPO					103	/* Tipo sconosciuto ! */
#define  NO_WIDGET					104	/* Widget non trovato */
#define  NO_VARIABLE				105	/* non esiste */
#define  PLC_GO						106	/* il plc sta andando mentre si tenta di cambiare programma (non piu' usata) */
#define  NO_SAVE					107	/* non sono riuscito a salvare il file */
#define  NO_WRITE					108	/* non sono riuscito a scrivere  tutto il file */
#define  NO_LOAD					109	/* non sono riuscito a leggere il  file */
#define  NO_MALLOC					110	/* fallita la  malloc */
#define  NO_READ					111 /* non sono riuscito a leggere l'intero file mancano delle parti */
#define  NO_plcModProp 				112 /* fallita la funzione plcModProp */
#define  NO_INDIRIZZO_VAR_NON_INIZ 	113 /* fallita la ricerca dell'indirizzo di una variabile non inizializzata */
#define	 DEBUG_IN_START				114 /* il debug sta andando, metterlo in stop (non piu' usata)*/
#define  NO_VARIABLE_BOOLEANA		115	/* la variabile indicata non e' booleana */
#define  MACCHINA_IN_START			116	/* la macchina e' in start mentre si tenta di mettere il plc in HALT */
#define  PLC2_ERD_FLGMAC			117	/* Errore lettura flags (start ...) (DAVIDE) */
#define  NO_OPEN					118	/* Non sono riuscito ad aprire il Plc dopo 2 minuti */
#define  NO_PLC_OPEN				119	/* il plc non e' ancora stato aperto */
#define  NO_ISAKER					120	/* E' fallita la plcGO probabilmente perche' manca isaker */
#define	 NO_FRONTE_TRIGGER			121 /* Non e' stato trovato il fronte del trigger nel caso di pretrigger */
#define  NO_ALLOC_COLOR				122 /* Non si e' riusciti ad allocare un colore */
#define	 NO_PLC_TRACE_START			123 /* Fallita la partenza del trace */
#define  NO_PLC_TRACE_DOWNLOAD		124 /* Fallito il download del buffer di trace */
#define	 NO_PLC_TRACE_STOP			125 /* Fallito lo stop del trace */
#define	 NO_PLC_VAR_CHECK			126 /* Fallito il controllo delle variabili globali del plc */
#define  NO_PLC_STATISTIC			127 /* Fallita la funzione che preleva la statistica */
#define  NO_PLC_STATISTIC_EN		128 /* Fallita la abilitazione o disabilitazione della statistica */
#define  NO_PLC_STATISTIC_CLEAR		129 /* Fallito il reset della statistica */
#define  NO_PLC_HALT				130 /* Fallito il tentativo di mettere il plc in Halt */
#define  NO_PLC_TRACE_CLEAR			131 /* Fallito il reset del trace */
#define  NO_PLC_TRACE_VAR			132 /* Fallito il tentativo di aggiungere delle variabili nel trace */
#define  NO_PLC_LOAD_PROG			133 /* Fallito il tentativo di caricare un programma plc */
#define	 NO_PLC_LD_STATUS			134 /* Fallito il restore del plc */
#define	 NO_PLC_SAVEIO				135 /* Fallito il salvataggio degli io da parte del plc */
#define	 NO_SET_FILESELBOX_TYPE		136 /* Fallito il settaggio del tipo del file selection box */
#define	 NO_SET_FILESELBOX_OK		137 /* Fallito il settaggio della callback dell'OK Button del File Selection BOX */
#define	 NO_PLC_SECOND_INDEX		138 /* Non c'e' la dimensione di colonna della variabile */

/* definizioni per i pixmap */

typedef struct {
        String      xpm_nomef;          /*Nome del file di pixmap*/
        String      xbm_dati;           /*address dei dati*/
        Dimension   xbm_width;          /*dimensione in ascissa*/
        Dimension   xbm_height;         /*dimensione in ordinata*/
        String      xbm_foreg;          /*colore di foreground*/
        } Icona;
#ifdef Linux
#define DIR_ICONE       "./bmp/"    /*direttorio di default icone*/
#else
#define DIR_ICONE       "/home/xnc/iconcni/"    /*direttorio di default icone*/
#endif

/* Codice di validazione della struttura registrata in ram non volatile. */

#define PLC_NVRAM_CHECK_V1 0xbe191061

/* Struttura immagazzinata nella zona privata della RAM non volatile.
 In previsione di futuri sconvolgimenti, il primo elemento della struttura
 e` un codice che fa sia da firma che da versione, e permette di decidere
 come interpretare la struttura stessa. */

union nvram_ioconf_t {
	struct {
    		unsigned long check;
    		unsigned char board[8];
    		unsigned char images[1][32];
	} v1;
};

/* 
	-------------------------------
		VARIE
	------------------------------
*/

#define VER_PLC 			"Plc Debug 2.6.2.11"
#define IDS_PLC2_VER		"2.6.2.11"
#define IDS_SIGLA_PLC2		"PDB"
#define NUMERO_VARIABILI	16
#define NUMERO_CAMPIONI		400
#define	CAMPIONI_SHIFT		50
#define	SEGNALE_DI_SCAMBIO	0
#define VARIABILE_PLC		1
#define MAX_PATH_LEN        100


