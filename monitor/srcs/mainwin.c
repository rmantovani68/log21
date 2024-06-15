#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>
#include <picking.h>

//#include <gimpwidgets.h>
#include <gimpwidgetstypes.h>
#include <gimpruler.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>
#include <msg-box.h>
#include <ep-about.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

void on_row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    g_printf("row-activated\n");
}

gboolean on_button_pressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
        g_print ("Single right click on the tree view.\n");

        /* optional: select row if no row is selected or only
        *  one other row is selected (will only do something
        *  if you set a tree selection mode as described later
        *  in the tutorial) */
        if (1) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

            /* Note: gtk_tree_selection_count_selected_rows() does not
            *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
            if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {
                GtkTreePath *path;

                /* Get tree path for row that was clicked */
                if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
                    gtk_tree_selection_unselect_all(selection);
                    gtk_tree_selection_select_path(selection, path);
                    gtk_tree_path_free(path);
                }
            }
        } /* end of optional bit */

        // TODO view_popup_menu(treeview, event, userdata);

        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;

}



static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){        
        trace_debug_gtk("RED",TRUE,TRUE,NULL,"Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        dlg_msg(GTK_WINDOW(win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Connection to database [%s:%s] failed.", Cfg.szDBHost, Cfg.szDBName);
    }

    return rc;
}

static void close_db_connection(void)
{
    DBDisconnect();
}

static void init_trace(MainWindow *win)
{
#ifdef TRACE
    MainWindowPrivate *priv = get_main_window_private_instance (win);


    char szBufTrace[80];

    strcpy(szBufTrace, Cfg.szPathTrace);
    strcat(szBufTrace,"/");
    strcat(szBufTrace, priv->pszAppName);
    strcat(szBufTrace, TRACE_FILE_SUFFIX);

    open_trace(priv->pszAppName, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Module Name     : %s",priv->pszAppName);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
    trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(TRUE, TRUE, "Started");
#endif
}

static void Exit(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    close_db_connection();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer)
        g_source_remove(Cfg.nMainTimer);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);


    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}

static void search_changed_callback (GtkSearchEntry *entry, gpointer win)
{
#ifdef TODO
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");
    char szParagraph[128];
    char szBuffer[128];
    char szData[128];
    int nRowIndex=0;
    int nRiga=-1;
    int nKeyIndex=-1;
    char *pszString;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    const gchar *pSearch = gtk_entry_get_text (GTK_ENTRY(entry));
    int n = strlen(pSearch);

    strcpy(szParagraph,"Tabella Ordini");
    strcpy(szBuffer,"lista_ordini");
    strcat(szBuffer,"_key");
    nKeyIndex=GetFileInt(szParagraph,szBuffer, 0, priv->pszCfgFileName,NULL); 


    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {

        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,nKeyIndex, &pszString);
        
        if(!g_ascii_strcasecmp (pszString, pSearch)){
            /* found */
            gtk_tree_view_set_cursor(TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, FALSE);
            gtk_tree_view_scroll_to_cell (TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, TRUE, 0.5, 0.0);
            break;
        }

        g_free(pszString);

    } while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst), &iter));

#endif
}

GtkWidget *create_evasione_oraria (gpointer win,GtkWidget *parent)
{
    GtkWidget *frame_data;
    GtkWidget *vbox;
    GtkWidget *vbox_data;
    GtkWidget *lb_data;
    GtkWidget *hbox;
    GtkWidget *vruler1;
    GtkWidget *hb_colli;
    GtkWidget *progressbar1;
    GtkWidget *lb_media;
    GtkWidget *vb_ora;
    GtkWidget *lb_ora;
    GtkWidget *lb_colli;
    DBresult *DBRes;
    DBresult *DBResData;
    int nTotGiorni=0;
    int nTotOre=0;
    int nTotColli=0;
    int nTotColliGiorno=0;
    int nMaxValue=-1;
    int nTuples;
    int nTuplesData;
    int nIndex;
    int nOra;
    int nOraIndex;
    char szBuffer[128];
    char szDataEvasione[128];

    vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name (vbox, "vbox");
    gtk_container_add (GTK_CONTAINER (parent), vbox);
    gtk_widget_show (vbox);


    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select data(cptmeva) as data from col_prod where cpstato in ('E','X','D','H') group by data(cptmeva);");
    if((nTuples=DBntuples(DBRes))){
        for(nIndex=0;nIndex<nTuples;nIndex++){
            strcpy(szDataEvasione,StrTrimAll(DBgetvalue(DBRes,nIndex,0)));
            if(strlen(szDataEvasione)){
                frame_data = gtk_frame_new (szDataEvasione);

                sprintf(szBuffer,"frame_data_%2d",nIndex);
                gtk_widget_set_name (frame_data, szBuffer);
                gtk_box_pack_start (GTK_BOX (vbox), frame_data, TRUE, TRUE, 0);
                gtk_widget_show (frame_data);

                vbox_data = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                sprintf(szBuffer,"vbox_data_%2d",nIndex);
                gtk_widget_set_name (vbox_data, szBuffer);
                gtk_container_add (GTK_CONTAINER (frame_data), vbox_data);
                gtk_widget_show (vbox_data);

                /* data */
                lb_data = gtk_label_new(DBgetvalue(DBRes,nIndex,0));
                sprintf(szBuffer,"lb_data_%2d",nIndex);
                gtk_widget_set_name (lb_data, szBuffer);
                gtk_box_pack_start (GTK_BOX (vbox_data), lb_data, FALSE, FALSE, 0);
                gtk_widget_show (lb_data);

                hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
                sprintf(szBuffer,"hbox_%2d",nIndex);
                gtk_widget_set_name (hbox, szBuffer);
                gtk_box_pack_start (GTK_BOX (vbox_data), hbox, TRUE, TRUE, 0);
                gtk_widget_show (hbox);

                vruler1 = gimp_ruler_new (GTK_ORIENTATION_VERTICAL);
                gimp_ruler_set_range((GimpRuler *)vruler1, 0.0 , 10.0, 10.0);
                gtk_widget_set_name (vruler1, "vruler1");
                gtk_box_pack_start (GTK_BOX (hbox), vruler1, FALSE, TRUE, 0);
                gtk_widget_show (vruler1);

                /* evasioni relative al giorno in esame */
                strcpy(szDataEvasione,StrTrimAll(DBgetvalue(DBRes,nIndex,0)));
                if(strlen(szDataEvasione)){
                    DBResData=DBExecQuery(Cfg.nDebugLevel>1,"select data(cptmeva) as data,substring(ora(cptmeva),1,2) as ora,count(ordprog) as colli,sum(cpnmrgh) as righe,sum(cpnmcpe) as copie from col_prod where cpstato in ('E','X','D','H') and data(cptmeva)='%s' group by data(cptmeva),substring(ora(cptmeva),1,2);",szDataEvasione);
                    if((nTuplesData=DBntuples(DBResData))){
                        hb_colli = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
                        gtk_widget_set_name (hb_colli, "hb_colli");
                        gtk_box_pack_start (GTK_BOX (hbox), hb_colli, TRUE, TRUE, 0);
                        gtk_widget_show (hb_colli);

                        nMaxValue=0;
                        /* calcolo valore max e media giornaliera */
                        for(nOra=0;nOra<nTuplesData;nOra++){
                            nMaxValue=max(nMaxValue,atoi(DBgetvalue(DBResData,nOra,2)));
                            nTotOre++;
                            nTotColliGiorno+=atoi(DBgetvalue(DBResData,nOra,2));
                            nTotColli+=atoi(DBgetvalue(DBResData,nOra,2));
                        }
                        /*
                        * Lascio un 10% vuoto in cima
                        */
                        nMaxValue+=nMaxValue/10;

                        for(nOraIndex=Cfg.nOraInizio;nOraIndex<=Cfg.nOraFine;nOraIndex++){
                            vb_ora = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
                            sprintf(szBuffer,"vb_ora_%2d",nIndex);
                            gtk_widget_set_name (vb_ora, szBuffer);
                            gtk_box_pack_start (GTK_BOX (hb_colli), vb_ora, TRUE, TRUE, 0);
                            gtk_widget_show (vb_ora);

                            /* colli */
                            lb_colli = gtk_label_new ("");
                            sprintf(szBuffer,"lb_colli_%2d",nOraIndex);
                            gtk_widget_set_name (lb_colli, szBuffer);
                            gtk_box_pack_start (GTK_BOX (vb_ora), lb_colli, FALSE, FALSE, 0);
                            gtk_widget_show (lb_colli);

                            /* grafico */
                            progressbar1 = gtk_progress_bar_new ();
                            gtk_orientable_set_orientation (GTK_ORIENTABLE(progressbar1),  GTK_ORIENTATION_VERTICAL);
                            sprintf(szBuffer,"pr_%2d",nOraIndex);
                            gtk_widget_set_name (progressbar1, szBuffer);
                            gtk_box_pack_start (GTK_BOX (vb_ora), progressbar1, TRUE, TRUE, 0);
                            gtk_widget_show (progressbar1);
                            for(nOra=0;nOra<nTuplesData;nOra++){
                                if(atoi(DBgetvalue(DBResData,nOra,1))==nOraIndex){
                                    // gtk_progress_set_fractyion(GTK_PROGRESS(progressbar1),(gfloat)atoi(DBgetvalue(DBResData,nOra,2)));
                                    gtk_label_printf(lb_colli,"%4d",atoi(DBgetvalue(DBResData,nOra,2)));
                                }
                            }
                            /* ora */
                            sprintf(szBuffer,"%02d",nOraIndex);
                            lb_ora = gtk_label_new (szBuffer);
                            sprintf(szBuffer,"lb_ora_%2d",nOraIndex);
                            gtk_widget_set_name (lb_ora, szBuffer);
                            gtk_box_pack_start (GTK_BOX (vb_ora), lb_ora, FALSE, FALSE, 0);
                            gtk_widget_show (lb_ora);
                        }
                    }
                    DBclear(DBResData);
                }


                lb_media = gtk_label_new ("");
                gtk_widget_set_name (lb_media, "lb_media");
                gtk_box_pack_start (GTK_BOX (vbox_data), lb_media, FALSE, FALSE, 0);
                gtk_widget_show (lb_media);
                gtk_label_printf(lb_media,"Media : %7.2f Colli/Ora",(float)((float)nTotColliGiorno/(float)nTotOre));

                if(nMaxValue>10000){
                    nMaxValue/=1000;
                } else if(nMaxValue>1000){
                    nMaxValue/=100;
                } else if(nMaxValue>100){
                    nMaxValue/=10;
                }
                gimp_ruler_set_range((GimpRuler *)vruler1, 0.0 , (gfloat)nMaxValue, 10.0);

                nTotGiorni++;
            }
        }
    }
    DBclear(DBRes);


  return vbox;
}

void create_table_settori(gpointer win,GtkWidget *parent,int nRows,int nCols)
{
    GtkWidget *table_settori;
    GtkWidget *frame_settore;
    GtkWidget *vbox_settore;
    GtkWidget *hbox155;
    GtkWidget *vbox339;
    GtkWidget *lb_operatore;
    GtkWidget *lb_ordine;
    GtkWidget *lb_prodotto;
    GtkWidget *frame_display;
    GtkWidget *vbox_display;
    GtkWidget *lb_riga_1;
    GtkWidget *lb_riga_2;
    GtkWidget *hsep_settore;
    GtkWidget *table_settore;
    GtkWidget *lb_null;
    GtkWidget *lb_copie_prelevate;
    GtkWidget *lb_righe_prelevate;
    GtkWidget *lb_righe_da_prelevare;
    GtkWidget *lb_copie_da_prelevare;
    GtkWidget *lb_copie_evase;
    GtkWidget *lb_righe_evase;
    GtkWidget *frame235;
    GtkWidget *lb_titolo_righe;
    GtkWidget *frame236;
    GtkWidget *lb_titolo_copie;
    GtkWidget *frame237;
    GtkWidget *lb_titolo_prelevato;
    GtkWidget *frame238;
    GtkWidget *lb_titolo_da_prelevare;
    GtkWidget *frame239;
    GtkWidget *lb_titolo_evaso;
    char szBuffer[128];
    int nCol,nRow,nSettore;

    // table_settori = gtk_table_new (nRows,nCols, FALSE);
    table_settori = gtk_grid_new ();
    gtk_widget_set_name (table_settori, "table_settori");
    gtk_container_add (GTK_CONTAINER (parent), table_settori);
    gtk_widget_show (table_settori);

    nSettore=0;
    for(nRow=0;nRow<nRows;nRow++){
        for(nCol=0;nCol<nCols;nCol++){
            sprintf(szBuffer,"Settore %d",nSettore+1);
            frame_settore = gtk_frame_new (szBuffer);
            sprintf(szBuffer,"frame_settore_%d",nSettore+1);
            gtk_widget_set_name (frame_settore, szBuffer);
            //gtk_table_attach (GTK_TABLE (table_settori), frame_settore, nCol, nCol+1, nRow, nRow+1,
            //                                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
            //                                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
            gtk_grid_attach(GTK_GRID (table_settori), frame_settore, nCol, nRow, 1,1);
            gtk_widget_show (frame_settore);
            gtk_container_set_border_width (GTK_CONTAINER (frame_settore), 5);

            vbox_settore = gtk_vbox_new (FALSE, 0);
            gtk_widget_set_name (vbox_settore, "vbox_settore");
            gtk_container_add (GTK_CONTAINER (frame_settore), vbox_settore);
            gtk_widget_show (vbox_settore);

            hbox155 = gtk_hbox_new (FALSE, 0);
            gtk_widget_set_name (hbox155, "hbox155");
            gtk_box_pack_start (GTK_BOX (vbox_settore), hbox155, TRUE, TRUE, 0);
            gtk_widget_show (hbox155);

            vbox339 = gtk_vbox_new (FALSE, 0);
            gtk_widget_set_name (vbox339, "vbox339");
            // gtk_widget_set_usize (vbox339, 150, -2);
            gtk_box_pack_start (GTK_BOX (hbox155), vbox339, TRUE, TRUE, 0);
            gtk_widget_show (vbox339);

            lb_operatore = gtk_label_new ("operatore");
            gtk_widget_set_name (lb_operatore, "lb_operatore");
            gtk_box_pack_start (GTK_BOX (vbox339), lb_operatore, TRUE, TRUE, 0);
            gtk_widget_show (lb_operatore);
            gtk_misc_set_alignment (GTK_MISC (lb_operatore), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_operatore), 5, 0);

            lb_ordine = gtk_label_new ("ordine");
            gtk_widget_set_name (lb_ordine, "lb_ordine");
            gtk_box_pack_start (GTK_BOX (vbox339), lb_ordine, TRUE, TRUE, 0);
            gtk_widget_show (lb_ordine);
            gtk_misc_set_alignment (GTK_MISC (lb_ordine), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_ordine), 5, 0);

            lb_prodotto = gtk_label_new ("prodotto");
            gtk_widget_set_name (lb_prodotto, "lb_prodotto");
            gtk_box_pack_start (GTK_BOX (vbox339), lb_prodotto, TRUE, TRUE, 0);
            gtk_widget_show (lb_prodotto);
            gtk_misc_set_alignment (GTK_MISC (lb_prodotto), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_prodotto), 5, 0);

            frame_display = gtk_frame_new (NULL);
            gtk_widget_set_name (frame_display, "frame_display");
            gtk_box_pack_start (GTK_BOX (hbox155), frame_display, FALSE, TRUE, 0);
            gtk_widget_show (frame_display);

            vbox_display = gtk_vbox_new (FALSE, 0);
            gtk_widget_set_name (vbox_display, "vbox_display");
            gtk_container_add (GTK_CONTAINER (frame_display), vbox_display);
            gtk_widget_show (vbox_display);

            lb_riga_1 = gtk_label_new ("");
            gtk_widget_set_name (lb_riga_1, "lb_riga_1");
            // gtk_widget_set_usize (lb_riga_1, 100, -2);
            gtk_box_pack_start (GTK_BOX (vbox_display), lb_riga_1, TRUE, TRUE, 0);
            gtk_widget_show (lb_riga_1);

            lb_riga_2 = gtk_label_new ("");
            gtk_widget_set_name (lb_riga_2, "lb_riga_2");
            // gtk_widget_set_usize (lb_riga_2, 100, -2);
            gtk_box_pack_start (GTK_BOX (vbox_display), lb_riga_2, TRUE, TRUE, 0);
            gtk_widget_show (lb_riga_2);

            hsep_settore = gtk_hseparator_new ();
            gtk_widget_set_name (hsep_settore, "hsep_settore");
            gtk_box_pack_start (GTK_BOX (vbox_settore), hsep_settore, FALSE, FALSE, 0);
            gtk_widget_show (hsep_settore);

            //table_settore = gtk_table_new (4, 3, FALSE);
            table_settore = gtk_grid_new ();
            gtk_widget_set_name (table_settore, "table_settore");
            gtk_box_pack_start (GTK_BOX (vbox_settore), table_settore, TRUE, TRUE, 0);
            gtk_widget_show (table_settore);
            gtk_container_set_border_width (GTK_CONTAINER (table_settore), 5);
            //gtk_table_set_row_spacings (GTK_TABLE (table_settore), 5);
            //gtk_table_set_col_spacings (GTK_TABLE (table_settore), 5);

            lb_null = gtk_label_new ("");
            gtk_widget_set_name (lb_null, "lb_null");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_null, 0, 1, 0, 1,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_null, 0, 0,1,1);
            gtk_widget_show (lb_null);
            gtk_misc_set_alignment (GTK_MISC (lb_null), 0, 0.5);

            lb_copie_prelevate = gtk_label_new ("");
            gtk_widget_set_name (lb_copie_prelevate, "lb_copie_prelevate");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_copie_prelevate, 2, 3, 1, 2,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_copie_prelevate, 2, 1, 1,1);
            gtk_widget_show (lb_copie_prelevate);
            gtk_misc_set_alignment (GTK_MISC (lb_copie_prelevate), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_copie_prelevate), 5, 0);

            lb_righe_prelevate = gtk_label_new ("");
            gtk_widget_set_name (lb_righe_prelevate, "lb_righe_prelevate");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_righe_prelevate, 1, 2, 1, 2,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_righe_prelevate, 1, 1, 1,1);
            gtk_widget_show (lb_righe_prelevate);
            gtk_misc_set_alignment (GTK_MISC (lb_righe_prelevate), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_righe_prelevate), 5, 0);

            lb_righe_da_prelevare = gtk_label_new ("");
            gtk_widget_set_name (lb_righe_da_prelevare, "lb_righe_da_prelevare");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_righe_da_prelevare, 1, 2, 2, 3,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_righe_da_prelevare, 1, 2, 1,1);
            gtk_widget_show (lb_righe_da_prelevare);
            gtk_misc_set_alignment (GTK_MISC (lb_righe_da_prelevare), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_righe_da_prelevare), 5, 0);

            lb_copie_da_prelevare = gtk_label_new ("");
            gtk_widget_set_name (lb_copie_da_prelevare, "lb_copie_da_prelevare");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_copie_da_prelevare, 2, 3, 2, 3,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_copie_da_prelevare, 2, 2, 1,1);
            gtk_widget_show (lb_copie_da_prelevare);
            gtk_misc_set_alignment (GTK_MISC (lb_copie_da_prelevare), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_copie_da_prelevare), 5, 0);

            lb_copie_evase = gtk_label_new ("");
            gtk_widget_set_name (lb_copie_evase, "lb_copie_evase");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_copie_evase, 2, 3, 3, 4,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_copie_evase, 2, 3, 1,1);
            gtk_widget_show (lb_copie_evase);
            gtk_misc_set_alignment (GTK_MISC (lb_copie_evase), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_copie_evase), 5, 0);

            lb_righe_evase = gtk_label_new ("");
            gtk_widget_set_name (lb_righe_evase, "lb_righe_evase");
            // gtk_table_attach (GTK_TABLE (table_settore), lb_righe_evase, 1, 2, 3, 4,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (0), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), lb_righe_evase, 1, 3, 1,1);
            gtk_widget_show (lb_righe_evase);
            gtk_misc_set_alignment (GTK_MISC (lb_righe_evase), 1, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_righe_evase), 5, 0);

            frame235 = gtk_frame_new (NULL);
            gtk_widget_set_name (frame235, "frame235");
            // gtk_table_attach (GTK_TABLE (table_settore), frame235, 1, 2, 0, 1,
            //                                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
            //                                     (GtkAttachOptions) (GTK_FILL), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), frame235, 1, 0, 1,1);
            gtk_widget_show (frame235);

            lb_titolo_righe = gtk_label_new ("Righe");
            gtk_widget_set_name (lb_titolo_righe, "lb_titolo_righe");
            gtk_container_add (GTK_CONTAINER (frame235), lb_titolo_righe);
            gtk_widget_show (lb_titolo_righe);
            gtk_misc_set_alignment (GTK_MISC (lb_titolo_righe), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_titolo_righe), 5, 0);

            frame236 = gtk_frame_new (NULL);
            gtk_widget_set_name (frame236, "frame236");
            // gtk_table_attach (GTK_TABLE (table_settore), frame236, 2, 3, 0, 1,
            //                                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
            //                                     (GtkAttachOptions) (GTK_FILL), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), frame236, 2, 0, 1,1);
            gtk_widget_show (frame236);

            lb_titolo_copie = gtk_label_new ("Copie");
            gtk_widget_set_name (lb_titolo_copie, "lb_titolo_copie");
            gtk_container_add (GTK_CONTAINER (frame236), lb_titolo_copie);
            gtk_widget_show (lb_titolo_copie);
            gtk_misc_set_alignment (GTK_MISC (lb_titolo_copie), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_titolo_copie), 5, 0);

            frame237 = gtk_frame_new (NULL);
            gtk_widget_set_name (frame237, "frame237");
            // gtk_table_attach (GTK_TABLE (table_settore), frame237, 0, 1, 1, 2,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (GTK_FILL), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), frame237, 0, 1, 1,1);
            gtk_widget_show (frame237);

            lb_titolo_prelevato = gtk_label_new ("Prelevato");
            gtk_widget_set_name (lb_titolo_prelevato, "lb_titolo_prelevato");
            gtk_container_add (GTK_CONTAINER (frame237), lb_titolo_prelevato);
            gtk_widget_show (lb_titolo_prelevato);
            gtk_misc_set_alignment (GTK_MISC (lb_titolo_prelevato), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_titolo_prelevato), 5, 0);

            frame238 = gtk_frame_new (NULL);
            gtk_widget_set_name (frame238, "frame238");
            // gtk_table_attach (GTK_TABLE (table_settore), frame238, 0, 1, 2, 3,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (GTK_FILL), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), frame238, 0, 2, 1,1);
            gtk_widget_show (frame238);

            lb_titolo_da_prelevare = gtk_label_new ("Da Prelevare");
            gtk_widget_set_name (lb_titolo_da_prelevare, "lb_titolo_da_prelevare");
            gtk_container_add (GTK_CONTAINER (frame238), lb_titolo_da_prelevare);
            gtk_widget_show (lb_titolo_da_prelevare);
            gtk_misc_set_alignment (GTK_MISC (lb_titolo_da_prelevare), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_titolo_da_prelevare), 5, 0);

            frame239 = gtk_frame_new (NULL);
            gtk_widget_set_name (frame239, "frame239");
            // gtk_table_attach (GTK_TABLE (table_settore), frame239, 0, 1, 3, 4,
            //                                     (GtkAttachOptions) (GTK_FILL),
            //                                     (GtkAttachOptions) (GTK_FILL), 0, 0);
            gtk_grid_attach (GTK_GRID (table_settore), frame239, 0, 3, 1,1);
            gtk_widget_show (frame239);

            lb_titolo_evaso = gtk_label_new ("Evaso");
            gtk_widget_set_name (lb_titolo_evaso, "lb_titolo_evaso");
            gtk_container_add (GTK_CONTAINER (frame239), lb_titolo_evaso);
            gtk_widget_show (lb_titolo_evaso);
            gtk_misc_set_alignment (GTK_MISC (lb_titolo_evaso), 0, 0.5);
            gtk_misc_set_padding (GTK_MISC (lb_titolo_evaso), 5, 0);
            nSettore++;
        }
    }
}


static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    /* read configuration from cfg file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * Verifico la presenza di una istanza attiva del processo MAIN
    * controllando la presenza della coda MSG
    */
    if( TRUE || OpenProcessMsgQ(priv->nPID)<0){
        /*
        * coda messaggi non presente - tutto ok
        * creo la coda locale per l'applicazione
        */
        if(CreateProcessMsgQ(priv->nPID, 1)<0){
            trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        }

        gchar *pszTitle=g_strdup_printf("%s %s-%s - %s %s", __application_name__, __version__, __customer__, priv->pszAppTitle, rcsid);
        gtk_window_set_title (GTK_WINDOW (win), pszTitle);
        g_free(pszTitle);


        /* Database */
        if(init_db_connection(win)){
            /*
            * Apertura del canale di comunicazione con il PLC server (QPLC)
            */
            if(Cfg.nPLC){
                if(!(plc_connect(Cfg.szPLCHost, Cfg.nPLCService))){
                    dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in PLC connection");
                } 
            }

            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

            gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
            g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);


        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
        gtk_widget_destroy (GTK_WIDGET (win));
        g_application_quit (G_APPLICATION (_app));
    }
}

static void search_activated    (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gboolean mode;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
}

static void setup_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{

}

static void about_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;
    
    w = about_window_new (GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);

    gtk_window_present (GTK_WINDOW (w));
}


static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    int rc=dlg_msg( GTK_WINDOW(win), "Exit", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Uscire dall'applicazione ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            Exit(win);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        break;
    }

}


static void settori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int nRows,nColumns;

    for(nColumns=4;nColumns>=1;nColumns--){
        if(Cfg.nNumeroSettori%nColumns==0){
            nRows=Cfg.nNumeroSettori/nColumns;
            break;
        }
    }
    GtkWidget *w = gtk_viewport_new (NULL, NULL);
    gtk_widget_set_name(w, "viewport");
    create_table_settori(win,w,nRows,nColumns);

    GtkWidget *dlg = dlg_msg_with_child(GTK_WINDOW(win), "Carico settori", GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, w, FALSE, "Carico settori");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_CLOSE:
        break;
    }
    gtk_widget_destroy(dlg);
}

static void statistiche_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w = gtk_viewport_new (NULL, NULL);
    gtk_widget_set_name(w, "viewport");
    create_evasione_oraria(win, w);

    GtkWidget *dlg = dlg_msg_with_child(GTK_WINDOW(win), "Statistiche di evasione", GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, w, FALSE, "Statistiche di evasione");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_CLOSE:
        break;
    }
    gtk_widget_destroy(dlg);
}

static void operatori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(sw, "sw");

    char szSelectCmd[4096];

    GetFileString("operatori","operatori", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName, NULL); 

    GList *PS=NULL;

    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);

    RefreshTable(sw, "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

    GtkWidget *dlg = dlg_msg_with_child( GTK_WINDOW(win), "Operatori", GTK_MESSAGE_QUESTION,GTK_BUTTONS_CLOSE, sw, TRUE, "Operatori");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_CLOSE:
        break;
    }
    gtk_widget_destroy(dlg);
}

static void imballi_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(sw, "sw");

    char szSelectCmd[4096];

    GetFileString("imballi","imballi", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName, NULL); 

    GList *PS=NULL;

    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);

    RefreshTable(sw, "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

    GtkWidget *dlg = dlg_msg_with_child( GTK_WINDOW(win), "Imballi", GTK_MESSAGE_QUESTION,GTK_BUTTONS_CLOSE, sw, TRUE, "Imballi");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_CLOSE:
        break;
    }
    gtk_widget_destroy(dlg);
}





static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}


static GActionEntry entries[] =
{
    { "search",     search_activated,     NULL, NULL, NULL },
    { "setup",      setup_activated,      NULL, NULL, NULL },
    { "about",      about_activated,      NULL, NULL, NULL },
    { "quit",       quit_activated,       NULL, NULL, NULL },
    { "settori",    settori_activated,    NULL, NULL, NULL },
    { "statistiche",statistiche_activated,NULL, NULL, NULL },
    { "operatori",  operatori_activated,  NULL, NULL, NULL },
    { "imballi",    imballi_activated,    NULL, NULL, NULL } 
};

MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW (win));
}


static void main_window_init (MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    priv->pszAppName       = g_strdup (APP_NAME);
    priv->pszAppTitle      = g_strdup (APP_TITLE);
    priv->pszAppClass      = g_strdup (APP_CLASS);
    priv->nPID             = APP_PID;
    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->screen = gdk_screen_get_default ();
    priv->provider = gtk_css_provider_new ();

    gchar *css_filename = g_strdup_printf("/org/%s/%s/%s.css", priv->pszAppClass, priv->pszAppName, priv->pszAppName);
    gtk_css_provider_load_from_resource (priv->provider, css_filename);
    g_free(css_filename);

    gtk_style_context_add_provider_for_screen (priv->screen, GTK_STYLE_PROVIDER(priv->provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);


    init_app(win);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void main_window_dispose (GObject *object)
{
    MainWindowPrivate *priv = get_main_window_private_instance (object);
    MainWindow *win = MAIN_WINDOW (object);

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

    if(priv->pszAppTitle){       g_free(priv->pszAppTitle);      priv->pszAppTitle = NULL;    }
    if(priv->pszAppName){        g_free(priv->pszAppName);       priv->pszAppName = NULL;    }
    if(priv->pszAppClass){       g_free(priv->pszAppClass);      priv->pszAppClass = NULL;    }
    if(priv->pszCfgFileName){    g_free(priv->pszCfgFileName);   priv->pszCfgFileName = NULL;    }

    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("main_window_class_init\n");

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_numero_operatori   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_media_colli        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_media_righe        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_media_copie        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_media_scarti       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_media_operatori    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_settori            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_statistica_evasione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_operatori          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_imballi            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_config             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_exit               );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, search_entry);
}                                                                                                             
                                                                                                              
MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    g_printf("main_window_new\n");
    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    priv->app = app;

    return win;
}
