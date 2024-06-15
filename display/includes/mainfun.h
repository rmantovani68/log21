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
void update_table       (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *pszKey, gboolean MoveToRow);
int refresh_table      (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode);
void init_actions      (gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);

void RefreshBlank(gpointer win);
void RefreshAsDisconnected(gpointer win);
void RefreshSettoreElements(gpointer win);
void RefreshLineaElements(gpointer win);
void RefreshStaticElements(gpointer win);
void RefreshDatiProdottoElements(gpointer win);
void RefreshStatisticheElements(gpointer win);
void RefreshListaColliElements(gpointer win);
ep_bool_t connect_linea_shm(PLINEA_STRUCT *pDatiLinea);
ep_bool_t connect_settori_shm(PSETTORE *pSettori);
ep_bool_t disconnect_linea_shm(PLINEA_STRUCT *pDatiLinea);
ep_bool_t disconnect_settori_shm(PSETTORE *pSettori);
gint Blinking( gpointer win, gpointer data );
char *GetTitolo(char *szCDPRO,char *szTitolo);
void do_shift_frame_sx(gpointer win);
void do_shift_frame_dx(gpointer win);
void do_lista_colli_up(gpointer win);
void do_lista_colli_down(gpointer win);
void do_aggiorna_frame(gpointer win);

/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer win );
