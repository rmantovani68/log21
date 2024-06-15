/*
* mainfun.h
* Dichiarazione Funzioni Utilizzate
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

/* 
* mainwin.c 
*/
void on_row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
gboolean on_button_pressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata);

/* 
* mainfun.c 
*/


/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer app );
