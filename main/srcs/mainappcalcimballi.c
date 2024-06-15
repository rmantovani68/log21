#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainappwin.h"
#include "mainappcalcimballi.h"
#include "mainfun.h"

struct _MainAppCalcImballi
{
    GtkDialog parent;
};

typedef struct _MainAppCalcImballiPrivate MainAppCalcImballiPrivate;

struct _MainAppCalcImballiPrivate
{
    gchar *pszCfgFileName;

    GtkWidget *entry_cedola;
    GtkWidget *date_dal;
    GtkWidget *date_al;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppCalcImballi, main_app_calcimballi, GTK_TYPE_DIALOG)

void do_calcolo_consumo_imballi(MainAppCalcImballi *win)
{
    MainAppCalcImballiPrivate *priv;
    priv = main_app_calcimballi_get_instance_private (win);

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
    RefreshTable(find_child(GTK_WIDGET (win),"sw_list_imballi_periodo"), "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS);

    GetFileString("imballi","lista_calcolo_imballi_pcf_periodo", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(find_child(GTK_WIDGET (win),"sw_list_preconfezionati_periodo"), "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS);

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

static void main_app_calcimballi_init (MainAppCalcImballi *win)
{
    MainAppCalcImballiPrivate *priv = main_app_calcimballi_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    
    init_actions(win, entries, G_N_ELEMENTS(entries), "calcola_imballi");

}

static void main_app_calcimballi_dispose (GObject *object)
{
    MainAppCalcImballiPrivate *priv;

    priv = main_app_calcimballi_get_instance_private (MAIN_APP_CALC_IMBALLI (object));

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (main_app_calcimballi_parent_class)->dispose (object);
}

static void main_app_calcimballi_class_init (MainAppCalcImballiClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_calcimballi_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/calcola_imballi.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCalcImballi, entry_cedola);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCalcImballi, date_dal);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCalcImballi, date_al);
}

MainAppCalcImballi * main_app_calcimballi_new (MainAppWindow *win)
{
    return g_object_new (MAIN_APP_CALC_IMBALLI_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
