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
gboolean LanciaOrdine     (gpointer win, gchar *ordine,gboolean forzatura);
void CheckAnno            (int nDebugLevel);
ep_bool_t CambiaStatoOrdine    (MainWindow *win, char *szOrdProg, char cStato);
ep_bool_t SetNoteOrdine        (MainWindow *win, char *szOrdProg);
ep_bool_t EvadiOrdine          (MainWindow *win, char *szOrdProg);
ep_bool_t OrdineProduzione     (char *szOrdine);
void SetIOS               (MainWindow *win, gboolean bStato);
void SetMaster            (MainWindow *win, gboolean bStato);
void StartLinea           (MainWindow *win);
void StopLinea            (MainWindow *win);
ep_bool_t ResetOrdine          (gpointer win, gchar *ordine);
void UpdateOrdine         (MainWindow *win, char *szOrdineKey);
int update_table          (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *pszKey, gboolean MoveToRow);
void init_actions         (gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);
void refresh_lista_ordini (gpointer win);


/* 
* mainproc.c 
*/
gint SaveStateTimer       (gpointer win );
gint ProcessMsgs          (gpointer win );
gint StatisticaImpianto   (gpointer win );
