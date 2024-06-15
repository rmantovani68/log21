#include <stdio.h>
#include <stdlib.h>
#ifdef Linux
#define MAXPATHLEN FILENAME_MAX
#include <lynx-a.out.h>
#include <lynx-nlist.h>
#else
#include <a.out.h>
#include <nlist.h>
#endif
#include <math.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <plclink.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#include <Xm/Text.h>
#include <Xm/DrawingA.h>
#include <Xm/TextF.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/FileSB.h>

#ifdef Linux
#include <libxad.h>
#else
#include <Xm/libxad.h>
#endif

#include "equat.h" 

#include <nvram.h>
#include <ios.h>
#ifdef Linux
#include <sys/msg.h>
#else
#include <msg.h>
#endif

#ifdef Linux
typedef void *PTMEMMESSAGE;
#else
#include <libpcftp.h>
#include <CniAlf.h>
#include <libxerr.h>
#endif

/*
	Dichiarazione degli external
	----------------------------
*/

extern 	struct  plc_mcopy_t Controllo_Var[NUMERO_VARIABILI];
extern	unsigned short int	LINE_STATE;
extern	struct  plcvar_t StrutturaVariabili[NUMERO_VARIABILI];
extern	struct  plcvar_t StrutturaTrigger;
extern	unsigned long    IndirizzoVariabili[NUMERO_VARIABILI];
extern  unsigned char   *CNI_iosbrd[];
extern	unsigned char 	**CNI_ios[];
extern	unsigned char 	*NvRam;
extern  unsigned char   status[32];
extern  unsigned char   error[32];
extern  XtAppContext    plc2AppContext;
extern	TRACERECORD		tracebuf[2][400];
extern 	XtIntervalId  	IntervaDebug;
extern 	XtIntervalId  	IntervaDebugFronte;
extern	XtIntervalId    IntervaStatistic;
#ifdef Linux
extern 	void *  	MemLin;
extern 	void * 	fbox;
#else
extern 	PTMEMMESSAGE  	MemLin;
#ifdef _OLD_FILE_SEL
extern 	PTFILESELBOX 	fbox;
#else
extern 	PTXNCNFSB 		fbox;
#endif
#endif
extern  XFontStruct  	*font_struct;
extern  XmFontList  	font_list;
extern  XsWdesc  		*mainWD;
extern	Boolean 		MODULO_SPECIALE;
extern	Boolean 		Plc_aperto;
extern 	Boolean 		Trovato_pre_trigger;
extern  nvram_t			Area_ios;
extern  Widget  		plc2ShW;
extern	Widget  		NomiVariabili[NUMERO_VARIABILI];
extern	Widget  		ValoreVariabili[NUMERO_VARIABILI];
extern	Widget  		drawing_a_stat;
extern  Pixmap  		pixmap;
extern	Pixmap			Plc_status;
extern	Pixmap			Debug_status;
extern	XColor 			colori_allocati[10];
extern 	Icona			IconPplc_run;
extern	Icona			IconPplc_halt;
extern	Icona			IconPplc_absent;
extern	Icona			IconPplc_trace;
extern	Icona			IconPplc_var;
extern	long    		Campioni_statistica[CAMPIONISTAT][3];
extern	char    		TestoNomiVariabili[NUMERO_VARIABILI][17];
extern	char    		TriggerNome[17];
extern  char    		elemio[8][32];
extern  char    		filtio[8][32];
extern  char    		resize;
extern  char    		changedebug;
extern	char   			Old_Trigger_Value;
extern	char   			*TipoModuloCaricato[];
extern	char 			Griglia_trace[9][10];
extern	char            InputName[];
extern	char            OutputName[];
extern	int             OffsetValue;
extern	int     		ContaVariabili;
extern	int     		TipoVariabili[NUMERO_VARIABILI];
extern	int 			numero_tracebuf;
extern	int     		LONG_FLOAT[NUMERO_VARIABILI];        
extern	int     		VariabileCorrente;
extern	int 			Lunghezza_Stringa[NUMERO_VARIABILI];
extern 	int 			n_board[IOS_MAX];
extern	int 			ios_dis[IOS_MAX];
extern	int 			board_selezionata;
extern	int 			displacement_selezionato;
extern  int     		typedebug;
extern  int     		typeshow;
extern  int     		ios;
extern	int 			SALITA_DISCESA;
extern	int     		TRIGGER_PRETRIGGER;
extern	int     		Trigger_Mask;
extern	int     		Ciclo_trigger;
extern 	int      		uppercase;
extern 	int      		FocusBool;
extern	int 			zoom;
extern	int 			posizione_cursore;
extern	int 			Min_Trace;
extern	int 			Max_Trace;
extern 	int 			numero_modulo;
extern 	int 			numero_byte;
extern  GC      		gc;

extern	Boolean	macchinaIsStart();
extern  Pixmap  loadIcona(Widget, Icona *);
extern  void 	set_color();
extern  void 	scadTdebug();      
extern  void 	scadFrontedebug();      
extern 	void 	stampaMes(String , ...);
extern	void 	VisualizzaTrace(int);
extern 	void	AggiornaControlloVariabili();
extern 	int     invXerror(char *, long, char *, ...);
extern  int  	CNI_opendev();
extern 	int 	CreaDialogShSelFile(Widget ,int );
extern 	int		TrovaIndirizzo(char *, long , char *,unsigned long *);
extern	Widget  GetWidgetByName(char *);

/*
    =====   dichiarazione funzioni =====
    ------------------------------------
*/
void file_cb(Widget, XtPointer, XtPointer);
void delvar(Widget, XtPointer, XmAnyCallbackStruct *);
void drawing(Widget, XtPointer, XmAnyCallbackStruct *);
void ripristino(Widget, XtPointer, XmAnyCallbackStruct *);
void allcaps(Widget, XtPointer, XmAnyCallbackStruct *);
void DefinisciTipo(char *);
void CercaVariabile(Widget, int);
void print_result(Widget, XtPointer, XmAnyCallbackStruct *);
void buttonchange(Widget, XtPointer, XmAnyCallbackStruct *);
void aggios_large();
void serial_large(Widget, XtPointer, XmAnyCallbackStruct *);
void traverse(Widget, XtPointer, XmAnyCallbackStruct *);
#ifdef _OLD_FILE_SEL
void cbkSelezionaFile (int, char *, XtPointer);
#else
void cbkSelezionaFileNew (int, int, char *, XtPointer);
#endif
void forza_uscita(Widget, XtPointer, XmAnyCallbackStruct *);
void TipoDiVariabile(Widget, XtPointer, XmAnyCallbackStruct *);
void scadStatistic(XtPointer, XtIntervalId *);
void Statistica(Widget, XtPointer, XmAnyCallbackStruct *);
void Aiuti(Widget, XtPointer, XmAnyCallbackStruct *);
void SpostaPointer(Widget, XtPointer, XmAnyCallbackStruct *);
int plcSaveIO(void);
void SaveIOPB(Widget, XtPointer, XmAnyCallbackStruct *);
void buttontrace(Widget, XtPointer, XmAnyCallbackStruct *);
void RicaricaTrace(Widget, XtPointer, XmAnyCallbackStruct *);
int GestioneInputOutputBS(char *);

/*
 --------------------------------------------------------------------
    Funzione file_cb 
  
    Funzione chiamata alla pressione di una delle voci del 
	sottomenu File
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void file_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
int     	reason=(int)client_data;
Widget 		drawing_a;
int     	rv;
#ifdef _OLD_FILE_SEL
int 		FileType;
#endif

if (reason == RESTORE_PLC)
	{
	rv=plcUnloadProg();
    if (rv==PLCERR_TIMEOUT) 
     	{
        while (plcLdStatus(&rv) == PLCERR_TIMEOUT) 
			{
			fflush(stdout);
            sleep(1);
            }
        if (rv)
			invXerror(IDS_SIGLA_PLC2,NO_PLC_LD_STATUS,"%v",rv);
        }
    else if (rv) 
		{
		invXerror(IDS_SIGLA_PLC2,NO_PLC_LOAD_PROG,"%v",rv);
        }
	return;
    }

if (reason==FILE_EXIT)
	{
	drawing_a=GetWidgetByName("DRAIOS");
	XFreePixmap(XtDisplay(drawing_a), pixmap);
	XFreeGC(XtDisplay(drawing_a),gc);
	XFreeFont(XtDisplay(drawing_a),font_struct);
	XmFontListFree(font_list);
	plcClose(); 	/* scollegati dal processo PLC */
   	exit (0);
	}

if (reason == FILE_OPEN)
	{
#ifdef Linux
#else
	if (!fbox)
		CreaDialogShSelFile(plc2ShW,ID_DIR_VARP);
#ifdef _OLD_FILE_SEL
	FileType=xncGetFileSelBoxType(fbox);
	if(FileType!=ID_DIR_VARP)
		{
		xncSetFileSelBoxType(fbox,ID_DIR_VARP);
		xncSetRescan(fbox);
		}
	if (!(xncSetFsbOkClient(fbox,(XtPointer) ID_DIR_FILEVR)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
		}
	xncShowFileSelBoxCB(NULL,fbox,NULL);
	xncMovePointerTo(fbox->hFileListWidget);
#else
	if(!(xncNfsbSetPath(fbox, ID_DIR_VARP)))
        invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
	if (!(xncNfsbSetOkClient(fbox,(XtPointer) ID_DIR_FILEVR)))
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
	xncNfsbShow(fbox);
	xncMovePointerTo(xncNfsbGetShell(fbox));
#endif
#endif
	}

if (reason == LOAD_PLC) 
   	{
#ifdef Linux
#else
	if (!fbox)
		CreaDialogShSelFile(plc2ShW,ID_DIR_PPLC);
#ifdef _OLD_FILE_SEL
	FileType=xncGetFileSelBoxType(fbox);
	if(FileType!=ID_DIR_PPLC)
		{
		xncSetFileSelBoxType(fbox,ID_DIR_PPLC);
        xncSetRescan(fbox);
		}
	if (!(xncSetFsbOkClient(fbox,(XtPointer) ID_DIR_FILEPR)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
        }
	xncShowFileSelBoxCB(NULL,fbox,NULL);
	xncMovePointerTo(fbox->hFileListWidget);
#else
	if(!(xncNfsbSetPath(fbox, ID_DIR_PPLC)))
        invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
	if (!(xncNfsbSetOkClient(fbox,(XtPointer) ID_DIR_FILEPR)))
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
	xncNfsbShow(fbox);
	xncMovePointerTo(xncNfsbGetShell(fbox));
#endif
#endif
    }

if (reason == FILE_SAVE) 
	{
#ifdef Linux
#else
	if (!fbox)
		CreaDialogShSelFile(plc2ShW,ID_DIR_VARP);
#ifdef _OLD_FILE_SEL
	FileType=xncGetFileSelBoxType(fbox);
	if(FileType!=ID_DIR_VARP)
		{
		xncSetFileSelBoxType(fbox,ID_DIR_VARP);
		xncSetRescan(fbox);
		}
	if(!(xncSetFsbOkClient(fbox,(XtPointer) ID_DIR_FILEVW)))
        {
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
        }
	xncShowFileSelBoxCB(NULL,fbox,NULL);
	xncMovePointerTo(fbox->hFileListWidget);
#else
	if(!(xncNfsbSetPath(fbox, ID_DIR_VARP)))
        invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
	if (!(xncNfsbSetOkClient(fbox,(XtPointer) ID_DIR_FILEVW)))
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_OK,"%d");
	xncNfsbShow(fbox);
	xncMovePointerTo(xncNfsbGetShell(fbox));
#endif
#endif
	}
}

/*
 --------------------------------------------------------------------
    Funzione delvar 
  
    Funzione chiamata alla pressione della voce Cancella
	sottomenu Opzioni
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void delvar(Widget w, XtPointer	clientData, XmAnyCallbackStruct	*callData)
{
XmString 	wid_label_string;

memset(TestoNomiVariabili[0],'\0',16);
for (ContaVariabili=1;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
    strncpy(TestoNomiVariabili[ContaVariabili],TestoNomiVariabili[0],17);

for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
	{
	XtVaSetValues(NomiVariabili[ContaVariabili],XmNvalue,
		&TestoNomiVariabili[ContaVariabili],NULL);  /* ripristina vecchio testo */ 

	wid_label_string=XmStringCreateSimple(" ");
	XtVaSetValues(ValoreVariabili[ContaVariabili],XmNlabelString,wid_label_string,NULL);
	XmStringFree(wid_label_string);
    LONG_FLOAT[ContaVariabili]=-1;
	} 
}

/*
 --------------------------------------------------------------------
    Funzione drawing 
  
    Funzione chiamata sull' evento expose e resize della drawing area
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void drawing(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) callData;
XEvent *event = cbs -> event;
Display *dpy = event ->xany.display;
XtVaGetValues(w, XmNuserData, &gc, NULL);
XCopyArea (dpy,pixmap,event->xany.window,gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
resize=1;		/* segnala resize */
}

/*
 --------------------------------------------------------------------
    Funzione ripristino 
  
    Funzione chiamata quando i campi editabili perdono il fuoco
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void ripristino(Widget w, XtPointer	clientData, XmAnyCallbackStruct	*callData)
{
char	testo[17];
switch((int)clientData)
	{
	case	1:
		strncpy(testo,TestoNomiVariabili[0],17);
		break;
    case    2:
        strncpy(testo,TestoNomiVariabili[1],17);
        break;
    case    3:
        strncpy(testo,TestoNomiVariabili[2],17);
        break;
    case    4:
        strncpy(testo,TestoNomiVariabili[3],17);
        break;
    case    5:
        strncpy(testo,TestoNomiVariabili[4],17);
        break;
    case    6:
        strncpy(testo,TestoNomiVariabili[5],17);
        break;
    case    7:
        strncpy(testo,TestoNomiVariabili[6],17);
        break;
    case    8:
        strncpy(testo,TestoNomiVariabili[7],17);
        break;
    case    9:
        strncpy(testo,TestoNomiVariabili[8],17);
        break;
    case    10:
        strncpy(testo,TestoNomiVariabili[9],17);
        break;
    case    11:
        strncpy(testo,TestoNomiVariabili[10],17);
        break;
    case    12:
        strncpy(testo,TestoNomiVariabili[11],17);
        break;
    case    13:
        strncpy(testo,TestoNomiVariabili[12],17);
        break;
    case    14:
        strncpy(testo,TestoNomiVariabili[13],17);
        break;
    case    15:
        strncpy(testo,TestoNomiVariabili[14],17);
        break;
    case    16:
        strncpy(testo,TestoNomiVariabili[15],17);
        break;
    case    17:
        strncpy(testo,TriggerNome,17);
        break;
	}
XtVaSetValues(w,XmNvalue,&testo,NULL);	/* ripristina vecchio testo */
}

/*
 --------------------------------------------------------------------
    Funzione allcaps 
  
    Funzione chiamata alla pressione di un tasto negli entry
	per rendere la scritta maiuscola
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void allcaps(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int 	len;
XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) callData;

if ( uppercase==True || (int)clientData == 17 )
	{
  	if (cbs->text->ptr == NULL) 
		return;

  	/* convert all input to upper-case if necessary */
  	for (len=0; len<cbs->text->length; len++)
		{
		if (islower (cbs->text->ptr[len]))
			cbs->text->ptr[len] = toupper (cbs->text->ptr[len]);
		}
  	}
}

/*
 --------------------------------------------------------------------
    Funzione DefinisciTipo 
  
   	Attraverso questa funzione definiamo il tipo della
   	variabile globale del programma PLC

    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void DefinisciTipo(char *NomeVariabile)
{
XmString    wid_label_string;


  wid_label_string=XmStringCreateSimple(NomeVariabile);
  XtVaSetValues(GetWidgetByName("VarTypeName"),XmNlabelString,
		wid_label_string,NULL);
  XmStringFree(wid_label_string);
  XtManageChild (GetWidgetByName("VarType"));
  xncMovePointerTo(GetWidgetByName("FloatPB"));
  if (FocusBool == True )
    XmProcessTraversal(GetWidgetByName("BoolPB"),XmTRAVERSE_CURRENT);
}

/*
 --------------------------------------------------------------------
    Funzione CercaVariabile 
  
    Attraverso questa funzione verifichiamo se la variabile
    e' un segnale di scambio oppure una variabile globale
    del programma PLC 

    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void CercaVariabile(Widget w, int VariabileSelezionata)
{
XmString    wid_label_string;
struct  modprop_t   prop;
int     rv;
struct  nlist       CercaIndirizzo[3];  
unsigned long       indirizzo;
char    IsakerPath[MAXPATHLEN];
char    NomeTemporaneo[MAXPATHLEN];
char    NomeTemporaneoBS[MAXPATHLEN];
char	*Posizioneparentesi;
int     ScorriModulo;
Boolean Trovata_variabile;
int		SpecialVar=False;
int		pezza;
int		oldTipoVar=-1;

/* Trasformo le scritture di vettori fatte con [x,x]
   nella convenzione adottata da isaker */
strcpy(NomeTemporaneo,TestoNomiVariabili[VariabileSelezionata]);
if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)'[')) != NULL )
  	{
  	Posizioneparentesi[0]='_';
  	if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)',')) != NULL )
    	{
    	Posizioneparentesi[0]='_';
    	}
    if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)']')) != NULL )
        {
        Posizioneparentesi[0]='\0';
        }
  	}

/* se testo e' una variabile di scambio compilazione delle strutture plcvar_t */
if ( dbGetVar(NomeTemporaneo,&StrutturaVariabili[VariabileSelezionata]))
   	{
	oldTipoVar=TipoVariabili[VariabileSelezionata];
	TipoVariabili[VariabileSelezionata]=SEGNALE_DI_SCAMBIO;
	if (oldTipoVar != SEGNALE_DI_SCAMBIO && w != NULL)
		AggiornaControlloVariabili(); 
    if ( w != NULL )
		{
		XmProcessTraversal (w, XmTRAVERSE_DOWN);
		}
   	}
else 
	{
	sprintf(NomeTemporaneoBS,"%s",TestoNomiVariabili[VariabileSelezionata]);
	SpecialVar=GestioneInputOutputBS(NomeTemporaneoBS);
    if ((Posizioneparentesi=strchr(NomeTemporaneoBS,(int)'[')) != NULL )
    	{
        strcpy(NomeTemporaneo,NomeTemporaneoBS);
		Posizioneparentesi=strtok(NomeTemporaneo, "[");
   		strcpy(NomeTemporaneo,Posizioneparentesi);
		}
	/* prima di cominciare la ricerca metto la variabile in una apposita struttura  
		di tipo nlist */
	Trovata_variabile=False;

	CercaIndirizzo[0].n_un.n_name=NomeTemporaneo;
    CercaIndirizzo[1].n_un.n_name = "";
    CercaIndirizzo[2].n_un.n_name = (char *)0;

	/* andiamo a cercare nel modulo caricato 
   	prima nella parte di programma stile 480, 
   	poi nella parte codice utente generico 
   	ed infine nella parte cappello e scarpe */

	for (ScorriModulo=0;ScorriModulo<3;ScorriModulo++)
		{	
  		if (Trovata_variabile==False)
			{
			for(pezza=0;pezza<10;pezza++)
				{
				rv = plcModProp(&prop,TipoModuloCaricato[ScorriModulo]);
       			if (rv == 0) 
					break;
				}
       			if (rv) 
					invXerror(IDS_SIGLA_PLC2,NO_plcModProp,"%d");
			if (prop.file[0])
				{
				nlist (prop.file,CercaIndirizzo);
				if ((CercaIndirizzo[0].n_type & N_TYPE)!= N_UNDF)
					{
					indirizzo=prop.text+CercaIndirizzo[0].n_value;
					TipoVariabili[VariabileSelezionata]=VARIABILE_PLC;
					IndirizzoVariabili[VariabileSelezionata]=indirizzo;
					Trovata_variabile=True;
					if (w != NULL)
						{
						if (SpecialVar == False)
                        	DefinisciTipo(NomeTemporaneo);
						else
							{
							LONG_FLOAT[VariabileSelezionata]=CHAR_TYPE;
							AggiornaControlloVariabili();
							}
						XmProcessTraversal (w, XmTRAVERSE_DOWN);
						}
					}
				else if (CercaIndirizzo[0].n_value != 0 )
					{
       				/* variabile non inizializzata */
					TrovaIndirizzo(prop.file,prop.common, NomeTemporaneo,&indirizzo);	
					if (indirizzo != 0)
						{
						TipoVariabili[VariabileSelezionata]=VARIABILE_PLC;
              			IndirizzoVariabili[VariabileSelezionata]=indirizzo;
						Trovata_variabile=True;
						if (w!= NULL)
							{
							if (SpecialVar == False)
                        		DefinisciTipo(NomeTemporaneo);
							else
								{
								LONG_FLOAT[VariabileSelezionata]=CHAR_TYPE;
								AggiornaControlloVariabili();
								}
							XmProcessTraversal (w, XmTRAVERSE_DOWN);
							}
						}
					}
				}
			}
		}
	if (Trovata_variabile==False)
		{
		/* per concludere vado a cercare in isaker */
		plcGetExeName(IsakerPath);
    	nlist (IsakerPath,CercaIndirizzo);
    	if ((CercaIndirizzo[0].n_type & N_TYPE)!= N_UNDF)
			{
       		indirizzo=CercaIndirizzo[0].n_value;
			TipoVariabili[VariabileSelezionata]=VARIABILE_PLC;
       		IndirizzoVariabili[VariabileSelezionata]=indirizzo;
			if (w != NULL)
				{
				if (SpecialVar == False)
                	DefinisciTipo(NomeTemporaneo);
				else
					{
					LONG_FLOAT[VariabileSelezionata]=CHAR_TYPE;
					AggiornaControlloVariabili();
					}
				XmProcessTraversal (w, XmTRAVERSE_DOWN);
				}
			}
		else
			{
			invXerror(IDS_SIGLA_PLC2,NO_VARIABLE,"%n",NomeTemporaneo);
		    memset(NomeTemporaneo,NULL,16);
			memset(TestoNomiVariabili[VariabileSelezionata],NULL,16); 
            XtVaSetValues(NomiVariabili[VariabileSelezionata],XmNvalue,&NomeTemporaneo,
				NULL);

       		/*ripristina vecchio testo */
       		wid_label_string=XmStringCreateSimple(" ");
       		XtVaSetValues(ValoreVariabili[VariabileSelezionata],XmNlabelString,
				wid_label_string,NULL);
       		XmStringFree(wid_label_string); 
			if (w != NULL)
				AggiornaControlloVariabili();
			LONG_FLOAT[VariabileSelezionata]=-1;
			}
		}
	}
}

/*
 --------------------------------------------------------------------
    Funzione print_result 
  
    Funzione chiamata alla pressione del tasto enter sugli edit field
	per la conferma del dato
	
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void print_result(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
XmString    wid_label_string;
int 	length;
int		VariabileSelezionata;
char	*testo;
char    NomeTemporaneo[MAXPATHLEN];
char    *Posizioneparentesi;

VariabileSelezionata=(int)clientData-1;
VariabileCorrente=(int)clientData-1;
XtVaGetValues(w,XmNvalue,&testo,NULL);
changedebug=1;
if(strcmp(testo,"") == 0)\
	{
	wid_label_string=XmStringCreateSimple(" ");
    XtVaSetValues(ValoreVariabili[VariabileCorrente],XmNlabelString,
		wid_label_string,NULL);
    XmStringFree(wid_label_string);
	}

if ((int)clientData<=NUMERO_VARIABILI)
	{
	strncpy(TestoNomiVariabili[VariabileSelezionata],testo,17);
	/* debug possibile : ricerca dei simboli richiesti se esistono */ 
	length=strlen(TestoNomiVariabili[VariabileSelezionata]);
	if(length!=0)
   		{   
	    CercaVariabile(w,VariabileSelezionata);	
		}
	}
else
	{
	strncpy(TriggerNome,testo,17);
	length=strlen(TriggerNome);
	if(length!=0)
        {   
        /* Trasformo le scritture di vettori fatte con [x,x]
           nella convenzione adottata da isaker */
		strcpy(NomeTemporaneo,TriggerNome);
		if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)'[')) != NULL )
    		{
    		Posizioneparentesi[0]='_';
    		if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)',')) != NULL )
        		{
        		Posizioneparentesi[0]='_';
        		}
    		if ( (Posizioneparentesi=strchr(NomeTemporaneo,(int)']')) != NULL )
        		{
        		Posizioneparentesi[0]='\0';
        		}
    		}
        /* se testo e' una variabile di scambio compilazione delle strutture plcvar_t */
        if ( dbGetVar(NomeTemporaneo,&StrutturaTrigger))
            {
			if (StrutturaTrigger.type==ISAVAR_T_BOOL)
				{
				XmProcessTraversal (w, XmTRAVERSE_DOWN);
            	XtFree(testo);
            	return;
				}
			else
            	invXerror(IDS_SIGLA_PLC2,NO_VARIABLE_BOOLEANA,"%n",TriggerNome);
            }
		else
			invXerror(IDS_SIGLA_PLC2,NO_VARIABLE,"%n",TriggerNome);
		}
	}
XtFree(testo);
}

/*
 --------------------------------------------------------------------
    Funzione buttonchange 
  
    Funzione chiamata alla pressione di uno dei bottoni verticali

    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void buttonchange(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int	rv,isa_status,i,j,index;
int length;
XmString dstart;
XmString str;
char 	*get_string;
Widget	sb_ios_wid,wid;
int     Old_y=0,Numero_buffer=0;
char    text[80];
int		indice_temporaneo=0;
char    NomeTemporaneo[MAXPATHLEN];
char    *Posizioneparentesi;

switch((int)clientData)
	{
	case	RUN_BUTTON:		/* go plc	*/
	isa_status=plcStStatus(&rv);
	if (isa_status==PLCERR_TIMEOUT)
		{
		invXerror(IDS_SIGLA_PLC2,NO_ISAKER,"%d");
		Plc_status = loadIcona(GetWidgetByName("Left_icon"), &IconPplc_absent);
		if (Plc_status!=NULL)
        	XtVaSetValues(GetWidgetByName("Left_icon"),XmNlabelPixmap, 
				Plc_status, NULL);
		}
	else
		{
		if (Plc_aperto==True)
			{
			if(plcGo())
				{
                XtSetSensitive(GetWidgetByName("RUNPB"),False);
                XtSetSensitive(GetWidgetByName("HALTPB"),True);

				if (XtIsManaged(GetWidgetByName("forout")))
					XtUnmanageChild(GetWidgetByName("forout"));
				XtSetSensitive(GetWidgetByName("LOAD-PLC"),False);
    			XtSetSensitive(GetWidgetByName("RESTORE"),False);
				Plc_status = loadIcona(GetWidgetByName("Left_icon"), 
							&IconPplc_run);
				if (Plc_status!=NULL)
    				XtVaSetValues(GetWidgetByName("Left_icon"),
						XmNlabelPixmap, Plc_status, NULL);

				/* debug possibile : ricerca dei simboli richiesti se esistono ovvero
				compilazione delle strutture plcvar_t                                */
				changedebug=1;
				for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
               		{
					length=strlen(TestoNomiVariabili[ContaVariabili]);
					if(length!=0)
   						{   
						CercaVariabile((Widget)NULL,ContaVariabili);
						}
    				}
				AggiornaControlloVariabili();
    			}		
			else
				{
				invXerror(IDS_SIGLA_PLC2,NO_ISAKER,"%d");
				}
			}
		else
			{
			invXerror(IDS_SIGLA_PLC2,NO_PLC_OPEN,"%d");
			}
		}
    break;

    case    HALT_BUTTON:     /* HALT plc   */
    isa_status=plcStStatus(&rv);
    if (isa_status==PLCERR_TIMEOUT)
        {
        invXerror(IDS_SIGLA_PLC2,NO_ISAKER,"%d");
        Plc_status = loadIcona(GetWidgetByName("Left_icon"), &IconPplc_absent);
        if (Plc_status!=NULL)
            XtVaSetValues(GetWidgetByName("Left_icon"),XmNlabelPixmap, 
					Plc_status, NULL);
        }
    else
        {
        if (!macchinaIsStart())
           	{
            if(plcHalt())
               	{
                XtSetSensitive(GetWidgetByName("RUNPB"),True);
                XtSetSensitive(GetWidgetByName("HALTPB"),False);

                XtSetSensitive(GetWidgetByName("LOAD-PLC"),True);
                XtSetSensitive(GetWidgetByName("RESTORE"),True);
                Plc_status = loadIcona(GetWidgetByName("Left_icon"), 
					&IconPplc_halt);
                if (Plc_status!=NULL)
                   	XtVaSetValues(GetWidgetByName("Left_icon"),XmNlabelPixmap,
						 Plc_status, NULL);
                }
            else
               	invXerror(IDS_SIGLA_PLC2,NO_PLC_HALT,"%d");
           	}
        else
            {
            invXerror(IDS_SIGLA_PLC2,MACCHINA_IN_START,NULL);
            }
		}
	break;

	case	START_BUTTON:			/* start debug e/o trace */
	if((typedebug==0)||(typedebug==1)) 
		{		/* debug variabili */
		if(plcStatus())
			{
            XtSetSensitive(GetWidgetByName("STARTPB"),False);
            XtSetSensitive(GetWidgetByName("STOPPB"),True);
			changedebug=1;
			IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
			}
		}
	break;

    case	STOP_BUTTON:			/* stop debug e/o trace */
	if((typedebug==0)||(typedebug==1)) 
        {
        XtSetSensitive(GetWidgetByName("STARTPB"),True);
        XtSetSensitive(GetWidgetByName("STOPPB"),False);

		if (IntervaDebug!=NULL)
			XtRemoveTimeOut( IntervaDebug );

		/* se era attivo il trace delle variabili provo a visualizzare il buffer 
			memorizzato fino a quel momento */
		if(typedebug==1)
			{
			rv = plcTraceStop();
            if (rv) 
				invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_STOP,"%v",rv);

			/* azzeramento dei buffer che non sono stati utilizzati */
			for(i=numero_tracebuf;i<MAX_BUFFER_NUMBER;i++)
				for(j=0;j<NUMERO_CAMPIONI;j++)
					tracebuf[i][j]= -1;

            VisualizzaTrace(0);

			if (TRIGGER_PRETRIGGER==1)
    			{
				if (Trovato_pre_trigger==True)
					{
					wid=GetWidgetByName("DRAIOS");
					Trovato_pre_trigger=False;
    				/* ricerca del ciclo di trigger nel caso di pretrigger */
    				Old_y=tracebuf[0][0] & Trigger_Mask;

    				index=1;
					Numero_buffer=0;
					Ciclo_trigger=0;
    				while ((Ciclo_trigger==0)&&(Numero_buffer<MAX_BUFFER_NUMBER))
        				{
        				if (SALITA_DISCESA==0)
           					{
           					if ((Old_y!=(tracebuf[Numero_buffer][index] & Trigger_Mask))
									&&(Old_y==Trigger_Mask))
               					Ciclo_trigger=(Numero_buffer*NUMERO_CAMPIONI)+index;
           					else
               					Old_y=tracebuf[Numero_buffer][index] & Trigger_Mask;
           					}
        				else
           					{
           					if ((Old_y!=(tracebuf[Numero_buffer][index] & Trigger_Mask))
									&&(Old_y==0))
               					Ciclo_trigger=(Numero_buffer*NUMERO_CAMPIONI)+index;
           					else
               					Old_y=tracebuf[Numero_buffer][index] & Trigger_Mask;
           					}
						if (index<400)
        					index++;
						else
							{
							index=0;
							Numero_buffer++;
							}	
        				}
					if (Ciclo_trigger<=NUMERO_CAMPIONI)
						{
						posizione_cursore=DISTXBOX+Ciclo_trigger;
						XSetForeground (XtDisplay(wid), gc, colori_allocati[4].pixel);
           				XDrawLine(XtDisplay(wid),pixmap,gc,DISTXBOX+Ciclo_trigger,
							DISTYBOX,DISTXBOX+Ciclo_trigger,LARGYBOX+DISTYBOX);
						}
           			XSetForeground(XtDisplay(wid), gc, BlackPixelOfScreen(XtScreen(wid)));
					XCopyArea(XtDisplay(wid),pixmap,XtWindow(wid),gc,0,0,
						LARGX_DRA,LARGY_DRA,0,0);

					get_string=GetMessageString(MemLin,"CICLO");
           			sprintf(text,"%s  %2d",get_string,Ciclo_trigger);
           			str=XmStringCreateLocalized(text);
           			XtVaSetValues(GetWidgetByName("Status_label"),
						XmNlabelString,str,NULL);
           			XmStringFree(str);
    				}
				else if (Trovato_pre_trigger==False)
					invXerror(IDS_SIGLA_PLC2,NO_FRONTE_TRIGGER,"%d");
				}

			XtVaGetValues(GetWidgetByName("SCWIOS"),XmNverticalScrollBar, 
				&sb_ios_wid, NULL);
            xncMovePointerTo(sb_ios_wid);

            Debug_status = loadIcona (GetWidgetByName("Right_icon"), 
				&IconPplc_var);
            if (Debug_status!=NULL)
               	XtVaSetValues(GetWidgetByName("Right_icon"),XmNlabelPixmap, 
					Debug_status, NULL);
            typedebug=0;    
            XtSetSensitive(GetWidgetByName("ios1"),True);
            XtSetSensitive(GetWidgetByName("ios2"),True);
            XtSetSensitive(GetWidgetByName("ios3"),True);
            XtSetSensitive(GetWidgetByName("ios4"),True);
			}
		}
	break;

	case    SAL_DIS_BUTTON:      /* fronte di salita o di discesa del trigger  */
        SALITA_DISCESA=!(SALITA_DISCESA);
    if (SALITA_DISCESA==0)
        {               
        XmString dstop;
		get_string=GetMessageString(MemLin,"DISCESA");
        dstop = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstop,NULL);
        XmStringFree(dstop);
        }
   else
        {              
		get_string=GetMessageString(MemLin,"SALITA");
        dstart = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstart,NULL);
        XmStringFree(dstart);
        }
    break;

	case    DEC_EX_BUTTON:      
		/* visualizzazione decimale o esadecimale per le variabili di scambio */
	if (typeshow==1)
		typeshow=2;
	else if (typeshow==2)
		typeshow=1;
    if (typeshow==1)
        {               
        XmString dstop;
		get_string=GetMessageString(MemLin,"Hexadecimal");
        dstop = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstop,NULL);
        XmStringFree(dstop);
        }
   else if (typeshow==2)
        {              
		get_string=GetMessageString(MemLin,"Decimal");
        dstart = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstart,NULL);
        XmStringFree(dstart);
        }
   break;

	case	TRACE_BUTTON:	/* trace variabili */
	rv = plcTraceStop();
    if (rv) 
		invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_STOP,"%v",rv);
    rv = plcTraceClear();
    if (rv) 
		invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_CLEAR,"%v",rv);
    /*
    Dopo il reset della situazione precedente procediamo
    al set della nuova situazione
    */
	Trigger_Mask=1;
	numero_tracebuf=0;
	/* azzera le variabili dello zoom */
    zoom=1;
    Min_Trace=0;
    Max_Trace=NUMERO_CAMPIONI;
	for(i=0;i<9;i++)
    	sprintf(Griglia_trace[i],"%d",0+i*50);
	
    indice_temporaneo=0;
    for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
    	{
       	if ((length=strlen(TestoNomiVariabili[ContaVariabili])) !=0)
			{
			if (TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO) 
				{
				strcpy(NomeTemporaneo,TestoNomiVariabili[ContaVariabili]);
				if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)'[')) != NULL )
    				{
    				Posizioneparentesi[0]='_';
    				if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)',')) != NULL )
        				{
        				Posizioneparentesi[0]='_';
        				}
    				if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)']')) != NULL )
        				{
        				Posizioneparentesi[0]='\0';
        				}
    				} 
           		rv = plcTraceVar(NomeTemporaneo);
       			if (rv) 
					invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_VAR,"%n",NomeTemporaneo);
				Trigger_Mask*=2;
				}
			else if (TipoVariabili[ContaVariabili]==VARIABILE_PLC)
				{
				rv = plcTraceAddr((unsigned char *)Controllo_Var[indice_temporaneo].addr);
                if (rv) 
                   	invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_VAR,"%n",NomeTemporaneo);
                Trigger_Mask*=2;
				indice_temporaneo++;
				}
			}
       	}       

    if(plcStatus()) 
		{
		if (XtIsManaged(GetWidgetByName("Visualizzatore"))==True)
			XtUnmanageChild(GetWidgetByName("Visualizzatore"));

		typedebug=1;

        XtSetSensitive(GetWidgetByName("STARTPB"),False);
        XtSetSensitive(GetWidgetByName("STOPPB"),True);

		Debug_status = loadIcona (GetWidgetByName("Right_icon"), 
				&IconPplc_trace);
		if (Debug_status!=NULL)
        	XtVaSetValues(GetWidgetByName("Right_icon"),XmNlabelPixmap, 
				Debug_status, NULL);

		XtSetSensitive(GetWidgetByName("ios1"),False);
        XtSetSensitive(GetWidgetByName("ios2"),False);
        XtSetSensitive(GetWidgetByName("ios3"),False);
        XtSetSensitive(GetWidgetByName("ios4"),False);

		length=strlen(TriggerNome); /* se non c'e' il trigger comincio subito il trace */
        if (length==0)
			{
            IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
			rv=plcTraceStart();
           	if (rv)
				{
				invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_START,"%v",rv);
				if (IntervaDebug!=NULL)
           			XtRemoveTimeOut( IntervaDebug );
				}
			}
		else 
			{
			if (TRIGGER_PRETRIGGER==0)
				{
				Old_Trigger_Value=*(char *)StrutturaTrigger.pval;
				IntervaDebugFronte=XtAppAddTimeOut(plc2AppContext,TEMPOQFRONTE, 
					scadFrontedebug, NULL);
				}
			else if (TRIGGER_PRETRIGGER==1)
				{
				/* inserisco nel trace il trigger */
				/* Trasformo le scritture di vettori fatte con [x,x]
                   nella convenzione adottata da isaker */
        		strcpy(NomeTemporaneo,TriggerNome);
        		if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)'[')) != NULL )
            		{
            		Posizioneparentesi[0]='_';
            		if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)',')) != NULL )
                		{
                		Posizioneparentesi[0]='_';
                		}
            		if ( (Posizioneparentesi=strrchr(NomeTemporaneo,(int)']')) != NULL )
                		{
                		Posizioneparentesi[0]='\0';
                		}
            		}
				rv = plcTraceVar(NomeTemporaneo);
        		if (rv) 
					invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_VAR,"%v",rv);

				/* lancio il trace */
				IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
            	rv=plcTraceStart();
            	if (rv)
                	{
					invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_START,"%v",rv);
                	if (IntervaDebug!=NULL)
                    	XtRemoveTimeOut( IntervaDebug );
                	}
				/* lancio la ricerca del fronte */
				Old_Trigger_Value=*(char *)StrutturaTrigger.pval;
                IntervaDebugFronte=XtAppAddTimeOut(plc2AppContext,TEMPOQFRONTE, 
					scadFrontedebug, NULL);
				}
			}
		}
	break;

	case    PRE_TRIPB_BUTTON:      /* trigger o pretrigger nel trace delle variabili */
        TRIGGER_PRETRIGGER=!(TRIGGER_PRETRIGGER);
    if (TRIGGER_PRETRIGGER==0)
        {               
        XmString dstop;
		get_string=GetMessageString(MemLin,"TRIGGER");
        dstop = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstop,NULL);
        XmStringFree(dstop);
        }
   else
        {              
		get_string=GetMessageString(MemLin,"PRETRIGGER");
        dstart = XmStringCreateLocalized (get_string); /* crea stringa */
        XtVaSetValues(w,XmNlabelString,dstart,NULL);
        XmStringFree(dstart);
        }
    break;
	}
}


/*
 --------------------------------------------------------------------
    Funzione aggios_large 
  
    Funzione che disegna la Ios

    Input: 	nessuno
    Output: nessuno 
 --------------------------------------------------------------------
*/
void aggios_large()
{
Window  window;
Display *display;
int	i,j,l,NumInt=0;
int	NumIn=0,NumOut=0;
Widget	wu;
int	length=0;
char	text1[10];
char	text[80];
XmString    str;
unsigned char status_temp;

sprintf(text,"Debug IOS %2d  Release ",ios+1);
for(i=0;i<4;i++) text1[i]= *(CNI_iosbrd[board_selezionata]+displacement_selezionato+
			0x225+i);
    text1[4]='\0';
strcat (text,text1);
XtVaSetValues(XtParent(GetWidgetByName("Visualizzatore")),XmNtitle,text,NULL);

sprintf(text,"Status ");
for(i=0;i<4;i++) text1[i]= *(CNI_iosbrd[board_selezionata]+displacement_selezionato+
		0x25b+i);
    text1[4]='\0';
strcat (text,text1);
str=XmStringCreateLocalized(text);
XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,str,NULL);
XmStringFree(str);

sprintf(text,"Global counter%5d",*(unsigned short int*) (CNI_iosbrd[board_selezionata]+
		displacement_selezionato+0x223));
str=XmStringCreateLocalized(text);
XtVaSetValues(GetWidgetByName("Error_label"),XmNlabelString,str,NULL);
XmStringFree(str);

LINE_STATE=*(unsigned short int*) (CNI_iosbrd[board_selezionata]+
		displacement_selezionato+0x25f);
if  (LINE_STATE) 
    sprintf(text,"Unstable");
else
    sprintf(text,"Stable");

str=XmStringCreateLocalized(text);
XtVaSetValues(GetWidgetByName("Stable_label"),XmNlabelString,str,NULL);
XmStringFree(str);

typedebug=3;

wu=GetWidgetByName("DRAIOS");
XtVaGetValues(wu,XmNuserData,&gc,NULL);
window=XtWindow(wu);
display=XtDisplay(wu);

/* Clear all the pixmap */
XSetForeground (display, gc, WhitePixelOfScreen (XtScreen (wu)));
XSetForeground (display, gc, colori_allocati[0].pixel);
XFillRectangle (display, pixmap, gc, 0, 0, LARGX_DRA, LARGY_DRA);
XSetForeground (display, gc, BlackPixelOfScreen (XtScreen (wu)));

/* memorizzo gli stati e gli errori */
for(i=0;i<32;i++)
    {
    status[i]=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+i);
    error[i]=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+0x22d+i);
    }

/* scrivo a sinistra il numero progressivo dei moduli */
XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (wu)));
for(i=0;i<32;i++)
   	{
    sprintf(text,"%2d",i);
    length=strlen(text);
    XDrawString(display,pixmap,gc,0,DIST_TOP_NUM+(i+1)*ELARGYL,text,length);
    }

/* visualizzazione contorno scatole piccole */
MODULO_SPECIALE=False;
for(j=0;j<32;j++)
    {
    if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x40)==0) 
        {
		/*il modulo  esiste*/
        l=8;        /* modulo speciale */
        if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x80)==0) 
			{
			NumOut=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x3;
			NumIn=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0xC;
			NumIn=NumIn>>2;
			l=NumIn+NumOut;
            /* l=3; il modulo  e` normale*/
			}
        else
            MODULO_SPECIALE=True; /* c'e' almeno un modulo speciale */
        for(i=0;i<l;i++)
            XDrawRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+j*ELARGYL,
				ILARGXL,ILARGYL);
        }
    }

/* Visualizzazione errori */
if (MODULO_SPECIALE==True)
    {
    for(j=0;j<32;j++)
        {
        if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x40)==0) 
            {
            sprintf(text,"%d",error[j]);
            length=strlen(text);
            XDrawString(display,pixmap,gc,DIST_ERR_SX_SPEC,DIST_TOP_ERR+j*ELARGYL,
				text,length);
            }
        }
    XtVaSetValues(GetWidgetByName("Visualizzatore"),XmNwidth,636,NULL);
    }
else
    {
    for(j=0;j<32;j++)
        {
        if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x40)==0) 
            {
            sprintf(text,"%d",error[j]);
            length=strlen(text);
            XDrawString(display,pixmap,gc,DIST_ERR_SX_NO_SPEC,DIST_TOP_ERR+j*ELARGYL,
				text,length);
            }
        }
    XtVaSetValues(GetWidgetByName("Visualizzatore"),XmNwidth,360,NULL);
    }

for(j=0;j<32;j++)
	{
	status_temp=status[j];
	
	if ((status_temp >> 4) & 1)
        {
        XFillRectangle(display,pixmap,gc,0,DIST_TOP_NUM+(j*ELARGYL),DIST_SX-3,ELARGYL+2);

        /* scrivo a sinistra il numero progressivo dei moduli */
        XSetForeground (display, gc, WhitePixelOfScreen (XtScreen (wu)));
        sprintf(text,"%2d",j);
        length=strlen(text);
        XDrawString(display,pixmap,gc,0,DIST_TOP_NUM+(j+1)*ELARGYL,text,length);

        XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (wu)));
        }
    else if ((status_temp >> 5) & 1)
        {
        }
	else 
		{
		if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x80)==0) 
			/* modulo  normale*/
			{
			switch(*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x3)
				{
				case	0:		/* nessun output ovvero 1 o 3 input */
					XSetForeground (display, gc, colori_allocati[3].pixel);
					NumInt=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0xC;
					NumInt=NumInt>>2;
					for(i=0;i<NumInt;i++)
						{
						if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+
							j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
						}
					break;

				case	1:		/* un output ovvero 0 o 2 input */
					XSetForeground (display, gc, colori_allocati[4].pixel);
					if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+
							512+j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+0*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
					XSetForeground (display, gc, colori_allocati[3].pixel);
					NumInt=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0xC;
					NumInt=NumInt>>2;
					for(i=1;i<1+NumInt;i++)
						{
						if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+
							512+j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
						}
					break;

				case	2:		/* 2 outpu  ovvero 1 input */
					XSetForeground (display, gc, colori_allocati[3].pixel);
					if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+
							512+j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+0*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
					XSetForeground (display, gc, colori_allocati[4].pixel);
					for(i=1;i<3;i++)
						{
						if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+
							512+j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
						}
					break;

				case	3:		/* tre output ovvero 0 input */
					XSetForeground (display, gc, colori_allocati[4].pixel);
					for(i=0;i<3;i++)
						{
						if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+
							512+j)&0x40)==0) /*il modulo  esiste*/
						XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
							j*ELARGYL,ILARGXL,ILARGYL);
						}
					break;

				default:
					break;
				}
			}
		else
			{				/* modulo speciale */
			XSetForeground (display, gc, colori_allocati[4].pixel);
			for(i=0;i<4;i++)
				{
				if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x40)
						==0) /*il modulo  esiste*/
					XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
						j*ELARGYL,ILARGXL,ILARGYL);
				}
			XSetForeground (display, gc, colori_allocati[3].pixel);
			for(i=4;i<8;i++)
				{
				if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)&0x40)
						==0) /*il modulo  esiste*/
					XFillRectangle(display,pixmap,gc,DIST_SX+i*ELARGXL,DIST_TOP+
						j*ELARGYL,ILARGXL,ILARGYL);
				}
			}
		}
	}
XCopyArea (display, pixmap, window, gc, 0, 0, LARGX_DRA, LARGY_DRA, 0, 0); 
}

/*
 --------------------------------------------------------------------
    Funzione serial_large 
  
    Funzione chiamata alla pressione di una delle voci del 
	sottomenu Ios
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void serial_large(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
Widget	sb_ios_wid;

XtManageChild (GetWidgetByName("Visualizzatore"));
XtUnmanageChild (GetWidgetByName("Zoom_In"));
XtUnmanageChild (GetWidgetByName("Zoom_Out"));
XtUnmanageChild (GetWidgetByName("Move_Left"));
XtUnmanageChild (GetWidgetByName("Move_Right"));
XtManageChild (GetWidgetByName("Module_label"));
XtManageChild (GetWidgetByName("Byte0_label"));
XtManageChild (GetWidgetByName("Byte1_label"));
XtManageChild (GetWidgetByName("Byte2_label"));
XtManageChild (GetWidgetByName("Bit0_label"));
XtManageChild (GetWidgetByName("Bit1_label"));
XtManageChild (GetWidgetByName("Bit2_label"));

XtVaGetValues(GetWidgetByName("SCWIOS"),XmNverticalScrollBar, 
		&sb_ios_wid, NULL);
xncMovePointerTo(sb_ios_wid);

ios=(int) (clientData);
board_selezionata=n_board[ios];
displacement_selezionato=ios_dis[ios];
aggios_large();

IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
}

/*
 --------------------------------------------------------------------
    Funzione cursore 
  
    Funzione chiamata sull'evento input nella drawing area
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void cursore(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
static	Position	x, y;
int		length=0;
int     NumIn=0,NumOut=0;
Window  window;
Display *display;
XmString	str;
char	text[20];
char    *get_string;
int		valore;
int		i,j,ciclo_corrente=0;
int 	forza=0;
int     mask=1;
int     y1=DISTYBOX+6;
unsigned char status_temp;
int		Buffer_corrente=0;

window=XtWindow(w);
display=XtDisplay(w);

if(typedebug==3)	 /* visualizzazione i/o ios */
	{
	XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) callData;
	XEvent *event = cbs -> event;
	Display *dpy = event ->xany.display;
	if(event->xany.type == ButtonPress)
      	{
        x= event->xbutton.x;
        y= event->xbutton.y;
		if ((y>=DIST_TOP)&&(x>=DIST_SX))
			{
			y=(y-DIST_TOP)/ELARGYL;
			x=(x-DIST_SX)/ELARGXL;
			XCopyArea (dpy,pixmap,event->xany.window,gc,DIST_SX+x*ELARGXL,DIST_TOP+
				y*ELARGYL,ELARGXL,ELARGYL,DIST_SX+x*ELARGXL,DIST_TOP+y*ELARGYL);

			status_temp=status[y];

			/* Esiste il modulo y ??? e non e' ne' in fault ne' in communication error	*/
			if(((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+y)&0x40)==0)&&(((status_temp >> 4) & 1)==0)&&(((status_temp >> 5) & 1)==0))
				{
				/*	modulo speciale ???	*/
				j=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+y)&0x80;
				NumOut=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+y)&0x3;
                NumIn=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+y)&0xC;
				NumIn=NumIn>>2;
				if( ((j==0)&&(x<(NumOut+NumIn))) || ((j==0x80)&&(x<8)) )
					{
					/* setta o resetta il byte di i/o in aggiornamento */
					if(elemio[x][y]==0)
						{
						elemio[x][y]=1;
        					IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
						valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+y*8+x);
						/* aggiona la copia */
						filtio[x][y]=valore;
						XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (w))); 
						for (j= 7,i=0; j>=0; j--,i++)
							{
							if ((valore >> j) & 1)
								text[i]='1';
							else
								text[i]='0';
							if(i==3)
								{
								text[i+1]=' ';
								i=i+1;
								}
							}
						text[9]='\0';
						length=strlen(text);
						XDrawString(display,window,gc,DIST_STRING_SX+x*ELARGXL,DIST_TOP_ERR+y*ELARGYL,text,length);

                        /* forzatura uscita  solo se il plc e' in halt */
                        if (plcStatus()==0)
                           	{
                            switch(*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+y)&0x3) /* modulo  normale*/   
                               	{
                                case    0:              /* nessun output ovvero 3 input */
                                       	break;

                                case    1:              /* un output ovvero 2 input */
                                       	if ((x==0))
                                           	{
                                           	XtManageChild(GetWidgetByName("forout"));
                                			XMapRaised (XtDisplay (plc2ShW),XtWindow(GetWidgetByName("forout")));
											xncMovePointerTo(GetWidgetByName("bit7"));
											get_string=GetMessageString(MemLin,"MODULO");
                                           	sprintf(text,"%s%2d  Byte%2d",get_string,y,x);
                                   			XtVaSetValues(XtParent(GetWidgetByName("forout")),XmNtitle,text,NULL);
											forza=1;
                                           	}
										else
											forza=0;
                                        break;

                                case    2:              /* un input ovvero 2 output */
                                      	if ((x==1)||(x==2))
                                           	{
                                           	XtManageChild(GetWidgetByName("forout"));
                                			XMapRaised (XtDisplay (plc2ShW),XtWindow(GetWidgetByName("forout")));
											xncMovePointerTo(GetWidgetByName("bit7"));
											get_string=GetMessageString(MemLin,"MODULO");
                                            sprintf(text,"%s%2d  Byte%2d",get_string,y,x);
                                   			XtVaSetValues(XtParent(GetWidgetByName("forout")),XmNtitle,text,NULL);
											forza=1;
                                           	}
										else
											forza=0;
                                        break;
                               	case    3:              /* tre output ovvero 0 input */
                                 		XtManageChild(GetWidgetByName("forout"));
                                   		XMapRaised (XtDisplay (plc2ShW),XtWindow(GetWidgetByName("forout")));
										xncMovePointerTo(GetWidgetByName("bit7"));
										get_string=GetMessageString(MemLin,"MODULO");
                                        sprintf(text,"%s%2d  Byte%2d",get_string,y,x);
                                   		XtVaSetValues(XtParent(GetWidgetByName("forout")),XmNtitle,text,NULL);
										forza=1;

                                      	break;

                         		}
                                numero_modulo=y;
                                numero_byte=x;
							if (forza==1)
								{
                           		/* aggiorna il widget di forzatura uscita */
 								if ((valore >> 7) & 1)
                               		XtVaSetValues(GetWidgetByName("bit7"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit7"),XmNset,False,NULL);
                           		if ((valore >> 6) & 1)
                               		XtVaSetValues(GetWidgetByName("bit6"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit6"),XmNset,False,NULL);   
                       			if ((valore >> 5) & 1)
                               		XtVaSetValues(GetWidgetByName("bit5"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit5"),XmNset,False,NULL);   
                       			if ((valore >> 4) & 1)
                               		XtVaSetValues(GetWidgetByName("bit4"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit4"),XmNset,False,NULL);   
                       			if ((valore >> 3) & 1)
                               		XtVaSetValues(GetWidgetByName("bit3"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit3"),XmNset,False,NULL);   
                       			if ((valore >> 2) & 1)
                               		XtVaSetValues(GetWidgetByName("bit2"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit2"),XmNset,False,NULL);   
                      			if ((valore >> 1) & 1)
                               		XtVaSetValues(GetWidgetByName("bit1"),XmNset,True,NULL);
                           		else
                               		XtVaSetValues(GetWidgetByName("bit1"),XmNset,False,NULL);   
                       			if ((valore >> 0) & 1)
                               		XtVaSetValues(GetWidgetByName("bit0"),XmNset,True,NULL);
                          		else
                              		XtVaSetValues(GetWidgetByName("bit0"),XmNset,False,NULL);
								}
							}
						}
					else
						{
						elemio[x][y]=0;
						XCopyArea (display, pixmap, window, gc,DIST_SX+x*ELARGXL,DIST_TOP+y*ELARGYL,ELARGXL,ELARGYL,DIST_SX+x*ELARGXL,DIST_TOP+y*ELARGYL);
						}
					}
				}
			}
		}
	}
else if ((typedebug==1)||(typedebug==0))
	{
	XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) callData;
    XEvent *event = cbs -> event;
    Display *dpy = event ->xany.display;
    if(event->xany.type == ButtonPress)
        {
		if ((event->xbutton.x>DISTXBOX)&&(event->xbutton.x<DISTXBOX+LARGXBOX)) /* se sono nel range del box */
			{
        	posizione_cursore= event->xbutton.x;
			VisualizzaTrace(1);
			XSetForeground (dpy, gc, colori_allocati[4].pixel);
			XDrawLine(dpy,pixmap,gc,posizione_cursore,DISTYBOX,posizione_cursore,LARGYBOX+DISTYBOX);
			ciclo_corrente=Min_Trace+((posizione_cursore-DISTXBOX)/zoom);
			get_string=GetMessageString(MemLin,"CICLO");
			sprintf(text,"%s  %2d",get_string,ciclo_corrente);
            str=XmStringCreateLocalized(text);
			XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,str,NULL);
			XmStringFree(str);
			Buffer_corrente=Min_Trace/NUMERO_CAMPIONI;
			XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (w)));
			for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
        		{
        		if (((StrutturaVariabili[ContaVariabili].type==ISAVAR_T_BOOL)&&(TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO))||((TipoVariabili[ContaVariabili]==VARIABILE_PLC)&&(LONG_FLOAT[ContaVariabili]==CHAR_TYPE)))
            		{
					if ((tracebuf[Buffer_corrente][ciclo_corrente-(NUMERO_CAMPIONI*Buffer_corrente)-1] & mask)==0)
						{
						sprintf(text,"0");
            			length=strlen(text);
            			XDrawString(dpy,pixmap,gc,DISTXBOX-15,y1+15,text,length);
						}
					else
						{
						sprintf(text,"1");
                        length=strlen(text);
                        XDrawString(dpy,pixmap,gc,DISTXBOX-15,y1+15,text,length);
						}
					y1+=DISTYTRACE;
					mask*=2;
					}
				}
			XCopyArea (display,pixmap,window,gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
			}
		}
	}
 /* make sure that dialog is raised to top of window stack */
XMapRaised (XtDisplay (plc2ShW),XtWindow(GetWidgetByName("SCWIOS")));
}

/*
 --------------------------------------------------------------------
    Funzione traverse 
  
    ??????????????????????????
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void traverse(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
 	XmTraverseObscuredCallbackStruct  *cbs =(XmTraverseObscuredCallbackStruct *)callData;

	XmScrollVisible (w,cbs->traversal_destination,0,0);
}

/*
 ----------------------------------------------------------------------------
    Funzione cbkSelezionaFile
  
    Funzione chiamata ogni qual volta si seleziona
    un programmaplc o un file di variabili.
    INPUT:	tipofile=	tipo di file
    		filename=	address nome del file
    		pdata=		puntatore ai dati:	ID_DIR_FILEVR variabili x lettura
   									ID_DIR_FILEVW variabili x scrittura
  									ID_DIR_FILEPR programma  plc x lettura
    OUTPUT:	Nessuno
 ----------------------------------------------------------------------------
*/
#ifdef _OLD_FILE_SEL
void cbkSelezionaFile (int tipofile, char *filename, XtPointer pdata)
#else
void cbkSelezionaFileNew (int iddir,int tipofile, char *filename, XtPointer pdata)
#endif
{
char    buf[MAX_PATH_LEN], *CampiRiga;
XmString        wid_label_string;
int     length,rv;
struct  stat statb;
long    len;
FILE    *fp;
int	val;

#ifdef Linux
#else
#ifndef _OLD_FILE_SEL
xncNfsbHide(fbox);
#endif
#endif

val=(int)pdata;

/*---file programma plc per lettura---*/

#ifdef _OLD_FILE_SEL
if(val==ID_DIR_FILEPR && tipofile==ID_DIR_PPLC)
#else
if(val==ID_DIR_FILEPR)
#endif
	{
	/* caricamento plc      */
	rv=plcLoadProg(filename);
  	if (rv == PLCERR_TIMEOUT) 
		{
        while (plcLdStatus(&rv) == PLCERR_TIMEOUT) 
			{
			fflush(stdout);
            sleep(1);
            }
        }
    else if (rv) 
		{
		invXerror(IDS_SIGLA_PLC2,NO_PLC_LOAD_PROG,"%v",rv);
       	}
	xncMovePointerTo(GetWidgetByName("RUNPB"));
	}

/*---file variabili per lettura---*/

#ifdef _OLD_FILE_SEL
else if(val==ID_DIR_FILEVR  && tipofile==ID_DIR_VARP)
#else
else if(val==ID_DIR_FILEVR)
#endif
	{
	/* reason == FILE_OPEN */
    /* make sure the file is a regular text file and open it */
    if (stat (filename, &statb) == -1 || (statb.st_mode & S_IFMT) != S_IFREG ||!(fp = fopen (filename, "r")))
    	{
       	perror (filename);
		invXerror(IDS_SIGLA_PLC2,NO_LOAD,NULL);
        XtFree (filename);
        return;
       	}

	/* reset della situazione precewdente */
	memset(TestoNomiVariabili[0],'\0',16);
    for (ContaVariabili=1;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
        strncpy(TestoNomiVariabili[ContaVariabili],TestoNomiVariabili[0],17);
        
    for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
        {
        wid_label_string=XmStringCreateSimple(" ");
        XtVaSetValues(ValoreVariabili[ContaVariabili],XmNlabelString,wid_label_string,NULL);
        XmStringFree(wid_label_string);
        LONG_FLOAT[ContaVariabili]=-1;
        }

	ContaVariabili=0;
	while (fgets(buf,MAX_PATH_LEN,fp) != NULL )
		{
		if ( (CampiRiga=strchr(buf,(int)'#')) != NULL )
			{
			CampiRiga= strtok(buf, "#");
			if (CampiRiga[0]=='l')
           		{
            	LONG_FLOAT[ContaVariabili]=LONG_TYPE;
           		}
        	else if (CampiRiga[0]=='i')
           		{
            	LONG_FLOAT[ContaVariabili]=INT_TYPE;
           		}
        	else if (CampiRiga[0]=='u')
           		{
            	LONG_FLOAT[ContaVariabili]=UINT_TYPE;
           		}
        	else if (CampiRiga[0]=='d')
           		{
            	LONG_FLOAT[ContaVariabili]=ULONG_TYPE;
           		}
        	else if (CampiRiga[0]=='f')
           		{
            	LONG_FLOAT[ContaVariabili]=FLOAT_TYPE;
           		}
			else if (CampiRiga[0]=='c')
            	{
            	LONG_FLOAT[ContaVariabili]=CHAR_TYPE;
            	}
			else if (CampiRiga[0]=='s')
            	{
            	LONG_FLOAT[ContaVariabili]=PUNT_CHAR_TYPE;
            	}
			else if (CampiRiga[0]=='x')
            	{
            	LONG_FLOAT[ContaVariabili]=STRING_TYPE;
		    	}
			Lunghezza_Stringa[ContaVariabili]=atoi(++CampiRiga);    
            CampiRiga= strtok(NULL, "+");
            strncpy(TestoNomiVariabili[ContaVariabili],CampiRiga,strlen(CampiRiga)-1);
			}
		else
			{
			/* compatibilita' con versioni precedenti */
			if (buf[0]=='l')
                {
                LONG_FLOAT[ContaVariabili]=LONG_TYPE;
				CampiRiga=buf;
				strncpy(TestoNomiVariabili[ContaVariabili],++CampiRiga,strlen(buf)-1);
                }
            else if (buf[0]=='f')
                {
                LONG_FLOAT[ContaVariabili]=FLOAT_TYPE;
				CampiRiga=buf;
                strncpy(TestoNomiVariabili[ContaVariabili],++CampiRiga,strlen(buf)-1);
                }
            else if (buf[0]=='c')
                {
                LONG_FLOAT[ContaVariabili]=CHAR_TYPE;
				CampiRiga=buf;
                strncpy(TestoNomiVariabili[ContaVariabili],++CampiRiga,strlen(buf)-1);
                }
			else 
				{
				strncpy(TestoNomiVariabili[ContaVariabili],buf,strlen(buf)-1);
				}
			}
		ContaVariabili++;
		}

	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
		XtVaSetValues(NomiVariabili[ContaVariabili],XmNvalue,&TestoNomiVariabili[ContaVariabili],NULL);  /* inserisci nuovo testo */

	if(plcStatus())
        {
		/* debug possibile : ricerca dei simboli richiesti se esistono ovvero
		compilazione delle strutture plcvar_t                                */
		changedebug=1;
		for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
           	{
			length=strlen(TestoNomiVariabili[ContaVariabili]);
			if(length!=0)
   				{   
				CercaVariabile((Widget)NULL,ContaVariabili);
				}
    		}
		AggiornaControlloVariabili();
    	}		
	typedebug=0;
	fclose (fp);
	xncMovePointerTo(GetWidgetByName("STARTPB"));
	}

/*---file variabili per scrittura---*/

#ifdef _OLD_FILE_SEL
else if(val==ID_DIR_FILEVW  && tipofile==ID_DIR_VARP)
#else
else if(val==ID_DIR_FILEVW)
#endif
	{
    if (!(fp = fopen (filename, "w"))) 
		{
       	perror (filename);
		invXerror(IDS_SIGLA_PLC2,NO_SAVE,NULL);
       	XtFree (filename);
       	return;
       	}
     /* saving -- get text from Text widget testo */
	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
		{
		if (TipoVariabili[ContaVariabili]==VARIABILE_PLC)
			{
			if (LONG_FLOAT[ContaVariabili]==LONG_TYPE)
				sprintf(buf,"l%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==INT_TYPE)
				sprintf(buf,"i%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==UINT_TYPE)
				sprintf(buf,"u%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==ULONG_TYPE)
				sprintf(buf,"d%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==FLOAT_TYPE)
				sprintf(buf,"f%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==CHAR_TYPE)
                sprintf(buf,"c%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==PUNT_CHAR_TYPE)
                sprintf(buf,"s%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			else if (LONG_FLOAT[ContaVariabili]==STRING_TYPE)
                sprintf(buf,"x%d#%s",Lunghezza_Stringa[ContaVariabili],TestoNomiVariabili[ContaVariabili]);
			}
		else
			sprintf(buf,"%s",TestoNomiVariabili[ContaVariabili]);

       	len = strlen(buf);
       	/* write it to file (check for error) */
       	if (fwrite (buf, sizeof (char), len, fp) != len)
			invXerror(IDS_SIGLA_PLC2,NO_WRITE,NULL);
       	else 
			{
       		/* make sure a newline terminates file */
       		if (TestoNomiVariabili[ContaVariabili][len-1] != '\n')
       			fputc ('\n', fp);
       		}
		}
	fclose (fp);
	xncMovePointerTo(GetWidgetByName("STARTPB"));
	}
XtFree (filename);
stampaMes("Tipo file: %d Nome selezionato: %s", tipofile,filename);
} 

/*
 --------------------------------------------------------------------
    Funzione forza_uscita 
  
    Funzione chiamata alla pressione di uno degli 8 bottoni
	per la forzatura delle uscite
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void forza_uscita(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int valore;
int maschera=1;
int bit_scelto;

bit_scelto=(int)clientData;

if (plcStatus()==0)
	{
	if (bit_scelto==0)
		{
		maschera=1;
		valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
		valore^=maschera;
		*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
		}
	else if (bit_scelto==1)
                {
                maschera=2;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }

	else if (bit_scelto==2)
                {
                maschera=4;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }

	else if (bit_scelto==3)
                {
                maschera=8;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }
	else if (bit_scelto==4)
                {
                maschera=16;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }
	else if (bit_scelto==5)
                {
                maschera=32;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }
	else if (bit_scelto==6)
                {
                maschera=64;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }
	else if (bit_scelto==7)
                {
                maschera=128;
                valore=*(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte);
                valore^=maschera;
                *(CNI_iosbrd[board_selezionata]+displacement_selezionato+numero_modulo*8+numero_byte)=(char) valore;
                }
	}

}

/*
 --------------------------------------------------------------------
    Funzione TipoDiVariabile 
  
    Funzione chiamata alla pressione di un bottone nel pannello
	di definizione delle variabili
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void TipoDiVariabile(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
XmString    wid_label_string;
char        *testo;
if ((int)clientData!=100)
	{
	LONG_FLOAT[VariabileCorrente]=(int)clientData;
	XtVaGetValues(GetWidgetByName("LungStr"),XmNvalue,&testo,NULL);
	if (testo!=NULL)
       	{
        Lunghezza_Stringa[VariabileCorrente]=atoi(testo);
        }
	else
		{
		if ((int)clientData==PUNT_CHAR_TYPE || (int)clientData==STRING_TYPE)
			Lunghezza_Stringa[VariabileCorrente]=10;
		else
			Lunghezza_Stringa[VariabileCorrente]=0;
		}
	AggiornaControlloVariabili();
    XtVaSetValues(GetWidgetByName("LungStr"),XmNvalue,"",NULL);
	XtUnmanageChild (GetWidgetByName("VarType"));
	xncMovePointerTo(GetWidgetByName("STARTPB"));
	}
else
	{
	memset(TestoNomiVariabili[VariabileCorrente],NULL,16);
    XtVaSetValues(NomiVariabili[VariabileCorrente],XmNvalue,&TestoNomiVariabili[VariabileCorrente],NULL);

    /*ripristina vecchio testo */

    wid_label_string=XmStringCreateSimple(" ");
    XtVaSetValues(ValoreVariabili[VariabileCorrente],XmNlabelString,wid_label_string,NULL);
    XmStringFree(wid_label_string);

	XmProcessTraversal (NomiVariabili[VariabileCorrente+1], XmTRAVERSE_UP);
	xncMovePointerTo(GetWidgetByName("STARTPB"));
    LONG_FLOAT[VariabileCorrente]=-1;
	}
}

/*
 -------------------------------------------------------------------------------
    Funzione scadStatistic
  
    Funzione chiamata ogni qual volta il timeout statistica impostato scade:
    preleva i valori max, min e curr del plc e li grafica
    Input:  client_data = eventuale puntatore ai dati
            tempoId = puntatore all'identificatore del timeout
    Output: Nessuno
 -------------------------------------------------------------------------------
*/
void scadStatistic(XtPointer client_data, XtIntervalId *tempoId)
{
static Campione_corrente=0;
int rv,x1,x2,y1,y2,index;
long tcurr,tmin,tmax;
GC  gc_stat_temp;
int     length=0;
char    text[80];
char 	*text1;
XmString    str;
int Abilita_stat;


XtVaGetValues(GetWidgetByName("DRASTAT"),XmNuserData, &gc_stat_temp,NULL);

XClearWindow(XtDisplay(drawing_a_stat),XtWindow (drawing_a_stat));

if (Campione_corrente<=CAMPIONISTAT-1)
	{
	text1=GetMessageString(MemLin,"Attendi");
	length=strlen(text1);
	XDrawString(XtDisplay(drawing_a_stat),XtWindow (drawing_a_stat),gc_stat_temp,10,50,text1,length);	
	rv = plcStatistics(&tcurr,&tmin,&tmax);
	if (rv)    
		invXerror(IDS_SIGLA_PLC2,NO_PLC_STATISTIC,"%v",rv);
	/* cerco il fattore di scala piu' opportuno */

    if ((tmax!=0) && (tmax != tmin)) 
      {
	  Campioni_statistica[Campione_corrente][0]=10+(int)((tcurr-tmin)*(80.0/(float)(tmax-tmin)));
	  }
	else 
	  {
	  Campioni_statistica[Campione_corrente][0]=10;
	  }
	Campioni_statistica[Campione_corrente][1]=10;
	Campioni_statistica[Campione_corrente][2]=90;

	sprintf(text,"%ld",tcurr);
	str=XmStringCreateLocalized(text);
	XtVaSetValues(GetWidgetByName("CurrStatPB"),XmNlabelString,str,NULL);
	XmStringFree(str);

	sprintf(text,"%ld",tmin);
    str=XmStringCreateLocalized(text);
    XtVaSetValues(GetWidgetByName("MinStatPB"),XmNlabelString,str,NULL);
	XmStringFree(str);

	sprintf(text,"%ld",tmax);
    str=XmStringCreateLocalized(text);
    XtVaSetValues(GetWidgetByName("MaxStatPB"),XmNlabelString,str,NULL);
	XmStringFree(str);

	Campione_corrente++;
	}
else if (Campione_corrente==CAMPIONISTAT)
	{
	for (index=1;index<CAMPIONISTAT;index++)
        {
		Campioni_statistica[index-1][0]=Campioni_statistica[index][0];
    	Campioni_statistica[index-1][1]=Campioni_statistica[index][1];
    	Campioni_statistica[index-1][2]=Campioni_statistica[index][2];
		}

	rv = plcStatistics(&tcurr,&tmin,&tmax);
    if (rv)    
		invXerror(IDS_SIGLA_PLC2,NO_PLC_STATISTIC,"%v",rv);

    if ((tmax!=0) && (tmax != tmin)) 
      {
      Campioni_statistica[CAMPIONISTAT-1][0]=10+(int)((tcurr-tmin)*(80.0/(float)(tmax-tmin)));
      } 
    else 
      { 
      Campioni_statistica[CAMPIONISTAT-1][0]=10;
      }
    /*Campioni_statistica[CAMPIONISTAT-1][0]=10+(int)((tcurr-tmin)*(80.0/(float)(tmax-tmin)));*/
    Campioni_statistica[CAMPIONISTAT-1][1]=10;
    Campioni_statistica[CAMPIONISTAT-1][2]=90;

	sprintf(text,"%ld",tcurr);
    str=XmStringCreateLocalized(text);
    XtVaSetValues(GetWidgetByName("CurrStatPB"),XmNlabelString,str,NULL);
    XmStringFree(str);

    sprintf(text,"%ld",tmin);
    str=XmStringCreateLocalized(text);
    XtVaSetValues(GetWidgetByName("MinStatPB"),XmNlabelString,str,NULL);
    XmStringFree(str);

    sprintf(text,"%ld",tmax);
    str=XmStringCreateLocalized(text);
    XtVaSetValues(GetWidgetByName("MaxStatPB"),XmNlabelString,str,NULL);
    XmStringFree(str);

	x1=0;
	x2=2;
	for (index=0;index<CAMPIONISTAT-2;index++)
		{
		y1=Campioni_statistica[index][0];
		y2=Campioni_statistica[index+1][0];
    	XDrawLine(XtDisplay(drawing_a_stat),XtWindow (drawing_a_stat),gc_stat_temp,x1,(100-y1),x2,(100-y2));
		y1=Campioni_statistica[index][1];
        y2=Campioni_statistica[index+1][1];
    	XDrawLine(XtDisplay(drawing_a_stat),XtWindow (drawing_a_stat),gc_stat_temp,x1,(100-y1),x2,(100-y2));
		y1=Campioni_statistica[index][2];
        y2=Campioni_statistica[index+1][2];
    	XDrawLine(XtDisplay(drawing_a_stat),XtWindow (drawing_a_stat),gc_stat_temp,x1,(100-y1),x2,(100-y2));
		x1=x2;
		x2+=2;
		}
	}
if (XtIsManaged(GetWidgetByName("MESSAGSTAT")))
	IntervaStatistic=XtAppAddTimeOut(plc2AppContext,TEMPOSTAT, scadStatistic, NULL);
else
	{
	XtRemoveTimeOut( IntervaStatistic );
	xncMovePointerTo(GetWidgetByName("STARTPB"));
	Campione_corrente=0;
	Abilita_stat=0;
	rv=plcStEnable(Abilita_stat);
	if (rv!=0)
		invXerror(IDS_SIGLA_PLC2,NO_PLC_STATISTIC_EN,"%v",rv);
	}
}

/*
 --------------------------------------------------------------------
    Funzione Statistica
  
    Funzione che attiva il box della statistica
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void Statistica(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int Abilita_stat;
int rv;

if (((int)clientData)==1)
	{
	XtRemoveTimeOut( IntervaStatistic );
	rv=plcStClear();
	if (rv!=0)
		invXerror(IDS_SIGLA_PLC2,NO_PLC_STATISTIC_CLEAR,"%v",rv);
	}
else
	{
	Abilita_stat=1;
	rv=plcStEnable(Abilita_stat);
	if (rv!=0)
		invXerror(IDS_SIGLA_PLC2,NO_PLC_STATISTIC_EN,"%v",rv);
	}
plcStStatus(&rv);
if (rv!=0)
	{
	XtManageChild (GetWidgetByName("MESSAGSTAT"));
	xncMovePointerTo(GetWidgetByName("MESSAGSTAT"));
	IntervaStatistic=XtAppAddTimeOut(plc2AppContext,TEMPOSTAT, scadStatistic, NULL);
	}
}


/*
 --------------------------------------------------------------------
    Funzione Aiuti 
  
    Funzione chiamata alla pressione di una delle voci del 
	sottomenu Aiuti
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void Aiuti(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
#ifdef Linux
printf("HELP!!!\n");
#else
invHelp(IDS_SIGLA_PLC2,1,"HELP_PLC2");
#endif
}

/*
 --------------------------------------------------------------------
    Funzione SpostaPointer 
  
    Funzione che sposta il cursore su un oggetto specifico
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void SpostaPointer(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
Widget  sb_ios_wid;
int i;

if ((int)clientData==0)
	{
	xncMovePointerTo(GetWidgetByName("STARTPB"));
	Trigger_Mask=1;
    numero_tracebuf=0;
	/* azzera le variabili dello zoom */
	zoom=1;
	Min_Trace=0;
	Max_Trace=NUMERO_CAMPIONI;
	for(i=0;i<9;i++)
    	sprintf(Griglia_trace[i],"%d",0+i*50);
	}
else if ((int)clientData==1)
	{
	XtVaGetValues(GetWidgetByName("SCWIOS"),XmNverticalScrollBar, &sb_ios_wid, NULL);
	xncMovePointerTo(sb_ios_wid);	
	}
}

/*
 -----------------------------------------------------------------------------
    Funzione plcSaveIO
  
    Questa funzione salva la configurazione in NVRAM.
    
    Input : Nessuno
    Output : Vale 1 se tutto e` andato bene, 0 se non e` installata la RAM non
    		volatile, -1 se non c'e` spazio sulla RAM non volatile.
 -----------------------------------------------------------------------------
*/
int plcSaveIO(void)
{
union nvram_ioconf_t * io;
int i,j,k,m,n;

if (! NvRam)    
    {
    return 0;
    }

io = (union nvram_ioconf_t *) Area_ios.addr; 

io -> v1.check = PLC_NVRAM_CHECK_V1;

n = 0;

for (i = 0; i < sizeof(io -> v1.board); ++i)
    io -> v1.board[i] = 0;
for (i = 0; i < sizeof(io -> v1.board); ++i) 
    {
    if (CNI_ios[i]) 
        {
        for (j = 0, m = 1; j < 8; ++j, m <<= 1) 
            {
            if (CNI_ios[i][j]) 
                {
                if (n >= (Area_ios.size
                          - sizeof(*io))/sizeof(io -> v1.images[0])+1)
                    return -1;
                io -> v1.board[i] |= m;
                for (k = 0; k < sizeof(io -> v1.images[0]); ++k) 
                    {
                    io->v1.images[n][k] =
                         CNI_ios[i][j][IOS_IOSTATUS + k]
                       & (IOS_STS_M_SPEC | IOS_STS_M_DIS | IOS_STS_M_NBTX);
                    }
                    ++n;
                }
            }
        }
    }
return 1;
} 

/*
 --------------------------------------------------------------------
    Funzione SaveIOPB 
  
    Funzione chiamata alla pressione della voce Salva IO
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void SaveIOPB(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int rv;

rv = plcSaveIO();

if (!rv)
	invXerror(IDS_SIGLA_PLC2,NO_PLC_SAVEIO,"%d");
}

/*
 --------------------------------------------------------------------
    Funzione buttontrace
  
    Funzione chiamata alla pressione del bottone trace
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void buttontrace(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
int     length=0;
Window  window;
Display *display;
XmString    str;
char    text[20];
char    *get_string;
int     i,ciclo_corrente=0;
int     mask=1;
int     y1=DISTYBOX+6;

window=XtWindow(GetWidgetByName("DRAIOS"));
display=XtDisplay(GetWidgetByName("DRAIOS"));

if (((int) clientData == 0) || ((int) clientData == 1)) 
	{
	ciclo_corrente=Min_Trace+((posizione_cursore-DISTXBOX)/zoom);
	/* zoom in */ 
	if ((int) clientData == 0)
		{
		if (zoom<=4)
			zoom*=2;
		else
			zoom=8;
		}
	 /* zoom out */ 
	else if ((int) clientData == 1)
		{
        if (zoom>=2)
        	zoom/=2;
        else
            zoom=1;
        }
	/* se possibile cerco di piazzare il ciclo selezionato al centro della finestra di trace */ 
	if (ciclo_corrente<(NUMERO_CAMPIONI/(zoom*2)))
		{
		Min_Trace=0;
		Max_Trace=NUMERO_CAMPIONI/zoom;
		posizione_cursore=DISTXBOX+(ciclo_corrente*zoom);
		if ((zoom==1)||(zoom==2))
			{
            for (i=0;i<9;i++)
				sprintf(Griglia_trace[i],"%d",0+i*(50/zoom));
			} 
		else if (zoom==4)
			{
            for (i=0;i<6;i++)
             	sprintf(Griglia_trace[i],"%d",0+i*20);
            }
        else if (zoom==8)
            {
            for (i=0;i<6;i++)
                sprintf(Griglia_trace[i],"%d",0+i*10);
            }
		}
	else if (ciclo_corrente>((NUMERO_CAMPIONI*MAX_BUFFER_NUMBER)-(NUMERO_CAMPIONI/(zoom*2))))
		{
		if (zoom==1)
           	{
            Min_Trace=NUMERO_CAMPIONI*(MAX_BUFFER_NUMBER-1);
            Max_Trace=NUMERO_CAMPIONI*MAX_BUFFER_NUMBER;
            posizione_cursore=DISTXBOX+(ciclo_corrente-Min_Trace);
			for(i=0;i<9;i++)
    		sprintf(Griglia_trace[i],"%d",Min_Trace+i*50);
            }
		else
			{
			Min_Trace=NUMERO_CAMPIONI*MAX_BUFFER_NUMBER-(NUMERO_CAMPIONI/zoom);
           	Max_Trace=NUMERO_CAMPIONI*MAX_BUFFER_NUMBER;
			posizione_cursore=DISTXBOX+((ciclo_corrente-Min_Trace)*zoom);
			if (zoom==2)
               	{
               	for (i=0;i<9;i++)
               	sprintf(Griglia_trace[i],"%d",Min_Trace+i*25);
               	} 
           	else if (zoom==4)
               	{
               	for (i=0;i<6;i++)
               	sprintf(Griglia_trace[i],"%d",Min_Trace+i*20);
               	}
			else if (zoom==8)
               	{
                for (i=0;i<6;i++)
                sprintf(Griglia_trace[i],"%d",Min_Trace+i*10);
                }
			}
		}
	else
		{
		Min_Trace=ciclo_corrente-(NUMERO_CAMPIONI/(zoom*2));
       	Max_Trace=ciclo_corrente+(NUMERO_CAMPIONI/(zoom*2));
		posizione_cursore=DISTXBOX+NUMERO_CAMPIONI/2;
		if ((zoom==1)||(zoom==2))
           	{
            for (i=0;i<9;i++)
              	sprintf(Griglia_trace[i],"%d",Min_Trace+i*(50/zoom));
            } 
        else if (zoom==4)
            {
            for (i=0;i<6;i++)
              	sprintf(Griglia_trace[i],"%d",Min_Trace+i*20);
            }
		else if (zoom==8)
            {
            for (i=0;i<6;i++)
              	sprintf(Griglia_trace[i],"%d",Min_Trace+i*10);
            }
		}
	VisualizzaTrace(1);
	XSetForeground (display, gc, colori_allocati[4].pixel);
	XDrawLine(display,pixmap,gc,posizione_cursore,DISTYBOX,posizione_cursore,LARGYBOX+DISTYBOX);
	get_string=GetMessageString(MemLin,"CICLO");
    sprintf(text,"%s  %2d",get_string,ciclo_corrente);
    str=XmStringCreateLocalized(text);
	XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,str,NULL);
	XmStringFree(str);
	XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (w)));
	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
       	{
       	if (((StrutturaVariabili[ContaVariabili].type==ISAVAR_T_BOOL)&&(TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO))||((TipoVariabili[ContaVariabili]==VARIABILE_PLC)&&(LONG_FLOAT[ContaVariabili]==CHAR_TYPE)))
       		{
			if ((tracebuf[0][ciclo_corrente] & mask)==0)
				{
				sprintf(text,"0");
       			length=strlen(text);
       			XDrawString(display,pixmap,gc,DISTXBOX-15,y1+15,text,length);
				}
			else
				{
				sprintf(text,"1");
                length=strlen(text);
                XDrawString(display,pixmap,gc,DISTXBOX-15,y1+15,text,length);
				}
			y1+=28;
			mask*=2;
			}
		}
	XCopyArea (display,pixmap,window,gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
	}
else if (((int) clientData == 2) || ((int) clientData == 3))
	{
	/* shift sinistra */ 
	if ((int) clientData == 2)
		{
		if (Min_Trace>=CAMPIONI_SHIFT)
			{
			Min_Trace-=CAMPIONI_SHIFT;
			Max_Trace-=CAMPIONI_SHIFT;
			}
		else
			{
			Min_Trace=0;
			Max_Trace=NUMERO_CAMPIONI/zoom;
			}
		}
	/* shift destra */ 
	else if ((int) clientData == 3)
		{
        if (Max_Trace<=((NUMERO_CAMPIONI*MAX_BUFFER_NUMBER)-CAMPIONI_SHIFT))
			{
			Min_Trace+=CAMPIONI_SHIFT;
            Max_Trace+=CAMPIONI_SHIFT;
			}
		else
			{
			Min_Trace=(NUMERO_CAMPIONI*MAX_BUFFER_NUMBER)-NUMERO_CAMPIONI/zoom;
            Max_Trace=NUMERO_CAMPIONI*MAX_BUFFER_NUMBER;
			}
       	}
	if ((zoom==1)||(zoom==2))
      	{
      	for (i=0;i<9;i++)
        	sprintf(Griglia_trace[i],"%d",Min_Trace+i*(50/zoom));
        } 
    else if (zoom==4)
        {
        for (i=0;i<6;i++)
          	sprintf(Griglia_trace[i],"%d",Min_Trace+i*20);
        }
    else if (zoom==8)
        {
        for (i=0;i<6;i++)
       		sprintf(Griglia_trace[i],"%d",Min_Trace+i*10);
        }
	VisualizzaTrace(1);
	if (TRIGGER_PRETRIGGER==1)
		{
		if ((Ciclo_trigger>Min_Trace)&&(Ciclo_trigger<Max_Trace))
			{
			posizione_cursore=DISTXBOX+((Ciclo_trigger-Min_Trace)*zoom);
			XSetForeground (display, gc, colori_allocati[4].pixel);
			XDrawLine(display,pixmap,gc,posizione_cursore,DISTYBOX,posizione_cursore,LARGYBOX+DISTYBOX);
			XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (w)));
            XCopyArea (display,pixmap,window,gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
			get_string=GetMessageString(MemLin,"CICLO");
            sprintf(text,"%s  %2d",get_string,Ciclo_trigger);
            str=XmStringCreateLocalized(text);
            XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,str,NULL);
            XmStringFree(str);
			}
		}
	}
 /* make sure that dialog is raised to top of window stack */
XMapRaised (XtDisplay (plc2ShW),XtWindow(GetWidgetByName("SCWIOS")));
XmProcessTraversal (GetWidgetByName("Visualizzatore"), XmTRAVERSE_CURRENT);
}

/*
 --------------------------------------------------------------------
    Funzione RicaricaTrace 
  
    Funzione chiamata alla pressione della voce ricarica trace
    Input: 	w			Nome del widget chiamante
			client_data	Parametro associato al widget chiamante	
			call_data	Parametri associati alla callback
    Output: nessuno 
 --------------------------------------------------------------------
*/
void RicaricaTrace(Widget w, XtPointer clientData, XmAnyCallbackStruct *callData)
{
  VisualizzaTrace(0);
}

/*
 --------------------------------------------------------------------
    Funzione GestioneInputOutputBS 
  
    Funzione che converte la variabile In con la stringa contenuta
    in InputName e gli assegna come indice n - il contenuto della
    variabile OffsetValue.
    Stessa cosa vale per gli Output
    Input: 	char *NomeVar	Nome della variabile digitata
    Output: True (Variabile Speciale) False (Variabile Normale) 
 --------------------------------------------------------------------
*/
int GestioneInputOutputBS(char *NomeVar)
{
char *strTemp;
int  intTemp=0;
int	 nRet=False;

if(strcmp(InputName,"") != 0)
	{
	/* Se la variabile inizia per I seguita da numero converto */
	if(NomeVar[0] == 'I' && isdigit((int) NomeVar[1])) 
		{
		strTemp=(char *)malloc(sizeof(char)*17);
		strncpy(strTemp,NomeVar,17);
		intTemp = atoi(++strTemp);
		intTemp = intTemp + OffsetValue;
		if (intTemp < 0)
			intTemp = 0;
		sprintf(NomeVar,"%s[%d]",InputName,intTemp);
		free(strTemp);
		nRet=True;
		}
	}
if(strcmp(OutputName,"") != 0)
	{
	/* Se la variabile inizia per O seguita da numero converto */
	if(NomeVar[0] == 'O' && isdigit((int) NomeVar[1])) 
		{
		strTemp=(char *)malloc(sizeof(char)*17);
		strncpy(strTemp,NomeVar,17);
		intTemp = atoi(++strTemp);
		intTemp = intTemp + OffsetValue;
		if (intTemp < 0)
			intTemp = 0;
		sprintf(NomeVar,"%s[%d]",OutputName,intTemp);
		free(strTemp);
		nRet=True;
		}
	}
return(nRet);
}
