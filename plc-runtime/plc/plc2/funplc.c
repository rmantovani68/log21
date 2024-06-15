#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
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
#ifdef Linux
#include <sys/msg.h>
#else
#include <msg.h>
#endif

#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#ifdef Linux
#include <libxad.h>
#else
#include <Xm/libxad.h>
#endif

#include "equat.h"
#include <plclink.h>
#ifdef Linux
#else
#include <services.h>
#include <libxerr.h>
#include <CniAlf.h>
#endif

/*
    =====   dichiarazione external =====
    ------------------------------------
*/
#ifdef Linux
extern 	void * 	fbox;
#else
#ifdef _OLD_FILE_SEL
extern PTFILESELBOX 	fbox;
#else
extern PTXNCNFSB 	fbox;
#endif
#endif
extern unsigned long    IndirizzoVariabili[NUMERO_VARIABILI];
extern struct plc_mcopy_t Controllo_Var[NUMERO_VARIABILI];
extern String   srdtm;
extern XsWdesc	*mainWD;
extern char    	TestoNomiVariabili[NUMERO_VARIABILI][17];
extern char    	Flag_di_validita[NUMERO_VARIABILI];
extern char		NomeFont[];
extern char		InputName[];
extern char		OutputName[];
extern int		OffsetValue;
extern int		nostdout;
extern int		nostderr;
extern int      noserverdm;
extern int      simessaggi;
extern int      uppercase;
extern int      FocusBool;
extern int     	Numero_Variabili_Plc;
extern int 		Byte_Variabili_Plc;
extern int     	ContaVariabili;
extern int     	TipoVariabili[NUMERO_VARIABILI];
extern int     	LONG_FLOAT[NUMERO_VARIABILI];
extern int     	Lunghezza_Stringa[NUMERO_VARIABILI];

#ifdef _OLD_FILE_SEL
extern void 	cbkSelezionaFile(int,char *,XtPointer);
#else
extern void 	cbkSelezionaFileNew(int,int,char *,XtPointer);
#endif
extern	int 	GestioneInputOutputBS(char *);

/*
    =====   dichiarazione funzioni =====
    ------------------------------------
*/
void 	inputArgvArgc(int , char **argv);
int 	iniServerDtm(void);
int 	iniGestioneXerr(void);
void 	stampaMes(String , ...);
void 	stampaErr(String , ...);
Boolean stringaIsVuota(String );
int 	contaSepStringa(String ,char );
int 	CreaDialogShSelFile(Widget ,int );
int 	TrovaIndirizzo(char *,long , char *, unsigned long *);
Pixmap 	loadIcona(Widget , Icona *);
void 	AggiornaControlloVariabili(void);
Widget 	GetWidgetByName(char *);

/*
 ----------------------------------------------------
    Funzione inputArgvArgc
  
    Funzione che gestisce i parametri in input

    Input:	argc argv del main
    Output:	Nessuno
 ----------------------------------------------------
*/
void inputArgvArgc(int argc, char *argv[])
{
int		j;

if (argc>1)
	{
	for(j=1;j<argc;j++)
		{
		if(!strcmp(argv[j],"-NoStdout"))
			nostdout=True;
		else if(!strcmp(argv[j],"-NoStderr"))
			nostderr=True;
		if(!strcmp(argv[j],"-NoServerdm"))
			noserverdm=True;
                if(!strcmp(argv[j],"-SiMessaggi"))
                        simessaggi=True;
		else if(!strcmp(argv[j],"-uppercase"))
			uppercase=True;
		else if(!strcmp(argv[j],"-FocuBool"))
			FocusBool=True;
		else if(!strcmp(argv[j],"-FocusBool"))
			FocusBool=True;
		else if(!strncmp(argv[j],"-DrawFont=",strlen("-DrawFont=")))
			strcpy(NomeFont,argv[j]+strlen("-DrawFont="));
		else if(!strncmp(argv[j],"-I=",strlen("-I=")))
			strncpy(InputName,argv[j]+strlen("-I="),10);
		else if(!strncmp(argv[j],"-O=",strlen("-O=")))
			strncpy(OutputName,argv[j]+strlen("-O="),10);
		else if(!strncmp(argv[j],"-Add=",strlen("-Add=")))
			OffsetValue=atoi(argv[j]+strlen("-Add="));
		else if(!strcmp(argv[j],"-Help") || !strcmp(argv[j],"-help"))
			{
			printf("plc2 : Versione %s \r\n", IDS_PLC2_VER);
			printf("opzioni previste:\r\n");
			printf("\t-NoStdout=non redirige stdout\r\n");
			printf("\t-NoStderr=non redirige stderr\r\n");
            printf("\t-NoServerdm=non apre verso il serverdm \r\n");
			printf("\t-SiMessaggi=fa comparire i messaggi di debug\r\n");
			printf("\t-uppercase=rende maiuscoli i nomi negli entry del debug\r\n");
			printf("\t-FocusBool=assegna il fuoco al bottone BOOL nel pannello Tipo Variabile\r\n");
			printf("\t-DrawFont=assegna il font della drawing area\r\n");
			printf("\t-I=assegna il nome del vettore di input \r\n");
			printf("\t-O=assegna il nome del vettore di output \r\n");
			printf("\t-Add=assegna l'offset da sottrarre al vettore I/O default = 1\r\n");
			exit(0);
			}
		}
	}
} 

/*
 -------------------------------------------------------------------
    Funzione iniServerDtm 
  
    Funzione che inizializza il colloquio e con
    il server dei dati macchina.

    Input:	Nessuno
    Output:	RETER in caso di errore RETOK nel caso di tutto Ok
 -------------------------------------------------------------------
*/
int iniServerDtm(void)
{
#ifdef Linux
#else
if(noserverdm==False)
	if ((srdtm=AttachServerDm(ID_SDM_CN))==NULL)
		return(RETER);
#endif
return(RETOK);
} 

/*
 ---------------------------------------------------------------------
    Funzione iniGestioneXerr 
    Funzione che inizializza il colloquio con i canali degli errori.

    Input:	Nessuno
    Output:	RETOK nel caso di tutto Ok
    		RETER errore
 ---------------------------------------------------------------------
*/
int iniGestioneXerr(void)
{
if(initXerror()==-1)
	return(RETER);
if(nostdout==False)
	if(initStdout()==-1)
		return(RETER);
if(nostderr==False)
	if(initStderr()==-1)
		return(RETER);
return(RETOK);
} 

/*
 ---------------------------------------------------------------------
    Funzione stampaMes 
    Funzione che stampa un messaggio su stdout

    Input:	fmt address formato da stampare
    Output:	Nessuno
 ---------------------------------------------------------------------
*/
void stampaMes(String fmt, ...)
{
va_list	argp;

if(simessaggi==True)
	{
	va_start(argp,fmt);
	printf("PLC");
	printf(" : ");
	vprintf(fmt,argp);
	printf("\r\n");
	va_end(argp);
	}
}

/*
 ---------------------------------------------------------------------
    Funzione stampaErr 
    Funzione che stampa un messaggio di errore su stderr

    Input:	fmt address formato da stampare
    Output:	Nessuno
 ---------------------------------------------------------------------
*/
void stampaErr(String fmt, ...)
{
va_list	argp;

va_start(argp,fmt);
fprintf(stderr,"PLC");
fprintf(stderr," : ");
vfprintf(stderr,fmt,argp);
fprintf(stderr,"\r\n");
va_end(argp);
}

/*
 -------------------------------------------------------------------
    Funzione stringaIsVuota 
  
    Funzione che ritorna se la stringa in input
    e' vuota oppure contiene almeno un carattere.
    E' una stringa vuota se il puntatore vale NULL
    se la stringa contiene solo dei bianchi o contiene
    la parola NULL oppure se c'e' solo un fine linea.

    Input:	str=	address stringa da esaminare
    Output:	True stringa vuota False stringa con almeno un carattere
 -------------------------------------------------------------------
*/
Boolean stringaIsVuota(String str)
{
int		j,len;

if(str==NULL)
	return(True);
if(!(len=strlen(str)))
	return(True);
if(!strcmp(str,"NULL"))
	return(True);
for(j=0;j<len;j++)
	if(!isspace(str[j]))
		return(False);
return(True);
}

/*
 --------------------------------------------------------------------
    Funzione contaSepStringa 
  
    Funzione che ritorna il numero di caratteri
    del tipo specificato in input presenti nella
    stringa in input.

    Input:	str=	address stringa da esaminare
    		sep=	carattere da esaminare
    Output:	True stringa vuota False stringa con almeno un carattere
 --------------------------------------------------------------------
*/
int contaSepStringa(String str,char sep)
{
int		j,len,nums;

if(str==NULL)
	return(0);
if(!(len=strlen(str)))
	return(0);
if(!strcmp(str,"NULL"))
	return(0);
for(nums=j=0;j<len;j++)
	if(str[j]==sep)
		nums++;
return(nums);
} 

/*
 -------------------------------------------------------------
    Funzione CreaDialogShSelFile 
  
    Funzione che crea la top level shell di
    selezione variabili o programmi PLC

    Input:	babboW:		widget del padre
   		  tipofile:	creazione per ID_DIR_VAR o ID_DIR_PPLC
    Output:	RETOK tutto ok RETER errore
 -------------------------------------------------------------
*/
int CreaDialogShSelFile(Widget babboW,int tipofile)
{
#ifdef Linux
#else
#ifdef _OLD_FILE_SEL
char		pplc[LUNGH_FILE];
char		varp[LUNGH_FILE];
#endif
char		*paths[3];

#ifdef _OLD_FILE_SEL
if(srdtm)
   	{
    if(GetMaccInfo(srdtm,"PlcObj",pplc)<0)
      	return(RETER);
    if(GetMaccInfo(srdtm,"VarPlc",varp)<0)
        return(RETER);
	}
else
	{
	strcpy(pplc,"/home/xnc/bin/plc-objs/");
	strcpy(varp,"/home/d_xnc/varplc/");
	}

paths[ID_DIR_VARP]=varp;
paths[ID_DIR_PPLC]=pplc;
paths[2]=NULL;

fbox=xncCreateFileSelBox(srdtm, babboW, paths,
	cbkSelezionaFile, NULL, NULL, NULL, ID_XNC_DIALOG_SH,NULL);

if(tipofile==ID_DIR_VARP)
	if (!(xncSetFileSelBoxType(fbox,ID_DIR_VARP)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
		}
else if(tipofile==ID_DIR_PPLC)
	if (!(xncSetFileSelBoxType(fbox,ID_DIR_PPLC)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
		}
XtUnmanageChild(fbox->hTipoFileCbWidget);

#else

paths[ID_DIR_VARP]="VarPlc";
paths[ID_DIR_PPLC]="PlcObj";
paths[2]=NULL;

fbox=xncNfsbCreate(babboW, srdtm, paths,
    ID_CA_NFSB_FLAG_NOQUO |
    ID_CA_NFSB_FLAG_NOEDT |
    ID_CA_NFSB_FLAG_NOGRF |
    ID_CA_NFSB_FLAG_NOWRB |
    ID_CA_NFSB_FLAG_NOPRINT |
    ID_CA_NFSB_FLAG_TOPLEVEL |
    ID_CA_NFSB_FLAG_NOBTNFILE |
    ID_CA_NFSB_FLAG_NODISKA |
    ID_CA_NFSB_FLAG_NOOPT |
    ID_CA_NFSB_FLAG_NOCLOSEONOK |
    ID_CA_NFSB_FLAG_CHANGEBGDRW |
/*    ID_CA_NFSB_FLAG_THIRDBTN | */
    ID_CA_NFSB_FLAG_NOREMOTE,
    NULL, NULL,
    cbkSelezionaFileNew,
    NULL);

if(tipofile==ID_DIR_VARP)
	if(!(xncNfsbSetPath(fbox, ID_DIR_VARP)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
		}
else if(tipofile==ID_DIR_PPLC)
	if(!(xncNfsbSetPath(fbox, ID_DIR_PPLC)))
		{
		invXerror(IDS_SIGLA_PLC2,NO_SET_FILESELBOX_TYPE,"%d");
		}

#endif
#endif /* !Linux */
return(RETOK);
}

/*
 ----------------------------------------------------------------------------
    Funzione TrovaIndirizzo
  
    Input: nome dell'oggetto,
  		   indirizzo del segmento common,
  		   nome della variabile da cercare,
           indirizzo di una variabile dove scrivere il risultato.
    Output: valore di ritorno : 0 in caso di successo, -1 in caso di errore,
            variabile puntata da "r" : indirizzo (eventualmente) trovato.
  
 ----------------------------------------------------------------------------
*/
int TrovaIndirizzo(char *objname,long bss, char *nome, unsigned long *r)
{
FILE 	*obj;
struct 	exec head;
struct 	nlist sym;
long 	str_pos,sym_pos;
unsigned long	addr;
char 	*p;
int		ch;
int 	ns;
int 	i;
long 	posizione;

/* apertura del file caricato */
obj = fopen(objname,"r");
if (! obj) 
	{
	goto ERRORE;
	}

/* prendo gli indirizzi ed i dati relativi al file considerato */
if (fread((char *)&head, sizeof(head), 1, obj) != 1) 
	{
	goto ERRORE;
	}

/* Naturalmente, qualche simbolo ci vuole... */
if (! head.a_syms) 
	{
	goto ERRORE;
	}

/* Calcolo della posizione del campo contenente la lunghezza
 della string table. */
str_pos = head.a_text + head.a_data +
		 head.a_trsize + head.a_drsize +
		 head.a_syms + sizeof(head);

/* Calcolo della posizione della symbol table. */
sym_pos = head.a_text + head.a_data +
		 head.a_trsize + head.a_drsize +
		 sizeof(head);

/* Posizionamento e lettura di simboli e nomi. */
if (fseek(obj,sym_pos,0)) 
	{
	goto ERRORE;
	}

ns = head.a_syms / sizeof(struct nlist);

for (i = 0; i < ns; ++i) 
	{
	if (fread((char *)&sym, sizeof(sym), 1, obj) != 1) 
		{
		goto ERRORE;
		}
	if ((sym.n_type & N_TYPE) == N_UNDF && sym.n_value != 0) 
		{
		addr =  bss;
		bss += sym.n_value;
		}
	posizione = ftell(obj);
	fseek(obj, str_pos + sym.n_un.n_strx,0);
	for (p = nome; ; ++p)
		{
		ch = fgetc(obj);
		if (ch == *p)
			{
			if (*p == '\0') 
				{
				fclose(obj);
				*r = addr;
				return 1;
				}
			}
		else
			break;
		}
	fseek(obj, posizione,0);
	}
ERRORE:
invXerror("IDS_SIGLA_PLC2",NO_INDIRIZZO_VAR_NON_INIZ,"%d");
fclose(obj);
return 0;
}

/*
 -------------------------------------------------------------------
    Funzione macchinaIsStart
  
    Input: NIENTE
    Output: valore di ritorno : True in caso di macchina in start, 
                                False in caso di macchina in stop,
 -------------------------------------------------------------------
*/
Boolean macchinaIsStart(void)
{
#ifdef Linux
    return(False);
#else
int     k;
float   appo;

if((k=GetGenericData(srdtm,ES_PAR_FLAG,1,ES_START,&appo))<0)
	{
    invXerror(IDS_SIGLA_PLC2,PLC2_ERD_FLGMAC,"%d%v",GetError(k));
	return(False);
	}
else
    return((!appo) ? False : True);
#endif
}

/*
 --------------------------------------------------------------------
    Funzione loadIcona
  
    Funzione che crea il pixmap.
    Input:  wdg:        widget che deve ospitare il pixmap
            icon:       address struttura di definizione icona
    Output: Pixmap creato (NULL=errore)
 --------------------------------------------------------------------
*/
Pixmap loadIcona(Widget wdg, Icona *icon)
{
char        dirb[MAX_PATH_LEN];
char        nome[MAX_PATH_LEN];
int         width,height;
Pixmap      pixmap;
XColor      xcolor,unused;
Pixel       fg, bg;
int         nRC;

/*---tenta di caricare il file di pixmap---*/

if(icon->xpm_nomef!=NULL)
    {
#ifdef Linux
        strcpy(dirb, DIR_ICONE);
#else
    nRC=GetMaccInfo(srdtm, DM_DIRICONCNI, dirb);
    if( nRC<0 )
        {
        invXerror(IDS_SIGLA_PLC2, SETSDMERROR(nRC), NULL);
        strcpy(dirb, DIR_ICONE);
        }
#endif
    sprintf(nome,"%s%s",dirb,icon->xpm_nomef);
    if((pixmap=xncReadXpmFile(wdg, nome, &width , &height)) != NULL)
        return(pixmap);
    }

/*---tenta di caricare i dati di bitmap---*/

XtVaGetValues(wdg, XmNforeground, &fg,
    XmNbackground, &bg, NULL);

if(icon->xbm_foreg!=NULL)
   if(XAllocNamedColor(XtDisplay(wdg),
       DefaultColormapOfScreen(XtScreen(wdg)),
       icon->xbm_foreg, &xcolor, &unused))
       fg=xcolor.pixel;

if(icon->xbm_dati!=NULL)
    {
    pixmap=XCreatePixmapFromBitmapData( XtDisplay(wdg),
        RootWindowOfScreen (XtScreen (wdg)),
        icon->xbm_dati, icon->xbm_width, icon->xbm_height, fg, bg,
        DefaultDepthOfScreen (XtScreen (wdg)));
    return(pixmap);
    }

return((Pixmap)NULL);
} 

/*
 ----------------------------------------------------------------------
    Funzione AggiornaControlloVariabili(void)
         aggiorna la struttura da dare in pasto al PLC per le variabili 
         globali del plc

    Input: nessuno.
    Output: nessuno.
 ----------------------------------------------------------------------
*/
void AggiornaControlloVariabili(void)
{
int lenght;
int rv;
int indice_riga=-1;
int indice_colonna=-1;
int offset=0;
char    temp[NUMERO_VARIABILI*4];
char    NomeTemporaneo[MAXPATHLEN];
char    NomeTemporaneoBS[MAXPATHLEN];
char    *Posizioneparentesi;

Numero_Variabili_Plc=0;
Byte_Variabili_Plc=0;
for (ContaVariabili=0;ContaVariabili<NUMERO_VARIABILI;ContaVariabili++)
    {
    lenght=strlen(TestoNomiVariabili[ContaVariabili]);
    if(lenght!=0) 
        {
        if (TipoVariabili[ContaVariabili]==VARIABILE_PLC)
            {
			sprintf(NomeTemporaneoBS,"%s",TestoNomiVariabili[ContaVariabili]);
    		GestioneInputOutputBS(NomeTemporaneoBS);
			strcpy(NomeTemporaneo,NomeTemporaneoBS);
            if ((Posizioneparentesi=
				strchr(NomeTemporaneoBS,(int)'[')) != NULL )
				{
			    if ((Posizioneparentesi=strtok(NomeTemporaneo,"[")) != NULL )
				    {
				    if ((Posizioneparentesi=
						strchr(NomeTemporaneoBS,(int)',')) != NULL )
					    {
					    Posizioneparentesi=strtok(NULL, ",");
					    indice_riga=atoi(Posizioneparentesi);
					    Posizioneparentesi=strtok(NULL, "]");
					    indice_colonna=atoi(Posizioneparentesi);
					    if (Lunghezza_Stringa[ContaVariabili] != 0)
					      offset=(indice_riga*Lunghezza_Stringa[ContaVariabili])
								 + indice_colonna;
					    else
					  	    {
						    offset=0;
						    invXerror(IDS_SIGLA_PLC2,NO_PLC_SECOND_INDEX,NULL);
						    }
					    }
				    else 
					    {
					    Posizioneparentesi=strtok(NULL, "]");
					    offset=atoi(Posizioneparentesi);
					    }
                    if ((LONG_FLOAT[ContaVariabili]==LONG_TYPE)||
						(LONG_FLOAT[ContaVariabili]==FLOAT_TYPE)||
                        (LONG_FLOAT[ContaVariabili]==ULONG_TYPE))
					    {
                	    Controllo_Var[Numero_Variabili_Plc].addr=
						(char *)(IndirizzoVariabili[ContaVariabili]+(4*offset));
					    }
				    else if ((LONG_FLOAT[ContaVariabili]==INT_TYPE)||
							(LONG_FLOAT[ContaVariabili]==UINT_TYPE))
						{
                	    Controllo_Var[Numero_Variabili_Plc].addr=
						(char *)(IndirizzoVariabili[ContaVariabili]+(2*offset));
						}
				    else if (LONG_FLOAT[ContaVariabili]==CHAR_TYPE)
					    {
                	    Controllo_Var[Numero_Variabili_Plc].addr=
						(char *)(IndirizzoVariabili[ContaVariabili]+offset);
					    }	
					}
				}
  			else
				{
                Controllo_Var[Numero_Variabili_Plc].addr=
					(char *)(IndirizzoVariabili[ContaVariabili]);
				}
			if ((LONG_FLOAT[ContaVariabili]==LONG_TYPE)||
				(LONG_FLOAT[ContaVariabili]==FLOAT_TYPE)||
                (LONG_FLOAT[ContaVariabili]==ULONG_TYPE))
				{
            	Controllo_Var[Numero_Variabili_Plc].len=4;
				Byte_Variabili_Plc+=4;
				}
            else if ((LONG_FLOAT[ContaVariabili]==INT_TYPE)||
					(LONG_FLOAT[ContaVariabili]==UINT_TYPE))
				{
            	Controllo_Var[Numero_Variabili_Plc].len=2;
				Byte_Variabili_Plc+=2;
				}
			else if (LONG_FLOAT[ContaVariabili]==CHAR_TYPE)
				{
				Controllo_Var[Numero_Variabili_Plc].len=1;
                Byte_Variabili_Plc+=1;
				}
			else if (LONG_FLOAT[ContaVariabili]==PUNT_CHAR_TYPE)
                {
                Controllo_Var[Numero_Variabili_Plc].len=
					Lunghezza_Stringa[ContaVariabili];
                Byte_Variabili_Plc+=Lunghezza_Stringa[ContaVariabili];
                } 
			else if (LONG_FLOAT[ContaVariabili]==STRING_TYPE)
                {
                /* Se e' una stringa, prendo l'indirizzo
				  tramite una memcopy ed inserisco l'indirizzo
				  ottenuto nell'array */ 
                Controllo_Var[Numero_Variabili_Plc].len=4;
				Numero_Variabili_Plc++;
				rv=plcMultiMemCheck(Flag_di_validita,Controllo_Var,Numero_Variabili_Plc);
				if (rv!=0)
    				invXerror(IDS_SIGLA_PLC2,NO_PLC_VAR_CHECK,"%v",rv);
                Byte_Variabili_Plc+=4;
				plcMultiMemCopy(temp,Byte_Variabili_Plc);
				Controllo_Var[Numero_Variabili_Plc].addr= 
					*(char **)&temp[Byte_Variabili_Plc-4];
				Controllo_Var[Numero_Variabili_Plc].len=
					Lunghezza_Stringa[ContaVariabili];
                Byte_Variabili_Plc+=Lunghezza_Stringa[ContaVariabili];
                } 
            Numero_Variabili_Plc++;
            }
        }
    }    
rv=plcMultiMemCheck(Flag_di_validita,Controllo_Var,Numero_Variabili_Plc);
if (rv!=0)
	invXerror(IDS_SIGLA_PLC2,NO_PLC_VAR_CHECK,"%v",rv);
}

/*
 -------------------------------------------------------------------------------
    Funzione GetWidgetByName
  
    Funzione che riceve il nome del widget e ne ritorna il widget
  
    INPUT:
    char      *pszName   -   Nome del Widget da cercare
  
    Ritorna:
        il Widget il cui nome e' pszName
  ------------------------------------------------------------------------------
-
*/
Widget GetWidgetByName(char *pszName)
{
int nIndex;

nIndex=XsParseWidgetName(mainWD, pszName);

return(mainWD[nIndex].wPnt);
}
