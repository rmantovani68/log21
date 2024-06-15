/*
* gtk-support.c
*/
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#include <trace.h>
#include <ep-common.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>


/*
* funzione di stampa formattata in un gtk label widget
*/
void gtk_label_printf(GtkWidget *w,char *fmt,...)
{
    va_list args;
    char msgbuf[1024];

    if(!w) return;

    va_start(args,fmt);
    (void)vsprintf(msgbuf,fmt,args);
    va_end(args);

    // gtk_label_set_text (GTK_LABEL(w), g_utf8_normalize(msgbuf,-1,G_NORMALIZE_DEFAULT));
    gtk_label_set_markup (GTK_LABEL(w),msgbuf);
}

/*
* funzione di stampa formattata in un gtk text widget
*/
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...)
{
    va_list args;
    char msgbuf[1024];
    GtkTextBuffer *buffer;
    GtkTextTag *tag;
    GtkTextIter iter; 
    ep_bool_t bOK=FALSE;

    if(w){
        va_start(args,fmt);
        (void)vsprintf(msgbuf,fmt,args);
        va_end(args);

        /* ricavo il buffer */
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w));

        if(szColor && strlen(szColor)){
            tag = gtk_text_buffer_create_tag (buffer, NULL, "foreground", szColor, NULL);  
            if(tag){
                gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_mark (buffer, "insert"));
                gtk_text_buffer_insert_with_tags (buffer, &iter, msgbuf, -1, tag,NULL);
                bOK=TRUE;
            }
        }
        if(!bOK){
            gtk_text_buffer_insert_at_cursor (buffer, g_utf8_normalize(msgbuf,-1,G_NORMALIZE_DEFAULT), -1);
        }

        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW (w), gtk_text_buffer_get_mark (buffer, "insert"), 0.0, FALSE, 0.0, 0.0);
    } else {
        g_printf("gtk_text_printf : widget is NULL\n");
    }
}

/*
* gtk_text_read_file(w,szFileName)
* legge il file szfilename e lo inserisce nella gtk_text specificata in input
*/
void gtk_text_read_file(GtkWidget *w,char *szFileName)
{
    int fd, num_bytes;
    char *buf;
    struct stat file_status;

    if ((fd = open(szFileName, O_RDONLY)) <= 0)
        return;
    else {
        if (fstat(fd, &file_status) == -1)
            return;
        if ((buf = malloc(file_status.st_size)) == (char *)NULL)
            return;
        if ((num_bytes = read(fd, buf, file_status.st_size)) <= 0)
            return;
    }
    buf[file_status.st_size]='\0';
    close(fd);
    /* gtk_text_insert (GTK_TEXT(w), NULL, NULL, NULL, buf, -1); */
    free(buf);
}


/*
* void gtk_update(void)
*/
void gtk_update(void)
{
  while (gtk_events_pending())
    gtk_main_iteration();
}

void gtk_widget_set_foreground(GtkWidget *w, GdkRGBA *Color)
{
#ifdef TODO
    GtkStyle *style;

    if(w==(GtkWidget *)NULL){
        return;
    }
    /* set foreground and text to color */
    style = gtk_style_copy(gtk_widget_get_style(w));
    gtk_style_ref(style);
    style->text[GTK_STATE_NORMAL] = *Color;
    style->fg[GTK_STATE_NORMAL] = *Color;
    gtk_widget_set_style(w, style);
    gtk_style_unref(style);
#endif
    // gtk_widget_modify_fg(w,GTK_STATE_NORMAL,Color);
	gtk_widget_override_color(w, GTK_STATE_NORMAL, Color);
}

void gtk_widget_set_background(GtkWidget *w,GdkRGBA *Color)
{
#ifdef TODO
    GtkStyle *style;

    if(w==(GtkWidget *)NULL){
        return;
    }

    /* set background to color */
    style = gtk_style_copy(gtk_widget_get_style(w));
    gtk_style_ref(style);
    style->bg[GTK_STATE_NORMAL] = *Color;
    gtk_widget_set_style(w, style);
    gtk_style_unref(style);
#endif
    // gtk_widget_modify_bg(w,GTK_STATE_NORMAL,Color);
	gtk_widget_override_background_color(w, GTK_STATE_NORMAL, Color);
}

void trace_debug_gtk(char *szColor,gboolean bDate,gboolean bNewLine,GtkWidget *txt,char *fmt,...)
{
    va_list args;
    char szMessage[1024];
    char tbuf[80];
    char dbuf[80];

    va_start(args,fmt);
    (void)vsprintf(szMessage,fmt,args);
    va_end(args);
#ifdef TRACE
    trace_out_vstr_date(bNewLine,szMessage);
#endif
    if(txt){
        strcat(szMessage,"\n");
        gtk_text_printf(szColor,txt,"[%s] [%s] : ", trace_getdate(time((long *)0), dbuf), trace_gettime_msec(time((long *)0), tbuf));
        gtk_text_printf(szColor,txt,szMessage);
    }
}

void SetCfgWidgets(GtkWidget *dlg,PCFGITEM pItems)
{
    int nIndex=0;
    char szTmp[256];
    int nTmp;
    GtkWidget *w;


    while(pItems[nIndex].pszItemName){
        switch(pItems[nIndex].ItemType){
            case CFG_TYPE_BOOL:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),*(pItems[nIndex].ItemValue.pbBool));
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),*(pItems[nIndex].ItemValue.pbBool));
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            sprintf(szTmp,"%d", *(pItems[nIndex].ItemValue.pbBool));
                            gtk_entry_set_text(GTK_ENTRY(w),szTmp);
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_INT:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),*(pItems[nIndex].ItemValue.pnInt));
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),*(pItems[nIndex].ItemValue.pnInt));
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            sprintf(szTmp,"%d", *(pItems[nIndex].ItemValue.pnInt));
                            gtk_entry_set_text(GTK_ENTRY(w),szTmp);
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_STRING:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            nTmp=atoi(pItems[nIndex].ItemValue.pszString);
                            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),nTmp);
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            nTmp=atoi(pItems[nIndex].ItemValue.pszString);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),nTmp);
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            gtk_entry_set_text(GTK_ENTRY(w),pItems[nIndex].ItemValue.pszString);
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_NONE:
                /* eliminate compiler warning */
            break;
        }
        nIndex++;
    }
}

void ApplyCfgItems(GtkWidget *dlg,PCFGITEM pItems)
{
    int nIndex=0;
    char szTmp[256];
    GtkWidget *w;


    while(pItems[nIndex].pszItemName){
        switch(pItems[nIndex].ItemType){
            case CFG_TYPE_BOOL:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            *(pItems[nIndex].ItemValue.pbBool) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            *(pItems[nIndex].ItemValue.pbBool) = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            strcpy(szTmp,gtk_entry_get_text(GTK_ENTRY(w)));
                            *(pItems[nIndex].ItemValue.pbBool) = atoi(szTmp);
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_INT:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            *(pItems[nIndex].ItemValue.pnInt) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            *(pItems[nIndex].ItemValue.pnInt) = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            strcpy(szTmp,gtk_entry_get_text(GTK_ENTRY(w)));
                            *(pItems[nIndex].ItemValue.pnInt) = atoi(szTmp);
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_STRING:
                switch(pItems[nIndex].WidgetType){
                    case CFG_TOGGLE_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            sprintf(pItems[nIndex].ItemValue.pszString,"%d",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
                        }
                    break;
                    case CFG_SPIN_BUTTON_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            sprintf(pItems[nIndex].ItemValue.pszString,"%d",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w)));
                        }
                    break;
                    case CFG_ENTRY_WIDGET:
                        if((w=find_child(dlg,pItems[nIndex].pszWidgetName))){
                            strcpy(pItems[nIndex].ItemValue.pszString,gtk_entry_get_text(GTK_ENTRY(w)));
                        }
                    break;
                    case CFG_NO_WIDGET:
                        /* eliminate compiler warning */
                    break;
                }
            break;
            case CFG_TYPE_NONE:
                /* eliminate compiler warning */
            break;
        }
        nIndex++;
    }
}

void gtk_tree_model_get_text(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, char **value)
{
    GType type;
    int nValue;
    float fValue;
    char *pszValue;

    type=gtk_tree_model_get_column_type(tree_model, column);

    switch(type){
        case G_TYPE_STRING: gtk_tree_model_get(tree_model,iter,column,&pszValue,-1);*value=g_strdup(pszValue);g_free(pszValue);break;
        case G_TYPE_FLOAT:  gtk_tree_model_get(tree_model,iter,column,&fValue,-1);*value=g_strdup_printf("%.2f",fValue);break;
        case G_TYPE_INT:    gtk_tree_model_get(tree_model,iter,column,&nValue,-1);*value=g_strdup_printf("%d",nValue);break;
        default: *value=g_strdup("NULL");break;
    }
}

void gtk_label_pango_printf(GtkLabel *label,char *pszCfgFileName,char *fmt,...)
{
    va_list args;
    char szMessage[1024];
    char szBuffer[1024];
    char szPangoItem[128];
    char szPangoValue[256];

    if(!label) return;

    va_start(args,fmt);
    (void)vsprintf(szMessage,fmt,args);
    va_end(args);
    
    sprintf(szPangoItem,"%s_pango_prefix",gtk_widget_get_name(GTK_WIDGET(label)));
    GetFileString("pango markups", szPangoItem, "", szPangoValue, 128,pszCfgFileName,NULL); 
    strcpy(szBuffer,szPangoValue);
    strcat(szBuffer,szMessage);
    sprintf(szPangoItem,"%s_pango_postfix",gtk_widget_get_name(GTK_WIDGET(label)));
    GetFileString("pango markups", szPangoItem, "", szPangoValue, 128,pszCfgFileName,NULL); 
    strcat(szBuffer,szPangoValue);
    gtk_label_set_markup (label,szBuffer);

}


GtkWidget *find_child(GtkWidget *parent, const gchar *name)
{
    static int tabs = 0;
    if(parent!=NULL){

        if (parent && g_ascii_strcasecmp(gtk_widget_get_name(parent), (gchar *)name) == 0) {
            return parent;
        }

        if (GTK_IS_BIN(parent)) {
            if(gtk_bin_get_child(GTK_BIN(parent))){
                tabs+=4;
                return find_child(gtk_bin_get_child(GTK_BIN(parent)), name);
                tabs-=4;
            }
        }

        if (GTK_IS_CONTAINER(parent)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
            do {
                if(children && children->data){
                    tabs+=4;
                    GtkWidget *widget = find_child(children->data, name);
                    if (widget != NULL) {
                        return widget;
                    }
                    tabs-=4;
                }
            } while ((children = g_list_next(children)) != NULL);
        }

    }

    return NULL;
}


int dlg_msg(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, char *fmt,...)
{
	GtkWidget *dlg;
	va_list args;
	char msgbuf[1024];

	va_start(args,fmt);
	(void)vsprintf(msgbuf,fmt,args);
	va_end(args);

	dlg = gtk_message_dialog_new (parent, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, type, buttons_type, NULL);
	if(title) gtk_window_set_title (GTK_WINDOW (dlg), title);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dlg), msgbuf);

	int rc = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy(dlg);

	return rc;
}

GtkWidget *get_ui_widget(GtkBuilder *builder, const gchar *name)                
{
	if(name != NULL)
		return GTK_WIDGET(gtk_builder_get_object(builder, name));
	return NULL;
}

WindowSizeInfo *window_size_info_new (WindowSizeInfo **wsi, gchar *window_name)
{
    *wsi = g_new0(WindowSizeInfo, 1);
    (*wsi)->window_name  = g_strdup(window_name);

    return *wsi;
}

void window_size_info_free (WindowSizeInfo *wsi)
{
    g_free(wsi->window_name);
    g_free(wsi);
}

void window_get_position (gpointer win, WindowSizeInfo *wsi)
{
    gtk_window_get_position(GTK_WINDOW(win), &(wsi->current_x), &(wsi->current_y));
}

gboolean window_save_state (gpointer win, WindowSizeInfo *wsi, GSettings *settings)
{
    gboolean ok = TRUE;

    window_get_position (win, wsi);
    
    gchar *pszBuffer;

    if(ok) {pszBuffer = g_strdup_printf("%s-x"             , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->current_width);  */ ok = g_settings_set_int     (settings, pszBuffer, wsi->current_x);      g_free(pszBuffer);}
    if(ok) {pszBuffer = g_strdup_printf("%s-y"             , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->current_height); */ ok = g_settings_set_int     (settings, pszBuffer, wsi->current_y);      g_free(pszBuffer);}
    if(ok) {pszBuffer = g_strdup_printf("%s-width"         , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->current_width);  */ ok = g_settings_set_int     (settings, pszBuffer, wsi->current_width);  g_free(pszBuffer);}
    if(ok) {pszBuffer = g_strdup_printf("%s-height"        , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->current_height); */ ok = g_settings_set_int     (settings, pszBuffer, wsi->current_height); g_free(pszBuffer);}
    if(ok) {pszBuffer = g_strdup_printf("%s-is-maximized"  , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->is_maximized);   */ ok = g_settings_set_boolean (settings, pszBuffer, wsi->is_maximized);   g_free(pszBuffer);}
    if(ok) {pszBuffer = g_strdup_printf("%s-is-fullscreen" , wsi->window_name); /* g_printf ("%s : %d\n", pszBuffer, wsi->is_fullscreen);  */ ok = g_settings_set_boolean (settings, pszBuffer, wsi->is_fullscreen);  g_free(pszBuffer);}

    /*
     * */
    g_printf ("save_state\n");
    g_printf ("%s-x              : %d\n", wsi->window_name, wsi->current_x);  
    g_printf ("%s-y              : %d\n", wsi->window_name, wsi->current_y);  
    g_printf ("%s-width          : %d\n", wsi->window_name, wsi->current_width);  
    g_printf ("%s-height         : %d\n", wsi->window_name, wsi->current_height); 
    g_printf ("%s-is-maximized   : %d\n", wsi->window_name, wsi->is_maximized);   
    g_printf ("%s-is-fullscreen  : %d\n", wsi->window_name, wsi->is_fullscreen);  
    /*
    */

    // g_printf ("->%s\n", ok?"OK":"ERR");
    return ok;
}

void window_load_state (gpointer win, WindowSizeInfo *wsi, GSettings *settings)
{
    gchar *pszBuffer;

    pszBuffer = g_strdup_printf("%s-x"             , wsi->window_name); wsi->current_x      = g_settings_get_int     (settings, pszBuffer); g_free(pszBuffer);
    pszBuffer = g_strdup_printf("%s-y"             , wsi->window_name); wsi->current_y      = g_settings_get_int     (settings, pszBuffer); g_free(pszBuffer);
    pszBuffer = g_strdup_printf("%s-width"         , wsi->window_name); wsi->current_width  = g_settings_get_int     (settings, pszBuffer); g_free(pszBuffer);
    pszBuffer = g_strdup_printf("%s-height"        , wsi->window_name); wsi->current_height = g_settings_get_int     (settings, pszBuffer); g_free(pszBuffer);
    pszBuffer = g_strdup_printf("%s-is-maximized"  , wsi->window_name); wsi->is_maximized   = g_settings_get_boolean (settings, pszBuffer); g_free(pszBuffer);
    pszBuffer = g_strdup_printf("%s-is-fullscreen" , wsi->window_name); wsi->is_fullscreen  = g_settings_get_boolean (settings, pszBuffer); g_free(pszBuffer);

    /*
     * */
    g_printf ("load_state\n");
    g_printf ("%s-x              : %d\n", wsi->window_name, wsi->current_x);  
    g_printf ("%s-y              : %d\n", wsi->window_name, wsi->current_y);  
    g_printf ("%s-width          : %d\n", wsi->window_name, wsi->current_width);  
    g_printf ("%s-height         : %d\n", wsi->window_name, wsi->current_height); 
    g_printf ("%s-is-maximized   : %d\n", wsi->window_name, wsi->is_maximized);   
    g_printf ("%s-is-fullscreen  : %d\n", wsi->window_name, wsi->is_fullscreen);  
    /*
    */

    // apply the loaded state
    gtk_window_set_default_size (GTK_WINDOW (win), wsi->current_width, wsi->current_height);

    if (wsi->is_maximized)
        gtk_window_maximize (GTK_WINDOW (win));

    if (wsi->is_fullscreen)
        gtk_window_fullscreen (GTK_WINDOW (win));

    if(!wsi->is_fullscreen && !wsi->is_maximized)
        gtk_window_move (GTK_WINDOW(win), wsi->current_x, wsi->current_y);
}

gboolean on_window_size_allocate (gpointer win, GtkAllocation *allocation, gpointer user_data)
{
    // g_printf ("on_window_size_allocate\n");

    gboolean res = GDK_EVENT_PROPAGATE;
    WindowSizeInfo *wsi = (WindowSizeInfo *)user_data;

    // save the window geometry only if we are not maximized of fullscreen
    if (!(wsi->is_maximized || wsi->is_fullscreen)) {
        gtk_window_get_size (GTK_WINDOW (win), &(wsi->current_width), &(wsi->current_height));
        // g_printf ("current_width  : %d\n", wsi->current_width);  
        // g_printf ("current_height : %d\n", wsi->current_height); 
    }

    return res;
}

#if 0
gboolean on_window_state_event (gpointer win, GdkEventWindowState *event, gpointer user_data)
{
    // g_printf ("on_window_state_event\n");

    gboolean res = GDK_EVENT_PROPAGATE;
    WindowSizeInfo *wsi = (WindowSizeInfo *)user_data;

    wsi->is_maximized  = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    wsi->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
    // g_printf ("is_maximized  : %d\n", wsi->is_maximized);   
    // g_printf ("is_fullscreen : %d\n", wsi->is_fullscreen);  

    return res;
}
#endif


static gpointer register_resources (gpointer data)
{
  _ep_register_resource ();
  return NULL;
}

void _ep_ensure_resources (void)
{
  static GOnce register_resources_once = G_ONCE_INIT;

  g_once (&register_resources_once, register_resources, NULL);
}


void init_actions(gpointer win, GActionEntry *entries, int n_entries, gchar *group_name)
{
    GActionGroup *actions;

    actions = G_ACTION_GROUP (g_simple_action_group_new ());

    g_action_map_add_action_entries(G_ACTION_MAP(actions), entries, n_entries,  win);  

    gtk_widget_insert_action_group (GTK_WIDGET (win), group_name, actions);

    g_object_unref (actions);
}

