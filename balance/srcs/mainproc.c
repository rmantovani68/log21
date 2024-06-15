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
* Gestione Eventi 
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

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(bFirstTime){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_lista),TRUE); 
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
                GdkColor *pcColor;

                switch (InMsgStruct.srce){
                    case PROC_VOLUM: pcColor=&cORANGE;break;
                    case PROC_SETTORI: pcColor=&cGREEN;break;
                    case PROC_PRINTLABEL: pcColor=&cBLUE;break;
                    case PROC_RECEIVE: pcColor=&cLBLUE;break;
                    case PROC_AUTO_TASKS: pcColor=&cGREY;break;
                    default: pcColor=&cBLACK;break;
                }
                trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "%s Display MSG [%s]", ProcessGetName(InMsgStruct.srce),szText);
            }
            break;
            case PROGRAM_STARTED:
                trace_debug(TRUE, 1,  "%s Started", ProcessGetName(InMsgStruct.srce));
            break;
            case PROGRAM_STOPPED:
                trace_debug(TRUE, 1,  "%s Stopped", ProcessGetName(InMsgStruct.srce));
            break;
        }

        switch (InMsgStruct.srce){
            case PROC_VOLUM:
                switch (InMsgStruct.code){
                }
            break;
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
