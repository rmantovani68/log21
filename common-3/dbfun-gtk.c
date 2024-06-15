/*
* modulo : dbfun-gtk.c
* --------------------
* Progetto Easy Picking - DB Management - PostgreSQL - GTK
*
* Data creazione 18/06/2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2002-2021
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

#include <ep-common.h>
#include <ep-db.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>

static void float_data_func(GtkTreeViewColumn *col, GtkCellRenderer   *renderer, GtkTreeModel      *model, GtkTreeIter       *iter, gpointer  user_data)
{
	gfloat  float_value;
	gchar   buf[20];

	gtk_tree_model_get(model, iter, (int)user_data, &float_value, -1);

	g_snprintf(buf, sizeof(buf), "%.3f", float_value);

	g_object_set(renderer, "text", buf, NULL);
}

/*
* CreateListFromSelect()
* Crea una lista GTK TREE VIEW da una select SQL
* ritorna il numero di tuple o -1 se errore
*/
int CreateListFromSelect(GtkWidget *parent, char *szListName, int nSelectMode, char *szSqlCmd)
{
	int nTuples;
	int nFields;
	int nIndex;
	int Type;
	int nFieldIndex;
	DBresult *DBRes = NULL;
	BOOL bOK=TRUE;
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
	DBRes=DBExecQuery(0,szSqlCmd);
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

        /*
        * TODO
		* g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(selection_changed), lst);
        */

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
			strcpy(szFieldName,Unescape(DBfname(DBRes,nFieldIndex)));

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
					gtk_tree_view_column_set_cell_data_func (gtk_tree_view_get_column (GTK_TREE_VIEW (lst), nFieldIndex), renderer, float_data_func, (gpointer)(nFieldIndex), NULL);
				break;
			}
		}

		gtk_container_add (GTK_CONTAINER (parent), lst);
		gtk_widget_show (lst);

	} else {
		bOK=FALSE;
	}


	DBFreeQueryResult(DBRes);

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
	BOOL bOK=TRUE;
	GtkListStore *store;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	char szFieldValue[256];

	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(0,szSqlCmd);
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


	DBFreeQueryResult(DBRes);

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
	BOOL bOK=TRUE;
	GtkListStore *store;
	GType pFieldTypes[MAX_FIELDS_NUMBER];
	char szFieldValue[256];


	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(0,szSqlCmd);
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


	DBFreeQueryResult(DBRes);

	return bOK?nTuples:-1;
}

/*
* UpdateTable()
* Aggiorna una riga di una lista GTK TREEVIEW_VERSION
*/
GtkTreeIter *UpdateTable(GtkWidget *parent,char *szListName,int nKeyIndex,char *szUpdateCmd, char *pszKey, GList *PSList,BOOL bSetPosition)
{
	BOOL bRecordFound=FALSE;
	int nTuples;
	int nFields;
	int nIndex;
	DBFieldType nType;
	int nFieldIndex;
	char szCmd[4096];
	char *pszText;
	DBresult *DBRes = NULL;
	char szDBValue[256];
	char szBuffer[256];
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
    DBRes=DBExecQuery(FALSE,szCmd);

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
    
    DBFreeQueryResult(DBRes);

	return &iter;
}



int RefreshTable(GtkWidget *parent,char *szListName,char *szItemValue,int nSelectionMode,GList *PSList)
{
	char szSelectCmd[4096];
	char szCmd[2048];
	char szBuffer[256];
	char szTitle[256];
	int nTuples=0;

    strcpy(szSelectCmd, szItemValue);

    if(PSList){
        ParseStatement(szSelectCmd,PSList);
    }

    strcpy(szCmd,szSelectCmd);

    nTuples=CreateListFromSelect(parent,szListName,nSelectionMode,szSelectCmd);

	return nTuples;
}

void selection_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel *model;
	GtkWidget *w=GTK_WIDGET(data);
	GtkTreeView *tree_view;
	GList *lista;
	char szBuffer[128];
	GList *lst;
	GtkTreePath *path;
	GtkTreeIter iter;
	GType type;
	char szValore[128];
	int nValue;
	float fValue;
	char *pszValue;
	GValue *value;

	lista=gtk_tree_selection_get_selected_rows(selection, NULL);
	tree_view=gtk_tree_selection_get_tree_view(selection);
	model=gtk_tree_view_get_model(tree_view);

	if((lst=g_list_first(lista))){
		do{
			path=(GtkTreePath *)(lst->data);

			if(gtk_tree_model_get_iter(model, &iter, path)){
				/* ho ottenuto l'iter */
				type=gtk_tree_model_get_column_type(model, 0);

				switch(type){
					case G_TYPE_STRING: gtk_tree_model_get(model,&iter,0,&pszValue,-1);strcpy(szValore,pszValue);g_free(pszValue);break;
					case G_TYPE_FLOAT:  gtk_tree_model_get(model,&iter,0,&fValue,-1);sprintf(szValore,"%.3f",fValue);break;
					case G_TYPE_INT:    gtk_tree_model_get(model,&iter,0,&nValue,-1);sprintf(szValore,"%d",nValue);break;
				}
				printf("Valore = %s\n",szValore);
			}
		} while((lst=g_list_next(lst)));
	}
}

