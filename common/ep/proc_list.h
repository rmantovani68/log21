/*
* ProcList.h
* Strutture per la gestione dei processi
* costanti di identificazione dei messaggi
*/
#ifndef _PROC_LIST_H
#define _PROC_LIST_H

/* Indice dati processo in ProcList[] */
enum {
	/*   00  */ PROC_MAIN,                /* Gestione Interfaccia Utente Processo MAIN */
	/*   01  */ PROC_SETTORI,             /* Gestione Settori di prelievo */
	/*   02  */ PROC_IOS,                 /* Gestione IOS A&L */
	/*   03  */ PROC_VOLUM,               /* Gestione Analisi Volumetrica */
	/*   04  */ PROC_PRINTLABEL,          /* Gestione Stampe */
	/*   05  */ PROC_PESO,                /* Interfaccia utente pesatura su fine linea */
	/*   06  */ PROC_BARCODE_BILANCIA,    /* Gestione Barcode Reader Zona Bilancia */
	/*   07  */ PROC_BILANCIA,            /* Gestione Bilancia */
	/*   08  */ PROC_RECEIVE,             /* Gestione Ricezione */
	/*   09  */ PROC_BANCALI,             /* Gestione Bancali */
	/*   10  */ PROC_EV2002,              /* Bilancia EV2002 */
	/*   11  */ PROC_SCARTI,              /* Gestione Scarti */
	/*   12  */ PROC_BARCODE_SCARTI,      /* Barcode lettura scarti */
	/*   13  */ PROC_CHECK_BANCALI,       /* Controllo bancali fine linea */
	/*   14  */ PROC_BARCODE_BANCALI,     /* Barcode lettura colli : controllo bancali fine linea */
	/*   15  */ PROC_BARCODE_SETTORI,     /* Barcode gestione stampa packing list */
	/*   16  */ PROC_AUTO_TASKS,          /* automatic tasks */
	/*   17  */ PROC_BALANCE,             /* equilibratura */
	/*   18  */ PROC_MONITOR,             /* monitoraggio */
	/*   19  */ PROC_WRITESHM,            /* scrittura shared memory to memcache */
	/*   20  */ PROC_READSHM,             /* lettura shared memory from memcache */
	/*   21  */ PROC_EDIT,                /* edit */
	/*   22  */ PROC_TEST,                /* test */
	/*   23  */ PROC_SETUP,               /* setup */
	/*   24  */ PROC_CARICHI,             /* carichi */
	/*   25  */ PROC_TEMPLATE,            /* template */
	/*   26  */ PROC_FINE_LINEA,          /* fine linea */
	/*   27  */ PROC_SOCKET_SERVER_SCANNER,   /* socket server scanner */
	/*   28  */ PROC_SOCKET_SERVER_BILANCIA,  /* socket server bilancia*/
	/*   29  */ PROC_DISPLAY_00,          /* display 00 */
	/*   30  */ PROC_DISPLAY_01,          /* display 01 */
	/*   31  */ PROC_DISPLAY_02,          /* display 02 */
	/*   32  */ PROC_DISPLAY_03,          /* display 03 */
	/*   33  */ PROC_DISPLAY_04,          /* display 04 */
	/*   34  */ PROC_DISPLAY_05,          /* display 05 */
	/*   35  */ PROC_DISPLAY_06,          /* display 06 */
	/*   36  */ PROC_DISPLAY_07,          /* display 07 */
	/*   37  */ PROC_DISPLAY_08,          /* display 08 */
	/*   38  */ PROC_DISPLAY_09,          /* display 09 */
	/*   39  */ PROC_DISPLAY_10,          /* display 10 */
	/*   40  */ PROC_DISPLAY_11,          /* display 11 */
	/*   41  */ PROC_DISPLAY_12,          /* display 12 */
	/*   42  */ PROC_DISPLAY_13,          /* display 13 */
	/*   43  */ PROC_DISPLAY_14,          /* display 14 */
	/*   44  */ PROC_DISPLAY_15,          /* display 15 */
	/*   45  */ PROC_DISPLAY_16,          /* display 16 */
	/*   46  */ PROC_DISPLAY_17,          /* display 17 */
	/*   47  */ PROC_DISPLAY_18,          /* display 18 */
	/*   48  */ PROC_DISPLAY_19,          /* display 19 */
	/*   49  */ PROC_DISPLAY_20,          /* display 20 */
	/*   50  */ PROC_DISPLAY_21,          /* display 21 */
	/*   51  */ PROC_DISPLAY_22,          /* display 22 */
	/*   52  */ PROC_DISPLAY_23,          /* display 23 */
	/*   53  */ PROC_DISPLAY_24,          /* display 24 */
	/*   54  */ PROC_DISPLAY_25,          /* display 25 */
	/*   55  */ PROC_DISPLAY_26,          /* display 26 */
	/*   56  */ PROC_DISPLAY_27,          /* display 27 */
	/*   57  */ PROC_DISPLAY_28,          /* display 28 */
	/*   58  */ PROC_DISPLAY_29,          /* display 29 */
	/*   59  */ PROC_DISPLAY_30,          /* display 30 */
	/*   60  */ PROC_DISPLAY_31,          /* display 31 */
	/*   61  */ PROC_DISPLAY_32,          /* display 32 */
	/*   62  */ PROC_DISPLAY_33,          /* display 33 */
	/*   63  */ PROC_DISPLAY_34,          /* display 34 */
	/*   64  */ PROC_DISPLAY_35,          /* display 35 */
	/*   65  */ PROC_DISPLAY_36,          /* display 36 */
	/*   66  */ PROC_DISPLAY_37,          /* display 37 */
	/*   67  */ PROC_DISPLAY_38,          /* display 38 */
	/*   68  */ PROC_DISPLAY_39,          /* display 39 */

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
	PRINT_RAC_LINEA,
	PRINT_ALL_RAC,
	PRINT_RAC_STORICO,
	PRINT_RAC_COLLO_STORICO,
	PRINT_PACKING_LIST_STORICO,
    PRINT_XAB_A4_LINEA_STORICO,

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

	SOCK_OPEN_SERVICE,
	SOCK_CLOSE_SERVICE,
	SOCK_RECEIVED_MSG,
    SOCK_OPEN_SERVICE_ERROR,
    SOCK_OPEN_SERVICE_OK,
    SOCK_OPEN_SERVICE_ALREADY_OPEN,
    SOCK_CLOSE_SERVICE_ERROR,
    SOCK_CLOSE_SERVICE_OK,
    SOCK_CLOSE_SERVICE_NOT_OPEN,

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
* definizioni per la gestione dei comandi da ricevere via socket
*/
enum Commands {
	CMD_DISPLAY_MSG,
	CMD_DISPLAY_MSGBOX,
	CMD_UPDATE_ORDINE,

	NUM_SOCK_CMDS,
};

#endif /* _PROC_LIST_H */
