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
void set_selection(gpointer win, GtkWidget *lst, char *barcode, gboolean set_position, gboolean select_row);
void update_collo(gpointer *win, char *szBarcode);
void refresh_lista_colli(gpointer win);


/* 
* mainproc.c 
*/
gint SaveStateTimer( gpointer win );
gint ProcessMsgs( gpointer win );
gint StatisticaImpianto( gpointer win );
