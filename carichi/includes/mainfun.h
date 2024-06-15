/*
* mainfun.h
* Dichiarazione Funzioni Utilizzate
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/
#include <ep-popover.h>

/* 
* mainfun.c 
*/
#define DEBUG_FLAG 2
#define LENGTH_CODICE_UDC 7
#define LENGTH_CODICE_UBICAZIONE 13


ep_bool_t InizioTransazione(void);
ep_bool_t FineTransazione(ep_bool_t bCommit);
ep_bool_t CheckLogin(char *szUserId,char *szPassword);
ep_bool_t InsertProdottoCatalogo(char *szCDPRO, char *szDSTIT);
int CreaDistinta(void);
ep_bool_t MovimentaProdotto(char *szCausale, char *szCDPRO, int nQTPRO, int nDistinta, char *szUDC, char *szTerminale, char *szUser);
ep_bool_t AnnullaMovimentazioniUdc(char *szCDUDC);
ep_bool_t AnnullaMovimentazioniProdottoSuUdc(char *szCDPRO,char *szCDUDC);
ep_bool_t AnnullaMovimentazione(int nProgressivoMovimentazione);
ep_bool_t DeltaGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t DeltaIngressoGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t DeltaSpeditoGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t DeltaRestituitoGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t DeltaDifferenzaInventarialeGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t GeneraCodiceProdotto(char *szCDPRO);
ep_bool_t CodiceProdottoEsistente(char *szCodiceProdotto);
int GetGiacenzaProdotto(char *szCodiceProdotto);
ep_bool_t CaricaProdottoSuUdc(char *szUDC,char *szCDPRO,int nQTPRO);
ep_bool_t ScaricaProdottoDaUdc(char *szUDC,char *szCDPRO,int nQTPRO);
ep_bool_t CreaUdc(char *szUDC);
ep_bool_t UdcEsistente(char *szUDC);
ep_bool_t SetDistintaPerUdc(char *szUDC, int nDistinta);
ep_bool_t DistintaEsistente(int nDistinta);
int CreaDistinta(void);
int GeneraNumeroDistinta(void);
ep_bool_t GeneraCodiceUdc(char *szCDUDC);
ep_bool_t AggiornaAnagraficaProdotto(char *szCDPRO, char *szBRCDE,char *szBRCDF,char *szDSTIT,char *szDSAUT,int nPZPRO, int nNMTIR);
ep_bool_t AggiornaIdentificativiProdotto(char *szCDPRO, char *szBRCDE,char *szBRCDF);
ep_bool_t AggiornaDatiProdotto(char *szCDPRO, char *szDSTIT,char *szDSAUT,int nPZPRO, int nNMTIR);
ep_bool_t InserisciProdottoInCatalogo(char *szCDPRO, char *szDSTIT);
ep_bool_t StringIsBarcodeTemplate(char *szString);
ep_bool_t StringIsCodiceTemplate(char *szString);
ep_bool_t StringIsDeltaTemplate(char *szString);
ep_bool_t GetCodiceProdottoFromBarcode(char *szCodiceProdotto,char *szBarcode);
ep_bool_t GeneraCodiceProdotto(char *szCDPRO);
ep_bool_t GeneraCodiceUdc(char *szCDUDC);
ep_bool_t CreaNuovoUdc(char *szCodiceUdc);
ep_bool_t GetNuovoCodiceUdc(char *szNuovoCodiceUdc);
int ImportazioneSpedizioniImportaSpedizioni(void);
int ImportazioneSpedizioniAggiungiCedola(int nCedola);
int ImportazioneSpedizioniAggiungiOrdine(char *szOrdine);
int ImportazioneSpedizioniUpdateSpedizioniDaImportare(void);
ep_bool_t ImportazioneSpedizioniSetSpedizioneImportata(int nPRSPE);
ep_bool_t GeneraBarcodeDaCodiceProdotto(char *szCDPRO, char *szBRCDE);
ep_bool_t ImportazioneSpedizioniCancellaProdotto(int nPRSPE);
ep_bool_t ImportazioneSpedizioniSetQuantita(int nPRSPE, int nQuantita);
ep_bool_t CatalogoImportSetProdottoImportato(int nPRANA);
ep_bool_t AggiornaCodiceProdotto(char *szOldCDPRO, char *szCDPRO);
ep_bool_t DeltaConteggioGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t DeltaDifferenzaGiacenza(char *szCDPRO, int nQTPRO);
ep_bool_t RestituzioneDistinta(int nDistinta, char *szTerminale, char *szUser);
ep_bool_t StoricizzaDistinta(int nDistinta);
ep_bool_t SetStatoDistinta(int nDistinta,char cStato);
ep_bool_t SetStatoDistintaStorico(int nDistinta,char cStato);
ep_bool_t SetStatoSpedizioneDistinta(int nDistinta,char cStato);
ep_bool_t SetStatoSpedizioneDistintaStorico(int nDistinta,char cStato);
ep_bool_t SetStatoFlussoDistinta(int nDistinta,char cStato);
ep_bool_t SetStatoFlussoDistintaStorico(int nDistinta,char cStato);
ep_bool_t SetStatoUdc(char *szUdc,char cStato);
ep_bool_t SetStatoProdottoUdc(char *szPRPRU,char cStato);
char GetStatoDistinta(int nDistinta);
char GetStatoSpedizioneDistinta(int nDistinta);
ep_bool_t StoricizzaProdottoInGiacenza(char *szCodiceProdotto);
ep_bool_t ProdottoInGiacenza(char *szCodiceProdotto);

ep_bool_t GetProdottoInUdc(char *szUDC,char *szCDPRO);
int GetQuantitaProdottoInUdc(char *szUDC,char *szCDPRO);
int GetContatoProdotto(char *szCodiceProdotto);
int GetDifferenzaProdotto(char *szCodiceProdotto);
ep_bool_t GeneraCodiceProdottoDaLetturaGenerica(char *szCDPRO);
ep_bool_t ImportaCatalogo(void);
ep_bool_t ConvertiProdotto(char *szOldCDPRO,char *szNewCDPRO);
ep_bool_t CancellaProdottoInCatalogo(char *szCDPRO);
int GetDistintaUdc(char *szUdc);
ep_bool_t EliminaDistinta(int nDistinta);
int GetNumeroProdottiSuUdc(char *szCDUDC);
ep_bool_t ResettaUdc(char *szCDUDC);
ep_bool_t AllineaCatalogoCarichiConCatalogoPicking(void);
ep_bool_t AllineaDatiProdottoConCatalogoPicking(char *szCodiceProdotto);
int MovimentaProdottiDaOrdiniSpediti(void);
int CheckImportCarichi(void);

typedef gboolean (*action_callback)(gpointer, gchar *, gchar *, int, gchar **);
gchar *get_field_from_list_selected_row(gpointer win, gchar *lst_name, int column);
void do_action_using_ep_popover (gpointer win, EpPopoverField *fields, gchar **values, GtkButton *button, EpPopoverType type, action_callback action_cb);

gboolean do_conteggio_movimentazioni(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_differenza_inventariale(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_crea_udc(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_stampa_etichetta(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_giacenze_totali_ingresso(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_giacenze_totali_spedizione(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_giacenze_totali_conteggia(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_giacenze_totali_differenza_inventariale(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg);
gboolean do_giacenze_totali_storicizza(gpointer win);
gboolean do_annulla_movimento(gpointer win, gchar *prgmov, gchar **msg);
gboolean do_nuova_distinta(gpointer win, gchar **msg);
gboolean do_elimina_distinta(gpointer win, gchar *nmdis, gchar **msg);
gboolean do_restituzione_distinta(gpointer win, gchar *nmdis, gchar **msg);
gboolean do_storicizza_distinta(gpointer win, gchar *nmdis, gchar **msg);

ep_bool_t StampaEtichettaUdc(char *szCodiceUdc);

void refresh_main_window(gpointer win);

void refresh_stack_main(gpointer win);

void refresh_box_giacenze(gpointer win);
void refresh_box_movimenti(gpointer win);
void refresh_box_catalogo(gpointer win);
void refresh_box_reports(gpointer win);

void clear_box_movimenti(gpointer win);

void refresh_stack_giacenze(gpointer win);

void refresh_box_giacenze_totali(gpointer win);
void refresh_box_giacenze_prodotti_udc(gpointer win);
void refresh_box_giacenze_udc(gpointer win);
void refresh_box_giacenze_distinte(gpointer win);

void clear_box_giacenze_totali(gpointer win);
void clear_box_giacenze_prodotti_udc(gpointer win);
void clear_box_giacenze_udc(gpointer win);
void clear_box_giacenze_distinte(gpointer win);

void refresh_stack_reports(gpointer win);

void refresh_box_reports_storico_giacenze(gpointer win);
void refresh_box_reports_storico_movimenti(gpointer win);
void refresh_box_reports_carichi_importati(gpointer win);

void clear_box_reports_storico_movimenti(gpointer win);
void clear_box_reports_carichi_importati(gpointer win);

void refresh_stack_report_giacenze(gpointer win);

void refresh_box_report_storico_giacenze_totali(gpointer win);
void refresh_box_report_storico_giacenze_prodotti_udc(gpointer win);
void refresh_box_report_storico_giacenze_udc(gpointer win);
void refresh_box_report_storico_giacenze_distinte(gpointer win);

void clear_box_report_storico_giacenze_totali(gpointer win);
void clear_box_report_storico_giacenze_prodotti_udc(gpointer win);
void clear_box_report_storico_giacenze_udc(gpointer win);
void clear_box_report_storico_giacenze_distinte(gpointer win);


/* 
* mainproc.c 
*/
gint ProcessMsgs( gpointer win );
