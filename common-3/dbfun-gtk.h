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
GtkTreeIter *UpdateTable(GtkWidget *parent,char *szListName,int nKeyIndex,char *szUpdateCmd, char *pszKey, GList *PSList,BOOL bSetPosition);
int InsertInTable(GtkTreeView *list,char *szCfgFileName,char *szParagraph,char *szTableName,GList *PSList,GtkTreeIter *iter);
int RefreshTable(GtkWidget *parent,char *szListName,char *szItemValue,int nSelectionMode,GList *PSList);
int CreateListFromSelect(GtkWidget *parent, char *szListName, int nSelectMode, char *szSqlCmd);
