/*
* ProcList.h
* Strutture per la gestione dei processi
* costanti di identificazione dei messaggi
*/
#ifndef _PROC_LIST_H
#define _PROC_LIST_H

// #include <sys/types.h>

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define BOOL_TYPE_DEFINED
#endif

/* Indice dati processo in ProcList[] */
enum {
	PROC_MAIN,                /* Gestione Interfaccia Utente Processo MAIN */
	PROC_SETTORI,             /* Gestione Settori di prelievo */
	PROC_IOS,                 /* Gestione IOS CNI */
	PROC_VOLUM,               /* Gestione Analisi Volumetrica */
	PROC_PRINTLABEL,          /* Gestione Stampe */
	PROC_PESO,                /* Interfaccia utente pesatura su fine linea */
	PROC_BARCODE_BILANCIA,    /* Gestione Barcode Reader Zona Bilancia */
	PROC_BILANCIA,            /* Gestione Bilancia */
	PROC_RECEIVE,             /* Gestione Ricezione */
	PROC_BANCALI,             /* Gestione Bancali */
	PROC_EV2002,              /* Bilancia EV2002 */
	PROC_SCARTI,              /* Gestione Scarti */
	PROC_BARCODE_SCARTI,      /* Barcode lettura scarti */
	PROC_CHECK_BANCALI,       /* Controllo bancali fine linea */
	PROC_BARCODE_BANCALI,     /* Barcode lettura colli : controllo bancali fine linea */
	PROC_BARCODE_SETTORI,     /* Barcode gestione stampa packing list */
	PROC_AUTO_TASKS,          /* automatic tasks */
	PROC_BALANCE,             /* equilibratura */
	PROC_MONITOR,             /* monitoraggio */
	PROC_WRITESHM,            /* scrittura shared memory to memcache */
	PROC_READSHM,             /* lettura shared memory from memcache */
	PROC_DISPLAY_00,          /* display (x 40) */
	PROC_DISPLAY_01,          /* display (x 40) */
	PROC_DISPLAY_02,          /* display (x 40) */
	PROC_DISPLAY_03,          /* display (x 40) */
	PROC_DISPLAY_04,          /* display (x 40) */
	PROC_DISPLAY_05,          /* display (x 40) */
	PROC_DISPLAY_06,          /* display (x 40) */
	PROC_DISPLAY_07,          /* display (x 40) */
	PROC_DISPLAY_08,          /* display (x 40) */
	PROC_DISPLAY_09,          /* display (x 40) */
	PROC_DISPLAY_10,          /* display (x 40) */
	PROC_DISPLAY_11,          /* display (x 40) */
	PROC_DISPLAY_12,          /* display (x 40) */
	PROC_DISPLAY_13,          /* display (x 40) */
	PROC_DISPLAY_14,          /* display (x 40) */
	PROC_DISPLAY_15,          /* display (x 40) */
	PROC_DISPLAY_16,          /* display (x 40) */
	PROC_DISPLAY_17,          /* display (x 40) */
	PROC_DISPLAY_18,          /* display (x 40) */
	PROC_DISPLAY_19,          /* display (x 40) */
	PROC_DISPLAY_20,          /* display (x 40) */
	PROC_DISPLAY_21,          /* display (x 40) */
	PROC_DISPLAY_22,          /* display (x 40) */
	PROC_DISPLAY_23,          /* display (x 40) */
	PROC_DISPLAY_24,          /* display (x 40) */
	PROC_DISPLAY_25,          /* display (x 40) */
	PROC_DISPLAY_26,          /* display (x 40) */
	PROC_DISPLAY_27,          /* display (x 40) */
	PROC_DISPLAY_28,          /* display (x 40) */
	PROC_DISPLAY_29,          /* display (x 40) */
	PROC_DISPLAY_30,          /* display (x 40) */
	PROC_DISPLAY_31,          /* display (x 40) */
	PROC_DISPLAY_32,          /* display (x 40) */
	PROC_DISPLAY_33,          /* display (x 40) */
	PROC_DISPLAY_34,          /* display (x 40) */
	PROC_DISPLAY_35,          /* display (x 40) */
	PROC_DISPLAY_36,          /* display (x 40) */
	PROC_DISPLAY_37,          /* display (x 40) */
	PROC_DISPLAY_38,          /* display (x 40) */
	PROC_DISPLAY_39,          /* display (x 40) */

	NUM_PROC,
};

/* 
* Messaggi
* I messaggi scambiabili fra i processi sono divisi in fasce da 20
* attenzione a non superare il totale di 255 messaggi perche' il limite 
* massimo di messaggi gestibili e' appunto 255 (vedi PutField in msq_lib.c)
*/
enum {
	PROGRAM_STARTED,
	PROGRAM_STOPPED,
	DISPLAY_MSG,

	REFRESH_DATA,
	REFRESH_ORDINE,
	REFRESH_CONFIGURATION,

	START_ANALISI,
	START_ANALISI_TEST,
	ANALISI_ORDINE,

	PRINT_RAC,
	REPRINT_RAC,
	PRINT_RAC_COLLO,
	PRINT_RAC_COLLO_ADD,
	PRINT_PACKING_LIST_STORICO,
	PRINT_RAC_LINEA,
	PRINT_ALL_RAC,

	PRINT_OLD_RAC,
	REPRINT_OLD_RAC,
	PRINT_OLD_RAC_COLLO,
	PRINT_OLD_RAC_COLLO_ADD,
	PRINT_OLD_PACKING_LIST_STORICO,
	PRINT_OLD_RAC_LINEA,
	PRINT_OLD_ALL_RAC,
	
	PRINT_ALL_PACKING_LIST,
	PRINT_PACKING_LIST,
	PRINT_PACKING_LIST_COLLO,
	PRINT_PACKING_LIST_COLLO_STORICO,
	PRINT_PACKING_LIST_COLLO_PREFINCATO,
	PRINT_PACKING_LIST_LINEA_PREFINCATO,
	PRINT_ORDINI_DATA,
	PRINT_INIZIO_ELABORAZIONE,
	PRINT_FINE_ELABORAZIONE,
	PRINT_XAB,
	PRINT_XAB_STORICO,
	PRINT_DATI_ORDINE,
	PRINT_DATI_ORDINE_STORICO,
	PRINT_XAB_A4,
	PRINT_XAB_STORICO_A4,
	PRINT_XAB_LINEA,
	PRINT_XAB_LINEA_STORICO,
	PRINT_DC,
	PRINT_DC_STORICO,
	PRINT_DC_LINEA,
	PRINT_DC_LINEA_STORICO,
	PRINT_DISTINTA,
	PRINT_DISTINTA_STORICO,
	PRINT_CONTEGGIO_IMBALLI,
	CREATE_PACKING_LIST_COLLO,
	CREATE_XAB_A4,

	PRINT_XAB_A4_LINEA,

	START_LINEA,
	STOP_LINEA,

	RESET_SETTORE,
	RESET_ALL,
	SAVE_STATE,
	RESTORE_STATE,

	MAIN_REQ_WEIGHT,

	IOS_PKL_SET_RC,
	IOS_PKL_RESET_RC,
	IOS_PKL_CONGELA,
	IOS_PKL_SCONGELA,
	IOS_PKL_RESET_ALL,
	IOS_PKL_TEST,
	IOS_PKL_BUTTON_PRESSED,
	IOS_PKL_LIGHT_DAMAGED,
	IOS_DPY_CHAR,
	IOS_DPY_STRING,
	IOS_DPY_BCD,
	IOS_TEST_CMD,

	SETTORI_ORDINE_IN_PRELIEVO,
	SETTORI_ORDINE_SETT_SUCC,
	SETTORI_ORDINE_COLLO_PRELEVATO,
	SETTORI_ORDINE_PRELEVATO,
	SETTORI_ORDINE_EVASO,

	VOLUM_START_ANALISI_ORDINE,
	VOLUM_END_ANALISI_ORDINE,
	VOLUM_START_ANALISI,
	VOLUM_END_ANALISI,
	VOLUM_START_ANALISI_TEST,
	VOLUM_END_ANALISI_TEST,
	VOLUM_ERRORE_ANALISI_ORDINE,

	PRINTLABEL_RAC_STAMPATO,
	PRINTLABEL_RAC_COLLO_STAMPATO,
	PRINTLABEL_RAC_ERROR,
	PRINTLABEL_RAC_COLLO_ERROR,
	PRINTLABEL_PACKING_LIST_STAMPATO,
	PRINTLABEL_ALL_RAC_STAMPATO,
	PRINTLABEL_ALL_PACKING_LIST_STAMPATO,
	PRINTLABEL_PACKING_LIST_ERROR,
	PRINTLABEL_ORDINI_DATA_STAMPATO,
	PRINTLABEL_ORDINI_DATA_ERROR,
	PRINTLABEL_ALL_RAC_ERROR,
	PRINTLABEL_RAC_LINEA_STAMPATA,
	PRINTLABEL_RAC_LINEA_ERROR,
	PRINTLABEL_XAB_STAMPATA,
	PRINTLABEL_XAB_ERROR,
	PRINTLABEL_DATI_ORDINE_STAMPATA,
	PRINTLABEL_DATI_ORDINE_ERROR,
	PRINTLABEL_XAB_LINEA_STAMPATA,
	PRINTLABEL_XAB_LINEA_ERROR,
	PRINTLABEL_XAB_RISTAMPA,
	PRINTLABEL_DISTINTA_STAMPATA,
	PRINTLABEL_DISTINTA_STAMPATA_ORDINE,
	PRINTLABEL_DISTINTA_ERROR,
	PRINTLABEL_PACKING_LIST_CREATED,
	PRINTLABEL_PACKING_LIST_NOT_CREATED,
	PRINTLABEL_XAB_A4_CREATED,
	PRINTLABEL_XAB_A4_NOT_CREATED,
	PRINTLABEL_PACKING_LIST_LINEA_STAMPATA,
	PRINTLABEL_PACKING_LIST_LINEA_ERROR,


	BILANCIA_RX_DATA,
	BILANCIA_RX_ERROR,
	BILANCIA_TX_ERROR,

	BCRD_RX_BARCODE,
	BCRD_RX_NOREAD,
	BCRD_RX_ERROR,

	SOCK_CMD_DISPLAY_MSG,
	SOCK_CMD_DISPLAY_MSGBOX,
	SOCK_CMD_UPDATE_ORDINE,

	SOCK_OPEN_SERVICE,
	SOCK_CLOSE_SERVICE,

	METRO_RX_BARCODE,
	METRO_RX_ERROR,

	ALFA_RX_DATA,
	ALFA_RX_ERROR,
	ALFA_TX_ERROR,

	EV2002_RX_DATA,
	EV2002_RX_ERROR,

	AUTO_TASKS_FILE_CORRIERE_CREATO,
	AUTO_TASKS_FILE_CORRIERE_IN_CODA,
	AUTO_TASKS_FILE_CORRIERE_SPEDITO,
	AUTO_TASKS_FILE_CORRIERE_DA_NON_SPEDIRE,

	RIASSEGNA_NOTE_ORDINE_CEDOLA,
	REFRESH_LISTA_ORDINI,

	SIMULA_LUCE,

};


/*
* definizioni per la gestione dei comandi da ricevere
*/
enum Commands {
	CMD_DISPLAY_MSG,
	CMD_DISPLAY_MSGBOX,
	CMD_UPDATE_ORDINE,

	NUM_SOCK_CMDS,
};

enum StatiLinea {
	LINEA_IN_START,
	LINEA_IN_STOP,

	NUM_STATI_LINEA,
};

/*
* Strutture comuni a tutti i moduli componenti il progetto
*/
typedef struct TagLineaStruct {
	unsigned int nStatoLinea;
	BOOL bAnalisi;
	BOOL bStampaRAC;
	BOOL bStampa;
	BOOL bRxData;
	BOOL bTxData;
	BOOL bRxActive;
	BOOL bTxActive;
	BOOL bPLCStatus;
} LINEA_STRUCT, *PLINEA_STRUCT;


#endif /* _PROC_LIST_H */
