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
int update_table       (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *pszKey, gboolean MoveToRow);
int refresh_table      (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode, gchar *key);
void init_actions      (gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);

void SetFase(gpointer win, int nFase);
int UpdateGtkWidget(gpointer win, char *szCmd, GtkWidget *w);
ep_bool_t EvadiCollo(gpointer win, char *szOrdProg,int nCollo);
ep_bool_t CheckOrdineEvaso(gpointer win, char *szOrdProg);
int CalcPesoRealeOrdine(gpointer win, char *szOrdProg);
void Beep(void);
void FlashError(void);
void stampa_linea(gpointer win, int nKey);
void stampa_bancale(gpointer win, int nKey,ep_bool_t bPackingList,ep_bool_t bHeader);
void ClearLabel(gpointer win, char *pszText);
ep_bool_t StampaPackingListCollo(gpointer win, char *szOrdProg, int nCollo, char *szPrinterName);
char *GetStatoCollo(char cStato,char *pszBuffer);
ep_bool_t ShowPesoBilancia(gpointer win);
void Gestev_2002msg(char *szText);
void ShowLabelBilancia(gpointer win);
void ReadBitmaskInfo(char *szFileConfig);

/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer win );
