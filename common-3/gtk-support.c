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
#include <gtk-support.h>

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
    BOOL bOK=FALSE;

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

void gtk_widget_set_foreground(GtkWidget *w,GdkColor *Color)
{
#ifdef DA_FARE
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
    gtk_widget_modify_fg(w,GTK_STATE_NORMAL,Color);
}

void gtk_widget_set_background(GtkWidget *w,GdkColor *Color)
{
#ifdef DA_FARE
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
    gtk_widget_modify_bg(w,GTK_STATE_NORMAL,Color);
}


void trace_debug(char *szColor,gboolean bDate,gboolean bNewLine,GtkWidget *txt,char *fmt,...)
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
        const gchar *widget_name = gtk_widget_get_name(parent);
        // g_printf("%*.*s-widget name : %s\n", tabs, tabs, " ", widget_name?widget_name:"noname");

        if (parent && g_ascii_strcasecmp(gtk_widget_get_name(parent), (gchar *)name) == 0) {
            // g_printf("%*.*s-trovato widget name : %s\n", tabs, tabs, " ", widget_name?widget_name:"noname");
            return parent;
        }

        if (GTK_IS_BIN(parent)) {
            // g_printf("%*.*s-parent is a BIN (1 child)\n", tabs, tabs, " ");
            if(gtk_bin_get_child(GTK_BIN(parent))){
                tabs+=4;
                return find_child(gtk_bin_get_child(GTK_BIN(parent)), name);
                tabs-=4;
            }
        }

        if (GTK_IS_CONTAINER(parent)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
            // g_printf("%*.*s-parent is a CONTAINER : it contains %d children\n", tabs,tabs,  " ", g_list_length (children));
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

        // g_printf("%*.*s-widget %s not found : %s\n", tabs, tabs, " ", name);
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

