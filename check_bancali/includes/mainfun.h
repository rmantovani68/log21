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
ep_bool_t EvadiCollo(gpointer win, char *szOrdProg,int nCollo);
ep_bool_t CheckOrdineEvaso(gpointer win, char *szOrdProg);
int CalcPesoRealeOrdine(gpointer win, char *szOrdProg);
void Beep(void);
void FlashError(void);
void stampa_linea(gpointer win, int nKey);
void stampa_bancale(gpointer win, int nKey,ep_bool_t bPackingList,ep_bool_t bHeader);

/* 
* mainwin.c 
*/
void do_inizio_linea(gpointer win);
void do_fine_linea(gpointer win);
void do_barcode_collo(gpointer win, char *szBarcode);
void inizio_linea(gpointer win);
void attiva_linea(gpointer win, GtkWidget *parent);
void do_apertura_bancale(gpointer win);
ep_bool_t do_chiusura_bancale(gpointer win);
void display_dati_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea);
void display_dati_bancale(gpointer win,PBANCALESTRUCT pBancale);
void display_dati_collo(gpointer win,PCOLLOSTRUCT pCollo);
void reset_dati_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea);
void reset_dati_bancale(gpointer win, PBANCALESTRUCT pBancale);
void reset_dati_collo(gpointer win, PCOLLOSTRUCT pCollo);
void refresh_lista_linee_spedizione(gpointer win);
void refresh_lista_bancali_spedizione(gpointer win);
void refresh_lista_colli_bancale(gpointer win);
void update_lista_bancali_spedizione(gpointer win,PBANCALESTRUCT pBancale);
void update_lista_colli_bancale(gpointer win,PCOLLOSTRUCT pCollo);
char *get_dati_distribuzione(char *tpspe,char *cdlin,char *dslin);
ep_bool_t get_dati_linea(gpointer win, int nKey,PLINEASPEDIZIONESTRUCT pLinea);
ep_bool_t get_dati_bancale(gpointer win, int nKey,PBANCALESTRUCT pBancale);
ep_bool_t get_dati_collo(gpointer win, char *szBarcodeCollo,PCOLLOSTRUCT pCollo);
void barcode_collo(gpointer win, char *szBarcode);
ep_bool_t update_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea);
ep_bool_t update_bancale(gpointer win, PBANCALESTRUCT pBancale);

/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer win );
