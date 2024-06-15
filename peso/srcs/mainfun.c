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
#include <picking.h>
#include <ep-db.h>
#include <trace.h>
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

void UpdateCollo(MainWindow *win, char *szOrdineKey, int nCollo)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_update_table(win, "sw_list", "lst", (gchar *)priv->pszCfgFileName, "Tabella Colli", "lista_colli",szOrdineKey, TRUE);
}


