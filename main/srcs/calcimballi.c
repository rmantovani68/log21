#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <picking.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "calcimballi.h"
#include "mainfun.h"

#define WINDOW_NAME "calcola-imballi-window"

struct _CalcImballiWindow
{
    GtkDialog parent;
};

typedef struct _CalcImballiWindowPrivate CalcImballiWindowPrivate;

struct _CalcImballiWindowPrivate
{
    gchar *pszCfgFileName;

    GtkWidget *entry_cedola;
    GtkWidget *date_dal;
    GtkWidget *date_al;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcImballiWindow, calc_imballi_window, GTK_TYPE_DIALOG)

void do_calcolo_consumo_imballi(CalcImballiWindow *win)
{
    CalcImballiWindowPrivate *priv;
    priv = calc_imballi_window_get_instance_private (win);

    time_t t_dal,t_al;
    struct tm * tmtime;
    char szDataDal[128];
    char szDataAl[128];
    char szCedola[128];
    char szCedolaKey[256];
    char szDataKey[128];
    char szSelectCmd[4096];
    guint year, month, day;

    GtkWidget *dlg;
    int nRC;
    gboolean bFine=FALSE;
    
    gtk_calendar_get_date ((GtkCalendar *)priv->date_dal, &year, &month, &day);
    sprintf(szDataDal,"%04d-%02d-%02d",year, month+1, day);

    gtk_calendar_get_date ((GtkCalendar *)priv->date_al, &year, &month, &day);
    sprintf(szDataAl,"%04d-%02d-%02d",year, month+1, day);

    strcpy(szCedola,gtk_entry_get_text(GTK_ENTRY(GTK_WIDGET(priv->entry_cedola))));

    sprintf(szDataKey," date(cptmeva)>=date('%s') and date(cptmeva)<=date('%s') ", szDataDal, szDataAl);
    
    if (strlen(szCedola)){
        sprintf(szCedolaKey," and ronmced='%s' ",szCedola);
        strcat(szDataKey,szCedolaKey); 
    }

    GList *PS=NULL;
    add_item_to_parse(&PS,"%KEY%",szDataKey,TRUE);

    GetFileString("imballi","lista_calcolo_imballi_periodo", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(find_child(GTK_WIDGET (win),"sw_list_imballi_periodo"), "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

    GetFileString("imballi","lista_calcolo_imballi_pcf_periodo", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(find_child(GTK_WIDGET (win),"sw_list_preconfezionati_periodo"), "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static void find_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_calcolo_consumo_imballi(win);
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
    {"close", close_activated,  NULL, NULL, NULL},
    {"print", action_activated, NULL, NULL, NULL},
    {"find",  find_activated, NULL, NULL, NULL}
};

static void calc_imballi_window_init (CalcImballiWindow *win)
{
    CalcImballiWindowPrivate *priv = calc_imballi_window_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    
    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);

}

static void calc_imballi_window_dispose (GObject *object)
{
    CalcImballiWindowPrivate *priv;

    priv = calc_imballi_window_get_instance_private (CALC_IMBALLI_WINDOW (object));

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (calc_imballi_window_parent_class)->dispose (object);
}

static void calc_imballi_window_class_init (CalcImballiWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = calc_imballi_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CalcImballiWindow, entry_cedola);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CalcImballiWindow, date_dal);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CalcImballiWindow, date_al);
}

CalcImballiWindow * calc_imballi_window_new (MainWindow *win)
{
    return g_object_new (CALC_IMBALLI_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
