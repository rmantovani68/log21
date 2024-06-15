/*
* printext.h
* 
* Progetto Easy Picking
*
* Copyright A&L srl 2003-2021
*/

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __copyright__[];
extern char __authors__[];
extern char __configuration_file__[];


extern CFGSTRUCT	Cfg;
extern int nPID;                   /* Process ID */

extern int nAllMsg;
extern int nCarIn;
extern MSGHDRINT InMsgStruct;
extern char szInMsg[INFO_LEN];
extern char szText[INFO_LEN];
extern PLINEA_STRUCT pDatiLinea;     /* Dati di gestione sistema */

extern char szBufTrace[80];
extern char *pszNomeModulo;
extern char szTimeBuffer[80];
extern char szDateBuffer[80];

extern POS Pos[NUM_FIELD_RAC]; 	     /* Posizione e font della packing list */
extern POS XABPos[NUM_FIELD_XAB];    /* Posizione e font della XAB */
extern POS XABPos_A4[NUM_FIELD_XAB]; /* Posizione e font della XAB */
extern POS PLPos_A4[NUM_FIELD_XAB];  /* Posizione e font della XAB */
extern POS DCPos[NUM_FIELD_XAB];     /* Posizione e font della XAB */
extern POS DisPos[NUM_FIELD_DIS];
extern int nPrintingRow[50];	/* Numeri di riga sui quali stampare la picking list */
extern char *pszForm[MAX_RAC_LINES];
extern char szPRXAB[80];

extern LABEL_FIELD_STRUCT LabelFields[256];
