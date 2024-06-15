#include <stdlib.h>
#include <stdio.h>
#ifdef Linux
#define MAXPATHLEN FILENAME_MAX
#include <lynx-a.out.h>
#include <lynx-nlist.h>
#include <sys/stat.h>
#else
#include <a.out.h>
#include <nlist.h>
#include <stat.h>
#endif
#include <math.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#ifdef Linux
#include <sys/msg.h>
#else
#include <msg.h>
#endif
#include <sys/types.h>

#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#include <Xm/DrawingA.h>

#ifdef Linux
#include <libxad.h>
#else
#include <Xm/libxad.h>
#endif


#include "equat.h"
#include <plclink.h>
#ifdef Linux
#define XcgLiteClueAddWidget _dummy_XcgLiteClueAddWidget
#else
#include <LiteClue.h>
#include <libpcftp.h>
#include <services.h>
#include <libxerr.h>
#include <CniAlf.h>
#endif
#include <ios.h> 
#include <hwcni.h>
#include <nvram.h>

#include "bmp/Dinit.bmp"
#include "bmp/S24IO.xbm"


/*
    =====   dichiarazione external =====
    ------------------------------------
*/
extern	char 	dataPlc[];
extern	char 	libPlc[];

extern	Pixmap 	loadIcona(Widget ,Icona *);
extern	Widget	GetWidgetByName(char *);
extern  void    aggios_large();
extern  void    inputArgvArgc(int , char **argv);
extern  void    stampaMes(String , ...);
extern  void    stampaErr(String , ...);
extern  void    AggiornaControlloVariabili(void);
extern  void    CercaVariabile(Widget ,int );
extern 	void 	file_cb(Widget,XtPointer,XtPointer);
extern 	void 	delvar(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	serial_large(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	cursore(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	drawing(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	traverse(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	ripristino(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	allcaps(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	print_result(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	buttonchange(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	pop_do(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	cbkSelezionaFile(int,char *,XtPointer);
extern 	void 	forza_uscita(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	TipoDiVariabile(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	Statistica(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	SaveIOPB(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	SpostaPointer(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	Aiuti(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	buttontrace(Widget,XtPointer,XmAnyCallbackStruct *);
extern 	void 	RicaricaTrace(Widget,XtPointer,XmAnyCallbackStruct *);
extern  int     iniGestioneXerr(void);
extern  int     iniServerDtm(void);
extern  int 	InizioTrace(Widget ,int );


/*
   =====    Variabili globali ======
   ---------------------------------
*/
#include "pdbxad.h"

static 	unsigned char *cni_ios[MAX_IOS_BOARD][MAX_IOS_CPU];
unsigned short int LINE_STATE;
struct	plc_mcopy_t Controllo_Var[NUMERO_VARIABILI*2];	
struct  plcvar_t StrutturaVariabili[NUMERO_VARIABILI];
struct  plcvar_t StrutturaTrigger;
unsigned long	IndirizzoVariabili[NUMERO_VARIABILI];
unsigned char   elemio[8][32];
unsigned char   filtio[8][32];
unsigned char   status[32];
unsigned char   error[32];
unsigned char 	*CNI_iosbrd[MAX_IOS_BOARD];
unsigned char 	**CNI_ios[MAX_IOS_BOARD];
unsigned char	*NvRam;
XtAppContext	plc2AppContext;
XtIntervalId    IntervaStatistic;
XtIntervalId    IntervaDebug;
XtIntervalId    IntervaDebugFronte;
XtIntervalId    IntervaDebugPlcOpen;
#ifdef Linux
void *  	MemLin;
void * 	fbox;
#else
PTMEMMESSAGE  	MemLin;
#ifdef _OLD_FILE_SEL
PTFILESELBOX 	fbox; 
#else
PTXNCNFSB       fbox;
#endif
#endif
TRACERECORD     tracebuf[MAX_BUFFER_NUMBER][400];
XFontStruct     *font_struct;
XmFontList      font_list;
XsWdesc         *mainWD;
Boolean			Trovato_pre_trigger=False;
Boolean			Plc_aperto=False;
Boolean			MODULO_SPECIALE;
nvram_t		  	Area_ios;
nvram_t		  	Area_ios_480;
Pixmap  		pixmap;
Pixmap			Plc_status;
Pixmap			Debug_status;
Pixmap			Zoom_in;
Pixmap			Zoom_out;
Pixmap			Sinistra;
Pixmap 			Destra;
Pixmap 			Annulla;
Pixmap 			AzzeraStat;
Widget			plc2ShW;	
Widget      	LiteCluePdbappWidget;
Widget 			drawing_a_stat;
Widget 			NomiVariabili[NUMERO_VARIABILI];
Widget 			ValoreVariabili[NUMERO_VARIABILI];
Widget			popup_int[NUMERO_VARIABILI]; 
Widget			popup_real[NUMERO_VARIABILI]; 
Widget			popup_sep[NUMERO_VARIABILI]; 
Widget			TriggerWidget;
long			Campioni_statistica[CAMPIONISTAT][3];
/* equivalente a cnifl */
#ifdef Linux
char			NomeFont[2*MAX_PATH_LEN]=
				"fixed";
#else
char			NomeFont[2*MAX_PATH_LEN]=
				"fixed"; 
#endif
char			TestoNomiVariabili[NUMERO_VARIABILI][17];
char			TriggerNome[17];
char			QuotaValoreVariabili[NUMERO_VARIABILI][15];
char    		changedebug;
char    		resize=0;
char			Old_Trigger_Value;
char			Flag_di_validita[NUMERO_VARIABILI*2];
char   			*TipoModuloCaricato[3]= {"C_CODE",
                                "U_CODE",
                                "HATSH_CODE"
								};
char  			*srdtm;
char 			Griglia_trace[9][10];
char			InputName[10]="";
char			OutputName[10]="";
int				OffsetValue=1;
int				ContaVariabili;
int				TipoVariabili[NUMERO_VARIABILI];
int     		typedebug;
int     		typeshow=2;
int				SALITA_DISCESA=1;
int 			TRIGGER_PRETRIGGER=0;
int				Trigger_Mask=1;	
int				Ciclo_trigger=0;
int				numero_tracebuf=0;
int  			ios;
int				Numero_Variabili_Plc=0;
int				Byte_Variabili_Plc=0;
int				Lunghezza_Stringa[NUMERO_VARIABILI];
int				LONG_FLOAT[NUMERO_VARIABILI];
int				VariabileCorrente;
int				PopUpCorrente=-1;
int     		nostdout=False;
int     		nostderr=False;
int     		noserverdm=False;
int     		simessaggi=False;
int     		uppercase=False;
int     		FocusBool=False;
int 			n_board[IOS_MAX];
int 			ios_dis[IOS_MAX];
int 			board_selezionata;
int 			displacement_selezionato;
int				numero_modulo;
int				numero_byte;
int 			zoom=1;
int 			posizione_cursore=DISTXBOX*2;
int 			Min_Trace=0;
int 			Max_Trace=NUMERO_CAMPIONI;
GC      		gc;
GC				gc_stat;
Icona   IconPplc_run= { "Pplc_run.xpm", Dinit_bits, Dinit_width, 
			Dinit_height,  NULL };
Icona   IconPplc_halt= { "Pplc_halt.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_absent= { "Pplc_absent.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_trace= { "Pplc_trace.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_var= { "Pplc_var.xpm", Dinit_bits, Dinit_width, Dinit_height, 
			NULL };
Icona   IconPplc_zoomin= { "Pplc_zoomin.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_zoomout= { "Pplc_zoomout.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_sinistra= { "Pplc_sinistra.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_destra= { "Pplc_destra.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_annulla= { "C_CANC.XPM", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
Icona   IconPplc_azzstat= { "Pplc_azzstat.xpm", Dinit_bits, Dinit_width, 
			Dinit_height, NULL };
XColor colori_allocati[10];  /*	0	lightcyan 
								1	blue4
								2	grey76
								3	Green
								4	Red
							*/
String      fallbackRes[] =
   {
"plc2.LiteCluePdbapp.background : yellow",
"plc2.LiteCluePdbapp.foreground : black",
"plc2.LiteCluePdbapp.fontSet : cnipm",
"plc2.LiteCluePdbapp.hoffSet : -2",
"plc2.LiteCluePdbapp.voffSet : 1",
"plc2.LiteCluePdbapp.timeOut : 500",
  NULL
   };


/*
    =====   dichiarazione funzioni =====
    ------------------------------------
*/
int 	CNI_opendev (int);
void 	VisualizzaTrace(int);
void 	set_color(Widget);
void 	scadFrontedebug(XtPointer, XtIntervalId *);
void 	scaddebug_plc_open(XtPointer, XtIntervalId *);
void 	scadTdebug(XtPointer, XtIntervalId *);
int 	loadLinguePlc(void);
int 	setMnemonicMenuBar(Widget);
void 	UpdateYellowLine(Widget, XtPointer,XEvent *);
void	DefType(Widget , Widget , XEvent *);
void	popup_cb(Widget , XtPointer , XtPointer);


/*
 -----------------------------------------------------------------
  
    MAIN
  
 -----------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
XmString	Intero,Reale,Trace;
XGCValues	gcv_stat;
Pixel		fg,bg;
Pixmap		S24Pixmap;
Widget		open_ios_widget,sb_wid;
Widget		popup_gen[NUMERO_VARIABILI]; 
Widget		popup_trace[NUMERO_VARIABILI]; 
char		NomeWdg[80];
char    	VER_DATA[80]; 		
Display		*display;
Widget		drawing_a;
int			i,j;
int			exist_ios;
int 		lingue_ok;
int 		errori_ok;
int 		server_ok;
int			QuanteIos=0;
int			ContaVariabili;

/* Visualizzo la versione del processo */
printf("CNi - Plc2 - version %s [%s] lib:%s\n", IDS_PLC2_VER, dataPlc, libPlc);

inputArgvArgc(argc,argv);

/*  inizializzo la gestione degli errori */
errori_ok=iniGestioneXerr();
if (errori_ok==RETER)
  	 stampaErr("non sono riuscito ad inizializzare gli errori");

/* Invio la versione */
sprintf(VER_DATA,"%s\t\t[%s]\tlib:%s",IDS_PLC2_VER, dataPlc, libPlc);
invVersione(IDS_SIGLA_PLC2, 1, VER_DATA);

/*  apertura server dati macchina */
server_ok=iniServerDtm();
if (server_ok==RETER)
         invXerror(IDS_SIGLA_PLC2,NO_SERVER,"%d");  

/* Create the Main Application Shell Widget */
	if((plc2ShW = XtVaAppInitialize(&plc2AppContext,
		"plc2", NULL, 0, &argc, argv,
		fallbackRes,XmNiconName,"Plc",XmNtitle,VER_PLC,
		NULL,
		0)) == (Widget)NULL) {
		invXerror(IDS_SIGLA_PLC2,NO_MAIN_SHELL,"%d");    
		exit(1);
	}

mainWD=plc2WD;

/*	Inizializzazioni del mio programma	*/
for(i=0;i<8;i++)
	{
	for(j=0;j<32;j++)
		elemio[i][j]=filtio[i][j]=0;
	}
for(i=0;i<CAMPIONISTAT;i++)
    {
    for(j=0;j<3;j++)
        Campioni_statistica[i][j]=0;
    }

for(i=0;i<NUMERO_VARIABILI;i++)
	{
	IndirizzoVariabili[i]=0;
	TipoVariabili[i]=0;
    LONG_FLOAT[i]=-1;
	}

for(i=0;i<9;i++)
	sprintf(Griglia_trace[i],"%d",0+i*50);

memset(TestoNomiVariabili[0],'\0',16);
for (ContaVariabili=1;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
	strncpy(TestoNomiVariabili[ContaVariabili],TestoNomiVariabili[0],17);

strncpy(TriggerNome,TestoNomiVariabili[0],17);

if(XsLibInit(XsQUIET, "./","plc2") == XsError) 
	{ 
	stampaErr("plc2 - Error: initializing Xs Library %s", XsStrErr(XsErrno));
	exit(2);
	}

if(XsWidgetDescCreate(plc2WD,plc2ShW,plc2AD) == XsError) 
	{
	stampaErr("plc2 - FATAL:error creating Widgets in main  Err %s\n",XsStrErr(XsErrno));
	exit(2);
	}

if((XsCbDescInstall(plc2WD,plc2CB)) == XsError) 
	{
	stampaErr("plc2 - FATAL: error installing Callbacks in MAIN Error %s\n",XsStrErr(XsErrno));
	exit(3);
	}

/* setto la lingua desiderata */
lingue_ok=loadLinguePlc();
if (lingue_ok==RETER)
   	invXerror(IDS_SIGLA_PLC2,NO_LINGUE,"%d");   

/* permetto il resize della shell principale */
XtVaSetValues(plc2ShW,XtNallowShellResize,True,NULL);

/* DRAWING AREA DELLA IOS */
drawing_a=GetWidgetByName("DRAIOS");
gc = XCreateGC (XtDisplay (drawing_a),RootWindowOfScreen 
		(XtScreen (drawing_a)), 0, NULL);
XtVaSetValues (drawing_a, XmNuserData, gc, NULL);
XSetForeground (XtDisplay (drawing_a), gc,WhitePixelOfScreen 
		(XtScreen (drawing_a)));
/* create a pixmap the same size as the drawing area. */
pixmap = XCreatePixmap (XtDisplay (drawing_a),RootWindowOfScreen 
		(XtScreen (drawing_a)), LARGX_DRA, LARGY_DRA,
		DefaultDepthOfScreen (XtScreen (drawing_a)));

/* alloco tutti i colori necessari una volta per tutte */
set_color (drawing_a);

/* clear pixmap with ... set_color (drawing_a, "...",NULL); */
XSetForeground (XtDisplay(drawing_a), gc, colori_allocati[0].pixel);
XFillRectangle (XtDisplay (drawing_a), pixmap, gc, 0, 0, LARGX_DRA, LARGY_DRA);
/* drawing is now drawn into with "black"; change the gc for future */
XSetForeground (XtDisplay (drawing_a), gc,BlackPixelOfScreen 
		(XtScreen (drawing_a)));

/* DRAWING AREA STATISTICA */
drawing_a_stat=GetWidgetByName("DRASTAT");
gcv_stat.foreground = BlackPixelOfScreen (XtScreen (drawing_a_stat));
gc_stat = XCreateGC (XtDisplay(drawing_a_stat),RootWindowOfScreen 
	(XtScreen (drawing_a_stat)), GCForeground, &gcv_stat);
XtVaSetValues (drawing_a_stat, XmNuserData, gc_stat, NULL);

/*creazione del font tipografico per la drawing area della ios */
display=XtDisplay(GetWidgetByName("DRAIOS"));
font_struct = XLoadQueryFont(display,NomeFont); 
XSetFont(display,gc,font_struct->fid);
font_list = XmFontListCreate(font_struct,XmSTRING_DEFAULT_CHARSET);  

XtAddEventHandler(GetWidgetByName("CancelButton1"), KeyReleaseMask, FALSE, (XtEventHandler)UpdateYellowLine, (XtPointer)NULL );

/*apertura collegamento con plc */	
if (plcTryOpen()!=0)
	{
	XtSetSensitive(GetWidgetByName("LOAD-PLC"),True);
    XtSetSensitive(GetWidgetByName("RESTORE"),True);
	Plc_status = loadIcona (GetWidgetByName("Left_icon"), 
			&IconPplc_absent);
    XtSetSensitive(GetWidgetByName("RUNPB"),True);
    XtSetSensitive(GetWidgetByName("HALTPB"),False);

	if (Plc_status!=NULL)
   		XtVaSetValues(GetWidgetByName("Left_icon"),
		XmNlabelType, XmPIXMAP, XmNlabelPixmap, Plc_status, NULL);
	IntervaDebugPlcOpen=XtAppAddTimeOut(plc2AppContext,TEMPO_PLC_OPEN, 
		scaddebug_plc_open, NULL);
	}
else
	{
    Plc_status = loadIcona (GetWidgetByName("Left_icon"), 
		&IconPplc_run);
    XtSetSensitive(GetWidgetByName("RUNPB"),False);
    XtSetSensitive(GetWidgetByName("HALTPB"),True);

	if (Plc_status!=NULL)
    	XtVaSetValues(GetWidgetByName("Left_icon"),
		XmNlabelType, XmPIXMAP, XmNlabelPixmap, Plc_status, NULL);
	XtSetSensitive(GetWidgetByName("LOAD-PLC"),False);
    XtSetSensitive(GetWidgetByName("RESTORE"),False);
	Plc_aperto=True;
	IntervaDebugPlcOpen=XtAppAddTimeOut(plc2AppContext,TEMPO_PLC_OPEN, 
		scaddebug_plc_open, NULL);
	}

/* Attivo il debug */
if(plcStatus())
	{
	XtSetSensitive(GetWidgetByName("STARTPB"),False);
	XtSetSensitive(GetWidgetByName("STOPPB"),True);
	changedebug=1;
	IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
	}

/* inizializza le strutture dell' help dei bottoni e setta gli mnemonici */
#ifdef Linux
LiteCluePdbappWidget=NULL;
#else
LiteCluePdbappWidget=XtVaCreatePopupShell("LiteCluePdbapp",
    xcgLiteClueWidgetClass, plc2ShW,
    NULL);
#endif

/* carico la icona di default */
Debug_status = loadIcona (GetWidgetByName("Right_icon"), 
	&IconPplc_var);
if (Debug_status!=NULL)
    XtVaSetValues(GetWidgetByName("Right_icon"),
	XmNlabelType, XmPIXMAP, XmNlabelPixmap, Debug_status, NULL);

/* carico le icone per il trace */
Zoom_in = loadIcona (GetWidgetByName("Zoom_In"), &IconPplc_zoomin);
if (Zoom_in!=NULL)
	{
    XtVaSetValues(GetWidgetByName("Zoom_In"),
        XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,(XtArgVal) Zoom_in, 
		NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("Zoom_In"), 
    	GetMessageString(MemLin,"Zoom_In") ,0,0);
	}
Zoom_out = loadIcona (GetWidgetByName("Zoom_Out"), 
		&IconPplc_zoomout);
if (Zoom_out!=NULL)
	{
    XtVaSetValues(GetWidgetByName("Zoom_Out"),
        XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,(XtArgVal) Zoom_out, 
		NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("Zoom_Out"), 
    	GetMessageString(MemLin,"Zoom_Out") ,0,0);
	}
Sinistra =  loadIcona (GetWidgetByName("Move_Left"), 
		&IconPplc_sinistra);
if (Sinistra!=NULL)
	{
    XtVaSetValues(GetWidgetByName("Move_Left"),
        XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,(XtArgVal) Sinistra, 
		NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("Move_Left"), 
    	GetMessageString(MemLin,"Move_Left") ,0,0);
	}
Destra = loadIcona (GetWidgetByName("Move_Right"), &IconPplc_destra);
if (Destra!=NULL)
	{
    XtVaSetValues(GetWidgetByName("Move_Right"),
        XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,(XtArgVal) Destra, 
		NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("Move_Right"), 
    	GetMessageString(MemLin,"Move_Right") ,0,0);
	}
Annulla = loadIcona (GetWidgetByName("Move_Right"), &IconPplc_annulla);
if (Annulla!=NULL)
	{
    XtVaSetValues(GetWidgetByName("CancelButton1"),
    	XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,
		(XtArgVal) Annulla, NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("CancelButton1"), 
    	GetMessageString(MemLin,"Cancel") ,0,0);
    XtVaSetValues(GetWidgetByName("CancelButton2"),
    	XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,
		(XtArgVal) Annulla, NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("CancelButton2"), 
    	GetMessageString(MemLin,"Cancel") ,0,0);
    XtVaSetValues(GetWidgetByName("CancelButton3"),
    	XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,
		(XtArgVal) Annulla, NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("CancelButton3"), 
    	GetMessageString(MemLin,"Cancel") ,0,0);
    XtVaSetValues(GetWidgetByName("CancelButton4"),
    	XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,
		(XtArgVal) Annulla, NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("CancelButton4"), 
    	GetMessageString(MemLin,"Cancel") ,0,0);
	} 
AzzeraStat = loadIcona (GetWidgetByName("OkButton"), &IconPplc_azzstat);
if (AzzeraStat!=NULL)
	{
    XtVaSetValues(GetWidgetByName("OkButton"),
    	XmNlabelType,(XtArgVal) XmPIXMAP, XmNlabelPixmap,
		(XtArgVal) AzzeraStat, NULL);
	XcgLiteClueAddWidget(LiteCluePdbappWidget, 
		GetWidgetByName("OkButton"), 
    	GetMessageString(MemLin,"AZZERSTAT") ,0,0);
	}

XtRealizeWidget(plc2ShW);

/* Inizializzo tutti i widget di edit e label che riguardano il debug 
	delle variabili di scambio */
NomiVariabili[0]=GetWidgetByName("Variable1");
NomiVariabili[1]=GetWidgetByName("Variable2");
NomiVariabili[2]=GetWidgetByName("Variable3");
NomiVariabili[3]=GetWidgetByName("Variable4");
NomiVariabili[4]=GetWidgetByName("Variable5");
NomiVariabili[5]=GetWidgetByName("Variable6");
NomiVariabili[6]=GetWidgetByName("Variable7");
NomiVariabili[7]=GetWidgetByName("Variable8");
NomiVariabili[8]=GetWidgetByName("Variable9");
NomiVariabili[9]=GetWidgetByName("Variable10");
NomiVariabili[10]=GetWidgetByName("Variable11");
NomiVariabili[11]=GetWidgetByName("Variable12");
NomiVariabili[12]=GetWidgetByName("Variable13");
NomiVariabili[13]=GetWidgetByName("Variable14");
NomiVariabili[14]=GetWidgetByName("Variable15");
NomiVariabili[15]=GetWidgetByName("Variable16");

ValoreVariabili[0]=GetWidgetByName("Channel1");
ValoreVariabili[1]=GetWidgetByName("Channel2");
ValoreVariabili[2]=GetWidgetByName("Channel3");
ValoreVariabili[3]=GetWidgetByName("Channel4");
ValoreVariabili[4]=GetWidgetByName("Channel5");
ValoreVariabili[5]=GetWidgetByName("Channel6");
ValoreVariabili[6]=GetWidgetByName("Channel7");
ValoreVariabili[7]=GetWidgetByName("Channel8");
ValoreVariabili[8]=GetWidgetByName("Channel9");
ValoreVariabili[9]=GetWidgetByName("Channel10");
ValoreVariabili[10]=GetWidgetByName("Channel11");
ValoreVariabili[11]=GetWidgetByName("Channel12");
ValoreVariabili[12]=GetWidgetByName("Channel13");
ValoreVariabili[13]=GetWidgetByName("Channel14");
ValoreVariabili[14]=GetWidgetByName("Channel15");
ValoreVariabili[15]=GetWidgetByName("Channel16");

TriggerWidget=GetWidgetByName("Tstart");

/* Gestione del PopUp per richiesta tipo del general */

Intero = XmStringCreateSimple ("DINT");
Reale = XmStringCreateSimple ("REAL");
XtVaGetValues(GetWidgetByName("TRACE"),XmNlabelString,&Trace,NULL);
for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
	{
	sprintf(NomeWdg,"popup%d",ContaVariabili);
	popup_gen[ContaVariabili] = XmVaCreateSimplePopupMenu (NomiVariabili[ContaVariabili],
		NomeWdg, NULL,
        NULL);
	XtAddCallback(popup_gen[ContaVariabili], XmNentryCallback,
        popup_cb, NULL);
	sprintf(NomeWdg,"popup_int%d",ContaVariabili);
    popup_int[ContaVariabili] = XtVaCreateManagedWidget(
                NomeWdg,
                xmPushButtonWidgetClass,popup_gen[ContaVariabili],
                XmNlabelString, Intero,
                XmNuserData, PDB_SRVZ_INT,
                NULL);
	sprintf(NomeWdg,"popup_real%d",ContaVariabili);
    popup_real[ContaVariabili] = XtVaCreateManagedWidget(
                NomeWdg,
                xmPushButtonWidgetClass,popup_gen[ContaVariabili],
                XmNlabelString, Reale,
                XmNuserData, PDB_SRVZ_REAL,
                NULL);
	sprintf(NomeWdg,"popup_sep%d",ContaVariabili);
    popup_sep[ContaVariabili] = XtVaCreateManagedWidget(
                NomeWdg,
                xmSeparatorWidgetClass,popup_gen[ContaVariabili],
                NULL);
	sprintf(NomeWdg,"popup_trace%d",ContaVariabili);
    popup_trace[ContaVariabili] = XtVaCreateManagedWidget(
                NomeWdg,
                xmPushButtonWidgetClass,popup_gen[ContaVariabili],
                XmNlabelString, Trace,
                XmNuserData, PDB_SRVZ_TRACE,
                NULL);
	XtAddEventHandler(NomiVariabili[ContaVariabili], ButtonPressMask, FALSE,  
        (XtEventHandler)DefType, (XtPointer)popup_gen[ContaVariabili]);
	}
XmStringFree (Intero);
XmStringFree (Reale);
XmStringFree (Trace);

/* rendo visibili solo le ios che esistono nell'ipotesi che esistano 
	solo 2 board e 2 ios per board */
exist_ios=CNI_opendev(0);
if (exist_ios==0)
   	{
    open_ios_widget=GetWidgetByName("ios1");      
    XtUnmanageChild(open_ios_widget);
    open_ios_widget=GetWidgetByName("ios2");      
    XtUnmanageChild(open_ios_widget);
	open_ios_widget=GetWidgetByName("ios3");      
    XtUnmanageChild(open_ios_widget);
    open_ios_widget=GetWidgetByName("ios4");      
    XtUnmanageChild(open_ios_widget);
    }
else
    {
    if (cni_ios[0][0]!=NULL)
        {
        n_board[QuanteIos]=0;
        ios_dis[QuanteIos]=NO_DISP;
		QuanteIos++;
        }
    if (cni_ios[0][1]!=NULL)
		{
        n_board[QuanteIos]=0;
        ios_dis[QuanteIos]=SI_DISP;
		QuanteIos++;
        }
	/* Vado alla ricerca della seconda */
	exist_ios=CNI_opendev(1);
	if (exist_ios!=0)
		{
		if (cni_ios[1][0]!=NULL)
        	{
        	n_board[QuanteIos]=1;
        	ios_dis[QuanteIos]=NO_DISP;
			QuanteIos++;
        	}
    	if (cni_ios[1][1]!=NULL)
       		{
			n_board[QuanteIos]=1;
			ios_dis[QuanteIos]=SI_DISP;
			QuanteIos++;
			}
		}
   	}
if (QuanteIos == 1)
	{
    open_ios_widget=GetWidgetByName("ios2");      
    XtUnmanageChild(open_ios_widget);
	open_ios_widget=GetWidgetByName("ios3");      
    XtUnmanageChild(open_ios_widget);
    open_ios_widget=GetWidgetByName("ios4");      
    XtUnmanageChild(open_ios_widget);
	}
else if (QuanteIos == 2)
	{
	open_ios_widget=GetWidgetByName("ios3");      
    XtUnmanageChild(open_ios_widget);
    open_ios_widget=GetWidgetByName("ios4");      
    XtUnmanageChild(open_ios_widget);
	}
else if (QuanteIos == 3)
	{
    open_ios_widget=GetWidgetByName("ios4");      
    XtUnmanageChild(open_ios_widget);
	}

/* tolgo la scrolled bar dalla navigation tab */
XtVaGetValues(GetWidgetByName("SCWVAR"),XmNverticalScrollBar, 
	&sb_wid, NULL);
XtVaSetValues(sb_wid,XmNtraversalOn,False,NULL);

/* tolgo i bottoni alle message box */
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("forout"),
	XmDIALOG_OK_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("forout"),
	XmDIALOG_CANCEL_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("forout"),
	XmDIALOG_HELP_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("Visualizzatore"),
	XmDIALOG_OK_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("Visualizzatore"),
	XmDIALOG_CANCEL_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("Visualizzatore"),
	XmDIALOG_HELP_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("VarType"),
	XmDIALOG_OK_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("VarType"),
	XmDIALOG_CANCEL_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("VarType"),
	XmDIALOG_HELP_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("MESSAGSTAT"),
	XmDIALOG_OK_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("MESSAGSTAT"),
	XmDIALOG_CANCEL_BUTTON));
XtUnmanageChild(XmMessageBoxGetChild(GetWidgetByName("MESSAGSTAT"),
	XmDIALOG_HELP_BUTTON));

XtVaGetValues(GetWidgetByName("VisualForm"), XmNforeground, &fg,
		XmNbackground, &bg, NULL);
S24Pixmap=XCreatePixmapFromBitmapData (XtDisplay(GetWidgetByName("VisualForm")),
		RootWindowOfScreen (XtScreen(GetWidgetByName("VisualForm"))),
		S24IO_bits, S24IO_width, S24IO_height,
		fg, bg, DefaultDepthOfScreen(XtScreen(GetWidgetByName("VisualForm"))));
XtVaSetValues(GetWidgetByName("Module_label"),XmNlabelType,XmPIXMAP,
			XmNlabelPixmap,S24Pixmap,NULL);

/* Associo la action ai bottoni */
XtOverrideTranslations(GetWidgetByName("RUNPB"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("HALTPB"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("STARTPB"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("STOPPB"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("SAL_DIS"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("DEC_EX"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("TRACE"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));
XtOverrideTranslations(GetWidgetByName("PRE_TRIPB"), 
	XtParseTranslationTable("<KeyPress>Return: ArmAndActivate()"));

setMnemonicMenuBar(GetWidgetByName("SELEZBAR"));

XtAppMainLoop(plc2AppContext);
exit(0);
}

/*
 -----------------------------------------------------------------------------
    Funzione CNI_opendev
  
    Se non e` ancora stato definito il blocco di shared memory associato
    al dispositivo CNI IOS "n", lo apre, ne acquisice l'indirizzo di memoria,
    lo condivide e lo rende disponibile per l'applicazione.

	Input  : n Dispositivo ios a cui agganciarsi.
    Output : Vale 1 in caso di successo o se il dispositivo e` gia` stato 
			aperto, 0 in caso di errore.
 -----------------------------------------------------------------------------
*/
int CNI_opendev (int n)
{
nvram_t Temp;

/* Apertura del dispositivo. */
if (!CNI_iosbrd[n])
    {
    CNI_ios[n] = cni_ios[n];

	if (n==NVRAM_BRD_PLC)
		{
		if (iosOpen(n,&CNI_iosbrd[n],CNI_ios[n],&NvRam, (int *)0) < 0)
            return 0;
	    if (AttachNvram(NVRAM_LBL_PLC,&Temp,NvRam,n)==0)
			{
			Area_ios.addr=Temp.addr;
			Area_ios.size=Temp.size;
			}
		if (AttachNvram(NVRAM_LBL_PLC480,&Temp,NvRam,n)==0)
			{
            Area_ios_480.addr=Temp.addr;
            Area_ios_480.size=Temp.size;
			dbRegisterNvram(Area_ios_480.addr);
			}
		}
	else
		{
    	if (iosOpen(n,&CNI_iosbrd[n],CNI_ios[n],(unsigned char **)0, (int *)0) < 0)
        	return 0;
		}
    }
/* Tutto bene. Fine. */
return 1;
} 

/*
 --------------------------------------------------------------------
    Funzione VisualizzaTrace 
  
    Funzione che visualizza il trace delle variabili di scambio 
    Input: FlagChiamata     ????????????
    Output: nessuno 
 --------------------------------------------------------------------
*/
void VisualizzaTrace(int FlagChiamata)
{
XmString 	str;
Widget 	wid;
Window  window;
Display *display;
int 	length=0;
char   	text[80];
int 	x1=DISTXBOX;
int		y1=DISTYBOX+6;
int		x2=DISTXBOX;
int		y2=DISTYBOX+21;
int		x_line=DISTXBOX;
int		y_line=DISTYBOX+21;
int		PassoTrace=0;
int		index;
int		mask=1;
int 	Old_y[NUMERO_VARIABILI];
int		Buffer_iniziale=0;
int		Buffer_finale=0;

Buffer_iniziale=Min_Trace/NUMERO_CAMPIONI;
Buffer_finale=Max_Trace/NUMERO_CAMPIONI;

wid=GetWidgetByName("DRAIOS");
if(wid!=NULL) 
    {
	XtManageChild (GetWidgetByName("Visualizzatore"));
    XtManageChild (GetWidgetByName("Zoom_In"));
    XtManageChild (GetWidgetByName("Zoom_Out"));
    XtManageChild (GetWidgetByName("Move_Left"));
    XtManageChild (GetWidgetByName("Move_Right"));
	XtUnmanageChild (GetWidgetByName("Module_label"));
	XtUnmanageChild (GetWidgetByName("Byte0_label"));
	XtUnmanageChild (GetWidgetByName("Byte1_label"));
	XtUnmanageChild (GetWidgetByName("Byte2_label"));
	XtUnmanageChild (GetWidgetByName("Bit0_label"));
	XtUnmanageChild (GetWidgetByName("Bit1_label"));
	XtUnmanageChild (GetWidgetByName("Bit2_label"));
    if (FlagChiamata == 0 )
	  XtVaSetValues(GetWidgetByName("Visualizzatore"),XmNwidth,570,
		NULL);

	sprintf(text,"Trace");
	XtVaSetValues(XtParent(GetWidgetByName("Visualizzatore")),
		XmNtitle,text,NULL);

	sprintf(text," ");
	str=XmStringCreateLocalized(text);
	XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,
		str,NULL);
	XmStringFree(str);

	sprintf(text," ");
	str=XmStringCreateLocalized(text);
	XtVaSetValues(GetWidgetByName("Error_label"),XmNlabelString,
		str,NULL);
	XmStringFree(str);

	sprintf(text," ");
	str=XmStringCreateLocalized(text);
	XtVaSetValues(GetWidgetByName("Stable_label"),XmNlabelString,
		str,NULL);
	XmStringFree(str);

    XtVaGetValues(wid,XmNuserData,&gc,NULL);
    window=XtWindow(wid);
    display=XtDisplay(wid);

    /* scambio foreground and background per pulire l'area grafica */
    XSetForeground (display, gc, WhitePixelOfScreen (XtScreen (wid)));

    /* riempio il rettangolo della dimensione del pixmap */
	XSetForeground (display, gc, colori_allocati[0].pixel);
    XFillRectangle (display, pixmap, gc, 0, 0, LARGX_DRA, LARGY_DRA);

    /* Reset */
    XSetForeground (display, gc, BlackPixelOfScreen (XtScreen (wid)));

    XSetForeground (display, gc, WhitePixelOfScreen (XtScreen (wid)));
	XFillRectangle(display,pixmap,gc,100,20,400,460);

	XSetForeground (display, gc, colori_allocati[2].pixel);
	XDrawRectangle(display,pixmap,gc,DISTXBOX,DISTYBOX,LARGXBOX,LARGYBOX);

	/* linee verticali */
	if ((zoom==1)||(zoom==2))
        {
		for (index=0; index<7; index++)
			{
			x_line+=50;
			XDrawLine(display,pixmap,gc,x_line,DISTYBOX,x_line,
				LARGYBOX+DISTYBOX);
			}
		}
	else
		{
        for (index=0; index<4; index++)
            {
            x_line+=80;
            XDrawLine(display,pixmap,gc,x_line,DISTYBOX,x_line,
				LARGYBOX+DISTYBOX);
            }
        }

	/* linee orizzontali */
	XSetForeground (display, gc, colori_allocati[2].pixel);
	for (index=0; index<NUMERO_VARIABILI; index++)
        {
        XDrawLine(display,pixmap,gc,DISTXBOX,y_line,LARGXBOX+DISTXBOX,y_line);
        y_line+=DISTYTRACE;
        }

	XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (wid)));
	if ((zoom==1)||(zoom==2))
		{
		for (index=0;index<9;index++)
			{
			sprintf(text,Griglia_trace[index]);
    		length=strlen(text);
			if (index==0)
				{
    			XDrawString(display,pixmap,gc,100+(index*50),10+3,text,length);
    			XDrawString(display,pixmap,gc,100+(index*50),490+3,text,length);
				}
			else if (index==1)
				{
            	XDrawString(display,pixmap,gc,100+(index*50)-5,10+3,text,
					length);
            	XDrawString(display,pixmap,gc,100+(index*50)-5,490+3,text,
					length);
            	}
			else
				{
            	XDrawString(display,pixmap,gc,100+(index*50)-10,10+3,text,
					length);
            	XDrawString(display,pixmap,gc,100+(index*50)-10,490+3,text,
					length);
            	}
			}
		}
	else
		{
        for (index=0;index<6;index++)
            {
            sprintf(text,Griglia_trace[index]);
            length=strlen(text);
            if (index==0)
                {
                XDrawString(display,pixmap,gc,100+(index*80),10+3,text,length);
                XDrawString(display,pixmap,gc,100+(index*80),490+3,text,length);
                }
            else if (index==1)
                {
                XDrawString(display,pixmap,gc,100+(index*80)-5,10+3,text,
					length);
                XDrawString(display,pixmap,gc,100+(index*80)-5,490+3,text,
					length);
                }
            else
                {
                XDrawString(display,pixmap,gc,100+(index*80)-10,10+3,text,
					length);
                XDrawString(display,pixmap,gc,100+(index*80)-10,490+3,text,
					length);
                }
            }
        }

	PassoTrace=zoom;
	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
		{
		if (TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO || 
				TipoVariabili[ContaVariabili]==VARIABILE_PLC)
			{
			XSetForeground (display, gc,BlackPixelOfScreen (XtScreen (wid)));
			sprintf(text,TestoNomiVariabili[ContaVariabili]);
        	length=strlen(text);
        	XDrawString(display,pixmap,gc,5,(y1+15),text,length);
			XSetForeground (display, gc, colori_allocati[1].pixel);
			if (Buffer_iniziale==Buffer_finale)
				{
				/* ciclo in un unico buffer */
                for (index = (Min_Trace-(NUMERO_CAMPIONI*Buffer_iniziale)); 
					index < (Max_Trace-(NUMERO_CAMPIONI*Buffer_iniziale)); 
					++index)
                    {
                    /* Inizializzo */
                    if (index == (Min_Trace-(NUMERO_CAMPIONI*Buffer_iniziale))) 
                      Old_y[ContaVariabili]=(tracebuf[Buffer_iniziale][index] 
							& mask);

                    /* Buffer non utilizzato */
                    if (tracebuf[Buffer_iniziale][index] == -1)
						break;

                    if (Old_y[ContaVariabili]!=
						(tracebuf[Buffer_iniziale][index] & mask))
                        XDrawLine(display,pixmap,gc,x1,y1,x1,y2);
                    x2+=PassoTrace;
                    if ((tracebuf[Buffer_iniziale][index] & mask)==0)
                        {
                        XDrawLine(display,pixmap,gc,x1,y2,x2,y2);
                        Old_y[ContaVariabili]=0;
                        }
                    else
                        {
                        XDrawLine(display,pixmap,gc,x1,y1,x2,y1);
                        Old_y[ContaVariabili]=tracebuf[Buffer_iniziale][index] 
							& mask;
                        }
                    x1=x2;
                    }
				}
			else
				{
				/* ciclo nel primo buffer */
				for (index = (Min_Trace-(NUMERO_CAMPIONI*Buffer_iniziale)); 
					index < NUMERO_CAMPIONI; ++index)
					{
                    /* Inizializzo */
                    if (index == (Min_Trace-(NUMERO_CAMPIONI*Buffer_iniziale))) 
                      Old_y[ContaVariabili]=(tracebuf[Buffer_iniziale][index] 
						& mask);

                    /* Buffer non utilizzato */
                    if (tracebuf[Buffer_iniziale][index] == -1)
                        break;

					if (Old_y[ContaVariabili]!=
							(tracebuf[Buffer_iniziale][index] & mask))
						XDrawLine(display,pixmap,gc,x1,y1,x1,y2);
					x2+=PassoTrace;
					if ((tracebuf[Buffer_iniziale][index] & mask)==0)
						{
						XDrawLine(display,pixmap,gc,x1,y2,x2,y2);
						Old_y[ContaVariabili]=0;
						}
					else
						{
						XDrawLine(display,pixmap,gc,x1,y1,x2,y1);
						Old_y[ContaVariabili]=tracebuf[Buffer_iniziale][index] 
							& mask;
						}
					x1=x2;
					}
				/* ciclo nel secondo buffer */
            	for (index = 0; index < 
					(Max_Trace-(NUMERO_CAMPIONI*Buffer_finale)); ++index)
                	{
                    /* Buffer non utilizzato */
                    if (tracebuf[Buffer_finale][index] == -1)
                        break;

                	if (Old_y[ContaVariabili]!=(tracebuf[Buffer_finale][index] 
							& mask))
                    	XDrawLine(display,pixmap,gc,x1,y1,x1,y2);
                	x2+=PassoTrace;
                	if ((tracebuf[Buffer_finale][index] & mask)==0)
                    	{
                    	XDrawLine(display,pixmap,gc,x1,y2,x2,y2);
                    	Old_y[ContaVariabili]=0;
                    	}
                	else 
                    	{
                    	XDrawLine(display,pixmap,gc,x1,y1,x2,y1);
                    	Old_y[ContaVariabili]=tracebuf[Buffer_finale][index] 
							& mask;
                    	}
                	x1=x2;
                	}
				}
			x1=DISTXBOX;
			x2=DISTXBOX;
			y1+=(DISTYTRACE);
			y2+=(DISTYTRACE);
			mask*=2;
			}
		}
	}
XCopyArea (display,pixmap,window,gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
}

/*
 ----------------------------------------------------------------------
    set_color
  
    Funzione che alloca i colori necessari per il trace e per la
    visualizzazione della IOS

    Input:  widget    Nome del widget della drawing area
    Output: Nessuno
 ----------------------------------------------------------------------
*/
void set_color(Widget widget)
{
char *color ="lightcyan";
Display *dpy = XtDisplay (widget);
Colormap cmap = DefaultColormapOfScreen (XtScreen (widget));
XColor unused;

if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[0], &unused)) 
    {
	invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
	colori_allocati[0].pixel=WhitePixelOfScreen (XtScreen (widget));
    }

color="blue4";
if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[1], &unused)) 
    {
	invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
	colori_allocati[1].pixel=BlackPixelOfScreen (XtScreen (widget));
    }

color="grey76";
if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[2], &unused)) 
    {
	invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
	colori_allocati[2].pixel=BlackPixelOfScreen (XtScreen (widget));
    }

color="green";
if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[3], &unused)) 
    {
	invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
	colori_allocati[3].pixel=BlackPixelOfScreen (XtScreen (widget));
    }

color="red";
if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[4], &unused)) 
    {
	invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
	colori_allocati[4].pixel=BlackPixelOfScreen (XtScreen (widget));
    }

color="slateblue2";
if (!XAllocNamedColor (dpy, cmap, color, &colori_allocati[5], &unused)) 
    {
    invXerror(IDS_SIGLA_PLC2,NO_ALLOC_COLOR,"%n",color);
    colori_allocati[5].pixel=BlackPixelOfScreen (XtScreen (widget));
    }
} 

/*
 -----------------------------------------------------------------------------
    Funzione scadFrontedebug
  
    Funzione chiamata ogni qual volta il timeout debug impostato scade:
    Gestisce la ricerca del cambiamento di fronte per il trigger del trace

    Input:  client_data = eventuale puntatore ai dati
            tempoId = puntatore all'identificatore del timeout
    Output: Nessuno
 ----------------------------------------------------------------------------
*/
void scadFrontedebug(XtPointer client_data, XtIntervalId *tempoId)
{
int rv;
char New_Trigger_Value;

New_Trigger_Value=(*(char *)StrutturaTrigger.pval);
if (SALITA_DISCESA==0)
	{
	if ((Old_Trigger_Value!=New_Trigger_Value)&&(Old_Trigger_Value==1))
		{
		if (TRIGGER_PRETRIGGER==0)
			{
			rv=plcTraceStart();
        	if (rv)
          		{
				invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_START,"%v",rv);
            	if (IntervaDebug!=NULL)
              		XtRemoveTimeOut( IntervaDebug );
            	}
			IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, 
				NULL);
			XtRemoveTimeOut(IntervaDebugFronte);
			}
		else if (TRIGGER_PRETRIGGER==1)
			{
			Trovato_pre_trigger=True;
			}
		}
	else
		{
		Old_Trigger_Value=*(char *)StrutturaTrigger.pval;
        IntervaDebugFronte=XtAppAddTimeOut(plc2AppContext,TEMPOQFRONTE, 
			scadFrontedebug, NULL);
		}
	}
else
	{
    if ((Old_Trigger_Value!=New_Trigger_Value)&&(Old_Trigger_Value==0))
        {
		if (TRIGGER_PRETRIGGER==0)
			{
			rv=plcTraceStart();
        	if (rv)
            	{
				invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_START,"%v",rv);
            	if (IntervaDebug!=NULL)
                	XtRemoveTimeOut( IntervaDebug );
            	}
        	IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, 
				NULL);
        	XtRemoveTimeOut(IntervaDebugFronte);
			}
		else if (TRIGGER_PRETRIGGER==1)
			{
			Trovato_pre_trigger=True;
			}
        }
    else
        {
        Old_Trigger_Value=*(int *)StrutturaTrigger.pval;
        IntervaDebugFronte=XtAppAddTimeOut(plc2AppContext,TEMPOQFRONTE, 
			scadFrontedebug, NULL);
        }
    }
}

/*
 ---------------------------------------------------------------------------
  
    Funzione scaddebug_plc_open
  
    Funzione chiamata ogni qual volta il timeout debug impostato scade:
    Gestisce l' attesa dell'inizializzazione del plc
    Input:  client_data = eventuale puntatore ai dati
            tempoId = puntatore all'identificatore del timeout
    Output: Nessuno
  
 ---------------------------------------------------------------------------
*/
void scaddebug_plc_open(XtPointer client_data, XtIntervalId *tempoId)
{
static int tentativi_apertura=0;
static int vecchio_stato=10;
int	stato_attuale;
int length;


if (plcTryOpen()!=0)
	{
	if (tentativi_apertura<120) 
		tentativi_apertura++;
	else if (tentativi_apertura==120)
        invXerror(IDS_SIGLA_PLC2,NO_OPEN,"%d");
		
    IntervaDebugPlcOpen=XtAppAddTimeOut(plc2AppContext,TEMPO_PLC_OPEN, 
		scaddebug_plc_open, NULL);
	}
else
	{
	Plc_aperto=True;
	stato_attuale=plcStatus();
	if (!stato_attuale)
		{
		if (tentativi_apertura<=120) 
           	tentativi_apertura++;
        IntervaDebugPlcOpen=XtAppAddTimeOut(plc2AppContext,TEMPO_PLC_OPEN, 
			scaddebug_plc_open, NULL);
		}
	else
		{
		tentativi_apertura=0;   
		IntervaDebugPlcOpen=XtAppAddTimeOut(plc2AppContext,TEMPO_PLC_OPEN, 
			scaddebug_plc_open, NULL);
		}
	if (stato_attuale!=vecchio_stato)
		{
		vecchio_stato=stato_attuale;
		if(!stato_attuale)
			{
			XtSetSensitive(GetWidgetByName("LOAD-PLC"),True);
            XtSetSensitive(GetWidgetByName("RESTORE"),True);
    		Plc_status = loadIcona (GetWidgetByName("Left_icon"), 
				&IconPplc_halt);
            XtSetSensitive(GetWidgetByName("RUNPB"),True);
            XtSetSensitive(GetWidgetByName("HALTPB"),False);

			if (Plc_status!=NULL)
    			XtVaSetValues(GetWidgetByName("Left_icon"),
					XmNlabelPixmap, Plc_status, NULL);
			}
		else
			{
    		XtSetSensitive(GetWidgetByName("LOAD-PLC"),False);
    		XtSetSensitive(GetWidgetByName("RESTORE"),False);
    		Plc_status = loadIcona (GetWidgetByName("Left_icon"), 
				&IconPplc_run);
            XtSetSensitive(GetWidgetByName("RUNPB"),False);
            XtSetSensitive(GetWidgetByName("HALTPB"),True);

			if (Plc_status!=NULL)
    			XtVaSetValues(GetWidgetByName("Left_icon"),
					XmNlabelPixmap, Plc_status, NULL);
            /* vado a riaggiornarmi gli indirizzi delle variabili che ho
			   correntemente in debug */
            changedebug=1;
            for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;
					ContaVariabili++)
              	{
                length=strlen(TestoNomiVariabili[ContaVariabili]);
                if(length!=0)
                   	{   
                    CercaVariabile((Widget)NULL,ContaVariabili);
                    }
                }
            AggiornaControlloVariabili();
			}
		}
	}
}

/*
 --------------------------------------------------------------------------
    Funzione scadTdebug
  
    Funzione chiamata ogni qual volta il timeout debug impostato scade:
    Gestisce la visualizzazione del debug 
    Input:  client_data = eventuale puntatore ai dati
            tempoId = puntatore all'identificatore del timeout
    Output: Nessuno
 --------------------------------------------------------------------------
*/
void scadTdebug(XtPointer client_data, XtIntervalId *tempoId)
{	
static 	int ora;
char    *get_string;
char    quota[30];
char    text[80];
XmString    str;
XmString    wid_label_string;
int		length;
int		rv;
Widget 	wid;
Widget 	wid_dra;
Window	window;
Display	*display;
int		i,j,k,l,qq;
int		index_var=0;
int		index_var_flag=0;
char	valore;
char	temp[NUMERO_VARIABILI*4];
float 	TempFloat;
char	TempChar;
char	*TempPuntChar;
Widget  sb_ios_wid;
static 	int visualizza_numero_moduli=0;
unsigned char status_temp;
int 	Old_y=0,index=0;
int		Numero_buffer=0;


if(typedebug==0)
	IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
else
	XtRemoveTimeOut(IntervaDebug);

wid=GetWidgetByName("RWCVAR");
if(wid!=NULL) 
	{
	if (Numero_Variabili_Plc!=0)
		plcMultiMemCopy(temp,Byte_Variabili_Plc);
	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
		{
		length=strlen(TestoNomiVariabili[ContaVariabili]);
		if(length!=0) 
			{
			if (TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO)
				{
				switch (StrutturaVariabili[ContaVariabili].type) 
					{
       				case ISAVAR_T_BOOL:
						sprintf(quota,"%d",
						*(char *)StrutturaVariabili[ContaVariabili].pval);
       					break;

       				case ISAVAR_T_ANA_I:

       				case ISAVAR_T_TIMER:
       					if(typeshow==1) 
							sprintf(quota,"0x%08lX",
							*(long *)StrutturaVariabili[ContaVariabili].pval);
               			else  
							sprintf(quota,"%ld",
							*(long *)StrutturaVariabili[ContaVariabili].pval);
       					break;

       				case ISAVAR_T_ANA_F:
       					sprintf(quota,"%g",
							*(float *)StrutturaVariabili[ContaVariabili].pval);
      					break;

       				default:
                  		sprintf(quota,"??");
       					break;
       				}
				if(strcmp(QuotaValoreVariabili[ContaVariabili],quota)
						||(changedebug==1))
                   	{       
                   	/* stringa diversa o cambiata la variabile da debuggare*/
                   	wid_label_string=XmStringCreateSimple(quota);
                   	XtVaSetValues(ValoreVariabili[ContaVariabili],
						XmNlabelString,wid_label_string,NULL);
					XmStringFree(wid_label_string);
                   	for(i=0;i<15;i++) 
						QuotaValoreVariabili[ContaVariabili][i]=quota[i];
                   	}
				}
			else
				{
				if (Flag_di_validita[index_var_flag]==1)
					{
					if (LONG_FLOAT[ContaVariabili]==LONG_TYPE)
						{
						if(typeshow==1) 
                           	sprintf(quota,"0x%08lX",
								*(unsigned long int *)&temp[index_var]);
                        else  
							sprintf(quota,"%ld",
								*(unsigned long int *)&temp[index_var]);
						}
                    else if (LONG_FLOAT[ContaVariabili]==INT_TYPE)
                        {
                        if(typeshow==1) 
                            sprintf(quota,"0x%04hX",
								*(unsigned short int *)&temp[index_var]);
                        else  
                            sprintf(quota,"%hd",*(short int *)&temp[index_var]);
                        }
                    else if (LONG_FLOAT[ContaVariabili]==UINT_TYPE)
                        {
                        if(typeshow==1) 
                            sprintf(quota,"0x%04hX",
								*(unsigned short int *)&temp[index_var]);
                        else  
                            sprintf(quota,"%hd",
								*(unsigned short int *)&temp[index_var]);
                        }
                    else if (LONG_FLOAT[ContaVariabili]==ULONG_TYPE)
                        {
                        if(typeshow==1) 
                            sprintf(quota,"0x%08lX",
								*(unsigned long int *)&temp[index_var]);
                        else  
                            sprintf(quota,"%lu",
								*(unsigned long int *)&temp[index_var]);
                        }
					else if (LONG_FLOAT[ContaVariabili]==FLOAT_TYPE)
						{
						TempFloat= *(float*)&temp[index_var];
                   		sprintf(quota,"%g",TempFloat);
						}
					else if (LONG_FLOAT[ContaVariabili]==CHAR_TYPE)
                        {
                        TempChar= *(char*)&temp[index_var];
                        sprintf(quota,"%d",TempChar);
                        }
					else if (LONG_FLOAT[ContaVariabili]==PUNT_CHAR_TYPE)
                        {
                        TempPuntChar= (char*)&temp[index_var];
						strncpy(text,TempPuntChar,
							Controllo_Var[index_var_flag].len);
						if (text[Controllo_Var[index_var_flag].len-1]!='\0')
							text[Controllo_Var[index_var_flag].len]='\0';
                        sprintf(quota,"%s",text);
                        } 
					else if (LONG_FLOAT[ContaVariabili]==STRING_TYPE)
                        {
                        /* Leggo l'indirizzo e vedo se e' cambiato */
                        TempPuntChar= *(char **)&temp[index_var];
                        index_var+=Controllo_Var[index_var_flag].len;
                        index_var_flag++;
						/* Se e' cambiato aggiorna il puntatore */
						if (TempPuntChar!=Controllo_Var[index_var_flag].addr) 
							{
							Controllo_Var[index_var_flag].addr=TempPuntChar;
							plcMultiMemCopy(temp,Byte_Variabili_Plc);
							}
                        if (Flag_di_validita[index_var_flag]==1)
							{
							TempPuntChar= (char*)&temp[index_var];
							strncpy(text,TempPuntChar,
								Controllo_Var[index_var_flag].len);
							if (text[Controllo_Var[index_var_flag].len-1]!='\0')
								text[Controllo_Var[index_var_flag].len]='\0';
                        	sprintf(quota,"%s",text);
                            }
						else 
							sprintf(quota,"??");	
                        } 
					index_var+=Controllo_Var[index_var_flag].len;
					}
				else
					sprintf(quota,"??");
				index_var_flag++;
				}
			if(strcmp(QuotaValoreVariabili[ContaVariabili],quota)||
					(changedebug==1))
               	{       
                /* stringa diversa o cambiata la variabile da debuggare*/
                wid_label_string=XmStringCreateSimple(quota);
                XtVaSetValues(ValoreVariabili[ContaVariabili],
					XmNlabelString,wid_label_string,NULL);
				XmStringFree(wid_label_string);
                for(i=0;i<15;i++) 
					QuotaValoreVariabili[ContaVariabili][i]=quota[i];
                }
			}
		}
	}
else
	{
	invXerror(IDS_SIGLA_PLC2,NO_WIDGET,"%d");
	}
if(typedebug==1)
	{
	/*	visualizzazione del trace una volta che ci siano elementi
		sufficienti						  */

	wid=GetWidgetByName("DRAIOS");
	if(wid!=NULL) 
		{
		rv = plcTraceDownLoad(tracebuf[numero_tracebuf]);
		if(rv)
			{
            if (rv != PLCERR_NOTREADY) 
               	{
				invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_DOWNLOAD,"%v",rv);
                }
            fflush(stdout);
			IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, 
				scadTdebug, NULL);
            }
		else
			{
			if (numero_tracebuf==(MAX_BUFFER_NUMBER-1))
				{
				if (TRIGGER_PRETRIGGER==1)
    				{
					if (Trovato_pre_trigger==True)
						{
                        /* ATTENZIONE */
                        rv = plcTraceStop();
                        if (rv) 
                         invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_STOP,"%v",rv);

                        VisualizzaTrace(0);
 
						Trovato_pre_trigger=False;
    					/* ricerca del ciclo di trigger nel caso pretrigger */
    					Old_y=tracebuf[0][0] & Trigger_Mask;

    					index=1;
						Numero_buffer=0;
						Ciclo_trigger=0;
    					while ((Ciclo_trigger==0)&&
								(Numero_buffer<MAX_BUFFER_NUMBER))
        					{
        					if (SALITA_DISCESA==0)
            					{
            					if ((Old_y!=(tracebuf[Numero_buffer][index] & 
										Trigger_Mask))&&(Old_y==Trigger_Mask))
                					Ciclo_trigger=(Numero_buffer*
										NUMERO_CAMPIONI)+index;
            					else
                					Old_y=tracebuf[Numero_buffer][index] & 
										Trigger_Mask;
            					}
        					else
            					{
            					if ((Old_y!=(tracebuf[Numero_buffer][index] & 
										Trigger_Mask))&&(Old_y==0))
                					Ciclo_trigger=(Numero_buffer*
										NUMERO_CAMPIONI)+index;
            					else
                					Old_y=tracebuf[Numero_buffer][index] & 
										Trigger_Mask;
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
							XSetForeground (XtDisplay(wid), gc, 
								colori_allocati[4].pixel);
            				XDrawLine(XtDisplay(wid),pixmap,gc,DISTXBOX+
								Ciclo_trigger,DISTYBOX,DISTXBOX+Ciclo_trigger,
								LARGYBOX+DISTYBOX);
							}
            			XSetForeground (XtDisplay(wid), gc,
							BlackPixelOfScreen (XtScreen (wid)));
						XCopyArea (XtDisplay(wid),pixmap,XtWindow(wid),gc,0,0,
							LARGX_DRA,LARGY_DRA,0,0);

						get_string=GetMessageString(MemLin,"CICLO");
            			sprintf(text,"%s  %2d",get_string,Ciclo_trigger);
            			str=XmStringCreateLocalized(text);
            			XtVaSetValues(GetWidgetByName("Status_label"),XmNlabelString,
							str,NULL);
            			XmStringFree(str);
    					}
					else if (Trovato_pre_trigger==False)
                        {
                        /* ATTENZIONE azzeramento del contatore di buffer */
                        numero_tracebuf=0;
				        IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, 
							scadTdebug, NULL);
                        return;
                        }
					}
                else 
                    {
                    rv = plcTraceStop();
                    if (rv) 
                        invXerror(IDS_SIGLA_PLC2,NO_PLC_TRACE_STOP,"%v",rv);

                    VisualizzaTrace(0);
                    }

				XtVaGetValues(GetWidgetByName("SCWIOS"),
					XmNverticalScrollBar, &sb_ios_wid, NULL);
    			xncMovePointerTo(sb_ios_wid);

				IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, 
					scadTdebug, NULL);
				Debug_status = loadIcona(GetWidgetByName("Right_icon"), &IconPplc_var);
				if (Debug_status!=NULL)
    				XtVaSetValues(GetWidgetByName("Right_icon"),
						XmNlabelPixmap, Debug_status, NULL);
				typedebug=0;	
				XtSetSensitive(GetWidgetByName("ios1"),True);
        		XtSetSensitive(GetWidgetByName("ios2"),True);
        		XtSetSensitive(GetWidgetByName("ios3"),True);
        		XtSetSensitive(GetWidgetByName("ios4"),True);
				}
			else
				{
				numero_tracebuf++;
				IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, 
					scadTdebug, NULL);
				}
			}
		}
	else
		XtRemoveTimeOut(IntervaDebug); 
	}

/* reset indicatore cambio nomi debug */
changedebug=0;

/* Gestione aggiornamento I/O su ios */
if(typedebug==3)  /* visualizzazione i/o ios in modo large */
	{
	wid_dra=GetWidgetByName("DRAIOS");
	XtVaGetValues(wid_dra,XmNuserData,&gc,NULL);
	window=XtWindow(wid_dra);
	display=XtDisplay(wid_dra);

	/* Verifico la stabilita' della linea */
	if (LINE_STATE!=(*(unsigned short int*) 
			(CNI_iosbrd[board_selezionata]+displacement_selezionato+0x25f)))
    	{
    	LINE_STATE=*(unsigned short int*) 
			(CNI_iosbrd[board_selezionata]+displacement_selezionato+0x25f);
    	if  (LINE_STATE) 
        	sprintf(text,"Unstable");
    	else
        	sprintf(text,"Stable");

    	str=XmStringCreateLocalized(text);
    	XtVaSetValues(GetWidgetByName("Stable_label"),
			XmNlabelString,str,NULL);
    	XmStringFree(str);
    	}

	/* Verifico se ci sono stati errori di comunicazione  o variazioni 
		dello stato ed aggiorno la pagina */
    if(ora==5)
        {
		visualizza_numero_moduli=!visualizza_numero_moduli;
        ora=0;  
        qq=0;
        for(j=0;j<32;j++)
            {
            if(status[j]!=*(CNI_iosbrd[board_selezionata]+
					displacement_selezionato+512+j)) 
                qq=1;
			if(error[j]!=*(CNI_iosbrd[board_selezionata]+
					displacement_selezionato+0x22d+j))
            	qq=1;
            }
        if(qq==1) 
            aggios_large();
        }
    else
        ora++;

	for(j=0;j<32;j++)
		{
		status_temp=status[j];
    	if ((status_temp >> 4) & 1)
			{
			/* modulo in fault non faccio nulla */
			}
		else if ((status_temp >> 5) & 1)
			{
			if (visualizza_numero_moduli==1)
				{
				XSetForeground (display, gc, colori_allocati[0].pixel);
				XFillRectangle(display,window,gc,0,DIST_TOP_NUM+(j*ELARGYL),
					DIST_SX-3,ELARGYL+2);
				}
			else if(visualizza_numero_moduli==0)
				{
				/* scrivo a sinistra il numero progressivo dei moduli */
				XSetForeground (display, gc,
					BlackPixelOfScreen(XtScreen (wid_dra)));
    			sprintf(text,"%2d",j);
    			length=strlen(text);
    			XDrawString(display,window,gc,0,DIST_TOP_NUM+(j+1)*ELARGYL,
					text,length);
				}
			}
		else
			{
			/* Esiste il modulo y ???	*/
			if((*(CNI_iosbrd[board_selezionata]+displacement_selezionato+512+j)
					&0x40)==0)
				{
				/*	modulo speciale ???	*/
				if((*(CNI_iosbrd[board_selezionata]+
						displacement_selezionato+512+j)&0x80)==0 ) 
					qq=3;
				else
					qq=8;
				for(i=0;i<qq;i++)
					{
					if (elemio[i][j]!=0)
						{
						if((filtio[i][j]!=(*(CNI_iosbrd[board_selezionata]+
								displacement_selezionato+j*8+i)))||(resize==1))
							{
							XCopyArea (display, pixmap, window, gc,
								DIST_SX+i*ELARGXL,DIST_TOP+j*ELARGYL,ELARGXL,
								ELARGYL,DIST_SX+i*ELARGXL,DIST_TOP+j*ELARGYL);
							valore=*(CNI_iosbrd[board_selezionata]+
								displacement_selezionato+j*8+i);
							filtio[i][j]=valore;
							for (k= 7,l=0; k>=0; k--,l++)
								{
								if ((valore >> k) & 1)
									text[l]='1';
								else
									text[l]='0';
								if(l==3)
									{
									text[l+1]=' ';
									l=l+1;
									}
								}
							text[9]='\0';
							length=strlen(text);
							XDrawString(display,window,gc,
								DIST_STRING_SX+i*ELARGXL,
								DIST_TOP_ERR+j*ELARGYL,text,length);
							}
						}
					}
				}
			}
		}
	if (XtIsManaged(GetWidgetByName("Visualizzatore")))
		IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
	else
		{
		IntervaDebug=XtAppAddTimeOut(plc2AppContext,TEMPOQ, scadTdebug, NULL);
		typedebug=0;
		}
	}
resize=0;		/* reset del force di resize finestra */
} 


/*
 ---------------------------------------------------------------
    Funzione loadLinguePlc
  
    Funzione che legge le lingue del modulo quote
    Input:  Nessuno
    Output: RETOK nel caso di tutto Ok
            RETER errore
 --------------------------------------------------------------
*/
int loadLinguePlc(void)
{
#ifdef Linux
MemLin=NULL;
        return(RETER);
#else
char    path[LUNGH_FILE];
char    nome[LUNGH_FILE];
char    lingua[LUNGH_NOME];

if(srdtm)
        {
        if(GetMaccInfo(srdtm,"Lingue",path)<0)
                return(RETER);
        if(GetMaccInfo(srdtm,ID_SDM_LINGUAATT,lingua)<0)
                return(RETER);
        strcpy(nome,"Lingue.1");
        }
else
        {
        strcpy(lingua,"ita");
        strcpy(path,"./");
        strcpy(nome,"Lingue.pdb");
        }
if(!(MemLin=xncReadLanguage(path,nome,lingua, plc2WD, plc2AD,True)))
        return(RETER);
return(RETOK);
#endif
}

/*
 --------------------------------------------------------------------
    Funzione setMnemonicMenuBar
  
    Widget      menubar         -   widget ID della menu' bar 
  
    
    Output: RETOK : operazione riuscita
          !=da RETOK errore:
             1 : allocazione memoria
             2 : menmonici non trovati
      Funzione che setta i mnemonici nella menu' bar. 
 --------------------------------------------------------------------
*/
int setMnemonicMenuBar(Widget menubar)
{
Cardinal		nfigli;
WidgetList		listaf;
Widget			helpw;
String			*strmenu;
XmString		xstr;
String			text;
String			mnemonic;
int				ncascade;
int				i,j,k;
char			mnemok;
char			mnhelp;
String			ptr;

/*---Cerca il numero di cascade che compone la menu bar---*/

XtVaGetValues(menubar,
	XmNchildren, &listaf,
	XmNnumChildren, &nfigli,
	XmNmenuHelpWidget, &helpw,
 	NULL);

for(ncascade=j=0;j<nfigli;j++)
	if(listaf[j]!=helpw && (XmIsCascadeButton(listaf[j]) ||
		XmIsCascadeButtonGadget(listaf[j])) )
			ncascade++;

/*---Cerca il mnemonico del bottone di help---*/

mnhelp='\0';
if(helpw)
	{
	XtVaGetValues(helpw,XmNlabelString,&xstr,NULL);
	XmStringGetLtoR(xstr, XmFONTLIST_DEFAULT_TAG, &text);
	for(ptr=text; *ptr ; ptr++)
		if(*ptr<0)
			memmove(ptr,ptr+1,strlen(ptr));
	mnhelp=text[0];
	XmStringFree(xstr);
	XtFree(text);
	}

/*---Crea l'array di stringhe della menu' bar---*/

if(!(strmenu=(String *)XtMalloc(ncascade*sizeof(String))))
	{
	invXerror(IDS_SIGLA_PLC2,NO_MALLOC,"%d");
	return(1);
	}
for(k=j=0;j<nfigli;j++)
	{
	if(listaf[j]!=helpw && (XmIsCascadeButton(listaf[j]) ||
		XmIsCascadeButtonGadget(listaf[j])) )
		{
		XtVaGetValues(listaf[j],XmNlabelString,&xstr,NULL);
		XmStringGetLtoR(xstr, XmFONTLIST_DEFAULT_TAG, &text);
		if(!(strmenu[k++]=XtNewString(text)))
			{
			XmStringFree(xstr);
			XtFree(text);
			for(i=0;i<j;i++)
				XtFree((XtPointer) strmenu[i]);
			XtFree((XtPointer)strmenu);
			invXerror(IDS_SIGLA_PLC2,NO_MALLOC,"%d");
			return(1);
			}
		XmStringFree(xstr);
		XtFree(text);
		for(ptr=strmenu[k-1]; *ptr ; ptr++)
			if(*ptr<0 || *ptr==mnhelp)
				memmove(ptr,ptr+1,strlen(ptr));
		}
	}

/*---Crea il vettore dei mnemonici della menu' bar---*/

if(!(mnemonic=(String)XtMalloc(ncascade*sizeof(char))))
	{
	for(j=0;j<ncascade;j++)
		XtFree((XtPointer) strmenu[j]);
	XtFree((XtPointer)strmenu);
	invXerror(IDS_SIGLA_PLC2,NO_MALLOC,"%d");
	return(1);
	}

/*---Tenta di depositare i mnemonici della menu' bar---*/

memset(mnemonic,0,ncascade);
mnemonic[0]=strmenu[0][0];
for(i=0,j=1; j<ncascade ; j++)
	{
	for(k=0,mnemok='\0'; strmenu[j][k]!='\0'; k++)
		{
		if(!strchr(mnemonic,strmenu[j][k]))
			{
			mnemok=strmenu[j][k];
			break;
			}
		}
	mnemonic[j]=mnemok;
	if (mnemok=='\0')
		{
		mnemok=strmenu[0][++i];
		if (mnemok=='\0')
			break;
		j=0;
		memset(mnemonic,0,ncascade);
		mnemonic[0]=mnemok;
		}
	}

/*---Setta la risorsa XmNmnemonic ai cascade della menu' bar---*/

if (mnemok)
	{
	if(helpw)
		XtVaSetValues(helpw,XmNmnemonic,mnhelp,NULL);
	for(k=j=0;j<nfigli;j++)
		if(listaf[j]!=helpw && (XmIsCascadeButton(listaf[j]) ||
			XmIsCascadeButtonGadget(listaf[j])) )
				XtVaSetValues(listaf[j],XmNmnemonic,mnemonic[k++],NULL);
	}

/*---Libera la memoria allocata---*/

for(j=0;j<ncascade;j++)
	XtFree((XtPointer) strmenu[j]);
XtFree((XtPointer)strmenu);
XtFree((XtPointer)mnemonic);
return(mnemok ? RETOK : 2);
}

/*
 ----------------------------------------------------------------------
    Funzione UpdateYellowLine
  
    Widget      hWidget         -   il widget proprietario
    NULL   
    XEvent      *pXEvent        -   struttura evento
  
    Ritorna:
        niente
  
 ----------------------------------------------------------------------
*/
void UpdateYellowLine(Widget hWidget, XtPointer clientData,XEvent *pXEvent)
{
KeySym 		Tasto;
Display 	*dpy;
char    	text[20];
XmString    str;
int     	mask=1;
int     	length=0;
int     	y1=DISTYBOX+6;
int			ciclo_corrente;
int			Buffer_corrente=0;

dpy=XtDisplay(GetWidgetByName("DRAIOS"));

if( (pXEvent->type==KeyRelease)&&(typedebug==0) )
    {
    Tasto=XKeycodeToKeysym( XtDisplay(hWidget), pXEvent->xkey.keycode, pXEvent->xkey.state );
	if ((Tasto==XK_Left)&&(posizione_cursore>DISTXBOX))
		posizione_cursore-=zoom;
	else if ((Tasto==XK_Right)&&(posizione_cursore<DISTXBOX+LARGXBOX))
		posizione_cursore+=zoom;
	if ((posizione_cursore>DISTXBOX)&&(posizione_cursore<DISTXBOX+LARGXBOX)) 
		{
		/* se sono nel range del box */
		VisualizzaTrace(1);
		XSetForeground (dpy, gc, colori_allocati[4].pixel);
		XDrawLine(dpy,pixmap,gc,posizione_cursore,DISTYBOX,posizione_cursore,
			LARGYBOX+DISTYBOX);
		ciclo_corrente=Min_Trace+((posizione_cursore-DISTXBOX)/zoom);
		sprintf(text,"Ciclo  %2d",ciclo_corrente);
        str=XmStringCreateLocalized(text);
		XtVaSetValues(GetWidgetByName("Status_label"),
			XmNlabelString,str,NULL);
		XmStringFree(str);
		Buffer_corrente=Min_Trace/NUMERO_CAMPIONI;
		XSetForeground (dpy, gc,BlackPixelOfScreen (XtScreen(GetWidgetByName("DRAIOS"))));
		for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
       		{
       		if (((StrutturaVariabili[ContaVariabili].type==ISAVAR_T_BOOL)&&
				(TipoVariabili[ContaVariabili]==SEGNALE_DI_SCAMBIO))||
				((TipoVariabili[ContaVariabili]==VARIABILE_PLC)&&
				(LONG_FLOAT[ContaVariabili]==CHAR_TYPE)))
           		{
				if ((tracebuf[Buffer_corrente][ciclo_corrente-
						(NUMERO_CAMPIONI*Buffer_corrente)-1] & mask)==0)
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
		XCopyArea (dpy,pixmap,XtWindow(GetWidgetByName("DRAIOS")),
			gc,0,0,LARGX_DRA,LARGY_DRA,0,0);
		}
	}
}

/*
 ----------------------------------------------------------------------
    Funzione DefType
  
    Widget      Widget         -   il widget proprietario
    Widget		popup_wdg		-  il widget del popup menu   
    XEvent      *pXEvent        -   struttura evento
  
	Chimata alla pressione del terzo bottone sui text field
    Ritorna:
        niente
  
 ----------------------------------------------------------------------
*/
void DefType(Widget widget, Widget popup_wdg, XEvent *pXEvent)
{
char	*DebVar;
int		ContaVariabili;

if( pXEvent->type==ButtonPress || pXEvent->type==ButtonRelease )
	{
	if ( pXEvent->type==ButtonPress )
		PopUpCorrente=-1;
	if(pXEvent->xbutton.button != 3)
		return;
	XtVaGetValues(widget,XmNvalue,&DebVar,NULL);
	}

	for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
		{
		if (widget == NomiVariabili[ContaVariabili])
			{
			PopUpCorrente=ContaVariabili;
			break;
			}
		}

	if (DebVar==NULL || strstr(DebVar,"GENERAL") == NULL)
		{
        XtUnmanageChild (popup_int[PopUpCorrente]);
        XtUnmanageChild (popup_real[PopUpCorrente]);
        XtUnmanageChild (popup_sep[PopUpCorrente]);
		}
	else
		{
		XtManageChild (popup_int[PopUpCorrente]);
		XtManageChild (popup_real[PopUpCorrente]);
		XtManageChild (popup_sep[PopUpCorrente]);
		}

    /* Position the menu where the event occurred */
    XmMenuPosition (popup_wdg, (XButtonPressedEvent *) (pXEvent));
    XtManageChild (popup_wdg);

}

/*

		+-------------------------------+
		|								|
		|	     getUserDataInt			|
		|								|
		+-------------------------------+

Funzione che ritorna lo user data quando
questo e' settato ad intero (int).
INPUT:	wdg:		widget a cui chiedere il dato
OUTPUT:	valore dello user data (0 nel caso il widget non esiste).
*/
int getUserDataInt(Widget wdg)
{
XtPointer	udata;
int			usrint;

usrint=0;
if(wdg)
	{
	XtVaGetValues(wdg,
		XmNuserData,&udata,
		NULL);
	usrint=(int) udata;
	}
return(usrint);
}
/*
 ----------------------------------------------------------------------
    Funzione popup_cb
  
    Widget      menu_item       -   Widget del popUp chiamato
    Widget		client_data		-   Numero della voce chiamata
    XtPointer   call_data       -   struttura della callback
  
	Chiamata alla selezione di una voce del pop up menu
    Ritorna:
        niente
  
 ----------------------------------------------------------------------
*/
void popup_cb(Widget menu_item, XtPointer client_data, XtPointer call_data)
{
XmRowColumnCallbackStruct       *cbs;
int								elem;

if (PopUpCorrente != -1)
	{
	cbs=(XmRowColumnCallbackStruct *) call_data;
	switch((elem=getUserDataInt(cbs->widget)))
		{
        case PDB_SRVZ_INT:
			if (TipoVariabili[PopUpCorrente]==SEGNALE_DI_SCAMBIO)
				StrutturaVariabili[PopUpCorrente].type=ISAVAR_T_TIMER;
        break;
        case PDB_SRVZ_REAL:
			if (TipoVariabili[PopUpCorrente]==SEGNALE_DI_SCAMBIO)
				StrutturaVariabili[PopUpCorrente].type=ISAVAR_T_ANA_F;
        break;
        case PDB_SRVZ_TRACE:
			buttonchange(menu_item,(XtPointer)TRACE_BUTTON,NULL);
        break;
		}
	}
}
