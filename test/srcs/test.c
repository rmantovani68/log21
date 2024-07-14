#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/time.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <ep-common.h>
#include <ep-db.h>
#include <shared.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "test.h"
#include "mainfun.h"

#define WINDOW_NAME "test-window"

struct _TestWindow
{
    GtkDialog parent;
};

/*
* FSM definition 
*/

/* states in the FSM */
typedef enum {
  STATE_STANDBY,/* standby state */
  STATE_START,  /* start state */
  STATE_STOP,  /* start state */
  STATE_TEST,   /* testing state */
  STATE_FINISH, /* finish state */
  STATE_ERROR   /* error state */
} FSMState;

typedef struct {
    int nSettore;
    int nDisplay;
    int nPLCNum;
    gboolean pressed;
    int nColonna;
    int nPiano;
} PULSANTE, *PPULSANTE;

struct _TestWindowPrivate
{

    GtkWidget *grid_isola;
    GtkWidget *rb_tutti_i_settori;
    GtkWidget *rb_tutte_le_isole;
    GtkWidget *rb_settore;
    GtkWidget *rb_isola;
    GtkWidget *rb_attesa_pulsante;
    GtkWidget *rb_pulsante_simulato;
    GtkWidget *tb_ciclo_infinito;
    GtkWidget *sw_list;
    GtkWidget *lst;
    GtkWidget *pb_start;
    GtkWidget *pb_stop;
    GtkWidget *pb_reset;
    GtkWidget *pb_cancel;
    GtkWidget *sb_settore;
    GtkWidget *sb_isola;
    GtkWidget *sb_delay;
    GtkWidget *lb_settore;
    GtkWidget *lb_isola;
    GtkWidget *lb_attesa;
    GtkWidget *lb_msg;
    GtkWidget *txt_msgs;

    int ubnmisl;
    int settore;
    int delay_simulazione;
    gboolean simulazione;
    gboolean priorita;
    gboolean loop;
    FSMState fsm_state;
    GList *pulsanti;
    PPULSANTE pulsante;
    PLINEA_STRUCT pDatiLinea;
    PSETTORE ptrSettori;
    PSETTORE pSettori[MAX_SETTORI];
    PUBICAZIONI pUbicazioni;
    gint idle;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    int nASETTORE ;
    int nALUCE   ;
    int nACCEN   ;
    int nRACCEN  ;

    int nNSETTORE ;
    int nNLUCE   ;
    int nANNUL   ;
    int nRANNUL  ;

    int nSSETTORE ;
    int nSLUCE   ;
    int nSPEGNI  ;
    int nRSPEGNI ;

    int nRESETL  ;
    int nRRESETL ;
    int nCONGELA ;

};

typedef struct _TestWindowPrivate TestWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(TestWindow, test_window, GTK_TYPE_DIALOG)


void on_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    g_printf("selection_changed\n");
}

gboolean WaitPLCVar(int nPLCVar,int nValue, int nDelay, int nTimeOut)
{
    struct timeval tvActualTime;
    struct timeval tvStartTime;
    int nTimePassed=0;    /* msecs */
    gboolean rc = TRUE;

    gettimeofday(&tvStartTime,NULL);

    plc_refresh_vars();

    /* attendo la risposta al comando plcvar */
    while(nTimePassed < PLC_TIMEOUT && plc_var_value(nPLCVar)!=nValue){
        gettimeofday(&tvActualTime,NULL);

        nTimePassed+=(tvActualTime.tv_sec-tvStartTime.tv_sec)*1000;     /* secondi */
        nTimePassed+=(tvActualTime.tv_usec-tvStartTime.tv_usec)/1000;   /* milli secondi */

        ep_usleep(1000*nDelay);

        plc_refresh_vars();
    }
    if(nTimePassed > nTimeOut){
        rc = FALSE;
    }
    return rc;
}

gboolean SetAndWaitVariable(gpointer win, int nPLCVarToSet, int nPLCVarToCheck, int nValue)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    if(Cfg.nPLC){
        plc_set_var_value(nPLCVarToSet, nValue);

        if(!WaitPLCVar(nPLCVarToCheck, nValue, Cfg.nDelay, PLC_TIMEOUT)){
            trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,"SetAndWaitVariable : Errore di TIMEOUT su fronte salita PLC [SET:%s] - [CHECK:%s]",plc_get_var_name(nPLCVarToSet), plc_get_var_name(nPLCVarToCheck));
            rc = FALSE;
        }
    }
    return rc;
}

gboolean GetVarsIndex(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    char szBuffer[128];
    gboolean rc = TRUE;

    sprintf(szBuffer,"ASETTORE"); if((priv->nASETTORE  = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"ALUCE"   ); if((priv->nALUCE     = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"NSETTORE"); if((priv->nNSETTORE  = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"NLUCE"   ); if((priv->nNLUCE     = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"ACCEN"   ); if((priv->nACCEN     = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"RACCEN"  ); if((priv->nRACCEN    = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"ANNUL"   ); if((priv->nANNUL     = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"RANNUL"  ); if((priv->nRANNUL    = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"SSETTORE"); if((priv->nSSETTORE  = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"SLUCE"   ); if((priv->nSLUCE     = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"SPEGNI"  ); if((priv->nSPEGNI    = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"RSPEGNI" ); if((priv->nRSPEGNI   = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"RESETL"  ); if((priv->nRESETL    = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"RRESETL" ); if((priv->nRRESETL   = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}
    sprintf(szBuffer,"CONGELA" ); if((priv->nCONGELA   = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Variabile %s assente !\n",szBuffer); rc = FALSE;}

    return rc;
}

gboolean PklSetRC(gpointer win, int settore, int numero)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    plc_set_var_value(priv->nASETTORE, settore);
    plc_set_var_value(priv->nALUCE, numero);

    SetAndWaitVariable (win, priv->nACCEN, priv->nRACCEN,TRUE);
    SetAndWaitVariable (win, priv->nACCEN, priv->nRACCEN,FALSE);

    return rc;
}

gboolean PklResetRC(gpointer win, int settore, int numero)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    plc_set_var_value(priv->nASETTORE, settore);
    plc_set_var_value(priv->nALUCE, numero);

    SetAndWaitVariable (win, priv->nANNUL, priv->nRANNUL,TRUE);
    SetAndWaitVariable (win, priv->nANNUL, priv->nRANNUL,FALSE);

    return rc;
}

gboolean PklIsFrozen(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    return plc_var_value(priv->nCONGELA) ? TRUE : FALSE;
}

gboolean PklFreeze(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    plc_set_var_value(priv->nCONGELA,TRUE);

    return rc;
}

gboolean PklUnfreeze(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    plc_set_var_value(priv->nCONGELA,FALSE);

    return rc;
}

gboolean PklReset(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    SetAndWaitVariable (win, priv->nRESETL, priv->nRRESETL,TRUE);
    SetAndWaitVariable (win, priv->nRESETL, priv->nRRESETL,FALSE);

    return rc;
}

gboolean PklCheckPressed(gpointer win, int *settore, int *numero)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = FALSE;

    if(Cfg.nPLC){
        if(plc_var_value(priv->nSPEGNI)){
            *settore = plc_var_value(priv->nSSETTORE);
            *numero  = plc_var_value(priv->nSLUCE);
            rc = TRUE;

            plc_set_var_value(priv->nRSPEGNI,TRUE);
            WaitPLCVar (priv->nSPEGNI, FALSE, Cfg.nDelay, PLC_TIMEOUT);
            plc_set_var_value(priv->nRSPEGNI,FALSE);
        }
    }
    return rc;
}

void ResetSettore(gpointer win, int nIsola,int nSettore)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    int nFirstDisplay=0;
    int nFirstPLCNum=0;
    DBresult *DBRes = NULL;
    int nIndex;


    /*
    * Cerco l'indice del settore relativo ai parametri isola e settore passati alla funzione
    */
    for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
        if(priv->pSettori[nIndex]->nIsola==nIsola && priv->pSettori[nIndex]->nSettore==nSettore){
            priv->pSettori[nIndex]->nIndiceRigaOrdine=0;        /* puntatore alla riga d'ordine da prelevare */
            priv->pSettori[nIndex]->nNumeroRigheOrdine=0;       /* numero delle righe d'ordine da prelevare */
            if(priv->pSettori[nIndex]->nFlagSettore!=DISABILITATO){
                SetStatoSettore(win, nIndex, ATTESA);
            }
            /*
            * Determino i dati della prima ubicazione del settore in oggetto
            */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubnmdsp, ubplcnm from ubicazioni where ubtpubi='%s' AND ubnmisl=%d AND ubnmset=%d order by ubprior;",Cfg.szTipoUbicazioni, nIsola, nSettore);

            if(DBntuples(DBRes)){
                nFirstPLCNum=atoi(DBgetvalue(DBRes,0,1));
            } else {
#ifdef TRACE
                trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "ResetSettore(Isola:%d,Settore:%d) : Non esistono ubicazioni per il settore in oggetto",nIsola, nSettore);
#endif
            }
            priv->pSettori[nIndex]->nOpBlinking=0;
            priv->pSettori[nIndex]->nFirstPLCNum=nFirstPLCNum;

            priv->pSettori[nIndex]->nPLCNum=nFirstPLCNum;       /* Nro PLC Tasto Premuto / da premere */
            priv->pSettori[nIndex]->nNumCopie=0;                /* Numero Copie */
            priv->pSettori[nIndex]->nNumCollo=0;                /* Numero Collo */
            priv->pSettori[nIndex]->nStatoLampada=0;            /* Stato Lampada */
            priv->pSettori[nIndex]->szCodSped[0]='\0';          /* Codice Ordine Attivo */
            priv->pSettori[nIndex]->szCodProd[0]='\0';          /* Codice Prodotto da prelevare/prelevato*/
            priv->pSettori[nIndex]->szCodUbi[0]='\0';           /* Codice Ubicazione del prodotto da prelevare/prelevato*/

            DBclear(DBRes);
        }
    }
}


void ResetSettori(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    int nIndex;

    for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
        ResetSettore(win, priv->pSettori[nIndex]->nIsola,priv->pSettori[nIndex]->nSettore);
    }
}

void SetStatoSettore(gpointer win, int settore_index, StatiSettore stato_settore)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    if(priv->pSettori[settore_index]) priv->pSettori[settore_index]->nStatoSettore = stato_settore;
}

StatiSettore GetStatoSettore(gpointer win, int settore_index)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    if(priv->pSettori[settore_index]) return (priv->pSettori[settore_index]->nStatoSettore);
    return SETTORE_GENERAL_ERROR;
}

void SetStatoLinea(gpointer win, StatiLinea stato_linea)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    if(priv->pDatiLinea) priv->pDatiLinea->nStatoLinea=stato_linea;
}

StatiLinea GetStatoLinea(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    if(priv->pDatiLinea) return (priv->pDatiLinea->nStatoLinea);
    return LINEA_GENERAL_ERROR;
}



gboolean SetupDatiLineaSharedMemory(gpointer win, int nShmKey)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    if((priv->pDatiLinea=(PLINEA_STRUCT)SetupShm(nShmKey,sizeof(LINEA_STRUCT)))==NULL){
        priv->pDatiLinea=(PLINEA_STRUCT)NULL;
        rc = FALSE;
    }
    return rc;
}

gboolean SetupSettoriSharedMemory(gpointer win, int nSettori, int nShmKey)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = FALSE;
    int i;

    /*
    * alloco la shared memory dei settori
    */
    if((priv->ptrSettori = (PSETTORE)SetupShm(nShmKey,nSettori*sizeof(SETTORE)))){
        memset(priv->ptrSettori,0,nSettori*sizeof(SETTORE));

        for(i = 0; i < nSettori; i++){
            priv->pSettori[i]=(PSETTORE)(priv->ptrSettori+i);
        }
        rc = TRUE;
    }

    return rc;
}


/*
* int ReadSettoriInfo(void)
*
* - Legge dalla relazione settori le informazioni relative ai settori della linea e
*   le memorizza nella struttura SETTORISTRUCT;
* - controlla che il numero dei settori memorizzati nella relazione del DB
*   coincida col il parametro <nNumeroSettori> passatogli
*/
gboolean ReadSettoriInfo(gpointer win, int nNumeroSettori)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = FALSE;
    DBresult *DBres;
    int nSettori=0;
    int nIndex=0;

    DBres=DBExecQuery(Cfg.nDebugLevel>1,"select stnmset,stnmisl,stcdflg,sttpset,stcdope,stnmced from settori order by stnmset;");

    if(DBresultStatus(DBres) == DBRES_TUPLES_OK && ((nSettori=DBntuples(DBres))==nNumeroSettori) ){

#ifdef TRACE
        trace_debug(TRUE, TRUE, "SETT I S T CEDOLA");
        trace_debug(TRUE, TRUE, "---- - - - ------");
#endif
        /* memorizzo le informazioni nella struttura preposta */
        for(nIndex=0; nIndex<nSettori; nIndex++){
            priv->pSettori[nIndex]->nSettore       = atoi(DBgetvalue(DBres,nIndex,0));
            priv->pSettori[nIndex]->nIsola         = atoi(DBgetvalue(DBres,nIndex,1));
            priv->pSettori[nIndex]->nFlagSettore   = atoi(DBgetvalue(DBres,nIndex,2));
            priv->pSettori[nIndex]->nTipoSettore   = atoi(DBgetvalue(DBres,nIndex,3));
            priv->pSettori[nIndex]->nCedola        = atoi(DBgetvalue(DBres,nIndex,5));
#ifdef TRACE
            trace_debug(TRUE, TRUE,  "%4d %1d %1d %1d %6d",
                priv->pSettori[nIndex]->nSettore,
                priv->pSettori[nIndex]->nIsola,
                priv->pSettori[nIndex]->nFlagSettore,
                priv->pSettori[nIndex]->nTipoSettore,
                priv->pSettori[nIndex]->nCedola);
#endif
        }
#ifdef TRACE
        trace_debug(TRUE, TRUE,  "---- - - - ------");
#endif
    

        rc = TRUE;
    }

    DBclear(DBres);

    return rc;
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}


gboolean fsm_set_state(gpointer win, FSMState fsm_state)
{
    gboolean rc=FALSE;
    if(win && fsm_state) {
        TestWindowPrivate *priv = test_window_get_instance_private (win);

        priv->fsm_state = fsm_state;
    }
    return rc;
}

gboolean pkl_press_button_timeout_cb (gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    /* pressione pulsante simulata */
    if((priv->pulsante = (PPULSANTE)(priv->pulsanti->data))){
        if (priv->pulsante->pressed == FALSE ){
            priv->pulsante->pressed = TRUE;
        }
    }
    /* it's a timeout cb - return false to get recalled */
    return FALSE;
}

/*
* lettura delle ubicazioni in archivio
*/
int ReadUbicazioni(PUBICAZIONI pUbi)
{
    int nIndex;
    DBresult *DBRes = NULL;
    int nTuples;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubcdubi, ubcdpro, ubnmisl, ubnmset, ubnmdsp, ubplcnm, ubprior, ubstato from ubicazioni where ubtpubi='%s';",Cfg.szTipoUbicazioni);

    nTuples=DBntuples(DBRes);

    for(nIndex=0;nIndex<nTuples;nIndex++){

        strcpy(pUbi->Ubicazione[nIndex].szCodUbi,         DBgetvalue(DBRes,nIndex,0));
        strcpy(pUbi->Ubicazione[nIndex].szCodProd,        DBgetvalue(DBRes,nIndex,1));
        pUbi->Ubicazione[nIndex].nIsola            = atoi(DBgetvalue(DBRes,nIndex,2));
        pUbi->Ubicazione[nIndex].nSettore          = atoi(DBgetvalue(DBRes,nIndex,3));
        pUbi->Ubicazione[nIndex].nDisplay          = atoi(DBgetvalue(DBRes,nIndex,4));
        pUbi->Ubicazione[nIndex].nPLCNum           = atoi(DBgetvalue(DBRes,nIndex,5));
        pUbi->Ubicazione[nIndex].nPriorita         = atoi(DBgetvalue(DBRes,nIndex,6));

        pUbi->Ubicazione[nIndex].nIndex=nIndex;
        
    }
    DBclear(DBRes);
    pUbi->nUbicazioni=nIndex;

    return nIndex;
}

int CmpUbicazione(PUBICAZIONE pUbi1,PUBICAZIONE pUbi2)
{
    if(pUbi1->nSettore < pUbi2->nSettore){
        return -1;
    }
    if(pUbi1->nSettore > pUbi2->nSettore){
        return 1;
    }
    if(pUbi1->nPLCNum < pUbi2->nPLCNum){
        return -1;
    }
    if(pUbi1->nPLCNum > pUbi2->nPLCNum){
        return 1;
    }
    return 0;
}

void SortUbicazioni(PUBICAZIONI pUbi)
{
    qsort(pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}


enum TagUbicazioniColumns {
    COLUMN_SETTORE,
    COLUMN_COLONNA,
    COLUMN_PIANO,
    COLUMN_PRESSED,

    NUM_COLUMNS,
} UbicazioniColumns;

PUBICAZIONE SearchUbicazione(PUBICAZIONI pUbi,int nSettore,int nPLCNum)
{
    UBICAZIONE Ubi;

    Ubi.nSettore=nSettore;
    Ubi.nPLCNum=nPLCNum;

    return bsearch(&Ubi,pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

void set_selection(gpointer win, GtkWidget *lst, int settore, int colonna, int piano, gboolean value, gboolean set_position, gboolean select_row)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    char *pszString;
    GtkTreeIter iter;
    int s, c, p;

    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {
        /*
        * colonne : settore, fila / colonna / piano 
        */
        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter, COLUMN_SETTORE,  &pszString); s = atoi(pszString); g_free(pszString); /* settore  */
        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter, COLUMN_COLONNA,  &pszString); c = atoi(pszString); g_free(pszString); /* colonna  */
        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter, COLUMN_PIANO,    &pszString); p = atoi(pszString); g_free(pszString); /* piano    */
        
        if(s == settore && c == colonna && p == piano){
            /* found */
            gtk_list_store_set (GTK_LIST_STORE(TREE_MODEL_LST(lst)), &iter, COLUMN_PRESSED, value, -1);

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

void pressed_data_func (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
    gboolean  pressed;

    gtk_tree_model_get(model, iter, COLUMN_PRESSED, &pressed, -1);

    g_object_set(renderer, "icon-name", pressed ? "gtk-yes" : "gtk-no", NULL);
}

gboolean fsm_manage(gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gboolean rc = TRUE;

    switch(priv->fsm_state){
        case STATE_START:
        {
            GtkListStore *store;
            GtkTreeIter iter;
            GtkCellRenderer *text_renderer;
            GtkCellRenderer *pixmap_renderer;
            GtkTreeViewColumn *col;
            char key[128];
            DBresult *DBRes = NULL;

            g_printf("Start\n");

            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_tutte_le_isole))){
                priv->ubnmisl=0;
            } else {
                priv->settore= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_isola));
            }

            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_tutti_i_settori))){
                priv->settore=0;
            } else {
                priv->settore= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_settore));
            }

            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_attesa_pulsante))){
                priv->simulazione=FALSE;
            } else {
                priv->simulazione=TRUE;
                priv->delay_simulazione = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_delay));
            }

            priv->loop=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->tb_ciclo_infinito));

            /* libero eventuale memoria allocata */
            if(priv->pulsanti){
                g_list_free_full(priv->pulsanti, g_free);
                priv->pulsanti = NULL;
            }

            /* 
            * Inizio test
            */
            gtk_widget_set_sensitive(priv->pb_cancel,FALSE);
            gtk_widget_set_sensitive(priv->pb_start,FALSE);
            gtk_widget_set_sensitive(priv->pb_stop,TRUE);
            gtk_widget_set_sensitive(priv->pb_reset,FALSE);

            trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs,"Iniziato test impianto");


            if(priv->settore){
               sprintf( key, "and settore = %d", priv->settore);
            } else {
               strcpy( key, "");
            }
            DBRes=DBExecQuery(TRUE,"select ubcdubi, ubcdpro, ubnmisl, ubnmset, ubnmdsp, ubplcnm, ubprior, ubstato, settore(ubcdubi), colonna(ubcdubi), piano(ubcdubi) from ubicazioni where ubtpubi = '%s' and ubcdflg='%c' %s order by ubnmisl,ubprior;",
                Cfg.szTipoUbicazioni,
                UBICAZIONE_AUTOMATICA,
                key);

            int nTuples=DBntuples(DBRes);
            char tmp_buffer[128];

            store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_BOOLEAN);

            /* popolo la lista dei pulsanti da testare */
            int nIndex;
            for(nIndex=0;nIndex<nTuples;nIndex++){
                PPULSANTE pulsante = g_new0(PULSANTE, 1);

                pulsante->nSettore    = atoi(DBgetvalue(DBRes,nIndex,3));
                pulsante->nDisplay    = atoi(DBgetvalue(DBRes,nIndex,4));
                pulsante->nPLCNum     = atoi(DBgetvalue(DBRes,nIndex,5));
                pulsante->nColonna    = atoi(DBgetvalue(DBRes,nIndex,8));
                pulsante->nPiano      = atoi(DBgetvalue(DBRes,nIndex,9));
                pulsante->pressed     = FALSE;

                priv->pulsanti        = g_list_append(priv->pulsanti, pulsante  );

                gtk_list_store_append (store, &iter);
                gtk_list_store_set (store, &iter,
                    COLUMN_SETTORE  , pulsante->nSettore, 
                    COLUMN_COLONNA  , pulsante->nColonna,
                    COLUMN_PIANO    , pulsante->nPiano,
                    COLUMN_PRESSED  , FALSE, -1);

            }
            DBclear(DBRes);

            if(priv->lst){
                gtk_widget_destroy(GTK_WIDGET(priv->lst));
            }
            priv->lst = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
            gtk_widget_set_name (priv->lst, "lst");

            text_renderer = gtk_cell_renderer_text_new ();
            pixmap_renderer = gtk_cell_renderer_pixbuf_new();

            g_object_set (text_renderer, "xalign",1.0,NULL) ;

            gtk_tree_view_append_column (GTK_TREE_VIEW (priv->lst), gtk_tree_view_column_new_with_attributes( "SETTORE", text_renderer,"text", COLUMN_SETTORE , NULL));
            gtk_tree_view_append_column (GTK_TREE_VIEW (priv->lst), gtk_tree_view_column_new_with_attributes( "COLONNA", text_renderer,"text", COLUMN_COLONNA , NULL));
            gtk_tree_view_append_column (GTK_TREE_VIEW (priv->lst), gtk_tree_view_column_new_with_attributes( "PIANO",   text_renderer,"text", COLUMN_PIANO   , NULL));
            gtk_tree_view_append_column (GTK_TREE_VIEW (priv->lst), col = gtk_tree_view_column_new_with_attributes( "PREMUTO", pixmap_renderer, NULL));
            gtk_tree_view_column_set_cell_data_func(col, pixmap_renderer, pressed_data_func, NULL, NULL);

            g_object_unref (G_OBJECT (store));

            GtkLabel *lb_column_title;
            lb_column_title=GTK_LABEL(gtk_label_new("SETTORE" )); gtk_widget_show (GTK_WIDGET(lb_column_title)); gtk_tree_view_column_set_widget (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->lst), COLUMN_SETTORE ), GTK_WIDGET(lb_column_title));
            lb_column_title=GTK_LABEL(gtk_label_new("COLONNA" )); gtk_widget_show (GTK_WIDGET(lb_column_title)); gtk_tree_view_column_set_widget (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->lst), COLUMN_COLONNA ), GTK_WIDGET(lb_column_title));
            lb_column_title=GTK_LABEL(gtk_label_new("PIANO"   )); gtk_widget_show (GTK_WIDGET(lb_column_title)); gtk_tree_view_column_set_widget (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->lst), COLUMN_PIANO   ), GTK_WIDGET(lb_column_title));
            lb_column_title=GTK_LABEL(gtk_label_new("PREMUTO" )); gtk_widget_show (GTK_WIDGET(lb_column_title)); gtk_tree_view_column_set_widget (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->lst), COLUMN_PRESSED ), GTK_WIDGET(lb_column_title));

            gtk_container_add (GTK_CONTAINER (priv->sw_list), priv->lst);
            gtk_widget_show (priv->lst);

            PklReset(win);
            /* ResetSettori(win); */
            if(PklIsFrozen(win)){
                PklUnfreeze(win);
            }


            /* accendo il primo pulsante della lista */
            if(priv->pulsanti && (priv->pulsante = (PPULSANTE)g_list_first(priv->pulsanti)->data)){
                PklSetRC(win, priv->pulsante->nSettore, priv->pulsante->nPLCNum);

                SetStatoSettore(win, priv->pulsante->nSettore-1, TEST);

                sprintf(priv->pSettori[priv->pulsante->nSettore-1]->szCodUbi, "S:%2d U:%02d-%02d-%02d", 
                        priv->pulsante->nSettore, 
                        priv->pulsante->nSettore, 
                        priv->pulsante->nColonna, 
                        priv->pulsante->nPiano);

                if(priv->simulazione){
                    g_timeout_add(priv->delay_simulazione,pkl_press_button_timeout_cb, win);
                }

                gtk_widget_set_sensitive(priv->pb_cancel, FALSE);
                gtk_widget_set_sensitive(priv->pb_stop,   TRUE);
                gtk_widget_set_sensitive(priv->pb_start,  FALSE);
                gtk_widget_set_sensitive(priv->pb_reset,  FALSE);

                fsm_set_state(win, STATE_TEST);
            } else {
                /* non ci sono pulsanti da premere */
                fsm_set_state(win, STATE_FINISH);
            }

        }
        break;

        case STATE_TEST:
        {
            PPULSANTE pulsante = priv->pulsante ;
            int settore, plc_num, col;

            if(pulsante){
                /* real button check */
                if (PklCheckPressed(win, &settore, &plc_num)){
                    if(SearchUbicazione(priv->pUbicazioni,settore,plc_num)){
                        pulsante->pressed=TRUE;
                    }
                }
                if(pulsante->pressed){

                    // g_printf("Premuto pulsante %02d-%02d-%02d - S:%2d N:%2d\n", pulsante->nSettore, pulsante->nColonna, pulsante->nPiano, pulsante->nSettore, pulsante->nPLCNum);
                    set_selection(win, priv->lst, 
                        pulsante->nSettore, 
                        pulsante->nColonna, 
                        pulsante->nPiano, 
                        pulsante->pressed, TRUE, TRUE);
                    strcpy(priv->pSettori[priv->pulsante->nSettore-1]->szCodUbi, "" );

                    /* spengo la luce */
                    PklResetRC(win, pulsante->nSettore, pulsante->nPLCNum);
                    /* passo al prossimo pulsante */
                    if((priv->pulsanti = priv->pulsanti->next)){
                        if((priv->pulsante = (PPULSANTE)(priv->pulsanti->data))){
                            PklSetRC(win, priv->pulsante->nSettore, priv->pulsante->nPLCNum);

                            SetStatoSettore(win, priv->pulsante->nSettore-1, TEST);

                            sprintf(priv->pSettori[priv->pulsante->nSettore-1]->szCodUbi, "S:%2d U:%2d-%2d-%2d", 
                                    priv->pulsante->nSettore, 
                                    priv->pulsante->nSettore, 
                                    priv->pulsante->nColonna, 
                                    priv->pulsante->nPiano);

                            if(priv->simulazione){
                                g_timeout_add(priv->delay_simulazione,pkl_press_button_timeout_cb, win);
                            }
                        } else {
                            fsm_set_state(win, STATE_ERROR);
                        }
                    } else {
                        g_printf("Ultimo pulsante\n");

                        if(priv->loop){
                            fsm_set_state(win, STATE_START);
                        } else {
                            fsm_set_state(win, STATE_FINISH);
                        }
                    }
                }
            }
        }
        break;

        case STATE_STANDBY:
            g_printf("Standby\n");
        break;


        case STATE_FINISH:
            g_printf("Finish\n");
            gtk_widget_set_sensitive(priv->pb_cancel, TRUE);
            gtk_widget_set_sensitive(priv->pb_stop,   FALSE);
            gtk_widget_set_sensitive(priv->pb_start,  TRUE);
            gtk_widget_set_sensitive(priv->pb_reset,  TRUE);
            rc = FALSE;
        break;

        case STATE_STOP:
            g_printf("Stop\n");
            fsm_set_state(win, STATE_FINISH);
        break;

        case STATE_ERROR:
            g_printf("Error\n");
            fsm_set_state(win, STATE_FINISH);
        break;
    }
    return rc;
}

static void start_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);


    fsm_set_state(win, STATE_START);
    priv->idle = g_idle_add (fsm_manage, win);

}

static void stop_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    fsm_set_state(win, STATE_STOP);

}

static void reset_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}

static void tutti_i_settori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_settore, FALSE);
    gtk_widget_set_sensitive(priv->sb_settore, FALSE);
}

static void settore_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_settore, TRUE);
    gtk_widget_set_sensitive(priv->sb_settore, TRUE);
}

static void tutte_le_isole_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_isola, FALSE);
    gtk_widget_set_sensitive(priv->sb_isola, FALSE);
}

static void isola_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_isola, TRUE);
    gtk_widget_set_sensitive(priv->sb_isola, TRUE);
}

static void attesa_pulsante_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_attesa, FALSE);
    gtk_widget_set_sensitive(priv->sb_delay, FALSE);
}

static void pulsante_simulato_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);
    gtk_widget_set_sensitive(priv->lb_attesa, TRUE);
    gtk_widget_set_sensitive(priv->sb_delay, TRUE);
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static GActionEntry entries[] = {
    {"start",             start_activated,              NULL, NULL, NULL},
    {"stop",              stop_activated,               NULL, NULL, NULL},
    {"reset",             reset_activated,              NULL, NULL, NULL},
    {"tutti_i_settori",   tutti_i_settori_activated,    NULL, NULL, NULL},
    {"settore",           settore_activated,            NULL, NULL, NULL},
    {"tutte_le_isole",    tutte_le_isole_activated,     NULL, NULL, NULL},
    {"isola",             isola_activated,              NULL, NULL, NULL},
    {"attesa_pulsante",   attesa_pulsante_activated,    NULL, NULL, NULL},
    {"pulsante_simulato", pulsante_simulato_activated,  NULL, NULL, NULL},
    {"close",             close_activated,              NULL, NULL, NULL}

};

static void test_window_init (TestWindow *win)
{
    TestWindowPrivate *priv = test_window_get_instance_private (win);

    g_printf("%s init\n", WINDOW_NAME);

    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);

    /*
    * Lettura Lista variabili PLC
    */
    plc_read_vars_list(priv->pszCfgFileName);

    /*
    * Lettura e ordinamento delle ubicazioni
    */
    /* TODO - free on exit */
    if((priv->pUbicazioni = g_new(UBICAZIONI, 1))){
        ReadUbicazioni(priv->pUbicazioni);
        SortUbicazioni(priv->pUbicazioni);
    }
    if(SetupDatiLineaSharedMemory(win, Cfg.nShmKey)){
        SetStatoLinea(win, LINEA_IN_TEST);
    } else {
        trace_debug(TRUE, TRUE,  "Error in Setup Linea Shared Memory (%d)", Cfg.nShmKey);
    }

    if(SetupSettoriSharedMemory(win, Cfg.nNumeroSettori, Cfg.nSettoriShmKey)){
        ReadSettoriInfo(win, Cfg.nNumeroSettori);
    } else {
        trace_debug(TRUE, TRUE,  "Error in Setup Settori Shared Memory (%d)", Cfg.nSettoriShmKey);
        g_printf("Error in Setup Settori Shared Memory (%d)", Cfg.nSettoriShmKey);
    }

    /*
    * Memorizzo gli indici nel vettore interno
    */
    if(Cfg.nPLC){
        GetVarsIndex(win);

        plc_set_var_value(priv->nASETTORE, FALSE);
        plc_set_var_value(priv->nALUCE,    FALSE);
        plc_set_var_value(priv->nACCEN,    FALSE);
        plc_set_var_value(priv->nANNUL,    FALSE);
        plc_set_var_value(priv->nRSPEGNI,  FALSE);
        plc_set_var_value(priv->nRESETL,   FALSE);
    }
    gtk_spin_button_set_range ( GTK_SPIN_BUTTON(find_child(GTK_WIDGET(win),"sb_settore")), (gdouble)0, (gdouble) Cfg.nNumeroSettori);
    gtk_spin_button_set_range ( GTK_SPIN_BUTTON(find_child(GTK_WIDGET(win),"sb_isola")), (gdouble)0, (gdouble) Cfg.nNumeroIsole);

    if(Cfg.nNumeroIsole==0){
        gtk_widget_hide( find_child(GTK_WIDGET(win),"grid_isola"));
    }

}

static void test_window_dispose (GObject *object)
{
    TestWindowPrivate *priv = test_window_get_instance_private (TEST_WINDOW (object));
    TestWindow *win = TEST_WINDOW (object);

    g_printf("%s dispose\n", WINDOW_NAME);

    if(priv->wsi){          
        g_settings_sync ();
        window_save_state (win,  priv->wsi, priv->settings);
        window_size_info_free (priv->wsi);
        priv->wsi = NULL;
    }

    if(priv->settings){          
        g_clear_object (&(priv->settings)); 
        priv->settings=NULL;    
    }

    if(priv->pszCfgFileName){ 
        g_free(priv->pszCfgFileName); 
        priv->pszCfgFileName = NULL; 
    }

    if(priv->ptrSettori){
        DetachSharedMemory((char *)priv->ptrSettori);

        if(!DeleteSharedMemory(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
            trace_debug(TRUE, TRUE,  "Error Removing Settori Shared Memory (%d)", Cfg.nSettoriShmKey);
        }
        priv->ptrSettori = NULL;
    }

    if(priv->pDatiLinea){
        DetachSharedMemory((char *)priv->pDatiLinea);

        if(!DeleteSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
            trace_debug(TRUE, TRUE,  "Error Removing Linea Shared Memory (%d)", Cfg.nShmKey);
        }
        priv->pDatiLinea = NULL;
    }

    G_OBJECT_CLASS (test_window_parent_class)->dispose (object);
}

static void test_window_class_init (TestWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = test_window_dispose;

    g_printf("%s class init\n", WINDOW_NAME);

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, grid_isola          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_tutti_i_settori  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_settore          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_tutte_le_isole   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_isola            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_attesa_pulsante  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, rb_pulsante_simulato);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, tb_ciclo_infinito   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, sw_list             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, pb_start            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, pb_stop             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, pb_reset            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, pb_cancel           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, sb_settore          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, sb_isola            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, sb_delay            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, lb_settore          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, lb_isola            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, lb_attesa           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, lb_msg              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TestWindow, txt_msgs            );

}

TestWindow * test_window_new (MainWindow *win)
{
    TestWindow *w = g_object_new (TEST_WINDOW_TYPE, NULL);

    return w;
}
