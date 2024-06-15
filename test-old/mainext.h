/*
* mainext.h
* Dichiarazioni External
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autore : Roberto Mantovani
*/

extern CFGSTRUCT Cfg;
extern PSETTORE pSettori;            /* shared memory pointer */
extern UBICAZIONI Ubicazioni;        /* Ubicazioni */
extern EVENTO Evento;                /* Evento */
extern PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */
extern char szDateBuffer[128];
extern char szTimeBuffer[128];
extern char szActualTableName[128];  /* selezione per tabella a video */
extern char szActualKey[128];  /* chiave per tabella a video */

/*
* Connessione al DataBase PostgreSQL
*/
extern PGconn *DBConn;

/*
* widgets utilizzati nell'applicazione 
*/
extern GtkWidget *main_window;
extern GtkWidget *txt_msgs;

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
