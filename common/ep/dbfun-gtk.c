/*
* modulo : dbfun-gtk.c
* --------------------
* Progetto Easy Picking - DB Management - PostgreSQL - GTK
*
* Data creazione 18/06/2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 18-06-2002 : Creazione
* -------------   -------------------------------------------------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <trace.h>

#include <gtk-support.h>
#include <ep-common.h>
#include <ep-db.h>
#include <dbfun-gtk.h>
#include <picking.h>

/**
 * @brief 
 * 
 * @param t 
 * @return char* 
 */
static char *_escape(char *t) 
{
    static char s[2048];

    int i, j;
    i = j = 0;

    while ( t[i] ) {

        /*  Translate the special character, if we have one  */

        switch( t[i] ) {
            case '\n': s[j++] = '\\';s[j] = 'n';  break;
            case '\t': s[j++] = '\\';s[j] = 't';  break;
            case '\a': s[j++] = '\\';s[j] = 'a';  break;
            case '\b': s[j++] = '\\';s[j] = 'b';  break;
            case '\f': s[j++] = '\\';s[j] = 'f';  break;
            case '\r': s[j++] = '\\';s[j] = 'r';  break;
            case '\v': s[j++] = '\\';s[j] = 'v';  break;
            case '\\': s[j++] = '\\';s[j] = '\\'; break;
            case '\"': s[j++] = '\\';s[j] = '\"'; break;

            default:
                /*  This is not a special character, so just copy it  */
                s[j] = t[i];
            break;
        }
        ++i;
        ++j;
    }
    s[j] = t[i];    /*  Don't forget the null character  */

    return s;
}

/**
 * @brief 
 * 
 * @param t 
 * @return char* 
 */
static char *_unescape(char *t) 
{
    static char s[2048];
    int i, j;
    i = j = 0;

    while ( t[i] ) {
        switch ( t[i] ) {
            case '\\':
                /*  We've found an escape sequence, so translate it  */
                switch( t[++i] ) {
                    case 'n':  s[j] = '\n'; break;
                    case 't':  s[j] = '\t'; break;
                    case 'a':  s[j] = '\a'; break;
                    case 'b':  s[j] = '\b'; break;
                    case 'f':  s[j] = '\f'; break;
                    case 'r':  s[j] = '\r'; break;
                    case 'v':  s[j] = '\v'; break;
                    case '\\': s[j] = '\\'; break;
                    case '\"': s[j] = '\"'; break;

                    default:
                        /*  We don't translate this escape sequence, so just copy it verbatim  */
                        s[j++] = '\\';
                        s[j] = t[i];
                    break;
                }
            break;

            default:
                /*  Not an escape sequence, so just copy the character  */
                s[j] = t[i];
            break;
        }
        ++i;
        ++j;
    }
    s[j] = t[i];    /*  Don't forget the null character  */

    return s;
}




static void float_data_func(GtkTreeViewColumn *col, GtkCellRenderer   *renderer, GtkTreeModel      *model, GtkTreeIter       *iter, gpointer  user_data)
{
	gfloat  float_value;
	gchar   buf[20];

	gtk_tree_model_get(model, iter, (intptr_t)user_data, &float_value, -1);

	g_snprintf(buf, sizeof(buf), "%.3f", float_value);

	g_object_set(renderer, "text", buf, NULL);
}

/*
* CreateListFromSelect()
* Crea una lista GTK TREE VIEW da una select SQL
* ritorna il numero di tuple o -1 se errore
*/
int CreateListFromSelect(GtkWidget *parent, char *szListName, int nSelectMode, char *szSqlCmd, GCallback on_selection_changed, gpointer user_data)
{
	int nTuples;
	int nFields;
	int nIndex;
	int Type;
	int nFieldIndex;
	DBresult *DBRes = NULL;
	ep_bool_t bOK=TRUE;
	GtkListStore *store;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GtkWidget *lst;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	GtkTreeSelection *selection;
	char szFieldValue[256];
	char szFieldName[256];
	GtkLabel *lb_column_title;


	if((lst=GTK_WIDGET(find_child(parent, szListName)))){
		gtk_widget_destroy(lst);
	}

	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(__ep_debug_level__,szSqlCmd);
	nTuples=DBntuples(DBRes);
	nFields=DBnfields(DBRes);

	if(DBresultStatus(DBRes) == DBRES_TUPLES_OK){

		for(nIndex=0;nIndex<nFields;nIndex++){
			Type=db_get_field_type(DBftype(DBRes,nIndex));
			switch(Type){
				default: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_STRING: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_FLOAT: pFieldTypes[nIndex]=G_TYPE_FLOAT; break;
				case DB_TYPE_INTEGER: pFieldTypes[nIndex]=G_TYPE_INT; break;
				case DB_TYPE_DATETIME: pFieldTypes[nIndex]=G_TYPE_STRING; break;
			}
		}
		/* aggiungo colonna colore foreground/background e bool di set */
		pFieldTypes[nFields+0]=G_TYPE_BOOLEAN;
		pFieldTypes[nFields+1]=G_TYPE_BOOLEAN;
		pFieldTypes[nFields+2]=G_TYPE_STRING;
		pFieldTypes[nFields+3]=G_TYPE_STRING;

		store = gtk_list_store_newv (nFields+4, pFieldTypes);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			gtk_list_store_append (store, &iter);  


			for (nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				strcpy(szFieldValue,DBgetvalue(DBRes,nIndex,nFieldIndex));
				switch(pFieldTypes[nFieldIndex]){
					case G_TYPE_STRING: gtk_list_store_set(store, &iter, nFieldIndex, g_utf8_normalize(szFieldValue,-1,G_NORMALIZE_DEFAULT) ,-1);break;
					case G_TYPE_FLOAT:  gtk_list_store_set(store, &iter, nFieldIndex, strtod(szFieldValue,NULL),-1); break;
					case G_TYPE_INT:    gtk_list_store_set(store, &iter, nFieldIndex, atoi(szFieldValue),-1); break;
				}
			}
			/* colore */
			gtk_list_store_set(store, &iter, nFields, NULL,-1);

		}
		lst = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
        gtk_widget_set_name ((GtkWidget *)lst, szListName);

		g_object_unref (G_OBJECT (store));

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lst));
        
        if(on_selection_changed){
            g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(on_selection_changed), user_data);
        }

		gtk_tree_selection_set_mode (selection, nSelectMode);

        /*
         * DEPRECATED 
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (lst), TRUE);
        */

		gtk_tree_view_set_enable_search (GTK_TREE_VIEW(lst), TRUE);


		for (nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/* allineamento a destra per i numeri */
			renderer = gtk_cell_renderer_text_new ();
			switch(pFieldTypes[nFieldIndex]){
				case G_TYPE_FLOAT:   g_object_set (renderer, "xalign",1.0,NULL) ; break;
				case G_TYPE_INT:     g_object_set (renderer, "xalign",1.0,NULL) ; break;
				default:     g_object_set (renderer, "xalign",0.0,NULL) ; break;
			}
			strcpy(szFieldName,_unescape(DBfname(DBRes,nFieldIndex)));

			gtk_tree_view_append_column (GTK_TREE_VIEW (lst), 
				gtk_tree_view_column_new_with_attributes( g_utf8_normalize(szFieldName,-1,G_NORMALIZE_DEFAULT) ,
					renderer,"text", nFieldIndex, 
					"foreground-set",nFields+0,
					"background-set",nFields+1,
					"foreground",nFields+2,
					"background",nFields+3,
					NULL));
			gtk_tree_view_column_set_sort_column_id (gtk_tree_view_get_column (GTK_TREE_VIEW (lst), nFieldIndex), nFieldIndex);

			lb_column_title=GTK_LABEL(gtk_label_new(szFieldName));
			gtk_widget_show (GTK_WIDGET(lb_column_title));
			gtk_label_set_use_markup (GTK_LABEL (lb_column_title), TRUE);
			gtk_tree_view_column_set_widget (gtk_tree_view_get_column (GTK_TREE_VIEW (lst), nFieldIndex), GTK_WIDGET(lb_column_title));

			switch(pFieldTypes[nFieldIndex]){
				case G_TYPE_FLOAT:   
					/* numero di cifre decimali (3) nei float */
					gtk_tree_view_column_set_cell_data_func (gtk_tree_view_get_column (GTK_TREE_VIEW (lst), nFieldIndex), renderer, float_data_func, (gpointer)(long)nFieldIndex, NULL);
				break;
			}
		}

		gtk_container_add (GTK_CONTAINER (parent), lst);
		gtk_widget_show (lst);

	} else {
		bOK=FALSE;
	}


	DBclear(DBRes);

	return bOK?nTuples:-1;
}


/*
* UpdateListFromSelect()
* Aggiorna un elemento in una lista GTK TREE VIEW da una select SQL
* ritorna il numero di tuple (1) o -1 se errore
*/
int UpdateListFromSelect(GtkTreeView *list, char *szSqlCmd,GtkTreeIter *iter)
{
	int nTuples;
	int nFields;
	int nIndex;
	int Type;
	int nFieldIndex;
	DBresult *DBRes = NULL;
	ep_bool_t bOK=TRUE;
	GtkListStore *store;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	char szFieldValue[256];

	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(__ep_debug_level__,szSqlCmd);
	nTuples=DBntuples(DBRes);
	nFields=DBnfields(DBRes);

	/* controllo che sia presenta una sola riga */ 
	if(DBresultStatus(DBRes) == DBRES_TUPLES_OK && nTuples==1){

		for(nIndex=0;nIndex<nFields;nIndex++){
			Type=db_get_field_type(DBftype(DBRes,nIndex));
			switch(Type){
				default: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_STRING: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_FLOAT: pFieldTypes[nIndex]=G_TYPE_FLOAT; break;
				case DB_TYPE_INTEGER: pFieldTypes[nIndex]=G_TYPE_INT; break;
				case DB_TYPE_DATETIME: pFieldTypes[nIndex]=G_TYPE_STRING; break;
			}
		}
		/* aggiungo colonna colore */
		pFieldTypes[nFields]=G_TYPE_STRING;

		store = (GtkListStore *)gtk_tree_view_get_model(list);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			for (nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				strcpy(szFieldValue,DBgetvalue(DBRes,nIndex,nFieldIndex));
				switch(pFieldTypes[nFieldIndex]){
					case G_TYPE_STRING:  gtk_list_store_set(store, iter, nFieldIndex, g_utf8_normalize(szFieldValue,-1,G_NORMALIZE_DEFAULT) ,-1);break;
					case G_TYPE_FLOAT:   gtk_list_store_set(store, iter, nFieldIndex, strtod(szFieldValue,NULL),-1); break;
					case G_TYPE_INT:     gtk_list_store_set(store, iter, nFieldIndex, atoi(szFieldValue),-1); break;
				}
			}
		}
	} else {
		bOK=FALSE;
	}


	DBclear(DBRes);

	return bOK?nTuples:-1;
}

/*
* InsertListFromSelect()
* Inserisce un elemento in una lista GTK TREE VIEW da una select SQL
* ritorna il numero di tuple (1) o -1 se errore
*/
int InsertListFromSelect(GtkTreeView *list, char *szSqlCmd,GtkTreeIter *iter)
{
	int nTuples;
	int nFields;
	int nIndex;
	int Type;
	int nFieldIndex;
	DBresult *DBRes = NULL;
	ep_bool_t bOK=TRUE;
	GtkListStore *store;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	char szFieldValue[256];


	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(__ep_debug_level__,szSqlCmd);
	nTuples=DBntuples(DBRes);
	nFields=DBnfields(DBRes);

	if(DBresultStatus(DBRes) == DBRES_TUPLES_OK){

		for(nIndex=0;nIndex<nFields;nIndex++){
			Type=db_get_field_type(DBftype(DBRes,nIndex));
			switch(Type){
				default: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_STRING: pFieldTypes[nIndex]=G_TYPE_STRING; break;
				case DB_TYPE_FLOAT: pFieldTypes[nIndex]=G_TYPE_FLOAT; break;
				case DB_TYPE_INTEGER: pFieldTypes[nIndex]=G_TYPE_INT; break;
				case DB_TYPE_DATETIME: pFieldTypes[nIndex]=G_TYPE_STRING; break;
			}
		}
		/* aggiungo colonna colore */
		pFieldTypes[nFields]=G_TYPE_STRING;

		store = (GtkListStore *)gtk_tree_view_get_model(list);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			gtk_list_store_append (store, iter);  

			for (nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				strcpy(szFieldValue,DBgetvalue(DBRes,nIndex,nFieldIndex));
				switch(pFieldTypes[nFieldIndex]){
					case G_TYPE_STRING:  gtk_list_store_set(store, iter, nFieldIndex, g_utf8_normalize(szFieldValue,-1,G_NORMALIZE_DEFAULT) ,-1);break;
					case G_TYPE_FLOAT:   gtk_list_store_set(store, iter, nFieldIndex, strtod(szFieldValue,NULL),-1); break;
					case G_TYPE_INT:     gtk_list_store_set(store, iter, nFieldIndex, atoi(szFieldValue),-1); break;
				}
			}
			/* colore */
			gtk_list_store_set(store, iter, nFields, NULL,-1);

		}
	} else {
		bOK=FALSE;
	}


	DBclear(DBRes);

	return bOK?nTuples:-1;
}

/*
* UpdateTable()
* Aggiorna una riga di una lista GTK TREEVIEW_VERSION
*/
GtkTreeIter *UpdateTable(GtkWidget *parent,char *szListName,int nKeyIndex,char *szUpdateCmd, char *pszKey, GList *PSList,ep_bool_t bSetPosition)
{
	ep_bool_t bRecordFound=FALSE;
	int nTuples;
	int nFields;
	int nIndex;
	DBFieldType nType;
	int nFieldIndex;
	char szCmd[4096];
	char *pszText;
	DBresult *DBRes = NULL;
	char szDBValue[256];
	GtkTreeView *tree_view=GTK_TREE_VIEW(find_child(parent,szListName));
	GtkTreeModel *model=gtk_tree_view_get_model(tree_view);
	static GtkTreeIter iter;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	
    strcpy(szCmd, szUpdateCmd);

    if(PSList){
        ParseStatement(szCmd,PSList);
    }

    /*
    * Eseguo la select specificata
    */
    DBRes=DBExecQuery(__ep_debug_level__,szCmd);

    nTuples=DBntuples(DBRes);
    nFields=DBnfields(DBRes);

    if(nTuples){
        for(nIndex=0;nIndex<nFields;nIndex++){
            nType=db_get_field_type(DBftype(DBRes,nIndex));
            switch(nType){
                default: pFieldTypes[nIndex]=G_TYPE_STRING; break;
                case DB_TYPE_STRING: pFieldTypes[nIndex]=G_TYPE_STRING; break;
                case DB_TYPE_FLOAT: pFieldTypes[nIndex]=G_TYPE_FLOAT; break;
                case DB_TYPE_INTEGER: pFieldTypes[nIndex]=G_TYPE_INT; break;
                case DB_TYPE_DATETIME: pFieldTypes[nIndex]=G_TYPE_STRING; break;
            }
        }
        /* aggiungo colonna colore */
        pFieldTypes[nFields]=G_TYPE_STRING;

        if(gtk_tree_model_get_iter_first(model, &iter)){
            do {
                gtk_tree_model_get_text(model,&iter,nKeyIndex,&pszText);
                if(!strcmp(pszKey,pszText)){
                    /*
                    * trovata la riga
                    */
                    bRecordFound=TRUE;

                    for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){

                        strcpy(szDBValue,DBgetvalue(DBRes,0,nFieldIndex));
                
                        switch(pFieldTypes[nFieldIndex]){
                            case G_TYPE_INT:    gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, atoi(szDBValue),-1); break;
                            case G_TYPE_STRING: gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, g_utf8_normalize(szDBValue,-1,G_NORMALIZE_DEFAULT) ,-1); break;
                            case G_TYPE_FLOAT:  gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, strtod(szDBValue,NULL),-1);break;
                        }
                    }
                    break;
                }
                g_free(pszText);
            } while( gtk_tree_model_iter_next(model, &iter));
        }
        /* se non trovo aggiungo */
        if(bRecordFound==FALSE){
            gtk_list_store_append ((GtkListStore *)model, &iter);  

            for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){

                strcpy(szDBValue,DBgetvalue(DBRes,0,nFieldIndex));
        
                switch(pFieldTypes[nFieldIndex]){
                    case G_TYPE_STRING: gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, g_utf8_normalize(szDBValue,-1,G_NORMALIZE_DEFAULT) ,-1); break;
                    case G_TYPE_FLOAT:  gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, strtod(szDBValue,NULL),-1);break;
                    case G_TYPE_INT:    gtk_list_store_set((GtkListStore *)model, &iter, nFieldIndex, atoi(szDBValue),-1); break;
                }
            }
            /* background */
            gtk_list_store_set((GtkListStore *)model, &iter, nFields, NULL,-1);

        }
        if(bSetPosition){
            gtk_tree_view_scroll_to_cell(tree_view, gtk_tree_model_get_path(model,&iter), NULL, FALSE, 0, 0);
        }
    }
    
    DBclear(DBRes);

	return &iter;
}

int RefreshTable(GtkWidget *parent,char *szListName,char *szItemValue,int nSelectionMode,GList *PSList, GCallback on_selection_changed, gpointer user_data)
{
	char szSelectCmd[4096];
	char szCmd[2048];
	int nTuples=0;

    strcpy(szSelectCmd, szItemValue);

    if(PSList){
        ParseStatement(szSelectCmd,PSList);
    }

    strcpy(szCmd,szSelectCmd);

    nTuples=CreateListFromSelect(parent,szListName,nSelectionMode,szSelectCmd, on_selection_changed, user_data);

	return nTuples;
}

gchar *_tipo_ordini = NULL;
gchar *_tipo_ubicazioni = NULL;

void ep_set_tipo_ordini(gchar *to)
{
    if(_tipo_ordini){
        g_free(_tipo_ordini);
        _tipo_ordini = NULL;
    }
    _tipo_ordini = g_strdup_printf(to);
}

void ep_set_tipo_ubicazioni(gchar *tu)
{
    if(_tipo_ubicazioni){
        g_free(_tipo_ubicazioni);
        _tipo_ubicazioni = NULL;
    }
    _tipo_ubicazioni = g_strdup_printf(tu);
}


void ep_update_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *key, gboolean MoveToRow)
{
    char szBuffer[128];
    char szUpdateCmd[4096];


    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_key");
    int nKeyIndex=GetFileInt(paragraph,szBuffer, 0,cfg_file_name, NULL); 

    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_update");
    GetFileString(paragraph,szBuffer, "", szUpdateCmd, sizeof(szUpdateCmd),cfg_file_name, NULL); 

    GList *PS=NULL;
    gboolean first = TRUE;
    if(_tipo_ordini)     add_item_to_parse(&PS, "%ORDTIPO%", _tipo_ordini,     first ? first--: first);
    if(_tipo_ubicazioni) add_item_to_parse(&PS, "%TIPOUBI%", _tipo_ubicazioni, first ? first--: first);
    if(key)              add_item_to_parse(&PS, "%KEY%",     key,              first ? first--: first);

    UpdateTable(find_child(GTK_WIDGET (win), parent_name), list_name, nKeyIndex, szUpdateCmd,  key, PS, MoveToRow);
}


int ep_refresh_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode, gchar *key)
{
    char szSelectCmd[4096];

    GetFileString(paragraph,table_item_name, "", szSelectCmd, sizeof(szSelectCmd),cfg_file_name, NULL); 

    GList *PS=NULL;
    gboolean first = TRUE;
    if(_tipo_ordini)     add_item_to_parse(&PS, "%ORDTIPO%", _tipo_ordini,     first ? first--: first);
    if(_tipo_ubicazioni) add_item_to_parse(&PS, "%TIPOUBI%", _tipo_ubicazioni, first ? first--: first);
    if(key)              add_item_to_parse(&PS, "%KEY%",     key,              first ? first--: first);

    int rc =  RefreshTable(find_child(GTK_WIDGET (win),parent_name), list_name, szSelectCmd, mode, PS, NULL, NULL);
    return rc;
}
