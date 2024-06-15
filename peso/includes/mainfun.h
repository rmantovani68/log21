/*
* mainfun.h
* Dichiarazione Funzioni Utilizzate
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

/* 
* mainfun.c 
*/
gboolean LanciaOrdine  (gpointer win, gchar *ordine,gboolean forzatura);
void CheckAnno         (int nDebugLevel);
ep_bool_t CambiaStatoOrdine (MainWindow *win, char *szOrdProg, char cStato);
ep_bool_t SetNoteOrdine     (MainWindow *win, char *szOrdProg);
ep_bool_t EvadiOrdine       (MainWindow *win, char *szOrdProg);
ep_bool_t OrdineProduzione  (char *szOrdine);
void SetIOS            (MainWindow *win, gboolean bStato);
void SetMaster         (MainWindow *win, gboolean bStato);
void StartLinea        (MainWindow *win);
void StopLinea         (MainWindow *win);
void UpdateOrdine      (MainWindow *win, char *szOrdineKey);
void init_actions      (gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);


/* 
* mainproc.c 
*/
gint SaveStateTimer( gpointer win );
gint ProcessMsgs( gpointer win );
gint StatisticaImpianto( gpointer win );
