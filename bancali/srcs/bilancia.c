#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "bilancia.h"
#include "mainfun.h"

#define WINDOW_NAME "bilancia-window"

struct _BilanciaWindow
{
    GtkDialog parent;
};

typedef struct _BilanciaWindowPrivate BilanciaWindowPrivate;

struct _BilanciaWindowPrivate
{
    GtkWidget *lb_ur_title;
    GtkWidget *lb_data_title;
    GtkWidget *lb_ur;
    GtkWidget *lb_data;
    GtkWidget *lb_pn_title;
    GtkWidget *lb_pn;
    GtkWidget *lb_tara_title;
    GtkWidget *lb_tara;
    GtkWidget *lb_um_title;
    GtkWidget *lb_um;
    GtkWidget *lb_0_0;
    GtkWidget *lb_0_1;
    GtkWidget *lb_0_2;
    GtkWidget *lb_0_3;
    GtkWidget *lb_1_0;
    GtkWidget *lb_1_1;
    GtkWidget *lb_1_2;
    GtkWidget *lb_1_3;
    GtkWidget *lb_2_0;
    GtkWidget *lb_2_1;
    GtkWidget *lb_2_2;
    GtkWidget *lb_2_3;
    GtkWidget *lb_3_0;
    GtkWidget *lb_3_1;
    GtkWidget *lb_3_2;
    GtkWidget *lb_3_3;
    GtkWidget *pixmap_0_0;
    GtkWidget *pixmap_0_1;
    GtkWidget *pixmap_0_2;
    GtkWidget *pixmap_0_3;
    GtkWidget *pixmap_2_0;
    GtkWidget *pixmap_2_1;
    GtkWidget *pixmap_2_2;
    GtkWidget *pixmap_2_3;
    GtkWidget *pixmap_1_0;
    GtkWidget *pixmap_1_1;
    GtkWidget *pixmap_1_2;
    GtkWidget *pixmap_1_3;
    GtkWidget *pixmap_3_0;
    GtkWidget *pixmap_3_1;
    GtkWidget *pixmap_3_2;
    GtkWidget *pixmap_3_3;
    GtkWidget *pb_close;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
};

G_DEFINE_TYPE_WITH_PRIVATE(BilanciaWindow, bilancia_window, GTK_TYPE_DIALOG)

/*
* gint ShowDatiBilancia(void);
* visualizza in dlg_bilancia le informazioni relative alla stringa 
* ricevuta da ev2002 (inviata 3 volte al secondo)
*/
static gint ShowDatiBilancia(gpointer win)
{
    BilanciaWindowPrivate *priv = bilancia_window_get_instance_private (win);
	int nBitStato=4;
	int nBit=4;
	int nIndexStato;
	int nIndexBit;

	char szPesonetto[9];
	char szTara[9];
	char szPixmap[20];
	static float fOldPeso=(float)-1;

	for (nIndexStato=0; nIndexStato<nBitStato; nIndexStato++) {
		for (nIndexBit=0; nIndexBit<nBit; nIndexBit++) {
			if(stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nValue!=stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nOldValue){
				stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nOldValue=stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nValue;
				sprintf(szPixmap,"pixmap_%1d_%1d", nIndexStato, nIndexBit);
				if (!(nIndexStato==1 && nIndexBit==0)){
                    /*
                    * TODO
                    */
                    gtk_image_set_from_icon_name (GTK_IMAGE(find_child(GTK_WIDGET(win), szPixmap)), stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nValue?"gtk-yes":"gtk-no", GTK_ICON_SIZE_BUTTON);
                    /*
                    */
				}
			}
		}
	}	
	
	/*
	* aggiorno i dati a video solo se il peso e' variato
	*/
	if (fOldPeso!=stringa_ev2002.fPesonetto){

		fOldPeso=stringa_ev2002.fPesonetto;
		sprintf(szPesonetto,"%6.3f",stringa_ev2002.fPesonetto);
		sprintf(szTara,"%6.3f",stringa_ev2002.fTara);
        gtk_label_set_text(GTK_LABEL(priv->lb_data),stringa_ev2002.szMessaggio);
		gtk_label_set_text(GTK_LABEL(priv->lb_ur)  ,stringa_ev2002.time);
		gtk_label_set_text(GTK_LABEL(priv->lb_pn)  ,szPesonetto);
		gtk_label_set_text(GTK_LABEL(priv->lb_tara),szTara);
		gtk_label_set_text(GTK_LABEL(priv->lb_um)  ,stringa_ev2002.szUnita_misura);
	}
	return(TRUE);
}


void on_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    g_printf("selection_changed\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}


static GActionEntry entries[] = {
  {"close",  close_activated,  NULL, NULL, NULL}
};

static void bilancia_window_init (BilanciaWindow *win)
{
    BilanciaWindowPrivate *priv = bilancia_window_get_instance_private (win);

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
}

static void bilancia_window_dispose (GObject *object)
{
    BilanciaWindow *win = BILANCIA_WINDOW (object);
    BilanciaWindowPrivate *priv = bilancia_window_get_instance_private (win);

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

    G_OBJECT_CLASS (bilancia_window_parent_class)->dispose (object);
}

static void bilancia_window_class_init (BilanciaWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = bilancia_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_ur_title  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_data_title);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_ur        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_data      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_pn_title  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_pn        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_tara_title);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_tara      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_um_title  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_um        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_0_0       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_0_1       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_0_2       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_0_3       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_1_0       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_1_1       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_1_2       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_1_3       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_2_0       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_2_1       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_2_2       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_2_3       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_3_0       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_3_1       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_3_2       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, lb_3_3       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_0_0   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_0_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_0_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_0_3   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_2_0   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_2_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_2_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_2_3   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_1_0   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_1_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_1_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_1_3   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_3_0   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_3_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_3_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pixmap_3_3   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BilanciaWindow, pb_close     );
}

BilanciaWindow * bilancia_window_new (MainWindow *win)
{
    return g_object_new (BILANCIA_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
