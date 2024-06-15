/*
* mainproc.c
* work procedures 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"


/*
* Gestione Eventi (Messaggi e PLC)
*/
gint ProcessMsgs( gpointer win )
{
    int nAllMsg = 0;
    int nCarIn;
    char szInMsg[INFO_LEN];
    char szText[INFO_LEN];
    ep_msg_header_t InMsgStruct;
    static time_t lActualTime=0;
    static ep_bool_t bFirstTime=TRUE;
    static LINEA_STRUCT OldLineData;
    static ep_bool_t bDatiSpediti=FALSE;
    static ep_bool_t bReindexEseguito=FALSE;
    char szTxMonitor[128];
    char szRxMonitor[128];
    char szCommand[128];

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(bFirstTime){
        gtk_text_printf(NULL,find_child(GTK_WIDGET(win), "txt_msgs"),"A&L - Easy Picking 4.0 - Application Ready\n");
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        /* TODO 
        ep_refresh_table(win, "sw_main","lst" , priv->pszCfgFileName, "test", "ubicazioni", GTK_SELECTION_SINGLE,NULL);
        */

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_movimenti),FALSE); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_catalogo),FALSE); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_reports),FALSE); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_giacenze),TRUE); 
    }

    /*
    * Controllo lo stato della Trasmissione/Ricezione
    */
    if(lActualTime!=time((time_t *)0)){

        time(&lActualTime);
    }


    /*
    * Messaggi da altri processi
    */
    while((nCarIn = ProcessReceiveMsgNoWait(PROC_MAIN, &nAllMsg, szInMsg))>=0){
        UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

#ifdef TRACE
        if(Cfg.nDebugLevel>3){
            trace_debug(TRUE, TRUE,  "Msg [%d] %s%s%s from [%s]",
                InMsgStruct.code,
                strlen(szText)?"[":"",strlen(szText)? szText : "",strlen(szText)?"]":"",
                ProcessGetName(InMsgStruct.srce));
        }
#endif

        /*
        * notifica di Start/Stop Processi
        */
        switch (InMsgStruct.code){
            case DISPLAY_MSG:
            {
                trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "%s Display MSG %s", ProcessGetName(InMsgStruct.srce),szText);
            }
            break;
        }

        /*
        * Gestione Messaggi da altri processi
        */
        switch (InMsgStruct.srce){
        }

    }

    if(bFirstTime){
        bFirstTime=FALSE;
    }
    /* 
    * As this is a timeout function, return TRUE so that it
    * continues to get called 
    */
    return(TRUE);
}
