/*
* mainext.h
* Dichiarazioni External
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

extern CFGSTRUCT Cfg;
extern PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */
extern PLINEA_STRUCT pDatiLinea;
extern char szDateBuffer[128];
extern char szTimeBuffer[128];
extern char szReadBarcode[128];

/*
* Connessione al DataBase PostgreSQL
*/
extern PGconn *DBConn;

/*
* widgets utilizzati nell'applicazione 
*/
extern GtkWidget *main_window;
extern GtkWidget *dlg_trace;
extern GtkWidget *txt_trace;

/*
* Definizioni costanti colore
*/
extern GdkColor cLBLUE;
extern GdkColor cBLUE;
extern GdkColor cBLACK;
extern GdkColor cRED;
extern GdkColor cLRED;
extern GdkColor cLCYAN;
extern GdkColor cLICE;
extern GdkColor cCYAN;
extern GdkColor cWHITE;
extern GdkColor cYELLOW;
extern GdkColor cLYELLOW;
extern GdkColor cLGREEN;
extern GdkColor cGREEN;
extern GdkColor cGREYCYAN;
extern GdkColor cLGREY;
extern GdkColor cGREY;
extern GdkColor cSLGREY;
extern GdkColor cSGREY;
extern GdkColor cORANGE;

extern COLORSTRUCT Colors[];
