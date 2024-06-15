/*
* modulo : dbfun-gtk.h
* --------------------
* Progetto Easy Picking - DB Management - PostgreSQL - GTK
*
* Data creazione 18/06/2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L 2002-2021
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 18-06-2002 : Creazione
* -------------   -------------------------------------------------------
*/

int UpdateListFromSelect(GtkTreeView *list, char *szSqlCmd,GtkTreeIter *iter);
int InsertListFromSelect(GtkTreeView *list, char *szSqlCmd,GtkTreeIter *iter);
GtkTreeIter *UpdateTable(GtkWidget *parent,char *szListName,int nKeyIndex,char *szUpdateCmd, char *pszKey, GList *PSList,ep_bool_t bSetPosition);
int InsertInTable(GtkTreeView *list,char *szCfgFileName,char *szParagraph,char *szTableName,GList *PSList,GtkTreeIter *iter);
int RefreshTable(GtkWidget *parent,char *szListName,char *szItemValue,int nSelectionMode,GList *PSList, GCallback on_selection_changed, gpointer user_data);
int CreateListFromSelect(GtkWidget *parent, char *szListName, int nSelectMode, char *szSqlCmd, GCallback on_selection_changed, gpointer user_data);
/* */
void ep_set_tipo_ordini(gchar *to);
void ep_set_tipo_ubicazioni(gchar *tu);
void ep_update_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *key, gboolean MoveToRow);
int ep_refresh_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode, gchar *key);
