#ifndef __BALANCE_H
#define __BALANCE_H

#include <gtk/gtk.h>
#include <linklist.h>
#include "mainwin.h"


#define BALANCE_WINDOW_TYPE (balance_window_get_type ())
G_DECLARE_FINAL_TYPE (BalanceWindow, balance_window, BALANCE, WINDOW, GtkDialog)


BalanceWindow *balance_window_new          (MainWindow *win);

typedef struct TagSettoreDisponibile {
	LINK_ITEM link;
	int nSettore;									/* Numero del settore	*/
	LINKED_LIST ListaScaffali;		/* Lista delle ubicazioni destinate a scaffale */
	LINKED_LIST ListaPallet;			/* Lista delle ubicazioni destinate a pallet */
	int nPesoAttuale;							/* Numero delle righe/pezzi assegnati al settore */
	int nUbiAssegnate;
} SETTOREDISPONIBILE;

typedef struct TagStructUbicazione {
	LINK_ITEM link;
	char szUbicazione[40];
} STRUCT_UBICAZIONE;

typedef struct TagProdotto {
	LINK_ITEM link;
	char szCodProd[40];
	int nPeso;
} PRODOTTO;

typedef struct TagSoluzione{
	LINK_ITEM link;
	int nVarianza;
	LINKED_LIST ListaAssociazioni; /* Associazione codice prodotto - ubicazione */
} SOLUZIONE;

typedef struct TagAssociazione{
	LINK_ITEM link;
	char szUbicazione[40];
	char szCodProd[40];
	int nSettore;
	int nPeso;
} ASSOCIAZIONE;

ep_bool_t AssegnatiUtente(gpointer win, int nEquilibratura,ep_bool_t bEscludiProdottiUbicati);
ep_bool_t CheckSettori(gpointer win);
ep_bool_t CreaListeProdotti(gpointer win, int nEquilibratura,ep_bool_t bEscludiProdottiUbicati);
ep_bool_t CreaSoluzioni(gpointer win);
ep_bool_t Euristico1(gpointer win, SOLUZIONE *Soluzione);
ep_bool_t InizializzaDati(gpointer win, GArray *settori_disponibili);
ep_bool_t InsertUbicaz(gpointer win, LINKED_LIST *pLista, char cStato, int nSettore);
ep_bool_t LiberaMemoria(gpointer win);
ep_bool_t SistemazioneDati(gpointer win, ep_bool_t bTuttiSettori);
ep_bool_t ScegliMigliore(gpointer win);
ep_bool_t EliminaUbicazioni(gpointer win, LINKED_LIST *Lista,int nNum);
ep_bool_t CreaCopia(gpointer win, LINKED_LIST *Pallet,LINKED_LIST *Scaffali,LINKED_LIST *CopiaListaSettori);
ep_bool_t RiempiUbicazione(gpointer win, LINKED_LIST *ListaProdotti, LINKED_LIST *ListaUbicazioni, LINKED_LIST *ListaAssociazioni, SETTOREDISPONIBILE *Settore, int nVMU);
int Varianza(gpointer win, int nVMS,LINKED_LIST *ListaSettori);
void AssegnaUbicazioniManuali(gpointer win);


#endif /* __BALANCE_H */
