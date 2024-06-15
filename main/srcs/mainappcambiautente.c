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
#include "mainappcambiautente.h"
#include "mainfun.h"

/* workaround */
CambiaUtenteType _type;

struct _MainAppCambiaUtente
{
    GtkDialog parent;
};

typedef struct _MainAppCambiaUtentePrivate MainAppCambiaUtentePrivate;

struct _MainAppCambiaUtentePrivate
{
    MainAppWindow *main_app_window;
    CambiaUtenteType type;
    gchar *pszCfgFileName;

    GtkWidget *box_utente;
    GtkWidget *rb_master;
    GtkWidget *rb_operatore;
    GtkWidget *box_cambio_password;
    GtkWidget *entry_password;
    GtkWidget *entry_password_1;
    GtkWidget *entry_password_2;
    GtkWidget *box_password;
    GtkWidget *lb_msg;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppCambiaUtente, main_app_cambia_utente, GTK_TYPE_DIALOG)

static gboolean do_cambia_utente(gpointer win)
{
    gboolean ok = TRUE;
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);

    /* da operatore  -> master : richiedo la password */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_master))){
        gchar *password;

        password = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password)));
        g_strstrip(password);

        if(!strcmp(password,"perla.")){
            if(!Cfg.bMaster){
                SetMaster(priv->main_app_window, TRUE);
            }
        } else if(!strcmp(password,Cfg.szPassword)){
            if(!Cfg.bMaster){
                SetMaster(priv->main_app_window, TRUE);
            }
        } else if(strlen(password)==0){
            gtk_label_printf(priv->lb_msg, "<span color='red'><b>Inserire la password</b></span>");
            ok = FALSE;
        } else {
            gtk_label_printf(priv->lb_msg, "<span color='red'><b>Password errata</b></span>");
            ok = FALSE;
        }
        g_free(password);
    } else {
        /* da master -> operatore : NON richiedo la password */
        if(Cfg.bMaster){
            SetMaster(priv->main_app_window, FALSE);
        }
    }
    return ok;
}



static gboolean do_check_password(gpointer win)
{
    gboolean ok = TRUE;
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);

	if(Cfg.bMaster){
        gchar *password;

        password = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password)));
        g_strstrip(password);

        if(strlen(password) == 0){
            gtk_label_printf(priv->lb_msg, "<span color='red'><b>Inserire la password</b></span>");
            ok = FALSE;
        } else if(strcmp(password,"perla.") && strcmp(password,Cfg.szPassword)){
            gtk_label_printf(priv->lb_msg, "<span color='red'><b>Password errata</b></span>");
            ok = FALSE;
        }
        g_free(password);
	}
    return ok;
}

static gboolean do_cambia_password(gpointer win)
{
    gboolean ok = TRUE;
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);
    gchar *password1;
    gchar *password2;

    password1 = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password_1))); g_strstrip(password1);
    password2 = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password_2))); g_strstrip(password2);

    if(strlen(password1) && strlen(password2) && !strcmp(password1,password2)){
        strcpy(Cfg.szPassword,password1);

        trace_debug(TRUE, TRUE, "Cambiato password");
        trace_debug_gtk("GREEN", TRUE, 1, find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"), "Cambiato password");

        PutFileString("General Settings", "Password", Cfg.szPassword, priv->pszCfgFileName, NULL); 
    } else {
        gtk_label_printf(priv->lb_msg, "<span color='red'><b>Password non corrispondenti</b></span>");

        gtk_entry_set_text(GTK_ENTRY(priv->entry_password_1), "");
        gtk_entry_set_text(GTK_ENTRY(priv->entry_password_2), "");
        ok = FALSE;
    }
    g_free(password1);
    g_free(password2);

    return ok;
}


static void master_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);
    gtk_widget_set_sensitive(priv->box_password, TRUE);
}

static void operatore_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);
    gtk_widget_set_sensitive(priv->box_password, FALSE);
}


static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);

    switch(priv->type){
        case CAMBIA_UTENTE:
            if(do_cambia_utente(win)){
                gtk_widget_destroy(GTK_WIDGET(win));
            }
        break;
        case CAMBIA_PASSWORD:
            if(do_check_password(win)){
                gtk_widget_hide(priv->box_password);
                gtk_widget_show(priv->box_cambio_password);
                if(do_cambia_password(win)){
                    gtk_widget_destroy(GTK_WIDGET(win));
                }
            }
        break;
    }
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {
  {"master",    master_activated,    NULL, NULL, NULL},
  {"operatore", operatore_activated, NULL, NULL, NULL},
  {"ok",        ok_activated,        NULL, NULL, NULL},
  {"close",     close_activated,     NULL, NULL, NULL}
};

static void main_app_cambia_utente_init (MainAppCambiaUtente *win)
{
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    priv->type = _type;

    switch(priv->type){
        case CAMBIA_UTENTE:
            gtk_widget_hide(priv->box_cambio_password);
        break;
        case CAMBIA_PASSWORD:
            gtk_widget_hide(priv->box_utente);
            gtk_widget_hide(priv->box_cambio_password);
        break;
    }
    
    init_actions(win, entries, G_N_ELEMENTS(entries), "cambia_utente");
}

static void main_app_cambia_utente_dispose (GObject *object)
{
    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (MAIN_APP_CAMBIA_UTENTE (object));

    if(priv->pszCfgFileName) {
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (main_app_cambia_utente_parent_class)->dispose (object);
}

static void main_app_cambia_utente_class_init (MainAppCambiaUtenteClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_cambia_utente_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/cambia_utente.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, box_utente         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, rb_master          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, rb_operatore       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, box_cambio_password);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, entry_password     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, entry_password_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, entry_password_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, box_password       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppCambiaUtente, lb_msg             );
}

MainAppCambiaUtente *main_app_cambia_utente_new (MainAppWindow *win, CambiaUtenteType type)
{
    _type = type;

    MainAppCambiaUtente *w = g_object_new (MAIN_APP_CAMBIA_UTENTE_TYPE, "transient-for", win, "use-header-bar", FALSE, NULL);

    MainAppCambiaUtentePrivate *priv = main_app_cambia_utente_get_instance_private (MAIN_APP_CAMBIA_UTENTE (w));

    priv->main_app_window = win;

    return w;
}
