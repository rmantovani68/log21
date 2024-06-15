/*
* mainbal.h
* Dichiarazione Funzioni Utilizzate in mainbal.c
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/


typedef struct TagSettoreDisponibile {
	LINK_ITEM link;
	int nSettore;									/* Numero del settore	*/
	LINKED_LIST ListaScaffali;		/* Lista delle ubicazioni destinate a scaffale */
	LINKED_LIST ListaPallet;			/* Lista delle ubicazioni destinate a pallet */
	int nPesoAttuale;							/* Numero delle righe/pezzi assegnati al settore */
	int nUbiAssegnate;
} SETTOREDISPONIBILE;

typedef struct TagUbicazione {
	LINK_ITEM link;
	char szUbicazione[40];
} UBICAZIONE;

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

ep_bool_t AssegnatiUtente(int nEquilibratura,ep_bool_t bEscludiProdottiUbicati);
ep_bool_t CheckSettori(LINKED_LIST ListaSettori);
ep_bool_t CreaListeProdotti(int nEquilibratura,ep_bool_t bEscludiProdottiUbicati);
ep_bool_t CreaSoluzioni(void);
ep_bool_t Euristico1(SOLUZIONE *Soluzione);
ep_bool_t InizializzaDati(GArray *settori_disponibili);
ep_bool_t InsertUbicaz(LINKED_LIST *pLista, char cStato, int nSettore);
ep_bool_t LiberaMemoria(void);
ep_bool_t SistemazioneDati(ep_bool_t bTuttiSettori);
ep_bool_t ScegliMigliore(void);
ep_bool_t EliminaUbicazioni(LINKED_LIST *Lista,int nNum);
ep_bool_t CreaCopia(LINKED_LIST *Pallet,LINKED_LIST *Scaffali,LINKED_LIST *CopiaListaSettori);
ep_bool_t RiempiUbicazione(LINKED_LIST *ListaProdotti, LINKED_LIST *ListaUbicazioni, LINKED_LIST *ListaAssociazioni, SETTOREDISPONIBILE *Settore, int nVMU);
int Varianza(int nVMS,LINKED_LIST *ListaSettori);
void AssegnaUbicazioniManuali(void);
