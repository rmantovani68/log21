/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "support.h"

/* This is an internally used function to check if a pixmap file exists. */
static gchar *check_file_exists(const gchar *directory, const gchar *filename);

/* This is an internally used function to create pixmaps. */
static GtkWidget *create_dummy_pixmap_widget(GtkWidget *widget);


#ifdef TODO
/* This is an internally used function to create pixmaps. */
GdkPixmap *create_dummy_pixmap(GtkWidget *widget,GdkPixmap *gdkpixmap, GdkBitmap *mask);
#endif

GtkWidget *lookup_widget(GtkWidget *widget, const gchar *widget_name)
{
#ifdef TODO
  GtkWidget *parent, *found_widget=NULL;

	if(widget==(GtkWidget *)NULL){
		return (GtkWidget *)0;
	}

  for (;;) {
		/* lo cerco nel widget */
		if(found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget), widget_name))
			break;

		if (GTK_IS_MENU (widget))
			parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
		else
			parent = widget->parent;
		if (parent == NULL)
			break;
		widget = parent;
	}

  return found_widget;
#endif
}

/* This is a dummy pixmap we use when a pixmap can't be found. */
static char *dummy_pixmap_xpm[] = {
/* columns rows colors chars-per-pixel */
"1 1 1 1",
"  c None",
/* pixels */
" "
};


#ifdef TODO
/* This is an internally used function to create pixmaps. */
static GtkWidget *create_dummy_pixmap_widget(GtkWidget *widget)
{
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask, NULL, dummy_pixmap_xpm);
  if (gdkpixmap == NULL)
    g_error ("Couldn't create replacement pixmap.");
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

GdkPixmap *create_dummy_pixmap(GtkWidget *widget,GdkPixmap *gdkpixmap, GdkBitmap *mask)
{
  GdkColormap *colormap;

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask, NULL, dummy_pixmap_xpm);
  if (gdkpixmap == NULL)
    g_error ("Couldn't create replacement pixmap.");
  return gdkpixmap;
}
#endif

GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void add_pixmap_directory (const gchar *directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories, g_strdup (directory));
}

/* derived from code build by glade 
*/
gchar *find_pixmap_file (const gchar *filename)
{
	GList *elem;

	if (!filename)
	return NULL;

	/* We step through each of the pixmaps directory to find it. */
	elem = pixmaps_directories;
	while (elem) {
		gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data, G_DIR_SEPARATOR_S, filename);

		if (g_file_test (pathname, G_FILE_TEST_EXISTS)){
			return pathname;
		}
		g_free (pathname);
		elem = elem->next;
	}
	return NULL;
}

GdkPixbuf *pixbuf_new(gchar *filename)
{
	GdkPixbuf *pix = NULL;
	gchar *imagefile = NULL;
	gchar *ptr;
	GError *error = NULL;

	g_return_val_if_fail(filename != NULL, NULL);

	if ((g_ascii_strcasecmp(filename, "") == 0)) return NULL;


	if((ptr=rindex(filename,'/'))){
		filename=(ptr+1);
	}

	imagefile = find_pixmap_file(filename);

	if (!imagefile) {
		g_warning ("Couldn't find pixmap file: %s", filename);
		return NULL;
	}

	pix = gdk_pixbuf_new_from_file(imagefile, &error);
	if (!pix) {
		fprintf (stderr, "Failed to load pixbuf file: %s - error: %s\n", filename, error->message);
		g_error_free (error);
	}  

	g_free(imagefile);

	return  pix;
}

GtkWidget* create_pixmap(GtkWidget *widget, gchar *filename)
{
	GdkPixbuf *pixbuf;
	GtkWidget *w;

	w=gtk_image_new ();

	if((pixbuf=pixbuf_new(filename))){
		gtk_image_set_from_pixbuf(GTK_IMAGE(w),pixbuf);
	}
	return w;
}

/* This is an internally used function to check if a pixmap file exists. */
gchar* check_file_exists (const gchar *directory, const gchar *filename)
{
  gchar *full_filename;
  struct stat s;
  gint status;

  full_filename = (gchar*) g_malloc (strlen (directory) + 1
                                     + strlen (filename) + 1);
  strcpy (full_filename, directory);
  strcat (full_filename, G_DIR_SEPARATOR_S);
  strcat (full_filename, filename);

  status = stat (full_filename, &s);
  if (status == 0 && S_ISREG (s.st_mode))
    return full_filename;
  g_free (full_filename);
  return NULL;
}

/* 
* This is an internally used function to create pixmaps. 
*/
int set_pixmap(GtkWidget *widget, const gchar *filename)
{
#ifdef TODO
  gchar *found_filename = NULL;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GList *elem;
	int bRetValue=1;

  /* We first try any pixmaps directories set by the application. */
  elem = pixmaps_directories;
  while (elem) {
		found_filename = check_file_exists ((gchar*)elem->data, filename);
		if (found_filename) break;
		elem = elem->next;
	}

  /* If we haven't found the pixmap, try the source directory. */
  if (!found_filename) {
		found_filename = check_file_exists ("../pixmaps", filename);
	}

  if (!found_filename) {
		g_warning ("Couldn't find pixmap file: %s", filename);
		gdkpixmap=create_dummy_pixmap (widget, gdkpixmap, mask);
		bRetValue=0;
	}

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask, NULL, found_filename);
  if (gdkpixmap == NULL) {
		g_warning ("Error loading pixmap file: %s", found_filename);
		g_free (found_filename);
		gdkpixmap=create_dummy_pixmap (widget, gdkpixmap, mask);
		bRetValue=0;
	}
  g_free (found_filename);
  gtk_pixmap_set(GTK_PIXMAP(widget),gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
	return bRetValue;
#endif
}

int gtk_clist_get_first_selected_row(GtkWidget *lst)
{
#ifdef TODO
	GList *glist;

	glist=g_list_first(GTK_TREEVIEW(lst)->selection);

	if(glist==NULL){
		/*
		* nessuna riga della lista e' selezionata
		*/
		return -1;
	} else {
		return GPOINTER_TO_INT(glist->data);
	}
#endif

}

#ifdef ELIMINATO
/* 
* This is an internally used function to create pixmaps. 
*/
int create_gdk_pixmap(GtkWidget *widget,GdkPixmap **pixmap, GdkBitmap **mask,const gchar *filename)
{
  gchar *found_filename = NULL;
  GdkColormap *colormap;
  GList *elem;
	int bRetValue=1;

  /* We first try any pixmaps directories set by the application. */
  elem = pixmaps_directories;
  while (elem) {
		found_filename = check_file_exists ((gchar*)elem->data, filename);
		if (found_filename) break;
		elem = elem->next;
	}

  /* If we haven't found the pixmap, try the source directory. */
  if (!found_filename) {
		found_filename = check_file_exists ("../pixmaps", filename);
	}

  if (!found_filename) {
		g_warning ("Couldn't find pixmap file: %s", filename);
		*pixmap=create_dummy_pixmap (widget, *pixmap, *mask);
		bRetValue=0;
	}

  colormap = gtk_widget_get_colormap (widget);
  *pixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, mask, NULL, found_filename);
  if (*pixmap == NULL) {
		g_warning ("Error loading pixmap file: %s", found_filename);
		g_free (found_filename);
		*pixmap=create_dummy_pixmap (widget, *pixmap, *mask);
		bRetValue=0;
	}
  g_free (found_filename);

	return bRetValue;
}

void free_gdk_pixmap(GdkPixmap *pixmap, GdkBitmap *mask)
{
  gdk_pixmap_unref (pixmap);
  gdk_bitmap_unref (mask);
}
#endif


#ifdef TODO

GtkTreeSelection * gtk_tree_view_get_selection (GtkTreeView *tree_view);
GList * gtk_tree_selection_get_selected_rows (GtkTreeSelection *selection, GtkTreeModel **model);
gint gtk_tree_selection_count_selected_rows (GtkTreeSelection *selection);




GtkTreeSelection *selection;
GtkTreeModel     *model;
GtkTreeIter       iter;

/* This will only work in single or browse selection mode! */

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
if (gtk_tree_selection_get_selected(selection, &model, &iter))
{
	gchar *name;

	gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);

	g_print ("selected row is: %s\n", name);

	g_free(name);
}
else
{
	g_print ("no row selected.\n");
}
#endif