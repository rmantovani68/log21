/*
* mainfun.c
* funzioni 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/
#include <gtk/gtk.h>
//#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

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
#include <msg-box.h>
#include "mainfun.h"

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

    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    UpdateTable(find_child(GTK_WIDGET (win), parent_name), list_name, nKeyIndex, szUpdateCmd,  pszKey, PS, MoveToRow);
}


int refresh_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode)
{
    char szSelectCmd[4096];

    GetFileString(paragraph,table_item_name, "", szSelectCmd, sizeof(szSelectCmd),cfg_file_name, NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    int rc =  RefreshTable(find_child(GTK_WIDGET (win),parent_name), list_name, szSelectCmd, mode, PS, NULL, NULL);
    return rc;
}

ep_bool_t connect_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
    ep_bool_t bOK=TRUE;
    /*
    * Creo la shared memory per la gestione della linea
    */
    if((*pDatiLinea=(PLINEA_STRUCT)GetShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Cannot Allocate Shared Memory [%d] !", Cfg.nShmKey);
#endif
        bOK=FALSE;
    } else {
        //???pDatiLinea.nStatoLinea=LINEA_IN_STOP;
    }

    return bOK;
}



ep_bool_t connect_settori_shm(PSETTORE *pSettori)
{
    ep_bool_t bOK=TRUE;
    char szBuffer[512];

    /*
    * Leggo la shared memory per la gestione della linea (SETTORI)
    */
    if((*pSettori=(PSETTORE)GetShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Cannot Get Shared Memory [%d] (SETTORI)", Cfg.nSettoriShmKey);
#endif
        
        bOK=FALSE;
    }
    return bOK;
}

ep_bool_t disconnect_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
    if(pDatiLinea){
        DetachShm((char *)*pDatiLinea);

        *pDatiLinea=(PLINEA_STRUCT)NULL;
        return TRUE;
    }
    return FALSE;
}

ep_bool_t disconnect_settori_shm(PSETTORE *pSettori)
{
    if(pSettori){
        DetachShm((char *)*pSettori);

        *pSettori=(PSETTORE)NULL;
        return TRUE;
    }
    return FALSE;
}

void RefreshBlank(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    
     gtk_label_printf ( GTK_WIDGET(priv->lb_ordine_collo),"            ");
     gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),"    ");
     gtk_label_printf ( GTK_WIDGET(priv->lb_copie),"   ");
     gtk_label_printf ( GTK_WIDGET(priv->lb_ubicazione),"         ");
     gtk_label_printf ( GTK_WIDGET(priv->lb_codice),"         ");
     gtk_label_printf ( GTK_WIDGET(priv->lb_titolo),"         ");
}

void RefreshAsDisconnected(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    
     gtk_label_printf ( GTK_WIDGET(priv->lb_ordine_collo),"------ / ---");
     gtk_label_printf ( GTK_WIDGET(priv->lb_codice_operatore),"--");
     gtk_label_printf ( GTK_WIDGET(priv->lb_dati_operatore),"------");
     gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),"----");
     gtk_label_printf ( GTK_WIDGET(priv->lb_copie),"---");
     gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),"-----");
     gtk_label_printf ( GTK_WIDGET(priv->lb_ubicazione),"---------");
     gtk_label_printf ( GTK_WIDGET(priv->lb_codice),"---------");
     gtk_label_printf ( GTK_WIDGET(priv->lb_titolo),"---------");
}

void RefreshSettoreElements(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(strlen(pSettori[priv->settore].szCodSped)){
         gtk_label_printf ( GTK_WIDGET(priv->lb_ordine_collo),"%s / %d",pSettori[priv->settore].szCodSped,pSettori[priv->settore].nNumCollo);
    } else {
         gtk_label_printf ( GTK_WIDGET(priv->lb_ordine_collo),"            ");
    }
    
    switch(pSettori[priv->settore].nStatoSettore){
        case TEST:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szTEST);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie)," ");
        break;
        case ATTESA:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),"");
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie),"");

        break;
        case PRELIEVO_IMBALLI:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szPRELIEVO_IMBALLO);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie),"%d",pSettori[priv->settore].nNumCopie);
            
        break;
        case PRELIEVO_COPIE:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szPRELIEVO);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie),"%d",pSettori[priv->settore].nNumCopie);
            
        break;
        case PASSA_SETT_SUCC:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione), Cfg.szPASSA_SETT_SUCC);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie)," ");
        break;
        case CHIUSURA_COLLO:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szCHIUSURA_COLLO);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie)," ");
            
        break;
        case FINE_ORDINE:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szFINE_ORDINE);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie)," ");
            
        break;
        case DISABILITATO:
             gtk_label_printf ( GTK_WIDGET(priv->lb_operazione),Cfg.szOFFLINE);
             gtk_label_printf ( GTK_WIDGET(priv->lb_copie)," ");
            
        break;
    }
    
    
     gtk_label_printf ( GTK_WIDGET(priv->lb_codice_operatore),"%s",pSettori[priv->settore].szCodOperatore);
     gtk_label_printf ( GTK_WIDGET(priv->lb_dati_operatore),"%s",pSettori[priv->settore].szCognomeOperatore);
}

void RefreshDatiProdottoElements(gpointer win)
{
    char szUbicazioneSettori[256];
    char szUbicazione[256];
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(pSettori[priv->settore].nStatoSettore!=TEST){

        strcpy(szUbicazioneSettori, pSettori[priv->settore].szCodUbi);

        if(strlen(pSettori[priv->settore].szCodUbi) == LEN_UBICAZIONE){
            strncpy(szUbicazione, szUbicazioneSettori,1);szUbicazione[1]='\0';
            strcat(szUbicazione, ".");
            strncat(szUbicazione, szUbicazioneSettori+7,2 );szUbicazione[4]='\0';
            strcat(szUbicazione, ".");
            strncat(szUbicazione, szUbicazioneSettori+10,2 );szUbicazione[7]='\0';
        } else {
            strcpy(szUbicazione, szUbicazioneSettori);
        }

         gtk_label_printf ( GTK_WIDGET(priv->lb_ubicazione),"%s",szUbicazione);
         gtk_label_printf ( GTK_WIDGET(priv->lb_codice),"%s",pSettori[priv->settore].szCodProd);
         gtk_label_printf ( GTK_WIDGET(priv->lb_titolo),"%s",pSettori[priv->settore].szTitolo);

    } else {
         gtk_label_printf ( GTK_WIDGET(priv->lb_ubicazione),"%s",pSettori[priv->settore].szCodUbi);
         gtk_label_printf ( GTK_WIDGET(priv->lb_codice),"RIGA    %d",pSettori[priv->settore].nRiga);
         gtk_label_printf ( GTK_WIDGET(priv->lb_titolo),"COLONNA %d",pSettori[priv->settore].nColonna);
    }

}

void RefreshStatisticheElements(gpointer win)
{
    DBresult *DBRes;
    int nTuples;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select count(distinct ordprog), count(distinct ordprog||rpnmcol), count(*), sum(rpnmcpe) from articoli,ubi_set where area(rpcdubi)='23' and ubicazione_noarea(rpcdubi)=ubicazione_noarea(ubicazione) and settore=%d and rpstset=' ';",priv->settore+1);
    if((nTuples=DBntuples(DBRes))){
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_ordini_da_prelevare),"%d",atoi(DBgetvalue(DBRes,0,0)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_colli_da_prelevare),"%d",atoi(DBgetvalue(DBRes,0,1)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_righe_da_prelevare),"%d",atoi(DBgetvalue(DBRes,0,2)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_copie_da_prelevare),"%d",atoi(DBgetvalue(DBRes,0,3)));
    }
    DBclear(DBRes);
    
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select count(distinct ordprog), count(distinct ordprog||rpnmcol), count(*), sum(rpnmcpe) from articoli,ubi_set where area(rpcdubi)='23' and ubicazione_noarea(rpcdubi)=ubicazione_noarea(ubicazione) and settore=%d and rpstset='P';",priv->settore+1);
    if((nTuples=DBntuples(DBRes))){
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_ordini_prelevati),"%d",atoi(DBgetvalue(DBRes,0,0)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_colli_prelevati),"%d",atoi(DBgetvalue(DBRes,0,1)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_righe_prelevati),"%d",atoi(DBgetvalue(DBRes,0,2)));
         gtk_label_printf ( GTK_WIDGET(priv->lb_stats_copie_prelevati),"%d",atoi(DBgetvalue(DBRes,0,3)));
    }
    DBclear(DBRes);
}

/* TODO : da rifare */
void RefreshListaColliElements(gpointer win)
{
    char szSettore[1024];
    char szListaColli[1024];
    char szCollo[32];
    DBresult *DBRes;
    int nTuples;
    int nIndex;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    
    strcpy(szListaColli,"Lista Colli:\n");
    
}

void RefreshStatoLinea(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(pDatiLinea->nStatoLinea==LINEA_IN_START){
        gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),Cfg.szSTART);
        gtk_widget_set_foreground(priv->lb_stato_linea,&cLGREEN);
    } else if(pDatiLinea->nStatoLinea==LINEA_IN_STOP){
        gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),Cfg.szSTOP);
        gtk_widget_set_foreground(priv->lb_stato_linea,&cRED);
    } else if(pDatiLinea->nStatoLinea==LINEA_IN_TEST){
        gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),Cfg.szTEST);
        gtk_widget_set_foreground(priv->lb_stato_linea,&cRED);
    } else if(pDatiLinea->nStatoLinea==LINEA_GENERAL_ERROR){
        gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),Cfg.szERROR);
        gtk_widget_set_foreground(priv->lb_stato_linea,&cRED);
    } else {
        gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea),Cfg.szWAIT);
        gtk_widget_set_foreground(priv->lb_stato_linea,&cORANGE);
    }
}

void RefreshLineaElements(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

     gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea)," ");
    
    gtk_update();
#ifdef __Linux__
    usleep(100000);
#endif

    RefreshStatoLinea(win);
    
    gtk_update();
#ifdef __Linux__
    usleep(100000);
#endif

     gtk_label_printf ( GTK_WIDGET(priv->lb_stato_linea)," ");

    gtk_update();
#ifdef __Linux__
    usleep(100000);
#endif

    RefreshStatoLinea(win);
}

void RefreshStaticElements(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gtk_label_printf ( GTK_WIDGET(priv->lb_settore),"%2d",priv->settore+1);
    
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_ordine_collo),Cfg.szORDINE_COLLO);
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_settore),Cfg.szSETTORE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_operatore),Cfg.szOPERATORE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_ubicazione),Cfg.szUBICAZIONE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_codice),Cfg.szCODICE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_titolo_titolo),Cfg.szTITOLO);
    
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_da_prelevare),Cfg.szDA_PRELEVARE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_prelevati),Cfg.szPRELEVATI);
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_ordini),Cfg.szORDINI);
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_colli),Cfg.szCOLLI);
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_righe),Cfg.szRIGHE);
    gtk_label_printf ( GTK_WIDGET(priv->lb_stats_titolo_copie),Cfg.szCOPIE);
}

gint Blinking(gpointer win, gpointer data )
{
    static ep_bool_t bOpBlinkOn=FALSE;
    static ep_bool_t bBlinking=FALSE;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(pSettori){
        if(pSettori[priv->settore].nOpBlinking==BLINK){
            bBlinking=TRUE;
            

            if (bOpBlinkOn) {
                gtk_widget_set_background(priv->eb_operazione,&cRED);
                gtk_widget_set_background(priv->eb_copie,&cRED);
            } else {
                gtk_widget_set_background(priv->eb_operazione,&cBLACK);
                gtk_widget_set_background(priv->eb_copie,&cBLACK);
            }
            bOpBlinkOn=(!bOpBlinkOn);
            return(TRUE);
        }
    }
    
    /*
    * rm 01-12-06 : per evitare di rifare sempre il set_background ..
    */
    if(bBlinking){
        gtk_widget_set_background(priv->eb_operazione,&cBLACK);
        gtk_widget_set_background(priv->eb_copie,&cBLACK);
        bBlinking=FALSE;
    }

    /* 
    * As this is a timeout function, return TRUE so that it
    * continues to get called 
    */
    return(TRUE);
}


char *GetTitolo(char *szCDPRO,char *szTitolo)
{
    DBresult *DBRes;
    int nTuples;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select substring(prdstit,1,30) from catalogo where prcdpro='%s';",szCDPRO);
    if((nTuples=DBntuples(DBRes))){
        strcpy(szTitolo,DBgetvalue(DBRes,0,0));
    }
    DBclear(DBRes);

    return szTitolo;    
}

void do_shift_frame_sx(gpointer win)
{
    if(nActualFrame>0){
        nActualFrame=nActualFrame-1;
    } else {
        nActualFrame=NUMERO_FRAME-1;
    }
    do_aggiorna_frame(win);
}

void do_shift_frame_dx(gpointer win)
{
    if(nActualFrame==NUMERO_FRAME-1){
        nActualFrame=0;
    } else {
        nActualFrame=nActualFrame+1;
    }
    do_aggiorna_frame(win);
}

void do_aggiorna_frame(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    switch(nActualFrame){
        case FRAME_DATI_PRODOTTO: gtk_widget_show(priv->frame_dati_prodotto); gtk_widget_hide(priv->frame_statistiche); gtk_widget_hide(priv->frame_lista_colli); break;
        case FRAME_STATISTICHE:   gtk_widget_hide(priv->frame_dati_prodotto); gtk_widget_show(priv->frame_statistiche); gtk_widget_hide(priv->frame_lista_colli); break;
        case FRAME_LISTA_COLLI:   gtk_widget_hide(priv->frame_dati_prodotto); gtk_widget_hide(priv->frame_statistiche); gtk_widget_show(priv->frame_lista_colli); break;
    }
    gtk_update();
    RefreshListaColliElements(win);

}

void do_simula_luce(gpointer win)
{
    char szSettore[128];
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(OpenProcessMsgQ(PROC_SETTORI)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "Apertura coda messaggi SETTORI fallita");
#endif
    }

    sprintf(szSettore,"%d",priv->settore+1);
#ifdef TRACE
    trace_debug(FALSE, TRUE, "do_simula_luce SETTORE [%s]",szSettore);
#endif
    SendMessage(PROC_SETTORI, priv->pid, SIMULA_LUCE, szSettore);

}

void do_stop_linea(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    SendMessage(PROC_SETTORI, priv->pid, STOP_LINEA, NULL);
}
void do_start_linea(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    SendMessage(PROC_SETTORI, priv->pid, START_LINEA, NULL);
}


gboolean on_main_window_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer win)
{

    switch (event->keyval){
        
        case GDK_KEY_Num_Lock:             trace_debug(FALSE, FALSE, "GDK_KEY_Num_Lock");                           break;
        case GDK_KEY_KP_Space:             trace_debug(FALSE, FALSE, "GDK_KEY_KP_Space");                           break;
        case GDK_KEY_KP_Tab:               trace_debug(FALSE, FALSE, "GDK_KEY_KP_Tab");                             break;
        case GDK_KEY_KP_Enter:             trace_debug(FALSE, FALSE, "GDK_KEY_KP_Enter");                           break;
        case GDK_KEY_KP_F1:                trace_debug(FALSE, FALSE, "GDK_KEY_KP_F1");                              break;
        case GDK_KEY_KP_F2:                trace_debug(FALSE, FALSE, "GDK_KEY_KP_F2");                              break;
        case GDK_KEY_KP_F3:                trace_debug(FALSE, FALSE, "GDK_KEY_KP_F3");                              break;
        case GDK_KEY_KP_F4:                trace_debug(FALSE, FALSE, "GDK_KEY_KP_F4");                              break;
        case GDK_KEY_KP_Home:              trace_debug(FALSE, FALSE, "GDK_KEY_KP_Home");                            break;
        case GDK_KEY_KP_Left:              trace_debug(FALSE, FALSE, "GDK_KEY_KP_Left");                            break;
        case GDK_KEY_KP_Up:                trace_debug(FALSE, FALSE, "GDK_KEY_KP_Up");                              break;
        case GDK_KEY_KP_Right:             trace_debug(FALSE, FALSE, "GDK_KEY_KP_Right");                           break;
        case GDK_KEY_KP_Down:              trace_debug(FALSE, FALSE, "GDK_KEY_KP_Down");                            break;
        case GDK_KEY_KP_Page_Up:           trace_debug(FALSE, FALSE, "GDK_KEY_KP_Page_Up");                         break;
        case GDK_KEY_KP_Page_Down:         trace_debug(FALSE, FALSE, "GDK_KEY_KP_Page_Down");                       break;
        case GDK_KEY_KP_End:               trace_debug(FALSE, FALSE, "GDK_KEY_KP_End");                             break;
        case GDK_KEY_KP_Begin:             trace_debug(FALSE, FALSE, "GDK_KEY_KP_Begin");                           break;
        case GDK_KEY_KP_Insert:            trace_debug(FALSE, FALSE, "GDK_KEY_KP_Insert");                          break;
        case GDK_KEY_KP_Delete:            trace_debug(FALSE, FALSE, "GDK_KEY_KP_Delete");                          break;
        case GDK_KEY_KP_Equal:             trace_debug(FALSE, FALSE, "GDK_KEY_KP_Equal");                           break;
        case GDK_KEY_KP_Multiply:          trace_debug(FALSE, FALSE, "GDK_KEY_KP_Multiply");                        break;
        case GDK_KEY_KP_Add:               trace_debug(FALSE, FALSE, "GDK_KEY_KP_Add");                             break;
        case GDK_KEY_KP_Separator:         trace_debug(FALSE, FALSE, "GDK_KEY_KP_Separator");                       break;
        case GDK_KEY_KP_Subtract:          trace_debug(FALSE, FALSE, "GDK_KEY_KP_Subtract");                        break;
        case GDK_KEY_KP_Decimal:           trace_debug(FALSE, FALSE, "GDK_KEY_KP_Decimal");                         break;
        case GDK_KEY_KP_Divide:            trace_debug(FALSE, FALSE, "GDK_KEY_KP_Divide");                          break;
        case GDK_KEY_KP_0:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_0");                               break;
        case GDK_KEY_KP_1:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_1");                               break;
        case GDK_KEY_KP_2:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_2");                               break;
        case GDK_KEY_KP_3:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_3");                               break;
        case GDK_KEY_KP_4:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_4");                               break;
        case GDK_KEY_KP_5:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_5");                               break;
        case GDK_KEY_KP_6:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_6");                               break;
        case GDK_KEY_KP_7:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_7");                               break;
        case GDK_KEY_KP_8:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_8");                               break;
        case GDK_KEY_KP_9:                 trace_debug(FALSE, FALSE, "GDK_KEY_KP_9");                               break;
    }


    /*
    * Tasti sempre disponibili
    */
    switch (event->keyval){
        case GDK_KEY_KP_Divide:
            // trace_debug(FALSE, FALSE, "SIMULA LUCE\n");
            do_simula_luce(win);
        break;
        case GDK_KEY_KP_Subtract:
            trace_debug(FALSE, FALSE, "STOP LINEA");
            //do_stop_linea(win);
        break;
        case GDK_KEY_KP_Add:
            trace_debug(FALSE, FALSE, "START LINEA");
            //do_start_linea(win);
        break;
        case GDK_KEY_KP_Right:
            //do_shift_frame_dx(win);
        break;
        case GDK_KEY_KP_Left:
            //do_shift_frame_sx(win);
        break;
    }
    
    /*
    * Tasti disponibili con un certo frame
    */
    switch(nActualFrame){
        case FRAME_DATI_PRODOTTO:
        break;
        case FRAME_STATISTICHE:
        break;
        case FRAME_LISTA_COLLI:
        break;
    }

  return FALSE;
}
