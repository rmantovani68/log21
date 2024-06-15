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
* main window events management
*/
gboolean ProcessMsgs( gpointer app )
{
    int nAllMsg = 0;
    int nCarIn;
    char szInMsg[INFO_LEN];
    char szText[INFO_LEN];
    ep_msg_header_t InMsgStruct;
    static time_t lActualTime=0;
    static ep_bool_t bFirstTime=TRUE;

    MainAppPrivate *priv = get_main_app_private_instance (app);

    if(bFirstTime){
        /*
        * do somwthing the first time 
        */
        /*
        gtk_text_printf(NULL,find_child(GTK_WIDGET(win), "txt_msgs"),"A&L - Easy Picking 4.0 - Application Ready\n");
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        ep_refresh_table(win, "sw_main","lst" , priv->pszCfgFileName, "proc_name", "table_name", GTK_SELECTION_SINGLE,NULL);
        g_signal_connect(find_child(GTK_WIDGET(win), "lst"), "row-activated"     , G_CALLBACK (on_row_activated),  win);
        g_signal_connect(find_child(GTK_WIDGET(win), "lst"), "button-press-event", G_CALLBACK (on_button_pressed), win);
        */
    }

    /*
    * do something every second
    */
    if(lActualTime!=time((time_t *)0)){

        time(&lActualTime);
    }


    /*
    * messages from the other processes
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
        * display msg from other process
        */
        switch (InMsgStruct.code){
            case DISPLAY_MSG:
                trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window), "txt_msgs"), "%s Display MSG %s", ProcessGetName(InMsgStruct.srce),szText);
            break;
        }

		/*
		* msgs from other processes
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
