#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <math.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <linklist.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "balance.h"
#include "mainfun.h"

#define WINDOW_NAME "balance-window"

/* TODO: workaround perchè nella funzione do_carico_settori uso il puntatore alla main_window */
static MainWindow *_main_window;

struct _BalanceWindow
{
    GtkDialog parent;
};

typedef struct _BalanceWindowPrivate BalanceWindowPrivate;

struct _BalanceWindowPrivate
{
    MainWindow *main_window;
    GtkWidget *txt_msgs;

    GSettings *settings;
    WindowSizeInfo *wsi;

    LINKED_LIST ListaSettori;
    LINKED_LIST ListaProdPallet;
    LINKED_LIST ListaProdScaffali;
    LINKED_LIST ListaSoluzioni;
    GArray *SettoriDisponibili;

    GtkWidget *frm_settori;
    GtkWidget *rb_righe;
    GtkWidget *rb_pezzi;
    GtkWidget *cb_tutti_settori_selezionati;
    GtkWidget *cb_select_tutti_settori;
    GtkWidget *cb_escludi_prodotti_ubicati;
    GtkWidget *cb_vuota_ubicazioni;
    GtkWidget *pb_ok;
    GtkWidget *pb_refresh;
    GtkWidget *pb_equilibra;
    GtkWidget *pb_conferma;
    GtkWidget *tb_settore_01;
    GtkWidget *tb_settore_02;
    GtkWidget *tb_settore_03;
    GtkWidget *tb_settore_04;
    GtkWidget *tb_settore_05;
    GtkWidget *tb_settore_06;
    GtkWidget *tb_settore_07;
    GtkWidget *tb_settore_08;
    GtkWidget *tb_settore_09;
    GtkWidget *tb_settore_10;
    GtkWidget *tb_settore_11;
    GtkWidget *tb_settore_12;

    GtkWidget *tb_settori[MAX_SETTORI];

};

G_DEFINE_TYPE_WITH_PRIVATE(BalanceWindow, balance_window, GTK_TYPE_DIALOG)


int nPesoTotale=0;

/*
* rm 23-09-2003 : Introdotto parametro : bEscludiProdottiUbicati
*/
ep_bool_t Equilibratura(gpointer win, int nTipoEquilibratura,ep_bool_t bTuttiSettori,GArray *settori_disponibili,ep_bool_t bEscludiProdottiUbicati)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;

	/*
	* Libera la memoria occupata dinamicamente dalle liste in gioco.
	* Pulisce il file di configurazione balance.cfg
	*/
	LiberaMemoria(win);

	/*
	* Inizializzo le strutture dati e le variabili che vengono impiegate per il bilanciamento.
	* Creo la lista dei settori disponibili. Ogni settore ha due puntatori a lista delle ubica-
	* zioni a pallet e a scaffale. Le ubicazioni in lista sono tutte quelle presenti in ubicazio.
	*/
	InizializzaDati(win, settori_disponibili);

	/*
	* assegna le ubicazioni manuali ai prodotti di fuori formato
	*/
	AssegnaUbicazioniManuali(win);

	/*
	* Funzione che scandisce l'archivio balance alla ricerca di ubicazioni gia' assegnate
	* manualmente. Se ne trova le elimina dalla lista delle ubicazioni disponibili.
	* rm 23-09-2003 : se bEscludiProdottiUbicati allora considera le ubicazioni dei prodotti 
	*                 in lista come manuali
	*/
	AssegnatiUtente(win, nTipoEquilibratura,bEscludiProdottiUbicati);


	/*
	* Crea due liste, una di prodotti da assegnare a pallet, l'altra di prodotti da assegnare a
	* scaffale.
	* Il prodotto viene messo in lista solo se il campo CNISTATO  dell'archivio balance, indica che
	* l'assegnamento del prodotto deve essere fatto in automatico (cioe' non in manuale).
	*/
	CreaListeProdotti(win, nTipoEquilibratura,bEscludiProdottiUbicati);
	
	/*
	* Elimina dalla lista i settori in eccesso. 
	* Elimina dai settori rimasti le ubicazioni in eccesso
	*/
	if(SistemazioneDati(win, bTuttiSettori)){

		/*
		* Lancia tutti gli euristici, mantenendo le soluzioni in una apposita lista.
		*/
		CreaSoluzioni(win);

		/*
		* Cerco dalla lista delle soluzioni quella migliore scegliendo come criterio di valutazione la
		* varianza. Si aggiorna quindi il campo ubicazione dell'archivio balance.
		*/
		ScegliMigliore(win);

		/*
		* CheckSettori() crea il file DatiPerBalance che contiene l'elenco delle ubicazioni disponibili
		* suddivise per settore (estratte da (priv->ListaSettori)).
		*/
		CheckSettori(win);
	}


	return bOK;
}

/*
* Libera la memoria occupata dinamicamente dal processo. In particolare libera la memoria
* occupata dalle seguenti liste:
*  - ListaSettori
*  - ListaProdPallet
*  - ListaProdScaffali
*  - ListaSoluzioni
*/

ep_bool_t LiberaMemoria(gpointer win)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	SETTOREDISPONIBILE *Settore;
	SOLUZIONE *Soluzione;

	/*
	* Libero la memoria allocata per la lista dei settori che vengono impiegati per il riempimento.
	* Occorre prestare attenzione che tale lista potrebbe avere associata liste ubicazioni a pallet
	* e a scaffale anch'esse da liberare.
	*/
	while((Settore=link_pop(&(priv->ListaSettori)))!=NULL){
		list_free(&(Settore->ListaPallet));
		list_free(&(Settore->ListaScaffali));
		free(Settore);
	}
	/*
	* Libero la memoria allocata per contenere l'elenco dei prodotti che vanno ubicati a pallet
	*/
	list_free(&(priv->ListaProdPallet));
	/*
	* Libero la memoria allocata per contenere l'elenco dei prodotti che vanno ubicati a scaffale
	*/
	list_free(&(priv->ListaProdScaffali));
	/*
	* Libero la memoria allocata per contenere l'elenco delle soluzioni.
	* Occorre eliminare anche le liste di associazioni codice prodotto - ubicazioni associate a 
	* ciascuna soluzione.
	*/
	while((Soluzione=link_pop(&(priv->ListaSoluzioni)))!=NULL){
		list_free(&(Soluzione->ListaAssociazioni));
		free(Soluzione);
	}
	return(bOK);
}

/*
* Crea la lista dei settori disponibili. Ogni settore ha un puntatore alla lista delle proprie
* ubicazioni disponibili a pallet e una per quelle a scaffale.
*/

ep_bool_t InizializzaDati(gpointer win, GArray *settori_disponibili)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	SETTOREDISPONIBILE *Settore;
	int nIndex;
	ep_bool_t bOK=TRUE;

	/*
	* Inizializzo la lista Settori
	*/
	while((Settore=link_pop(&(priv->ListaSettori)))!=NULL){
		list_free(&(Settore->ListaPallet));
		list_free(&(Settore->ListaScaffali));
		free(Settore);
	}


	for(nIndex=0;nIndex<settori_disponibili->len;nIndex++){
		Settore = malloc(sizeof(SETTOREDISPONIBILE));	
		/* da fare : gestione per isola */
		Settore->nSettore = g_array_index(settori_disponibili,int,nIndex);
		Settore->nPesoAttuale = 0;
		Settore->nUbiAssegnate = 0;
		list_reset(&(Settore->ListaScaffali));
		list_reset(&(Settore->ListaPallet));
		/*
		* Aggiorna i puntatori della lista
		*/
		link_add_after(&(priv->ListaSettori),link_last(&(priv->ListaSettori)),Settore);
		/*
		* Crea liste ubicazioni a pallet e a scaffali
		*/
		InsertUbicaz(win, &(Settore->ListaScaffali),UBICAZIONE_SCAFFALE,Settore->nSettore);
		InsertUbicaz(win, &(Settore->ListaPallet),UBICAZIONE_PALLET,Settore->nSettore);
	}

	return(bOK);
}

/*
* Elimina le ubicazioni assegnate manualmente 
* dalla lista delle ubicazioni disponibili.
*/
ep_bool_t AssegnatiUtente(gpointer win, int nEquilibratura,ep_bool_t bEscludiProdottiUbicati)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
	DBresult *DBResUbi;
	char szCDPRO[128];
	char szCDUBI[128];
	char szTPUBI[128];
	char szSTATO[128];
	int nNMCPE;
	int nNMRGH;
	int nCDSET;
	int nIndex;
	int nTuples;
	STRUCT_UBICAZIONE *Ubicazione;
	SETTOREDISPONIBILE *Settore;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select bacdpro,bacdubi,banmrgh,banmcpe,batpubi,bastato from bal_art_tmp_%s_%s order by bacdpro;",Cfg.szTmpSuffix,Cfg.szTipoOrdini);
	nTuples=DBntuples(DBRes);
	for(nIndex=0;nIndex<nTuples;nIndex++){
		strcpy(szCDPRO,DBgetvalue(DBRes,nIndex,0));
		strcpy(szCDUBI,DBgetvalue(DBRes,nIndex,1));
		nNMCPE=atoi(DBgetvalue(DBRes,nIndex,2));
		nNMRGH=atoi(DBgetvalue(DBRes,nIndex,3));
		strcpy(szTPUBI,DBgetvalue(DBRes,nIndex,4));
		strcpy(szSTATO,DBgetvalue(DBRes,nIndex,5));
		/*
		* Esclude l'ubicazione anche se attivo il flag 'bEscludiProdottiUbicati'
		*/
		if(szTPUBI[0]==UBICAZIONE_MANUALE || bEscludiProdottiUbicati){
			DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select settore from ubicazioni where ubicazione='%s';",szCDUBI);
			if(DBntuples(DBResUbi)==1){
				nCDSET=atoi(DBgetvalue(DBResUbi,0,0));
			} else {
				bOK=FALSE;
				break;
			}
			DBclear(DBResUbi);

			/* trovo il settore relativo */
			Settore=link_first(&(priv->ListaSettori));
			while (Settore && Settore->nSettore!=nCDSET) 
				Settore=link_next(&(priv->ListaSettori),Settore);

			if(Settore){
				if (nEquilibratura==EQUILIBRATURA_PEZZI){
					/* pezzi */
					Settore->nPesoAttuale+=nNMCPE;
					Settore->nUbiAssegnate+=(nNMCPE>0);
				} else{
					/* righe */
					Settore->nPesoAttuale+=nNMRGH;
					Settore->nUbiAssegnate+=(nNMRGH>0);
				}

				/* ubicazione tipo pallet o scaffale */
				switch(szSTATO[0]){
					default:
					case UBICAZIONE_PALLET:
						Ubicazione=link_first(&Settore->ListaPallet);
						while (Settore && Ubicazione && strcmp(Ubicazione->szUbicazione,szCDUBI)) {
							Ubicazione=link_next(&Settore->ListaPallet,Ubicazione);
						}
						if(Ubicazione) {
							link_remove(&Settore->ListaPallet,Ubicazione);
							free(Ubicazione);
						}
					break;
					case UBICAZIONE_SCAFFALE:
						Ubicazione=link_first(&Settore->ListaScaffali);
						while (Settore && Ubicazione && strcmp(Ubicazione->szUbicazione,szCDUBI)){
							Ubicazione=link_next(&Settore->ListaScaffali,Ubicazione);
						}
						if(Ubicazione) {
							link_remove(&Settore->ListaScaffali,Ubicazione);
							free(Ubicazione);
						}
					break;
				}
			}
		}
	}
	DBclear(DBRes);

	return bOK;
}

/*
* Funzione di controllo che scrive sul file DatiPerBalance il contenuto della struttura dati
* dinamica (priv->ListaSettori).
*/
ep_bool_t CheckSettori(gpointer win)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	SETTOREDISPONIBILE *Settore;
	STRUCT_UBICAZIONE *Ubicazione;
	FILE *fp;
	int nCount;
	char szFileName[PATH_MAX];

	sprintf(szFileName,"%s/%s",Cfg.szPathData,"DatiPerBalance.trc");
	if ((fp=fopen(szFileName,"w"))==(FILE *)NULL) {
		printf("\nProblemi con l'apertura del file\n");
		exit(-1);
	}

	Settore=link_first(&(priv->ListaSettori));
	while (Settore){
		fprintf(fp,"\nSettore numero: %d\n",Settore->nSettore);
		fprintf(fp,"Peso attuale: %d\n",Settore->nPesoAttuale);
		/*
		* Stampa della lista ubicazioni a pallet
		*/
		nCount=1;
		Ubicazione=link_first(&Settore->ListaPallet);
		while (Ubicazione!=NULL){
			fprintf(fp,"%2d) Ubicazione a pallet: %s\n",nCount++,Ubicazione->szUbicazione);
			Ubicazione=link_next(&Settore->ListaPallet,Ubicazione);
		}
		fprintf(fp,"\n");
		/*
		* Stampa della lista ubicazioni a scaffale
		*/
		nCount=1;
		Ubicazione=link_first(&Settore->ListaScaffali);
		while (Ubicazione!=NULL){
			fprintf(fp,"%2d) Ubicazione a scaffale: %s\n",nCount++,Ubicazione->szUbicazione);
			Ubicazione=link_next(&Settore->ListaScaffali,Ubicazione);
		}
		Settore=link_next(&(priv->ListaSettori),Settore);
	}

	fclose(fp);
	return(bOK);
}

/*
* Scandisce balance.dbf e crea le liste di prodotti che devono essere assegnati a ubicazioni a
* pallet e a scaffale. Un prodotto va aggiunto in lista se e' del tipo della lista che si sta
* costruendo (pallet/scaffale) e non e' gia' stato assegnato a una ubicazione.
*/

ep_bool_t CreaListeProdotti(gpointer win, int nEquilibratura,ep_bool_t bEscludiProdottiUbicati)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	ep_bool_t bEscludiProdotto=TRUE;
	PRODOTTO *Prodotto;
	char szOrderField[128];
	char szCDPRO[128];
	char szCDUBI[128];
	char szTPUBI[128];
	char szSTATO[128];
	int nNMCPE;
	int nNMRGH;
	DBresult *DBRes;
	DBresult *DBResUbi;
	int nTuples;
	int nIndex;

	if (nEquilibratura==EQUILIBRATURA_PEZZI){
		strcpy(szOrderField,"banmcpe");
	} else {
		strcpy(szOrderField,"banmrgh");
	}

	list_free(&(priv->ListaProdPallet));
	list_free(&(priv->ListaProdScaffali));

	nPesoTotale=0;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select bacdpro,bacdubi,banmrgh,banmcpe,batpubi,bastato from bal_art_tmp_%s_%s order by %s;",Cfg.szTmpSuffix,Cfg.szTipoOrdini,szOrderField);
	nTuples=DBntuples(DBRes);
	for(nIndex=0;nIndex<nTuples;nIndex++){
		strcpy(szCDPRO,DBgetvalue(DBRes,nIndex,0));
		strcpy(szCDUBI,DBgetvalue(DBRes,nIndex,1));
		nNMRGH=atoi(DBgetvalue(DBRes,nIndex,2));
		nNMCPE=atoi(DBgetvalue(DBRes,nIndex,3));
		strcpy(szTPUBI,DBgetvalue(DBRes,nIndex,4));
		strcpy(szSTATO,DBgetvalue(DBRes,nIndex,5));

		/* default */
		bEscludiProdotto=FALSE;

		DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione from ubicazioni where ubicazione='%s';",szCDUBI);
		if(DBntuples(DBResUbi) && bEscludiProdottiUbicati){
			/*
			* Il prodotto e' gia ubicato e devo escluderlo dall'equilkibratura
			*/
			bEscludiProdotto=TRUE;
		}
		DBclear(DBResUbi);

		/* 
		* prodotto da assegnare in automatico 
		*/
		if(szTPUBI[0]==UBICAZIONE_AUTOMATICA && !bEscludiProdotto){
			/*
			* Posso aggiungere il prodotto alla lista dei prodotti da assegnare a pallet
			*/
			Prodotto=malloc(sizeof(PRODOTTO));
			strcpy(Prodotto->szCodProd,szCDPRO);
			if (nEquilibratura==EQUILIBRATURA_PEZZI){
				Prodotto->nPeso=nNMCPE;
			} else {
				Prodotto->nPeso=nNMRGH;
			}
			nPesoTotale+=Prodotto->nPeso;

			switch(szSTATO[0]){
				default:
				case UBICAZIONE_PALLET:
					/*
					* Prodotto che va assegnato (o e' gia' stato assegnato) a una ubicazione a pallet
					*/
					link_add_after(&(priv->ListaProdPallet),link_last(&(priv->ListaProdPallet)),Prodotto);
				break;
				case UBICAZIONE_SCAFFALE:
					/*
					* Prodotto che va assegnato (o e' gia' stato assegnato) a una ubicazione a scaffale
					*/
					link_add_after(&(priv->ListaProdScaffali),link_last(&(priv->ListaProdScaffali)),Prodotto);
				break;
			}
		}
	}
	DBclear(DBRes);

	return(bOK);
}

/*
* Partendo dal primo settore scandisce la lista settori fino ad arrivare a contenere tutta la cedola.
* Elimino dalla lista i settori in eccesso. Gestisce anche le ubicazioni in eccesso.
* Occorre stare attenti a non eliminare settori a cui e' gia' stato assegnato qualcosa manualmente.
*/

ep_bool_t SistemazioneDati(gpointer win, ep_bool_t bTuttiSettori)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	int nProdPallet=0;
	int nProdScaff=0;
	int nProdUbi=0;
	int nSettDisp=0;
	int nSettPallet=0;
	int nSettScaff=0;
	int nPalletDisp=0;
	int nScaffDisp=0;
	int nMedia=0;
	int nMediaScaff=0;
	int nMediaPallet=0;
	int nUbiAssPallet=0;
	int nUbiAssScaff=0;
	int nEccessiPallet=0;
	int nEccessiScaff=0;
	int nUbiPalletEliminate=0;
	int nUbiScaffEliminate=0;

	ep_bool_t bInsuff=FALSE;
	ep_bool_t bErr=FALSE;


	ep_bool_t bOK=TRUE;
	int nUbicazDaTogliere;
	int nTotPalletRichiesti=0;
	int nTotScaffaliRichiesti=0;
	int nTotSettScaff=0;
	int nTotSettPallet=0;
	int nPallet=0;
	int nScaffali=0;
	int nEccessiScaffali=0;
	SETTOREDISPONIBILE *Settore,*NextSettore;


	/*
	* Scandisco tutti i settori fino a che non riesco ad avere a disposizione un numero di ubicazioni
	* a pallet e a scaffali sufficiente a contenere la cedola.
	*/
	nTotPalletRichiesti=(priv->ListaProdPallet).n_link;
	nTotScaffaliRichiesti=(priv->ListaProdScaffali).n_link;
	Settore=link_first(&(priv->ListaSettori));
	while (Settore && ((nPallet < nTotPalletRichiesti) || (nScaffali < nTotScaffaliRichiesti))){
		if (Settore->ListaPallet.n_link) nTotSettPallet++;
		if (Settore->ListaScaffali.n_link) nTotSettScaff++;
		nPallet+=Settore->ListaPallet.n_link;
		nScaffali+=Settore->ListaScaffali.n_link;
		Settore=link_next(&(priv->ListaSettori),Settore);
	}
	if ((nPallet < nTotPalletRichiesti) || (nScaffali < nTotScaffaliRichiesti)){ 
		/*
		* Numero di ubicazioni richieste superiore al numero di ubicazioni disponibili nella linea.
		* Gestire l'errore!!
		*/
		gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni non sufficenti per l'equilibratura!\n");
		gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni a Pallets  Richieste : %d  Disponibili %d\n",nTotPalletRichiesti,nPallet);
		gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni a Scaffali Richieste : %d  Disponibili %d\n",nTotScaffaliRichiesti,nScaffali);
		bOK=FALSE;
	}
	if(bOK==FALSE){
		return FALSE;
	}


	if(bTuttiSettori){

		nProdPallet=(priv->ListaProdPallet).n_link;
		nProdScaff=(priv->ListaProdScaffali).n_link;
		nProdUbi=nProdPallet+nProdScaff;

		Settore=link_first(&(priv->ListaSettori));
		while (Settore){
			if (Settore->ListaPallet.n_link || Settore->ListaScaffali.n_link) nSettDisp++;
			if (Settore->ListaPallet.n_link) nSettPallet++;
			if (Settore->ListaScaffali.n_link) nSettScaff++;
			nPalletDisp+=Settore->ListaPallet.n_link;
			nScaffDisp+=Settore->ListaScaffali.n_link;
			Settore=link_next(&(priv->ListaSettori),Settore);
		}

		nEccessiPallet=nPalletDisp-nProdPallet;
		nEccessiScaff=nScaffDisp-nProdScaff;

		if ((nPalletDisp < nProdPallet) || (nScaffDisp < nProdScaff)){ 
			/*
			* Numero di ubicazioni richieste superiore al numero di ubicazioni disponibili nella linea.
			* Gestire l'errore!!
			*/
			gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni non sufficenti per l'equilibratura!\n");
			gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni a Pallets  Richieste : %d  Disponibili %d\n",nProdPallet,nPalletDisp);
			gtk_text_printf("RED",priv->txt_msgs,"Ubicazioni a Scaffali Richieste : %d  Disponibili %d\n",nProdScaff,nScaffDisp);
			bOK=FALSE;
		} 
		if(bOK==FALSE){
			return FALSE;
		}

		Settore=link_first(&(priv->ListaSettori));
		while (Settore){
			/*
			* Calcolo dei valori medi
			*/
			if (nSettDisp)
				nMedia=(int)ceil((float)nProdUbi/(float)nSettDisp);
			else nMedia=0;
			if (nSettScaff)
				nMediaScaff=(int)ceil((float)nProdScaff/(float)nSettScaff);
			else nMediaScaff=0;
			if (nSettPallet)
				nMediaPallet=(int)ceil((float)nProdPallet/(float)nSettPallet);
			else nMediaPallet=0;

			/*
			* Kernel. Pezzo di codice che decide quante ubicazioni assegnare
			*/
			if (Settore->ListaScaffali.n_link){
				/*
				* Il settore dispone di ubicazioni a scaffale
				* Determino il numero degli scaffali da impiegare in questo settore
				*/
				if (Settore->ListaScaffali.n_link>=nMediaScaff)
					nUbiAssScaff=nMediaScaff;
				else nUbiAssScaff=Settore->ListaScaffali.n_link;
				/*
				* Controllo per evitare di eliminare ubicazioni che comporterebbero
				* una insufficienza delle ubicazioni
				*/
				nUbiScaffEliminate=Settore->ListaScaffali.n_link-nUbiAssScaff;
				if (nUbiScaffEliminate>nEccessiScaff)
					nUbiAssScaff=Settore->ListaScaffali.n_link-nEccessiScaff;

				/*
				* Determino il numero dei pallet da impiegare in questo settore
				* 17/09/99 DR Modifica calcolo della distribuzione delle ubicazioni utilizzate
				* Si cerca di avere lo stesso numero di ubicazioni a pallet utilizzate in ogni settore
				*/
				if (Settore->ListaPallet.n_link>=nMediaPallet)
					nUbiAssPallet=nMediaPallet;
				else nUbiAssPallet=Settore->ListaPallet.n_link;
				if (nUbiAssPallet<0) nUbiAssPallet=0;
				/*
				* Controllo per evitare di eliminare ubicazioni che comporterebbero
				* una insufficienza delle ubicazioni
				*/
				nUbiPalletEliminate=Settore->ListaPallet.n_link-nUbiAssPallet;
				if (nUbiPalletEliminate>nEccessiPallet)
					nUbiAssPallet=Settore->ListaPallet.n_link-nEccessiPallet;
			}
			else{
				/*
				* Il settore non dispone di ubicazioni a scaffale
				* Tento di assegnare a pallet nMedia ubicazioni
				*/
				nUbiAssScaff=0;
				nUbiScaffEliminate=0;
				if (Settore->ListaPallet.n_link>=nMedia)
					nUbiAssPallet=nMedia;
				else nUbiAssPallet=Settore->ListaPallet.n_link;
				/*
				* Controllo per evitare di eliminare ubicazioni che comporterebbero
				* una insufficienza delle ubicazioni
				*/
				nUbiPalletEliminate=Settore->ListaPallet.n_link-nUbiAssPallet;
				if (nUbiPalletEliminate>nEccessiPallet)
					nUbiAssPallet=Settore->ListaPallet.n_link-nEccessiPallet;
			}

			/*
			* Controllo che non si tenti di assegnare un numero di ubicazioni maggiore
			* del numero dei prodotti da ubicare
			*/
			if (nUbiAssPallet>nProdPallet)
				nUbiAssPallet=nProdPallet;
			if (nUbiAssScaff>nProdScaff)
				nUbiAssScaff=nProdScaff;

			/*
			* Aggiornamento delle variabili in uso
			*/
			nSettDisp--;
			nUbiPalletEliminate=Settore->ListaPallet.n_link-nUbiAssPallet;
			nUbiScaffEliminate=Settore->ListaScaffali.n_link-nUbiAssScaff;
			nEccessiPallet-=nUbiPalletEliminate;
			nEccessiScaff-=nUbiScaffEliminate;
			nProdPallet-=nUbiAssPallet;
			nProdScaff-=nUbiAssScaff;
			nProdUbi-=(nUbiAssPallet+nUbiAssScaff);
			if (Settore->ListaPallet.n_link) nSettPallet--;
			if (Settore->ListaScaffali.n_link) nSettScaff--;
			nPalletDisp-=Settore->ListaPallet.n_link;
			nScaffDisp-=Settore->ListaScaffali.n_link;
			/*
			* Controllo che il codice non abbia prodotto errori
			*/
			if (nSettDisp<0 || nUbiPalletEliminate<0 || nUbiScaffEliminate<0 ||
					nEccessiPallet<0 || nEccessiScaff<0 || nProdPallet<0 ||
					nProdScaff<0 || nProdUbi<0 || nSettPallet<0 || nSettScaff<0 ||
					nPalletDisp<0 || nScaffDisp<0){
				bErr=TRUE;
			}

#ifdef TRACE
			if (nSettDisp<0)           trace_debug(FALSE, TRUE, "nSettDisp           %d",nSettDisp);
			if (nUbiPalletEliminate<0) trace_debug(FALSE, TRUE, "nUbiPalletEliminate %d",nUbiPalletEliminate);
			if (nUbiScaffEliminate<0)  trace_debug(FALSE, TRUE, "nUbiScaffEliminate  %d",nUbiScaffEliminate);
			if (nEccessiPallet<0)      trace_debug(FALSE, TRUE, "nEccessiPallet      %d",nEccessiPallet);
			if (nEccessiScaff<0)       trace_debug(FALSE, TRUE, "nEccessiScaff       %d",nEccessiScaff);
			if (nProdPallet<0)         trace_debug(FALSE, TRUE, "nProdPallet         %d",nProdPallet);
			if (nProdScaff<0)          trace_debug(FALSE, TRUE, "nProdScaff          %d",nProdScaff);
			if (nProdUbi<0)            trace_debug(FALSE, TRUE, "nProdUbi            %d",nProdUbi);
			if (nSettPallet<0)         trace_debug(FALSE, TRUE, "nSettPallet         %d",nSettPallet);
			if (nSettScaff<0)          trace_debug(FALSE, TRUE, "nSettScaff          %d",nSettScaff);
			if (nPalletDisp<0)         trace_debug(FALSE, TRUE, "nPalletDisp         %d",nPalletDisp);
			if (nScaffDisp<0)          trace_debug(FALSE, TRUE, "nScaffDisp          %d",nScaffDisp);
#endif
					
			/*
			* Eliminazione dalle liste (pallet e scaffale) del settore delle ubicazioni in eccesso
			*/
			EliminaUbicazioni(win, &Settore->ListaPallet,Settore->ListaPallet.n_link-nUbiAssPallet);
			EliminaUbicazioni(win, &Settore->ListaScaffali,Settore->ListaScaffali.n_link-nUbiAssScaff);

			Settore=link_next(&(priv->ListaSettori),Settore);
		}
	} else {
		/*
		* Eliminazione dei settori che non hanno ubicazioni a cui devono essere assegnati dei prodotti
		* dall'euristico.
		*/
		while (Settore){
			NextSettore=link_next(&(priv->ListaSettori),Settore);
			link_remove(&(priv->ListaSettori),Settore);
			free(Settore);
			Settore=NextSettore;
		}
		/*
		* Eliminazione delle ubicazioni in eccesso in maniera distribuita cercando di fare in modo che per
		* ciascun settore si usino lo stesso numero di ubicazioni a pallet e a scaffale per quanto 
		* possibile.
		*/
		nEccessiPallet=nPallet-nTotPalletRichiesti;
		nEccessiScaffali=nScaffali-nTotScaffaliRichiesti;
		Settore=link_first(&(priv->ListaSettori));
		while (Settore){
			if (Settore->ListaPallet.n_link && nEccessiPallet){
				/*
				* Ci sono ubicazioni a pallet da eliminare nel settore
				*/
				nUbicazDaTogliere=max(0,Settore->ListaPallet.n_link-(int)(nTotPalletRichiesti/nTotSettPallet));

				nUbicazDaTogliere=min(nUbicazDaTogliere,min(Settore->ListaPallet.n_link,nEccessiPallet));

				EliminaUbicazioni(win, &Settore->ListaPallet,nUbicazDaTogliere);

				nEccessiPallet-=nUbicazDaTogliere;
				nTotPalletRichiesti-=Settore->ListaPallet.n_link;
				nTotSettPallet--;
			}
			if (Settore->ListaScaffali.n_link && nEccessiScaffali){
				/*
				* Ci sono ubicazioni a scaffale da eliminare nel settore
				*/
				nUbicazDaTogliere=Settore->ListaScaffali.n_link-(int)(nTotScaffaliRichiesti/nTotSettScaff);

				nUbicazDaTogliere=min(nUbicazDaTogliere,min(Settore->ListaScaffali.n_link,nEccessiScaffali));

				EliminaUbicazioni(win, &Settore->ListaScaffali,nUbicazDaTogliere);

				nEccessiScaffali-=nUbicazDaTogliere;
				nTotScaffaliRichiesti-=Settore->ListaScaffali.n_link;
				nTotSettScaff--;
			}
			Settore=link_next(&(priv->ListaSettori),Settore);
		}
	}



	return(bOK);
}

/*
* Lancia tutti gli euristici, mantenedo in memoria le soluzioni (su lista). Valuta la migliore
* in base allo scarto quadratico medio e aggiorna balance completando cosi' il processo di 
* bilanciamento.
*/

ep_bool_t CreaSoluzioni(gpointer win)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	SOLUZIONE *Soluzione;


	/*
	* Queste tre righe di codice sono le uniche che devono essere aggiunte per aggiungere un euristico
	* oltre che, naturalmente, la funzione dell'euristico.
	*/
	Soluzione=malloc(sizeof(SOLUZIONE));
	list_reset(&(Soluzione->ListaAssociazioni));
	link_add(&(priv->ListaSoluzioni),Soluzione);
	Euristico1(win, Soluzione);

	return(bOK);
}



/*
* Le funzioni che implementano gli euristici devono avere anche l'accortezza di fare due cose:
*	 - come prima operazione lanciare la funzione CreaCopia() per duplicare le liste prodotti a
*    pallet e a scaffale, dato che queste vengono cancellate dall'euristico stesso
*  - lanciare la funzione di calcolo della varianza
*  - liberare la memoria allocata per le liste copiate.
*/

ep_bool_t Euristico1(gpointer win, SOLUZIONE *Soluzione)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	LINKED_LIST ProdPallet,ProdScaffali,LSettori;
	SETTOREDISPONIBILE *Settore;
	int nUbicazioni;
	int nVMS,nVMU;	/* Valor Medio per Settore, Valor Medio per Ubicazione */

	if((priv->ListaSettori).n_link==0){
		return FALSE;
	}

	list_reset(&ProdPallet);
	list_reset(&ProdScaffali);
	list_reset(&LSettori);

	CreaCopia(win, &ProdPallet,&ProdScaffali,&LSettori);

	/*
	* Esamino un settore alla volta fino a esaurimento settori.
	* Per ogni settore considero tutte le ubicazioni e associo a ciascuna un prodotto prestando
	* attenzione al tipo (pallet/scaffale).
	*/

	nVMS=nPesoTotale/LSettori.n_link;

	Settore=NULL;
	while((Settore=link_next(&LSettori,Settore))!=NULL){
		if((nUbicazioni=Settore->ListaPallet.n_link+Settore->ListaScaffali.n_link)){
			nVMU=(int)(nVMS-Settore->nPesoAttuale)/nUbicazioni;
			RiempiUbicazione(win, &ProdScaffali,&(Settore->ListaScaffali),&(Soluzione->ListaAssociazioni),Settore,nVMU);
			RiempiUbicazione(win, &ProdPallet,&(Settore->ListaPallet),&(Soluzione->ListaAssociazioni),Settore,nVMU);
		}
	}
	Soluzione->nVarianza=Varianza(win, nVMS,&LSettori);

	/*
	* Libero l'area di memoria occupata dalla lista dei settori e dei prodotti su pallet e scaffali
	*/
	list_free(&LSettori);
	list_free(&ProdScaffali);
	list_free(&ProdPallet);

	link_add(&(priv->ListaSoluzioni),Soluzione);

	return(bOK);
}


ep_bool_t InsertUbicaz(gpointer win, LINKED_LIST *pLista, char cStato, int nSettore)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	DBresult *DBRes;
	STRUCT_UBICAZIONE *Ubicazione;
	ep_bool_t bOK=TRUE;
	int nIndex;
	int nTuples;
	char szCDUBI[40];
	char szSTATO[40];

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione,cnistato from ubicazioni where settore=%d order by priorita;",nSettore);
	nTuples=DBntuples(DBRes);
	for(nIndex=0;nIndex<nTuples;nIndex++){
		strcpy(szCDUBI,DBgetvalue(DBRes,nIndex,0));
		strcpy(szSTATO,DBgetvalue(DBRes,nIndex,1));
		if(szSTATO[0]==cStato){
			Ubicazione=malloc(sizeof(UBICAZIONE));
			strcpy(Ubicazione->szUbicazione,szCDUBI);
			link_add_after(pLista,link_last(pLista),Ubicazione);	
		}
	}
	DBclear(DBRes);

	return(bOK);
}

/*
* Scandisce la lista delle soluzioni e valuta la migliore in base alla varianza, quindi aggiorna
* balance.dbf.
*/

ep_bool_t ScegliMigliore(gpointer win)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	ep_bool_t bOK=TRUE;
	SOLUZIONE *Soluzione,*BestSoluz;
	ASSOCIAZIONE *Associazione;
	LINKED_LIST *Lista;
	int nBest;
	char szCDUBI[40];
	char szCDPRO[40];
	int nCDSET;
	DBresult *DBRes;

	Soluzione=link_first(&(priv->ListaSoluzioni));
	nBest=Soluzione->nVarianza;
	BestSoluz=Soluzione;
	while (Soluzione){
		if (Soluzione->nVarianza<nBest){
			nBest=Soluzione->nVarianza;
			BestSoluz=Soluzione;
		}
		Soluzione=link_next(&(priv->ListaSoluzioni),Soluzione);
	}
	/*
	* Aggiorno la tmp table
	*/
	Lista=&(BestSoluz->ListaAssociazioni);
	Associazione=link_first(Lista);
	while (Associazione){
		strcpy(szCDUBI,Associazione->szUbicazione);
		nCDSET=Associazione->nSettore;
		strcpy(szCDPRO,Associazione->szCodProd);
		/*
		* Cerco in tmp table il codice prodotto e gli associo l'ubicazione
		*/
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set bacdubi='%s',bacdset=%d where bacdpro='%s';", Cfg.szTmpSuffix,Cfg.szTipoOrdini, szCDUBI,nCDSET,szCDPRO);
		if (DBresultStatus(DBRes) != DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==0) {
			bOK=FALSE;
			gtk_text_printf("RED",priv->txt_msgs,"Codice [%s] Ubicazione [%s] non assegnata\n",szCDPRO,szCDUBI);
		}
		DBclear(DBRes);

		Associazione=link_next(Lista,Associazione);
	}

	return(bOK);
}

/*
* Elimina le ultime nNum ubicazioni dalla lista
*/

ep_bool_t EliminaUbicazioni(gpointer win, LINKED_LIST *Lista,int nNum)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	STRUCT_UBICAZIONE *Ubicazione,*PrevUbicazione;
	int nCounter=0;
	ep_bool_t bOK=TRUE;

	Ubicazione=link_last(Lista);
	PrevUbicazione=link_prev(Lista,Ubicazione);
	while (nCounter++<nNum){
		link_remove(Lista,Ubicazione);
		free(Ubicazione);
		Ubicazione=PrevUbicazione;
		PrevUbicazione=link_prev(Lista,Ubicazione);
	}
	return(bOK);
}

/*
* Crea una copia delle liste dei prodotti da assegnare a pallet e a scaffale
* e una copia delle ubicazioni a settore
*/
ep_bool_t CreaCopia(gpointer win, LINKED_LIST *Pallet,LINKED_LIST *Scaffali,LINKED_LIST *CopiaListaSettori)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	PRODOTTO *Prodotto,*CopiaProdotto;
	SETTOREDISPONIBILE *Settore,*CopiaSettore;
	STRUCT_UBICAZIONE *Ubicazione,*CopiaUbicazione;
	ep_bool_t RetVal=TRUE;

	Prodotto=link_first(&(priv->ListaProdPallet));
	while (Prodotto!=NULL){
		CopiaProdotto=malloc(sizeof(PRODOTTO));	
		strcpy(CopiaProdotto->szCodProd,Prodotto->szCodProd);
		CopiaProdotto->nPeso=Prodotto->nPeso;
		link_add_after(Pallet,link_last(Pallet),CopiaProdotto);
		Prodotto=link_next(&(priv->ListaProdPallet),Prodotto);
	}

	Prodotto=link_first(&(priv->ListaProdScaffali));
	while (Prodotto!=NULL){
		CopiaProdotto=malloc(sizeof(PRODOTTO));	
		strcpy(CopiaProdotto->szCodProd,Prodotto->szCodProd);
		CopiaProdotto->nPeso=Prodotto->nPeso;
		link_add_after(Scaffali,link_last(Scaffali),CopiaProdotto);
		Prodotto=link_next(&(priv->ListaProdScaffali),Prodotto);
	}

	Settore=link_first(&(priv->ListaSettori));
	while (Settore!=NULL){
		CopiaSettore=malloc(sizeof(SETTOREDISPONIBILE));	
		CopiaSettore->nSettore=Settore->nSettore;
		CopiaSettore->nPesoAttuale=Settore->nPesoAttuale;
		CopiaSettore->nUbiAssegnate=Settore->nUbiAssegnate;
		list_reset(&(CopiaSettore->ListaScaffali));
		list_reset(&(CopiaSettore->ListaPallet));
		/*
		* Copia la lista delle ubicazioni a scaffale
		*/
		Ubicazione=link_first(&Settore->ListaScaffali);
		while (Ubicazione!=NULL){
			CopiaUbicazione=malloc(sizeof(UBICAZIONE));
			strcpy(CopiaUbicazione->szUbicazione,Ubicazione->szUbicazione);
			link_add(&CopiaSettore->ListaScaffali,CopiaUbicazione);
			Ubicazione=link_next(&Settore->ListaScaffali,Ubicazione);
		}
		/*
		* Copia la lista delle ubicazioni a pallet
		*/
		Ubicazione=link_first(&Settore->ListaPallet);
		while (Ubicazione!=NULL){
			CopiaUbicazione=malloc(sizeof(UBICAZIONE));
			strcpy(CopiaUbicazione->szUbicazione,Ubicazione->szUbicazione);
			link_add_after(&CopiaSettore->ListaPallet,link_last(&CopiaSettore->ListaPallet),CopiaUbicazione);
			Ubicazione=link_next(&Settore->ListaPallet,Ubicazione);
		}
		link_add(CopiaListaSettori,CopiaSettore);
		Settore=link_next(&(priv->ListaSettori),Settore);
	}

	return(RetVal);
}

/*
* Questa funzione associa a ciascuna ubicazione di un certo tipo (pallet o scaffale) del settore
* passatogli, un prodotto estratto dalla lista prodotti, cercando di equlibrare per quanto 
* possibile i settori.
*/

ep_bool_t RiempiUbicazione(gpointer win, LINKED_LIST *ListaProdotti, LINKED_LIST *ListaUbicazioni, LINKED_LIST *ListaAssociazioni, SETTOREDISPONIBILE *Settore, int nVMU)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
    ep_bool_t RetVal=TRUE;
	STRUCT_UBICAZIONE *Ubicazione,*NextUbicazione;
	PRODOTTO *Prodotto,*SelProd;
	ASSOCIAZIONE *Associazione;
	int nPesoIdeale;
	long int nValoreOttimo;

    Ubicazione=link_first(ListaUbicazioni);
    while (Ubicazione){
        /*
        * - cercare il codice prodotto dalla lista prodotti a scaffale da assegnare all'ubicazione
        * - creare una nuova associazione e aggiornarla
        * - linkare la nuova associazione alla lista delle associazioni di questa soluzione
        * - eliminare l'ubicazione dalla lista delle ubicazioni e liberare la memoria
        * - eliminare il codice prodotto dalla lista dei prodotti a scaffali e liberare la memoria
        */
        nPesoIdeale=nVMU*(Settore->nUbiAssegnate+1)-Settore->nPesoAttuale;
        Settore->nUbiAssegnate++;
        Prodotto=link_first(ListaProdotti);
        nValoreOttimo=pow((Prodotto->nPeso-nPesoIdeale),2);
        SelProd=Prodotto;
        while (Prodotto){
            if (nValoreOttimo>pow((Prodotto->nPeso-nPesoIdeale),2)){
                nValoreOttimo=pow((Prodotto->nPeso-nPesoIdeale),2);
                SelProd=Prodotto;
            }
            Prodotto=link_next(ListaProdotti,Prodotto);
        }
        /*StampaProdottoScelto(SelProd,fp);*/
        Associazione=malloc(sizeof(ASSOCIAZIONE));
        strcpy(Associazione->szCodProd,SelProd->szCodProd);
        strcpy(Associazione->szUbicazione,Ubicazione->szUbicazione);
        Associazione->nSettore = Settore->nSettore;
        Associazione->nPeso = SelProd->nPeso;
        link_add(ListaAssociazioni,Associazione);
        NextUbicazione=link_next(ListaUbicazioni,Ubicazione);
        link_remove(ListaUbicazioni,Ubicazione);
        free(Ubicazione);
        Ubicazione=NextUbicazione;
        Settore->nPesoAttuale+=SelProd->nPeso;
        link_remove(ListaProdotti,SelProd);
        free(SelProd);
    }
    return(RetVal);
}

/*
* Calcolo della varianza. 
*/

int Varianza(gpointer win, int nVMS,LINKED_LIST *ListaSettori)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	double nSigma=0;
	SETTOREDISPONIBILE *Settore;
	
	Settore=link_first(ListaSettori);
	while(Settore){
		nSigma+=sqrt(pow((Settore->nPesoAttuale-nVMS),2));
		Settore=link_next(ListaSettori,Settore);
	}
	return(nSigma);
}

/* 
* assegna le ubicazioni manuali ai prodotti di fuori formato
*/
void AssegnaUbicazioniManuali(gpointer win)
{
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));
	DBresult *DBResArticoli;
	DBresult *DBResCat;
	DBresult *DBResUbiMan;
	DBresult *DBResUpdate;
	int nIndex;
	int nTuples;
	int nQTFFO;
	int nSWFFO;
	char szCDUBI[128];
	char szSTATO[128];
	char szCDFLG[128];
	int nCDSET;
	char szTPUBI[128];
	int nUbicazioniManuali;
	int nUbicazioniManualiUtilizzate;
	/*
	* Seleziono le ubicazioni manuali per uso successivo
	*/
	DBResUbiMan=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione,settore,cnistato,ubcdflg from ubicazioni where ubcdflg='%c' order by fila(ubicazione),montante(ubicazione)::int2,colonna(ubicazione)::int2,piano(ubicazione)::int2;", UBICAZIONE_MANUALE);
	nUbicazioniManuali=DBntuples(DBResUbiMan);
	nUbicazioniManualiUtilizzate=0;

	/* ricavo i dati da catalogo */
	DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"select bacdpro from bal_art_tmp_%s_%s;",Cfg.szTmpSuffix,Cfg.szTipoOrdini);
	nTuples=DBntuples(DBResArticoli);
	for(nIndex=0;nIndex<nTuples;nIndex++){

		DBResCat=DBExecQuery(Cfg.nDebugLevel>1,"select prswffo,prqtffo,prqtpcf,prqtpcp,prfcpcf from catalogo where prcdpro='%s';", DBgetvalue(DBResArticoli,nIndex,0));
		if(DBntuples(DBResCat)){
			nSWFFO=atoi(DBgetvalue(DBResCat,0,0));
			nQTFFO=atoi(DBgetvalue(DBResCat,0,1));
			/* 
			* fuori formato 
			* metto in ubicazione manuale
			*/
			if(nSWFFO){
				if(nUbicazioniManualiUtilizzate+1<nUbicazioniManuali){
					/* ubico in ubicazione manuale */
					strcpy(szCDUBI, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,0));
					nCDSET=atoi(DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,1));
					strcpy(szSTATO, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,2));
					strcpy(szCDFLG, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,3));
					strcpy(szTPUBI, "M");

					nUbicazioniManualiUtilizzate++;

					/* aggiorno ubicazione in tabella temporanea */
					DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set bacdubi='%s', bacdset=%d, bastato='%s' , bacdflg='%s', batpubi='%s' where bacdpro='%s';",
						Cfg.szTmpSuffix,
						Cfg.szTipoOrdini,
						szCDUBI,                          /* CDUBI */
						nCDSET,                           /* CDSET */
						szSTATO,                          /* STATO */
						szCDFLG,                          /* CDFLG */
						szTPUBI,                          /* TPUBI */
						DBgetvalue(DBResArticoli,nIndex,0)); 
					DBclear(DBResUpdate);

				} else {
					trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,"Ubicazioni manuali insufficienti [%d]",nUbicazioniManuali);
				}
			}
		} else {
			trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,"Lista Articoli : Articolo [%s] non presente in catalogo",DBgetvalue(DBResArticoli,nIndex,0));
		}
		DBclear(DBResCat);
	}
	DBclear(DBResArticoli);

	/* libero la DBResUbi */
	DBclear(DBResUbiMan);
}

/*
* do_carico_settori()
* costruisce la finestra di carico (copie prelevate) diviso per settori
*/
void do_carico_settori(gpointer win)
{
	DBresult *DBRes;
	DBresult *DBResUbi;
	DBresult *DBResInsert;
	DBresult *DBResUpdate;
	int nSettore[20];
	GtkWidget *lb_widget;
	GtkWidget *level_widget;
	GtkWidget *rl_settori;
	char szSettore[128];
	int nMaxValue=0;
	int nIndex;
	int nSettoreIndex;
	int nTuples;
    BalanceWindowPrivate *priv= balance_window_get_instance_private (BALANCE_WINDOW (win));

	/* 
	* Gestisce anche il settore 0 (Manuale)
	*/
	for(nSettoreIndex=0;nSettoreIndex<Cfg.nNumeroSettori+1;nSettoreIndex++){
		nSettore[nSettoreIndex]=0;

		sprintf(szSettore,"level_settore_%02d",nSettoreIndex);
		if((level_widget=find_child(GTK_WIDGET(win),szSettore))){
			gtk_level_bar_set_value (GTK_LEVEL_BAR(level_widget), (gdouble)nSettore[nSettoreIndex]);
		}

		sprintf(szSettore,"lb_settore_%02d",nSettoreIndex);
		if((lb_widget=find_child(GTK_WIDGET(win),szSettore))){
			gtk_label_printf(lb_widget,"%5d",nSettore[nSettoreIndex]);
		}
	}

	/*
	* seleziono gli ordini ancora da terminare (' '/S/R/P)
	*/

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where rostato in ('%c','%c','%c','%c','%c') and ordtipo='%s';",
		Cfg.szTmpSuffix,
		Cfg.szTipoOrdini,
		ORDINE_IN_PRELIEVO,
		ORDINE_STAMPATA_RAC,
		ORDINE_ELABORATO,
		ORDINE_SPEDITO,
		ORDINE_RICEVUTO,
		Cfg.szTipoOrdini);

	DBclear(DBRes);
	/* 
	* Cancellazione e creazione tabella sel_art_tmp_[SUFFIX]_[TIPO_ORDINI] 
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_art_tmp_%s_%s ( SACDPRO text, SACDUBI text, SANMRGH integer, SANMCPE integer);", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.racdpro,count(r.racdpro),sum(r.raqtord) from ric_art r,sel_ord_tmp_%s_%s s where r.ordprog=s.ordprog group by r.racdpro;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nTuples=DBntuples(DBRes);
	for(nIndex=0;nIndex<nTuples;nIndex++){
		DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_art_tmp_%s_%s (sacdpro,sanmrgh,sanmcpe) values ('%s',%d,%d);",
			Cfg.szTmpSuffix,
			Cfg.szTipoOrdini,
			DBgetvalue(DBRes,nIndex,0), 
			atoi(DBgetvalue(DBRes,nIndex,1)), 
			atoi(DBgetvalue(DBRes,nIndex,2)));
		DBclear(DBResInsert);

		DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione from ubicazioni where ubitipo = '%s' and codprod='%s' order by priorita;", Cfg.szTipoOrdini,DBgetvalue(DBRes,nIndex,0));
		if(DBntuples(DBResUbi)){
			/*
			* Prendo la prima ubicazione in ordine di priorita
			*/
			DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update sel_art_tmp_%s_%s set sacdubi='%s' where sacdpro='%s';",
				Cfg.szTmpSuffix,
				Cfg.szTipoOrdini,
				DBgetvalue(DBResUbi,0,0), 
				DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResUpdate);
		} else {
			gtk_text_printf("RED",find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Prodotto [%s] non ubicato !\n", DBgetvalue(DBRes,nIndex,0));
		}
		DBclear(DBResUbi);
	}

	DBclear(DBRes);

	/*
	* Seleziono le qta per settore
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select u.settore,sum(s.sanmcpe) from ubicazioni as u,sel_art_tmp_%s_%s as s where u.ubicazione=s.sacdubi group by u.settore order by u.settore;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	if((nTuples=DBntuples(DBRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			nSettoreIndex=max(atoi(DBgetvalue(DBRes,nIndex,0)),0);

			nSettore[nSettoreIndex]+=atoi(DBgetvalue(DBRes,nIndex,1));
			nMaxValue=max(nMaxValue,nSettore[nSettoreIndex]);
		}

		/*
		* Lascio un 10% vuoto in cima
		*/
		nMaxValue+=nMaxValue/10;

		for(nIndex=0;nIndex<nTuples;nIndex++){
			nSettoreIndex=atoi(DBgetvalue(DBRes,nIndex,0));

			sprintf(szSettore,"lb_settore_%02d",nSettoreIndex);
			if((lb_widget=find_child(GTK_WIDGET(win),szSettore))){ 
				gtk_label_printf(lb_widget,"%d",nSettore[nSettoreIndex]); 
			}

			sprintf(szSettore,"level_settore_%02d",nSettoreIndex);
			if((level_widget=find_child(GTK_WIDGET(win),szSettore))){
				gtk_level_bar_set_max_value (GTK_LEVEL_BAR(level_widget),(gdouble)nMaxValue);
				gtk_level_bar_set_value(GTK_LEVEL_BAR(level_widget),(gdouble)nSettore[nSettoreIndex]);
			}
		}
	}

	DBclear(DBRes);
}


static void create_carico_settori (gpointer win, GtkWidget *container)
{
    GtkWidget *grid;
    GtkWidget *rl_settori;
    GtkWidget *level_settore;
    GtkWidget *lb;
    GtkWidget *hsep;
    GtkWidget *vbox;
    GtkWidget *hbox;
	int nIndex;
	char szBuffer[128];

	/*
	grid 3 Righe N Colonne
		1 PR_SETTORE_XX
		2 H SEPARATOR
		3 VBOX 3 Righe
			LB_SETTORE_XX
			H SEP
			LB_SETTORE_XX_TITLE
	*/
		

	/*
	* Colonne : Numero Settori + 1 (ruler) + 1 (manuali)
	*/
    // grid = gtk_grid_new (3, Cfg.nNumeroSettori+2, FALSE);
    /*
    grid = gtk_grid_new ();
    gtk_widget_set_name (grid, "grid");
    gtk_grid_set_column_homogeneous (GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous    (GTK_GRID(grid), TRUE);
    g_object_ref (G_OBJECT(grid));

    gtk_widget_show (grid);
    gtk_container_add (GTK_CONTAINER (container), grid);
    */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    sprintf(szBuffer,"hb_settori");
    gtk_widget_set_name (hbox, szBuffer);
    gtk_box_set_homogeneous (GTK_BOX(hbox), TRUE);
    g_object_ref (hbox);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (container), hbox);

	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){

		/* V Box */
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
		sprintf(szBuffer,"vb_settore_%02d",nIndex);
		gtk_widget_set_name (vbox, szBuffer);
		g_object_ref (vbox);
		gtk_widget_show (vbox);
		// gtk_grid_attach (GTK_GRID (grid), vbox, nIndex, 2, 1, 1);
		gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
		
		/* Level Bar */
		level_settore = gtk_level_bar_new ();
		sprintf(szBuffer,"level_settore_%02d",nIndex);
		gtk_widget_set_name (level_settore, szBuffer);
        gtk_level_bar_set_inverted(GTK_LEVEL_BAR(level_settore), TRUE);
        gtk_orientable_set_orientation (GTK_ORIENTABLE(level_settore), GTK_ORIENTATION_VERTICAL);

		g_object_ref (G_OBJECT(level_settore));

		gtk_widget_show (level_settore);
		// gtk_grid_attach (GTK_GRID (grid), level_settore, nIndex, 0, 1, 1);
		gtk_box_pack_start (GTK_BOX (vbox), level_settore, TRUE, TRUE, 0);

		g_object_unref (G_OBJECT(level_settore));

		/* H Separator */
		hsep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		sprintf(szBuffer,"hsep_sett_%02d",nIndex);
		gtk_widget_set_name (hsep, szBuffer);
		g_object_ref (hsep);
		gtk_widget_show (hsep);
		// gtk_grid_attach (GTK_GRID (grid), hsep, nIndex, 1, 1, 1);
		gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
		g_object_unref (hsep);


		/* Label */
		lb = gtk_label_new ("");
		sprintf(szBuffer,"lb_settore_%02d",nIndex);
		gtk_widget_set_name (lb, szBuffer);
		g_object_ref (lb);
		gtk_widget_show (lb);
		gtk_box_pack_start (GTK_BOX (vbox), lb, FALSE, FALSE, 0);
		g_object_unref (lb);

		/* H Separator */
		hsep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		sprintf(szBuffer,"hsep_settore_%02d",nIndex);
		gtk_widget_set_name (hsep, szBuffer);
		g_object_ref (hsep);
		gtk_widget_show (hsep);
		gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
		g_object_unref (hsep);

		/* Label Title */
		if(nIndex){
			sprintf(szBuffer,"S.%2d",nIndex);
		} else {
			sprintf(szBuffer,"MANUALE");
		}
		lb = gtk_label_new (szBuffer);
		sprintf(szBuffer,"lb_settore_%02d_title",nIndex);
		gtk_widget_set_name (lb, szBuffer);
		g_object_ref (lb);
		gtk_widget_show (lb);
		gtk_box_pack_start (GTK_BOX (vbox), lb, FALSE, FALSE, 0);
		g_object_unref (lb);
		g_object_unref (vbox);
	}
    g_object_unref (hbox);
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gchar *name;

    g_object_get (G_OBJECT(action), "name", &name, NULL);
    g_printf("action [%s] activated\n", name);
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void seleziona_tutti_settori_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);

	ep_bool_t bFine=FALSE;
	ep_bool_t bState=TRUE;
	char szToggleButtonName[128];

	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->cb_select_tutti_settori))){
		bState=TRUE;
	} else {
		bState=FALSE;
	}
    for (int i=0; i < Cfg.nNumeroSettori; i++){
		sprintf(szToggleButtonName,"tb_settore_%02d",i+1);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(find_child(GTK_WIDGET(win), szToggleButtonName)), bState);
	}
}

void do_conferma_equilibratura(gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);
	DBresult *DBRes;
	DBresult *DBResUpdate;
	ep_bool_t bOK=TRUE;
	int nTuples;
	int nIndex;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->cb_vuota_ubicazioni))){
		/* cancello le ubicazioni - 14-10-2002 rm : solo se richiesto */
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set codprod='' where ubitipo='%s';",Cfg.szTipoOrdini);
		if (DBresultStatus(DBRes) != DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==0) {
			gtk_text_printf("RED",priv->txt_msgs,"Errore in aggiornamento ubicazioni\n");
			bOK=FALSE;
		}
		DBclear(DBRes);
	}

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select bacdubi,bacdpro from bal_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	if((nTuples=DBntuples(DBRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set codprod='%s' where ubicazione='%s';",
				DBgetvalue(DBRes,nIndex,1),
				DBgetvalue(DBRes,nIndex,0));
			if (DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK && atoi(DBcmdTuples(DBResUpdate))==0) {
				gtk_text_printf("RED",priv->txt_msgs,"Errore in aggiornamento ubicazione [%s]\n",DBgetvalue(DBRes,nIndex,0));
				bOK=FALSE;
			}
			DBclear(DBResUpdate);
		}
	}
	DBclear(DBRes);

	if(bOK){
		gtk_text_printf("GREEN",priv->txt_msgs,"Aggiornamento ubicazioni avvenuto correttamente\n");
	} else {
		gtk_text_printf("RED",priv->txt_msgs,"Si sono verificati errori in aggiornamento ubicazioni\n");
	}
}

static void balance_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);
	int nTipoEquilibratura;
	ep_bool_t bTuttiSettoriSelezionati=FALSE;
	ep_bool_t bEscludiProdottiUbicati=FALSE;
	ep_bool_t bFine=FALSE;
	int nIndex;
	char szToggleButtonName[128];
	GtkWidget *toggle_button;

	ep_bool_t bSettoreDisponibile=FALSE;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->cb_tutti_settori_selezionati))){
		bTuttiSettoriSelezionati=TRUE;
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->cb_escludi_prodotti_ubicati))){
		bEscludiProdottiUbicati=TRUE;
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_righe))){
		nTipoEquilibratura=EQUILIBRATURA_RIGHE;
	} else {
		nTipoEquilibratura=EQUILIBRATURA_PEZZI;
	}

	if(priv->SettoriDisponibili!=(GArray *)NULL){
		g_array_free(priv->SettoriDisponibili,TRUE);
	}
	priv->SettoriDisponibili=g_array_new(FALSE,FALSE,sizeof(int));
	bFine=FALSE;
	nIndex=1;
	do {
		sprintf(szToggleButtonName,"tb_settore_%02d",nIndex);
		if((toggle_button=find_child(GTK_WIDGET(win),szToggleButtonName))){
			bSettoreDisponibile = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(find_child(GTK_WIDGET(win),szToggleButtonName)));
			if(bSettoreDisponibile){
				/* offset 1 */
				/* da fare : gestione per isole */
				g_array_append_val(priv->SettoriDisponibili,nIndex);
			}
			nIndex++;
		} else {
			bFine=TRUE;
		}
	} while (!bFine);
	Equilibratura(win, nTipoEquilibratura,bTuttiSettoriSelezionati,priv->SettoriDisponibili,bEscludiProdottiUbicati);

	do_aggiorna_lista(priv->main_window);
		
	do_carico_settori(win);
}

static void confirm_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);

    int rc = dlg_msg( GTK_WINDOW(win), "Conferma equilibratura cedola", GTK_MESSAGE_INFO,GTK_BUTTONS_OK_CANCEL, "Conferma equilibratura");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_conferma_equilibratura(win);
        break;
    }

	do_conferma_equilibratura(win);
}

static void refresh_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);

	do_aggiorna_lista(priv->main_window);
}

static void ok_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);

}

static GActionEntry entries[] = {
  {"close",                                close_activated,                   NULL, NULL, NULL},
  {"ok",                                   ok_activated,                      NULL, NULL, NULL},
  {"refresh",                              refresh_activated,                 NULL, NULL, NULL},
  {"balance",                              balance_activated,                 NULL, NULL, NULL},
  {"confirm",                              confirm_activated,                 NULL, NULL, NULL},
  {"righe",                                action_activated,                  NULL, NULL, NULL},
  {"pezzi",                                action_activated,                  NULL, NULL, NULL},
  {"utilizza_tutti_i_settori_selezionati", action_activated,                  NULL, NULL, NULL},
  {"seleziona_tutti_i_settori",            seleziona_tutti_settori_activated, NULL, NULL, NULL},
  {"escludi_prodotti_ubicati",             action_activated,                  NULL, NULL, NULL},
  {"vuota_ubicazioni_non_utilizzate",      action_activated,                  NULL, NULL, NULL}
};

static void balance_window_init (BalanceWindow *win)
{
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    for (int i = 0;i < Cfg.nNumeroSettori; i++){
        char name[128];

        sprintf(name, "tb_settore_%02d", i+1);
        priv->tb_settori[i] = find_child(GTK_WIDGET(win), name);
    }

	create_carico_settori (win, priv->frm_settori);

	do_carico_settori(win);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);
    priv->main_window = _main_window;

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void balance_window_dispose (GObject *object)
{
    BalanceWindow *win = BALANCE_WINDOW (object);
    BalanceWindowPrivate *priv = balance_window_get_instance_private (win);

    g_printf("%s dispose\n", WINDOW_NAME);

    if(priv->wsi){          
        g_settings_sync ();
        window_save_state (win,  priv->wsi, priv->settings);
        window_size_info_free (priv->wsi);
        priv->wsi = NULL;
    }

    if(priv->settings){          
        g_clear_object (&(priv->settings)); 
        priv->settings=NULL;    
    }


    G_OBJECT_CLASS (balance_window_parent_class)->dispose (object);
}

static void balance_window_class_init (BalanceWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = balance_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, frm_settori);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, rb_righe);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, rb_pezzi);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, cb_tutti_settori_selezionati);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, cb_select_tutti_settori);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, cb_escludi_prodotti_ubicati);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, cb_vuota_ubicazioni);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, pb_ok);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, pb_refresh);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, pb_equilibra);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, pb_conferma);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_01);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_02);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_03);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_04);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_05);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_06);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_07);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_08);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_09);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_10);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_11);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), BalanceWindow, tb_settore_12);


}

BalanceWindow * balance_window_new (MainWindow *main_window)
{
    _main_window = main_window;

    BalanceWindow *win = g_object_new (BALANCE_WINDOW_TYPE, "transient-for", main_window, "use-header-bar", TRUE, NULL);

    BalanceWindowPrivate *priv = balance_window_get_instance_private (BALANCE_WINDOW (win));

    /*
    priv->ListaSettori=)NULL;
    priv->ListaProdPallet=NULL;
    priv->ListaProdScaffali=NULL;
    priv->ListaSoluzioni=NULL;
    priv->SettoriDisponibili=NULL;
    */
    return win;
}
