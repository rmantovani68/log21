#include <gtk/gtk.h>

#include "mainapp.h"

gint settore;
gboolean maximized;
gboolean fullscreen;

int main (int argc, char *argv[])
{
    GError *error = NULL;
	gchar **remaining_args = NULL;
	gint arg_settore=0;
	gboolean arg_maximized=FALSE;
	gboolean arg_fullscreen=FALSE;
	GOptionContext *option_context;
	GOptionEntry option_entries[] = {
		{ "settore",         's',  0, G_OPTION_ARG_INT,            &arg_settore,    NULL, NULL },
		{ "maximized",       'm',  0, G_OPTION_ARG_NONE,           &arg_maximized,  NULL, NULL },
		{ "fullscreen",      'f',  0, G_OPTION_ARG_NONE,           &arg_fullscreen, NULL, NULL },
		{ G_OPTION_REMAINING, 0 ,  0, G_OPTION_ARG_FILENAME_ARRAY, &remaining_args, "Special option that collects any remaining arguments for us" },
		{ NULL }
	};

    /* 
    * Since this main is running uninstalled, we have to help it find its schema. This
    * is *not* necessary in properly installed application.
    */
    g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);

	option_context = g_option_context_new ("display");

	g_option_context_add_main_entries (option_context, option_entries, NULL);

    g_option_context_add_group (option_context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (option_context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        exit (1);
    }

	settore    = arg_settore;
	maximized  = arg_maximized;
	fullscreen = arg_fullscreen;

    return g_application_run (G_APPLICATION (main_app_new ()), argc, argv);
}
