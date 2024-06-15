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
#include <math.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>
#include <picking.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

void on_selection_changed( GtkTreeSelection* self, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GList *lista;
    
    gboolean ok = FALSE;

    lista=gtk_tree_selection_get_selected_rows(self, NULL);
    if(lista && g_list_length(lista)){
        ok = TRUE;
        g_printf("selection changed\n");
    } else {
        g_printf("no selection\n");
    }
    gtk_widget_set_sensitive(priv->pb_edit,ok);
    gtk_widget_set_sensitive(priv->pbm_edit,ok);
}

void load_table (gpointer win, char *table_name,char *szKey, GCallback on_selection_changed)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szBuffer[256];

    gtk_widget_set_sensitive(priv->pb_edit,FALSE);
    gtk_widget_set_sensitive(priv->pbm_edit,FALSE);

    refresh_table(win, "sw_list","lst" , priv->pszEditCfgFileName, "edit", table_name, GTK_SELECTION_SINGLE, szKey, on_selection_changed);

    strcpy(priv->szTableName, table_name);

    strcpy(szBuffer,table_name);
    strcat(szBuffer,"_name");
    // GetFileString("edit",szBuffer, "", szSelectTable, sizeof(szSelectTable),Cfg.szCfgFile,NULL); 
}



void update_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *pszKey, gboolean MoveToRow)
{
    char szBuffer[128];
    char szUpdateCmd[4096];

    GList *PS=NULL;

    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_key");
    int nKeyIndex=GetFileInt(paragraph,szBuffer, 0,cfg_file_name, NULL); 

    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_update");
    GetFileString(paragraph,szBuffer, "", szUpdateCmd, sizeof(szUpdateCmd),cfg_file_name, NULL); 

    add_item_to_parse(&PS, "%ORDTIPO%", Cfg.szTipoOrdini, TRUE);
    add_item_to_parse(&PS, "%KEY%",     pszKey,           FALSE);
    UpdateTable(find_child(GTK_WIDGET (win), parent_name), list_name, nKeyIndex, szUpdateCmd,  pszKey, PS, MoveToRow);
}


int refresh_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode, gchar *key, GCallback on_selection_changed)
{
    char szSelectCmd[4096];

    GetFileString(paragraph,table_item_name, "", szSelectCmd, sizeof(szSelectCmd),cfg_file_name, NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    add_item_to_parse(&PS, "%TIPOUBI%",Cfg.szTipoUbicazioni, FALSE);
    if(key){
        add_item_to_parse(&PS, "%KEY%",key, FALSE);
    }

    int rc =  RefreshTable(find_child(GTK_WIDGET (win),parent_name), list_name, szSelectCmd, mode, PS, G_CALLBACK(on_selection_changed), win);
    return rc;
}



