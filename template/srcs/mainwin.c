#include <gtk/gtk.h>
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
#include <msg-box.h>
#include <ep-about.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"
#include "template_source.h"

GtkApplication *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);


static void about_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w = about_window_new (GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);

    gtk_window_present (GTK_WINDOW (w));
}

static void template_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    TemplateSourceWindow *w = template_source_window_new (GTK_WINDOW(win));

    gtk_window_present (GTK_WINDOW (w));
}

static GtkPrintSettings *settings = NULL;

static void print_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkPrintOperation *print;
    GtkPrintOperationResult res;

    print = gtk_print_operation_new ();

    if (settings != NULL)
        gtk_print_operation_set_print_settings (print, settings);

    /*
    g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), NULL);
    g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), NULL);
    */

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (win), NULL);

    switch(res){
        case GTK_PRINT_OPERATION_RESULT_ERROR:
        case GTK_PRINT_OPERATION_RESULT_CANCEL:
        case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:
        case GTK_PRINT_OPERATION_RESULT_APPLY:
            if (settings != NULL)
                g_object_unref (settings);
            settings = g_object_ref (gtk_print_operation_get_print_settings (print));

            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PRINTER         ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PRINTER         ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_ORIENTATION     ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_ORIENTATION     ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PAPER_FORMAT    ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PAPER_FORMAT    ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PAPER_WIDTH     ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PAPER_WIDTH     ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PAPER_HEIGHT    ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PAPER_HEIGHT    ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_N_COPIES        ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_N_COPIES        ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_DEFAULT_SOURCE  ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_DEFAULT_SOURCE  ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_QUALITY         ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_QUALITY         ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_RESOLUTION      ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_RESOLUTION      ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_USE_COLOR       ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_USE_COLOR       ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_DUPLEX          ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_DUPLEX          ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_COLLATE         ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_COLLATE         ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_REVERSE         ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_REVERSE         ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_MEDIA_TYPE      ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_MEDIA_TYPE      ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_DITHER          ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_DITHER          ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_SCALE           ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_SCALE           ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PRINT_PAGES     ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PRINT_PAGES     ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PAGE_RANGES     ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PAGE_RANGES     ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PAGE_SET        ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PAGE_SET        ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_FINISHINGS      ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_FINISHINGS      ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_NUMBER_UP       ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_NUMBER_UP       ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_NUMBER_UP_LAYOUT,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_NUMBER_UP_LAYOUT));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_OUTPUT_BIN      ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_OUTPUT_BIN      ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_RESOLUTION_X    ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_RESOLUTION_X    ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_RESOLUTION_Y    ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_RESOLUTION_Y    ));
            g_printf("%s - %s\n", GTK_PRINT_SETTINGS_PRINTER_LPI     ,gtk_print_settings_get (settings, GTK_PRINT_SETTINGS_PRINTER_LPI     ));

        break;

    }



    g_object_unref (print);
}


static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    int rc=dlg_msg( GTK_WINDOW(win), "Exit", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Uscire dall'applicazione ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            if(priv->wsi){          
                if(priv->app){
                    MainAppPrivate *main_app_priv  = get_main_app_private_instance (priv->app);
                    g_printf("%s window save state\n", WINDOW_NAME);
                    g_settings_sync ();
                    window_save_state (win,  priv->wsi, main_app_priv->settings);
                }
                window_size_info_free (priv->wsi);
                priv->wsi = NULL;
            }

            gtk_window_close (GTK_WINDOW (win));

            main_app_quit (MAIN_APP(priv->app));
        break;
    }

}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

void search_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gboolean mode;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
}

static GActionEntry entries[] =
{
    { "about",                         about_activated,                       NULL, NULL, NULL },
    { "search",                        search_activated,                      NULL, NULL, NULL },
    { "template",                      template_activated,                    NULL, NULL, NULL },
    { "print",                         print_activated,                       NULL, NULL, NULL },
    { "quit",                          quit_activated,                        NULL, NULL, NULL }
};

MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW(win));
}

static void main_window_init (MainWindow *win)
{
    g_printf("%s_init\n", WINDOW_NAME);

}

static void main_window_dispose (GObject *object)
{
    g_printf("%s_dispose\n", WINDOW_NAME);

    MainWindowPrivate *priv = get_main_window_private_instance (object);
    MainAppPrivate *main_app_priv = get_main_app_private_instance (priv->app);
    MainWindow *win = MAIN_WINDOW (object);

    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_finalize (GObject *object)
{
    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (main_window_parent_class)->finalize (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("%s_class_init\n", WINDOW_NAME);

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;
    G_OBJECT_CLASS (class)->finalize = main_window_finalize;

    /* TODO */
    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_entry);
    // gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  main_menu);
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


static void main_window_setup (MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    if(priv->app){
        MainAppPrivate *main_app_priv  = get_main_app_private_instance (priv->app);

        priv->screen = gdk_screen_get_default ();
        priv->provider = gtk_css_provider_new ();

        gchar *css_filename = g_strdup_printf("/org/%s/%s/%s.css", main_app_priv->pszAppClass, main_app_priv->pszAppName, main_app_priv->pszAppName);
        gtk_css_provider_load_from_resource (priv->provider, css_filename);
        g_free(css_filename);

        /*
        gchar *menu_filename = g_strdup_printf("/org/%s/%s/%s-menu.ui", main_app_priv->pszAppClass, main_app_priv->pszAppName, WINDOW_NAME);
        GtkBuilder *builder = gtk_builder_new_from_resource (menu_filename);
        GMenuModel *menu = G_MENU_MODEL (gtk_builder_get_object (builder, "menu"));
        gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (priv->main_menu), menu);
        g_object_unref (builder);
        g_free(menu_filename);
        */

        gtk_style_context_add_provider_for_screen (priv->screen, GTK_STYLE_PROVIDER(priv->provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        gtk_widget_init_template (GTK_WIDGET (win));

        priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

        window_load_state (win, priv->wsi, main_app_priv->settings);

        g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
        g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);

        gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
        g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);
    }

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

/*
* public methods
*/
MainWindow *main_window_new (GtkApplication *app)
{
    g_printf("%s_window_new\n", WINDOW_NAME);

    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    priv->app = app;
    main_window_setup (win);

    return win;
}
