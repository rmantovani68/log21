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
#include "cambiautente.h"
#include "mainfun.h"

#define WINDOW_NAME "cambia-utente-window"

/* workaround */
static CambiaUtenteType _type;

struct _CambiaUtenteWindow
{
    GtkDialog parent;
};

typedef struct _CambiaUtenteWindowPrivate CambiaUtenteWindowPrivate;

struct _CambiaUtenteWindowPrivate
{
    MainWindow *main_window;
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

G_DEFINE_TYPE_WITH_PRIVATE(CambiaUtenteWindow, cambia_utente, GTK_TYPE_DIALOG)

static gboolean do_cambia_utente(gpointer win)
{
    gboolean ok = TRUE;
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);

    /* da operatore  -> master : richiedo la password */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_master))){
        gchar *password;

        password = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password)));
        g_strstrip(password);

        if(!strcmp(password,"perla.")){
            if(!Cfg.bMaster){
                SetMaster(priv->main_window, TRUE);
            }
        } else if(!strcmp(password,Cfg.szPassword)){
            if(!Cfg.bMaster){
                SetMaster(priv->main_window, TRUE);
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
            SetMaster(priv->main_window, FALSE);
        }
    }
    return ok;
}



static gboolean do_check_password(gpointer win)
{
    gboolean ok = TRUE;
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);

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
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);
    gchar *password1;
    gchar *password2;

    password1 = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password_1))); g_strstrip(password1);
    password2 = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_password_2))); g_strstrip(password2);

    if(strlen(password1) && strlen(password2) && !strcmp(password1,password2)){
        strcpy(Cfg.szPassword,password1);

        trace_debug(TRUE, TRUE, "Cambiato password");
        trace_debug_gtk("GREEN", TRUE, 1, find_child(GTK_WIDGET(priv->main_window), "txt_msgs"), "Cambiato password");

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
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);
    gtk_widget_set_sensitive(priv->box_password, TRUE);
}

static void operatore_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);
    gtk_widget_set_sensitive(priv->box_password, FALSE);
}


static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);

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

static void cambia_utente_init (CambiaUtenteWindow *win)
{
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (win);
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
    
    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void cambia_utente_dispose (GObject *object)
{
    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (CAMBIA_UTENTE_WINDOW (object));

    if(priv->pszCfgFileName) {
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (cambia_utente_parent_class)->dispose (object);
}

static void cambia_utente_class_init (CambiaUtenteWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = cambia_utente_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, box_utente         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, rb_master          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, rb_operatore       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, box_cambio_password);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, entry_password     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, entry_password_1   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, entry_password_2   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, box_password       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), CambiaUtenteWindow, lb_msg             );
}

CambiaUtenteWindow *cambia_utente_new (MainWindow *win, CambiaUtenteType type)
{
    _type = type;

    CambiaUtenteWindow *w = g_object_new (CAMBIA_UTENTE_WINDOW_TYPE, "transient-for", win, "use-header-bar", FALSE, NULL);

    CambiaUtenteWindowPrivate *priv = cambia_utente_get_instance_private (CAMBIA_UTENTE_WINDOW (w));

    priv->main_window = win;

    return w;
}
