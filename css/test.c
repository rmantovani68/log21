#include <gtk/gtk.h>
#include <string.h>  

static void
activate (GtkApplication *app,
          gpointer        user_data)
{

  GtkWidget *window = gtk_application_window_new (app);;
  GtkWidget *button_01 = gtk_button_new_with_label("This is button 01");
  GtkWidget *button_02 = gtk_button_new_with_label("This is button 02");
  GtkWidget *label0 = gtk_label_new("hello 0");
  GtkWidget *label1 = gtk_label_new("hello 1");
  GtkWidget *label2 = gtk_label_new("hello 2");
  GtkWidget * main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 25);

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "theme.css", NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                               GTK_STYLE_PROVIDER(cssProvider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_box_set_homogeneous (GTK_BOX (main_box), TRUE);
  gtk_container_add (GTK_CONTAINER (window), main_box);
  gtk_container_add (GTK_CONTAINER (main_box), button_01);
  gtk_container_add (GTK_CONTAINER (main_box), label0);
  gtk_container_add (GTK_CONTAINER (main_box), label1);
  gtk_container_add (GTK_CONTAINER (main_box), label2);
  gtk_container_add (GTK_CONTAINER (main_box), button_02);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
