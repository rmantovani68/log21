/*
* mainfun.c
* funzioni 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <picking.h>
#include <pmx_msq.h>

#include <gtk-support.h>
#include <msg-box.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

void set_selection(gpointer win, GtkWidget *lst, char *barcode, gboolean set_position, gboolean select_row)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char *pszString;
    GtkTreeIter iter;
    char b[128];

    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {
        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter, 0,  &pszString); strcpy(b, pszString); g_free(pszString);
        if(!strcmp(barcode, b)) {
            if(set_position){
                gtk_tree_view_scroll_to_cell (TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, TRUE, 0.5, 0.0);
            }
            if(select_row){
                gtk_tree_view_set_cursor(TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, FALSE);
            }
            break;
        }
    } while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst), &iter));
}

void update_collo(gpointer *win, char *szBarcode)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_update_table(win, "sw_list", "lst", (gchar *)priv->pszCfgFileName, "fine_linea", "lista_colli",szBarcode, TRUE);
}


void refresh_lista_colli(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_refresh_table(win, (gchar *)"sw_list", (gchar *)"lst", (gchar *)priv->pszCfgFileName, (gchar *)"fine_linea", (gchar *)"lista_colli", Cfg.nTipoSelezione, NULL);
    /*
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    g_signal_connect(lst, "row-activated"     , G_CALLBACK (on_lista_colli_row_activated),  win);
    g_signal_connect(lst, "button-press-event", G_CALLBACK (on_lista_colli_button_pressed), win);
    GtkTreeSelection *selection = TREE_SELECTION_LST(lst);
    g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK (on_lst_selection_changed), win);
    */
}
