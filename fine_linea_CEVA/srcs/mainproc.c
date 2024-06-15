/*
* mainproc.c
* work procedures 
* Easy Picking 4.0
* Copyright A&L srl 2003-2024
* Autore : Roberto Mantovani
*/

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gprintf.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>


#include <sql.h>
#include <sqlext.h>

#include <ep-common.h>
#include <ep-db.h>
#include <ep-odbc.h>
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

typedef enum _execution_reply_t {
    REPLY_BUONO,
    REPLY_SCARTO,
    REPLY_FORZATURA,

    REPLY_ERROR
} execution_reply_t;

static char szBarcodeColloBilancia[512]="";
static int nPesoColloBilancia=0;
static ep_bool_t bColloInBilancia=FALSE;
static ep_bool_t bColloScarto=FALSE;
static ep_bool_t bColloBuono=FALSE;
static ep_bool_t bDataFound=FALSE;
static guint _timeout_collo_bilancia=0;

void GestPlc(gpointer win);

static void _odbc_print_result(odbc_result_t *odbc_res)
{
    if (odbc_res && odbc_res->columns && odbc_res->data) {
        trace_debug(FALSE, TRUE, "");
        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            trace_debug(FALSE, FALSE, "| %*.*s ", odbc_get_flen(odbc_res, j), odbc_get_flen(odbc_res, j), odbc_get_fname(odbc_res, j));
            if (j+1==odbc_get_nfields(odbc_res)) trace_debug(FALSE, FALSE, "|");
        }
        trace_debug(FALSE, TRUE, "");
        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            trace_debug(FALSE, FALSE, "| ");
            for(int k=0; k < odbc_get_flen(odbc_res, j); k++) trace_debug(FALSE, FALSE, "-");
            trace_debug(FALSE, FALSE, " ");
            if (j+1==odbc_get_nfields(odbc_res)) trace_debug(FALSE, FALSE, "|");
        }
        trace_debug(FALSE, TRUE, "");
        for (int i=0; i < odbc_get_ntuples(odbc_res); i++){
            for (int j=0; j < odbc_get_nfields(odbc_res); j++){
                trace_debug(FALSE, FALSE, "| %*.*s ", odbc_get_flen(odbc_res, j), odbc_get_flen(odbc_res, j), odbc_get_fvalue(odbc_res, i, j));
                if (j+1==odbc_get_nfields(odbc_res)) trace_debug(FALSE, FALSE, "|");
            }
            trace_debug(FALSE, TRUE, "");
        }
    }
}

static bool _connect_database(char *DSN, char *user, char *pwd)
{
    trace_debug(TRUE, TRUE, "ODBC CONNECT : DSN : [%s] - User [%s] - pwd [%s]\n", DSN, user, pwd);

    return odbc_connect (DSN, user, pwd);
}

/**
 * @brief extract weight from this string : "0254254I!GV01|GW06|7|GD01|kg;-2;48|LX02"
 * 
 * @param recvd_data 
 * @return int 
 */
static int _get_peso_collo_from_recvd_data(char *recvd_data)
{
    char **str_list = g_strsplit (recvd_data,"|",-1);
    gint64 peso_deca_grammi = 0;

    if(str_list[0] && str_list[1] && str_list[2] && str_list[3] && str_list[4] && str_list[5]) {
        trace_debug(TRUE, TRUE, "Received string : %s | %s | %s | %s | %s | %s\n", 
            str_list[0], 
            str_list[1], 
            str_list[2], 
            str_list[3], 
            str_list[4], 
            str_list[5]);

        char **str_weight = g_strsplit (str_list[4],";",-1);
        if(str_weight[0] && str_weight[1] && str_weight[2]) {
            trace_debug(TRUE, TRUE, "um : %s stability: %-2.2s weight : %-10.10s\n", 
                str_weight[0],
                str_weight[1],
                str_weight[2]);

            peso_deca_grammi = g_ascii_strtoll(str_weight[2], NULL, 10);
        }
        g_strfreev(str_weight);
    }

    g_strfreev(str_list);

    return (int)peso_deca_grammi;
}

bool _print_error_page(char *printer_name)
{
	bool bOK=TRUE;
	char szCommand[512];

	sprintf(szCommand,"sh -c 'cd %s;/usr/bin/xvfb-run -a rptrender -databaseURL=pgsql://localhost/mr -username=roberto -passwd=3zin -printerName=Kyocera_ECOSYS_P3055dn_2_ -autoprint -close %s/%s'",Cfg.szPathStampe,Cfg.szPathData,"collo_scartato_log21.xml");
#ifdef TRACE
	trace_out_vstr_date(1,"RPTrender : %s",szCommand);
#endif
	system(szCommand);

	return bOK;
}

static gint _timeout_collo_bilancia_cb( gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if (bColloInBilancia) {
        bColloScarto = TRUE;
        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "PC -> PLC - COLLO SCARTATO PER MANCANZA DI LETTURA O PESO");
        _print_error_page(Cfg.szPrinterConsole);
    }

    /* 
    * As this is a timeout function, return FALSE so that it
    * do not continues to get called 
    */
    return(FALSE);
}

execution_reply_t _execute_request(gpointer win, char *table_name, char *szBarcode, int nPeso)
{
    char sql_cmd[512];
    odbc_result_t *odbc_res;
    gboolean rc = FALSE;
    execution_reply_t res = REPLY_ERROR;
    char forza[256];
    int pesot;
    int cnt;
    int toll;
    int tuples = -1;

    trace_debug(TRUE, TRUE, "ODBC CONNECT : DSN : [%s] - User [%s] - pwd [%s] - table [%s]\n", Cfg.DSN, Cfg.user_name, Cfg.pwd, table_name);

    if (odbc_connect (Cfg.DSN, Cfg.user_name, Cfg.pwd)){
        sprintf(sql_cmd, "select forza, pesot, toll, cntpesatura from %s where barcode='%s'", table_name, szBarcode);
        trace_debug(TRUE, TRUE, "execute : %s", sql_cmd);
        if((odbc_res = odbc_execute_stmt (FALSE, sql_cmd))){
            _odbc_print_result(odbc_res);
            tuples = odbc_get_ntuples(odbc_res);
            trace_debug(TRUE, TRUE, "tuples : %d", tuples);
            strncpy(forza, odbc_get_fvalue(odbc_res, 0, 0), sizeof(forza));
            pesot = strtol(odbc_get_fvalue(odbc_res, 0, 1), NULL, 10);
            toll  = strtol(odbc_get_fvalue(odbc_res, 0, 2), NULL, 10);
            cnt   = strtol(odbc_get_fvalue(odbc_res, 0, 3), NULL, 10);
            odbc_free_result(odbc_res);
            rc = TRUE;
        } else {
            trace_debug(TRUE, TRUE, "ODBC EXECUTE STMT ERROR");
        } 
        odbc_disconnect();
    } else {
        trace_debug(TRUE, TRUE, "ODBC CONNECT ERROR");
    }

    if (rc){
        if(tuples){

            trace_debug(TRUE, TRUE, "forza : %s", forza);
            trace_debug(TRUE, TRUE, "pesot : %d", pesot);
            trace_debug(TRUE, TRUE, "toll  : %d", toll);
            trace_debug(TRUE, TRUE, "cnt   : %d", cnt);

            cnt++;

            if(!strcmp(forza, "P")){
                if(nPeso > pesot+toll || nPeso < pesot-toll){
                    /* peso fuori tolleranza - scartato */
                    res = REPLY_SCARTO;
                } else {
                    /* peso ok  - buono */
                    res = REPLY_BUONO;
                }
            } else {
                /* forzatura - buono */
                res = REPLY_FORZATURA;
            }
            if (odbc_connect (Cfg.DSN, Cfg.user_name, Cfg.pwd)){
                sprintf(sql_cmd, "update %s set pesor=%d, datapesa=CURRENT, cntpesatura=%d where barcode='%s'", table_name, nPeso, cnt, szBarcode);
                trace_debug(TRUE, TRUE, "execute : %s", sql_cmd);
                if((odbc_res = odbc_execute_stmt (FALSE, sql_cmd)) && SQL_SUCCEEDED(odbc_res->odbc_rc)){
                    odbc_free_result(odbc_res);
                    rc = TRUE;
                } else {
                    trace_debug(TRUE, TRUE, "ODBC EXECUTE STMT ERROR");
                } 
                odbc_disconnect();
            } else {
                trace_debug(TRUE, TRUE, "ODBC CONNECT ERROR");
            }
        }
    }

    return res;
}

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
    static LINEA_STRUCT DatiLinea;
    static PLINEA_STRUCT pDatiLinea;
    static ep_bool_t bDatiSpediti=FALSE;
    static ep_bool_t bReindexEseguito=FALSE;
    char szTxMonitor[128];
    char szRxMonitor[128];
    char szCommand[128];
    char szBuffer[128];
    char szStato[256];

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(bFirstTime){

        refresh_lista_colli(win);
        srand(time(NULL));

        RunSimpleProcess(PROC_SOCKET_SERVER_SCANNER,  Cfg.szPathExe );
        RunSimpleProcess(PROC_SOCKET_SERVER_BILANCIA, Cfg.szPathExe );

        /*
        if(!_connect_database(Cfg.DSN, Cfg.user_name, Cfg.pwd)){
            trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Errore in connessione DB [%s] [%s]", Cfg.DSN, Cfg.user_name);
        }
        */
        if(Cfg.nPlc) {
            plc_read_vars_list(priv->pszCfgFileName);

            plc_connect(Cfg.szPLCHost, Cfg.nPLCService);
        }

        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "A&L - Easy Picking 4.0 - Fine Linea - Application Ready\n");
        SendMessage(PROC_MAIN, PROC_FINE_LINEA, PROGRAM_STARTED, NULL);
        pDatiLinea = &DatiLinea;
        memset(pDatiLinea, 0, sizeof(LINEA_STRUCT));

    } else {
        static int printed=0;
        if (!printed) {
            // test 
            //_print_error_page(Cfg.szPrinterConsole);
            printed=1;
        }
    }

    /*
    * Messaggi da altri processi
    */
    while((nCarIn = ProcessReceiveMsgNoWait(PROC_FINE_LINEA, &nAllMsg, szInMsg))>=0){
        UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

        if(Cfg.nDebugLevel>1){
            trace_debug(TRUE, TRUE, "Msg [%s] - [%d] %s%s%s from [%s]",
                szInMsg,
                InMsgStruct.code,
                strlen(szText)?"[":"",strlen(szText)? szText : "",strlen(szText)?"]":"",
                ProcessGetName(InMsgStruct.srce));
        }

        /*
        * messaggi generici
        */
        switch (InMsgStruct.code){
            case DISPLAY_MSG:
            {
                GdkColor *pcColor;

                trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "%s Display MSG %s", ProcessGetName(InMsgStruct.srce),szText);
            }
            break;
        }

        switch (InMsgStruct.srce){
            case PROC_SOCKET_SERVER_SCANNER:
                switch (InMsgStruct.code){
                    case PROGRAM_STARTED:
                        sprintf(szBuffer, "%d", Cfg.nScannerPort);  SendMessage(PROC_SOCKET_SERVER_SCANNER,  PROC_FINE_LINEA, SOCK_OPEN_SERVICE, szBuffer);
                    break;
                    case SOCK_RECEIVED_MSG:
                        delete_new_line(szText);
                        strcpy(szBarcodeColloBilancia, szText);
                        szBarcodeColloBilancia[12]='\0';
                        trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs, "Letto Barcode [%s]", szBarcodeColloBilancia);
                        gtk_label_printf         (priv->lb_barcode_bilancia, szBarcodeColloBilancia);
                        gtk_widget_set_foreground(priv->lb_barcode_bilancia, &cGREEN);
                    break;
                }
            break;
            case PROC_SOCKET_SERVER_BILANCIA:
                switch (InMsgStruct.code){
                    case PROGRAM_STARTED:
                        sprintf(szBuffer, "%d", Cfg.nBilanciaPort); SendMessage(PROC_SOCKET_SERVER_BILANCIA, PROC_FINE_LINEA, SOCK_OPEN_SERVICE, szBuffer);
                    break;
                    case SOCK_RECEIVED_MSG:
                        delete_new_line(szText);
                        nPesoColloBilancia = _get_peso_collo_from_recvd_data(szText)*10;
                        trace_debug_gtk("CYAN", TRUE, TRUE, priv->txt_msgs, "Letto Peso [%7.2f]", (float)((float)nPesoColloBilancia/(float)1000));
                        gtk_label_printf         (priv->lb_peso_rea, "%7.2f", (float)((float)nPesoColloBilancia/(float)1000));
                        gtk_widget_set_foreground(priv->lb_peso_rea, &cGREEN);
                    break;
                }
            break;
        }
    }

    /*
    * Gestione dei Messaggi PLC (NON LA PRIMA VOLTA)
    */
    GestPlc(win);

    /*
    * Controllo lo stato della Trasmissione/Ricezione
    */
    if(lActualTime!=time((time_t *)0)){
        static unsigned int count=0;


        /* se collo in bilancia e ultimo peso valido e ultimo barcode valido e richiesta non in corso */
        if(bColloInBilancia && nPesoColloBilancia && strlen(szBarcodeColloBilancia) && !bDataFound){
            /* verifico sincronismo */
            if (bColloBuono || bColloScarto) {
                trace_debug(TRUE, TRUE, "errore di sincronismo 1: bColloInBilancia:%d bColloBuono:%d bColloScarto:%d", bColloInBilancia, bColloBuono, bColloScarto);
            }
            if (!bDataFound) {
                /* eseguo richiesta */
                execution_reply_t rc = _execute_request(win, "AEL_COLLI", szBarcodeColloBilancia, nPesoColloBilancia);
                switch(rc){
                    case REPLY_SCARTO:
                        bColloScarto = TRUE;
                        gtk_widget_set_foreground(priv->lb_host_result, &cORANGE);
                    break;
                    case REPLY_BUONO:
                        bColloBuono = TRUE;
                        gtk_widget_set_foreground(priv->lb_host_result, &cGREEN);
                    break;
                    case REPLY_FORZATURA:
                        bColloBuono = TRUE;
                        gtk_widget_set_foreground(priv->lb_host_result, &cBLUE);
                    break;
                    case REPLY_ERROR:
                        /* errore */
                        gtk_widget_set_foreground(priv->lb_host_result, &cRED);
                    break;
                }
                gtk_label_printf(priv->lb_host_result, szStato);
                if(bColloScarto || bColloBuono) {
                    DBresult *DBRes;
                    DBRes=DBExecQuery(Cfg.nDebugLevel,"insert into colli_fine_linea (BARCODE, PESO, STATO, DEST) values ('%s','%d','%s','%s');", szBarcodeColloBilancia, nPesoColloBilancia, szStato, bColloBuono ? "BUONO" : "SCARTATO");
                    DBclear(DBRes);
                    update_collo(win, szBarcodeColloBilancia);
                    set_selection(win, find_child(GTK_WIDGET(win),"lst"), szBarcodeColloBilancia, TRUE, TRUE);
                    /* rimuovo il timeout di collo non letto / non processato */
                    g_source_remove ( _timeout_collo_bilancia );

                }
                bDataFound = TRUE;
            }
        }


        /* ogni 5 secondi in simulazione collo in */
        if(Cfg.nSimulation && !(count%5)) {
            if(!bColloInBilancia) {

                bColloInBilancia = TRUE;

                sprintf(szBarcodeColloBilancia, "%d", rand());

                gtk_label_printf         (priv->lb_barcode_bilancia, szBarcodeColloBilancia);
                gtk_widget_set_foreground(priv->lb_barcode_bilancia, &cGREEN);

                nPesoColloBilancia = rand();

                gtk_label_printf         (priv->lb_peso_rea, "%7.2f", (float)((float)nPesoColloBilancia/(float)1000));
                gtk_widget_set_foreground(priv->lb_peso_rea, &cGREEN);
            }
        }
        /* ogni 10 secondi in simulazione collo fuori */
        if(Cfg.nSimulation && !(count%10)) {
            if(bColloInBilancia) {

                bColloInBilancia = FALSE;
                bColloBuono = FALSE;
                bColloScarto = FALSE;

                gtk_widget_set_foreground(priv->lb_barcode_bilancia, &cORANGE);
                gtk_widget_set_foreground(priv->lb_peso_rea, &cORANGE);
                gtk_widget_set_foreground(priv->lb_host_result, &cORANGE);
            }
        }

        /*
        * Stato PLC . Check ogni secondo
        */
        if(Cfg.nPlc){
            if(plc_get_status() == -1) {
                plc_connect(Cfg.szPLCHost, Cfg.nPLCService);
                trace_debug(TRUE, TRUE, "PLC CONNECT");
            } else {
                pDatiLinea->bPLCStatus = plc_get_status();
                // trace_debug(TRUE, TRUE, "PLC STATUS : %d", pDatiLinea->bPLCStatus);
            }
        }
        /*
        * Gestione stati linea
        */
        if(bFirstTime || pDatiLinea->bPLCStatus!=OldLineData.bPLCStatus){
            OldLineData.bPLCStatus = pDatiLinea->bPLCStatus;

            gtk_label_printf(priv->lb_plc, pDatiLinea->bPLCStatus ? "OK" : "KO");
            gtk_widget_set_foreground(priv->lb_plc, pDatiLinea->bPLCStatus ? &cGREEN : &cRED);
            gtk_image_set_from_stock ( GTK_IMAGE(priv->pixmap_plc), pDatiLinea->bPLCStatus ? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
            trace_debug(TRUE, TRUE, "PLC : %d", pDatiLinea->bPLCStatus);
        }

        time(&lActualTime);
        count++;
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

/*
* Gestione colloquio con PLC
*/
void GestPlc(gpointer win)
{
    static int nCPB;
    static int nCB;
    static int nCS;
    static int first_time = 1;
    ep_bool_t bOK=TRUE;
    /*
    * Richiesta informazioni sulle variabili registrate per il DEBUG
    */
    if(!Cfg.nPlc){
        return;
    }
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if (first_time) {
        char szBuffer[128];

        sprintf(szBuffer,"CPB");  if((nCPB = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Variabile PLC %s assente !\n",szBuffer); bOK=FALSE;}
        sprintf(szBuffer,"CB" );  if((nCB  = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Variabile PLC %s assente !\n",szBuffer); bOK=FALSE;}
        sprintf(szBuffer,"CS" );  if((nCS  = plc_get_var_index(szBuffer))==-1){ trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Variabile PLC %s assente !\n",szBuffer); bOK=FALSE;}
        if (bOK) {
            /*
            * La prima volta azzero var di scambio PLC
            */
            plc_set_var_value(nCPB,FALSE);
            plc_set_var_value(nCB,FALSE);
            plc_set_var_value(nCS,FALSE);
        }
    }

    plc_refresh_vars();

    /* risposta a collo in bilancia */
    if(bColloInBilancia && (bColloBuono || bColloScarto)){
        if(bColloBuono){
            plc_set_var_value(nCB, bColloBuono);
            bColloBuono = FALSE;
            trace_debug(TRUE, TRUE, "PC -> PLC - COLLO BUONO");
        }
        if(bColloScarto){
            plc_set_var_value(nCS, bColloScarto);
            bColloScarto = FALSE;
            trace_debug(TRUE, TRUE, "PC -> PLC - COLLO SCARTATO");
        }
    }

    /*
    * Fronte di salita CPB (collo in bilancia)
    */
    if(plc_up_front(nCPB)){
        trace_debug_gtk("CYAN", TRUE, TRUE, priv->txt_msgs, "Fronte di salita CPB (collo in bilancia)");
        bColloInBilancia = TRUE;
        _timeout_collo_bilancia = g_timeout_add(10*1000,_timeout_collo_bilancia_cb, win);
    }
    /*
    * Fronte di discesa CPB (collo fuori bilancia)
    */
    if(plc_down_front(nCPB)) {
        trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Fronte di discesa CPB (collo in bilancia)");
        /*
        * Cancello il barcode ed il peso del collo per il collo successivo
        */
        // TODO: szBarcodeColloBilancia[0]='\0';
        // TODO: nPesoColloBilancia=0;
        bColloInBilancia = FALSE;

        /* risposta attiva */
        bDataFound=FALSE;

        szBarcodeColloBilancia[0]='\0';
        nPesoColloBilancia=0;

        /* 
        * resetto le variabili di scambio PLC su CPB=0
        */
        plc_set_var_value(nCB,FALSE);
        plc_set_var_value(nCS,FALSE);
        /* set barcode and weight labels color to orange to mark that the barcode is old */
        gtk_widget_set_foreground(priv->lb_barcode_bilancia,  &cORANGE);
        gtk_widget_set_foreground(priv->lb_peso_rea, &cORANGE);
    }
    plc_update_var_values(first_time);

    first_time = 0;
}
