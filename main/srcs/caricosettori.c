#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "caricosettori.h"
#include "mainfun.h"

#define WINDOW_NAME "carico-settori-window"

// workaround perchè nella funzione do_carico_settori uso il puntatore alla main_window
static MainWindow *main_window;

struct _CaricoSettoriWindow
{
    GtkDialog parent;
};

typedef struct _CaricoSettoriWindowPrivate CaricoSettoriWindowPrivate;

struct _CaricoSettoriWindowPrivate
{
    MainWindow *main_window;
    GtkWidget *frm_settori;
};

G_DEFINE_TYPE_WITH_PRIVATE(CaricoSettoriWindow, carico_settori, GTK_TYPE_DIALOG)

/*
* do_carico_settori()
* costruisce la finestra di carico (copie prelevate) diviso per settori
*/
void do_carico_settori(gpointer win)
{
	DBresult *DBRes;
	DBresult *DBResUbi;
	DBresult *DBResInsert;
	DBresult *DBResUpdate;
	int nSettore[20];
	GtkWidget *lb_widget;
	GtkWidget *level_widget;
	GtkWidget *rl_settori;
	char szSettore[128];
	int nMaxValue=0;
	int nIndex;
	int nSettoreIndex;
	int nTuples;
    CaricoSettoriWindowPrivate *priv;

    priv = carico_settori_get_instance_private (CARICO_SETTORI_WINDOW (win));

	/* 
	* Gestisce anche il settore 0 (Manuale)
	*/
	for(nSettoreIndex=0;nSettoreIndex<Cfg.nNumeroSettori+1;nSettoreIndex++){
		nSettore[nSettoreIndex]=0;

		sprintf(szSettore,"level_settore_%02d",nSettoreIndex);
		if((level_widget=find_child(GTK_WIDGET(win),szSettore))){
			gtk_level_bar_set_value (GTK_LEVEL_BAR(level_widget), (gdouble)nSettore[nSettoreIndex]);
		}

		sprintf(szSettore,"lb_settore_%02d",nSettoreIndex);
		if((lb_widget=find_child(GTK_WIDGET(win),szSettore))){
			gtk_label_printf(lb_widget,"%5d",nSettore[nSettoreIndex]);
		}
	}

	/*
	* seleziono gli ordini ancora da terminare (' '/S/R/P)
	*/

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where rostato in ('%c','%c','%c','%c','%c') and ordtipo='%s';",
		Cfg.szTmpSuffix,
		Cfg.szTipoOrdini,
		ORDINE_IN_PRELIEVO,
		ORDINE_STAMPATA_RAC,
		ORDINE_ELABORATO,
		ORDINE_SPEDITO,
		ORDINE_RICEVUTO,
		Cfg.szTipoOrdini);

	DBclear(DBRes);
	/* 
	* Cancellazione e creazione tabella sel_art_tmp_[SUFFIX]_[TIPO_ORDINI] 
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_art_tmp_%s_%s ( SACDPRO text, SACDUBI text, SANMRGH integer, SANMCPE integer);", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.racdpro,count(r.racdpro),sum(r.raqtord) from ric_art r,sel_ord_tmp_%s_%s s where r.ordprog=s.ordprog group by r.racdpro;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nTuples=DBntuples(DBRes);
	for(nIndex=0;nIndex<nTuples;nIndex++){
		DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_art_tmp_%s_%s (sacdpro,sanmrgh,sanmcpe) values ('%s',%d,%d);",
			Cfg.szTmpSuffix,
			Cfg.szTipoOrdini,
			DBgetvalue(DBRes,nIndex,0), 
			atoi(DBgetvalue(DBRes,nIndex,1)), 
			atoi(DBgetvalue(DBRes,nIndex,2)));
		DBclear(DBResInsert);

		DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione from ubicazioni where ubitipo = '%s' and codprod='%s' order by priorita;", Cfg.szTipoOrdini,DBgetvalue(DBRes,nIndex,0));
		if(DBntuples(DBResUbi)){
			/*
			* Prendo la prima ubicazione in ordine di priorita
			*/
			DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update sel_art_tmp_%s_%s set sacdubi='%s' where sacdpro='%s';",
				Cfg.szTmpSuffix,
				Cfg.szTipoOrdini,
				DBgetvalue(DBResUbi,0,0), 
				DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResUpdate);
		} else {
			gtk_text_printf("RED",find_child(GTK_WIDGET(main_window), "txt_msgs"),"Prodotto [%s] non ubicato !\n", DBgetvalue(DBRes,nIndex,0));
		}
		DBclear(DBResUbi);
	}

	DBclear(DBRes);

	/*
	* Seleziono le qta per settore
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select u.ubnmset,sum(s.sanmcpe) from ubicazioni as u,sel_art_tmp_%s_%s as s where u.ubicazione=s.sacdubi group by u.settore order by u.settore;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	if((nTuples=DBntuples(DBRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			nSettoreIndex=max(atoi(DBgetvalue(DBRes,nIndex,0)),0);

			nSettore[nSettoreIndex]+=atoi(DBgetvalue(DBRes,nIndex,1));
			nMaxValue=max(nMaxValue,nSettore[nSettoreIndex]);
		}

		/*
		* Lascio un 10% vuoto in cima
		*/
		nMaxValue+=nMaxValue/10;

		for(nIndex=0;nIndex<nTuples;nIndex++){
			nSettoreIndex=atoi(DBgetvalue(DBRes,nIndex,0));

			sprintf(szSettore,"lb_settore_%02d",nSettoreIndex);
			if((lb_widget=find_child(GTK_WIDGET(win),szSettore))){ 
				gtk_label_printf(lb_widget,"%d",nSettore[nSettoreIndex]); 
			}

			sprintf(szSettore,"level_settore_%02d",nSettoreIndex);
			if((level_widget=find_child(GTK_WIDGET(win),szSettore))){
				gtk_level_bar_set_max_value (GTK_LEVEL_BAR(level_widget),(gdouble)nMaxValue);
				gtk_level_bar_set_value(GTK_LEVEL_BAR(level_widget),(gdouble)nSettore[nSettoreIndex]);
			}
		}
	}

	DBclear(DBRes);
}


void create_carico_settori (gpointer win, GtkWidget *container)
{
    GtkWidget *grid;
    GtkWidget *rl_settori;
    GtkWidget *level_settore;
    GtkWidget *lb;
    GtkWidget *hsep;
    GtkWidget *vbox;
    GtkWidget *hbox;
	int nIndex;
	char szBuffer[128];

	/*
	grid 3 Righe N Colonne
		1 PR_SETTORE_XX
		2 H SEPARATOR
		3 VBOX 3 Righe
			LB_SETTORE_XX
			H SEP
			LB_SETTORE_XX_TITLE
	*/
		

	/*
	* Colonne : Numero Settori + 1 (ruler) + 1 (manuali)
	*/
    // grid = gtk_grid_new (3, Cfg.nNumeroSettori+2, FALSE);
    /*
    grid = gtk_grid_new ();
    gtk_widget_set_name (grid, "grid");
    gtk_grid_set_column_homogeneous (GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous    (GTK_GRID(grid), TRUE);
    g_object_ref (G_OBJECT(grid));

    gtk_widget_show (grid);
    gtk_container_add (GTK_CONTAINER (container), grid);
    */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    sprintf(szBuffer,"hb_settori");
    gtk_widget_set_name (hbox, szBuffer);
    gtk_box_set_homogeneous (GTK_BOX(hbox), TRUE);
    g_object_ref (hbox);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (container), hbox);

	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){

		/* V Box */
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
		sprintf(szBuffer,"vb_settore_%02d",nIndex);
		gtk_widget_set_name (vbox, szBuffer);
		g_object_ref (vbox);
		gtk_widget_show (vbox);
		// gtk_grid_attach (GTK_GRID (grid), vbox, nIndex, 2, 1, 1);
		gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
		
		/* Level Bar */
		level_settore = gtk_level_bar_new ();
		sprintf(szBuffer,"level_settore_%02d",nIndex);
		gtk_widget_set_name (level_settore, szBuffer);
        gtk_level_bar_set_inverted(GTK_LEVEL_BAR(level_settore), TRUE);
        gtk_orientable_set_orientation (GTK_ORIENTABLE(level_settore), GTK_ORIENTATION_VERTICAL);

		g_object_ref (G_OBJECT(level_settore));

		gtk_widget_show (level_settore);
		// gtk_grid_attach (GTK_GRID (grid), level_settore, nIndex, 0, 1, 1);
		gtk_box_pack_start (GTK_BOX (vbox), level_settore, TRUE, TRUE, 0);

		g_object_unref (G_OBJECT(level_settore));

		/* H Separator */
		hsep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		sprintf(szBuffer,"hsep_sett_%02d",nIndex);
		gtk_widget_set_name (hsep, szBuffer);
		g_object_ref (hsep);
		gtk_widget_show (hsep);
		// gtk_grid_attach (GTK_GRID (grid), hsep, nIndex, 1, 1, 1);
		gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
		g_object_unref (hsep);


		/* Label */
		lb = gtk_label_new ("");
		sprintf(szBuffer,"lb_settore_%02d",nIndex);
		gtk_widget_set_name (lb, szBuffer);
		g_object_ref (lb);
		gtk_widget_show (lb);
		gtk_box_pack_start (GTK_BOX (vbox), lb, FALSE, FALSE, 0);
		g_object_unref (lb);

		/* H Separator */
		hsep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		sprintf(szBuffer,"hsep_settore_%02d",nIndex);
		gtk_widget_set_name (hsep, szBuffer);
		g_object_ref (hsep);
		gtk_widget_show (hsep);
		gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
		g_object_unref (hsep);

		/* Label Title */
		if(nIndex){
			sprintf(szBuffer,"S.%2d",nIndex);
		} else {
			sprintf(szBuffer,"MANUALE");
		}
		lb = gtk_label_new (szBuffer);
		sprintf(szBuffer,"lb_settore_%02d_title",nIndex);
		gtk_widget_set_name (lb, szBuffer);
		g_object_ref (lb);
		gtk_widget_show (lb);
		gtk_box_pack_start (GTK_BOX (vbox), lb, FALSE, FALSE, 0);
		g_object_unref (lb);
		g_object_unref (vbox);
	}
    g_object_unref (hbox);
}



static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Ok Action : activated\n");
}

static GActionEntry entries[] = {
  {"ok", ok_activated, NULL, NULL, NULL},
  {"close", close_activated, NULL, NULL, NULL}
};

static void carico_settori_init (CaricoSettoriWindow *win)
{
    CaricoSettoriWindowPrivate *priv;

    priv = carico_settori_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

	create_carico_settori (win, priv->frm_settori);

	do_carico_settori(win);
    
    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void carico_settori_dispose (GObject *object)
{
    CaricoSettoriWindowPrivate *priv;

    priv = carico_settori_get_instance_private (CARICO_SETTORI_WINDOW (object));

    G_OBJECT_CLASS (carico_settori_parent_class)->dispose (object);
}

static void carico_settori_class_init (CaricoSettoriWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = carico_settori_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CaricoSettoriWindow, frm_settori );

}

CaricoSettoriWindow * carico_settori_new (MainWindow *win)
{
    /* valorizzo la variabile prima della new ... */
    main_window = win;

    CaricoSettoriWindow *w = g_object_new (CARICO_SETTORI_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    CaricoSettoriWindowPrivate *priv = carico_settori_get_instance_private (CARICO_SETTORI_WINDOW (w));

    priv->main_window = win;

    return w;
}
