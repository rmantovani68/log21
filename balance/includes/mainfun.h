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
int refresh_table      (gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode);
void init_actions      (gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);


/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer win );
