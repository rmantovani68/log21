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
#include <msg-box.h>
#include "mainfun.h"


static void set_radio_buttons_value(GtkWidget *parent, int value, gchar *set_if_true, gchar *set_if_false)
{
    gchar *ptr;
    GtkWidget *w;

    if(value){
        ptr = set_if_true;
    } else {
        ptr = set_if_false;
    }
    if((w=find_child(parent,ptr))){ 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),TRUE); 
    }
}

void SetUserCfgWidgets(GtkWidget *parent)
{
    GtkWidget *w;

    set_radio_buttons_value(parent, Cfg.nChiusuraCollo,                          "rb_gestione_collo",                      "rb_gestione_ordine");
    set_radio_buttons_value(parent, Cfg.nImballiUbicati,                         "rb_imballi_ubicati",                     "rb_imballi_non_ubicati");
    set_radio_buttons_value(parent, Cfg.nStampaRAC,                              "rb_all_rac",                             "rb_settori");
    set_radio_buttons_value(parent, Cfg.nTipoSelezione==GTK_SELECTION_MULTIPLE,  "rb_selezione_multipla",                  "rb_selezione_singola");
    set_radio_buttons_value(parent, !strcmp(Cfg.szTipoOrdinePreDistinta,"X"),    "rb_ordine_stampato_xab",                 "rb_ordine_evaso");
    set_radio_buttons_value(parent, Cfg.nInvioFileCorriereAutomatico,            "rb_invio_mail_file_corriere_automatico", "rb_invio_mail_file_corriere_manuale");
    set_radio_buttons_value(parent, Cfg.nInvioXABMailAutomatico,                 "rb_invio_mail_xab_automatico",           "rb_invio_mail_xab_manuale"     );
    set_radio_buttons_value(parent, Cfg.nInvioDistintaMailAutomatico,            "rb_invio_mail_distinta_automatico",      "rb_invio_mail_distinta_manuale");
} 
