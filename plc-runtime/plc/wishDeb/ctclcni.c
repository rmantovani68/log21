/*
   Versione 1.0.0.1

   	Modificata la funzione QualeRegistroCmd .
   	Il tipo di pacchetto viene ora riconosciuto tramite una 
   	funzione di libreria (LeggeStringa).
   	Il settaggio del registro in cui andare a pescare
   	i dati e' demandato al tcl 

   Versione 1.0.0.2

   	Modificata la funzione WatchShowRemoteCmd .
   	Aumentate le dimensioni del buffer temp_buffer a 1024.

   Versione 1.0.0.3

   	Modificato la gestione di watch_id. Prima era un vettore
	di venti elementi ora c'e' una gestione a bit.
	Portata fuori dall' ifdef XNC la lettura e scrittura
	dei segnali di scambio

   Versione 1.0.0.4

	??? Non documentato.

   Versione 1.0.0.3b

	(GG - Mon Nov 13 16:47:51 CET 2000)
	Modifiche per trasporto su Linux. Sono condizionate all'esistenza
	della macro "Linux".
*/
#ifdef WIN32

#define STRICT
#define OEMRESOURCE
#include <windows.h>
#include <keylibcni.h>

#endif

#include <tcl.h>
#include <tk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <aout.h>
#else
#ifdef Linux
#include "lynx-a.out.h"
#include "lynx-nlist.h"
#else
#include <a.out.h>
#endif
#endif

/* definizioni, variabili globali,prototipi generali UNIX/WINDOWS */
#define POLINOMIO_GENERATORE 19
#define	BIT_TOTALI 12
#define DIMENSIONE_BYTE 8
#define GRADO_POLINOMIO_GENERATORE 4
#define POLINOMIO_GENERATORE_SHIFTATO 19<<7

union real
  {
  long temp;
  float temp1;
  };

#include "custom.h"

#ifndef WIN32
/* definizioni, variabili globali,prototipi per UNIX */
#ifdef Linux
#include <sys/msg.h>
#else
#include <msg.h>
#endif

#include <plclink.h>
#ifdef Linux
#include <libpcftp.h>
#define MAXNAMLEN 1024
#else
#include <CniAlf.h>
#include <libxerr.h>
#include <libpcftp.h>
#include <hserax.h>
#endif

/* Definizione delle variabili per CN */
int plcAperto=0;
#ifdef XNC
int 		CanaleErrori=0;
int 		ServerAssi=0;
char  		*ptRamDM=NULL;
Sh_Address	shmem;
#endif

#ifdef DEBUG_PLC
/* Definizione di una mappa di nome "x" di  "n" bit. */
#define _brk_def_bittab(x,n) char x[(n+7)/8]
/* Inizializzazione. */
#define _brk_clr_bittab(x) memset((x),0,sizeof(x));
/* Accensione, spegnimento e test del bit "b" della mappa "x". */
#define _brk_set_bit(x,b) ((x)[(b) / 8] |= (1 << ((b) % 8)))
#define _brk_res_bit(x,b) ((x)[(b) / 8] &= ~(1 << ((b) % 8)))
#define _brk_tst_bit(x,b) (((x)[(b) / 8] & (1 << ((b) % 8))) != 0)

_brk_def_bittab(watch_id,(1<<9));
#define MAX_WATCH (1<<9)
static watch_descr_t w_param;
struct brk {
    watch_t bp;
    struct brk *next;
};

static struct brk br[MAX_WATCH*4];
static struct brk *bp[MAX_WATCH];
static struct brk *br_free = (struct brk *)0;
int StartCycle = -1;
#endif

#else

// definizioni, variabili globali,prototipi per WINDOWS

#define  MAX_WATCH 	20
#include "ctclcniw.h"
#include "plclink.h"
#include "libpcftp.h"

HINSTANCE salvaInst;
HDC printHDC;		// HDC stampante
HWND printHwnd;		// Handler window principale tcl
PRINTDLG pd;
DOCINFO di;
DEVMODE *pDevMode = NULL ;
RESCALEDAT datiConv;
HBITMAP  hbm;		// oggetto bitmap letto da file su disco 

// prototipi funzioni

int InitScale ( RESCALEDAT *rd );
int GimmeHatch  ( int tipo );
int GimmePenStyle  ( int tipo );
int ReadInput (int argc, char **argv, PRTINPUT *ps );
int LoadBmpFile( char *nomeFile, HDC hDBmp, HBITMAP *pHBmp );

#endif


#ifndef WIN32

/* 
 ***********************************************************************************
 * OpenPLCCmd --
 * Apre la connessione con il PLC
 ***********************************************************************************
 */
int OpenPLCCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv;

  if ((argc > 1))
    {
    interp->result = " Usage: OpenPLC";
    return TCL_ERROR;
    }

  rv=plcOpen();
  if ( rv == 0 ) 
    {
    plcAperto=1;
    interp->result = "1";
    } 
  else
    {
    interp->result = "0";
    }

  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ClosePLCCmd --
 * Chiude la connessione con il PLC
 ***********************************************************************************
 */
int ClosePLCCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  if ((argc > 1))
    {
    interp->result = " Usage: ClosePLC";
    return TCL_ERROR;
    }

  plcClose();

  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ReadSegnaleScambioCmd --
 * legge il segnale di scambio
 ***********************************************************************************
 */
int ReadSegnaleScambioCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  char    NomeTemporaneo[MAXNAMLEN];
  char    *Posizioneparentesi;
  struct  plcvar_t StrutturaVariabile;

  if (plcAperto == 0 )
    {
    interp->result = "There is no attachment with plc";
    return TCL_ERROR;
    }

  if (argc != 2)
    {
    interp->result = "wrong # args: should be ReadSegnaleScambio varName";
    return TCL_ERROR;
    }

   /* Trasformo le scritture di vettori fatte con [x,x]
   nella convenzione adottata da isaker */
   strcpy(NomeTemporaneo,argv[1]);
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

  if ( dbGetVar(NomeTemporaneo,&StrutturaVariabile))
    {
    switch (StrutturaVariabile.type) 
      {
      case ISAVAR_T_BOOL:
                    sprintf(interp->result,"%d",*(char *)StrutturaVariabile.pval);
                    break;

      case ISAVAR_T_ANA_I:

      case ISAVAR_T_TIMER:
                    sprintf(interp->result,"%ld",*(long *)StrutturaVariabile.pval);
                    break;

      case ISAVAR_T_ANA_F:
                    sprintf(interp->result,"%g",*(float *)StrutturaVariabile.pval);
                    break;
      }
    }
  else 
    {
    sprintf(interp->result,"%s %s", argv[1], "doesn't exist.");
    return TCL_ERROR;
    }

  return TCL_OK;
}

/* 
 ***********************************************************************************
 * WriteSegnaleScambioCmd --
 * scrive il segnale di scambio
 ***********************************************************************************
 */
int WriteSegnaleScambioCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  char	  NomeTemporaneo[MAXNAMLEN];
  char	  *PosizioneParentesiAperta;
  char	  *PosizioneParentesiChiusa;
  char	  *PosizioneVirgola;
  int	  col=-1;
  int	  riga=-1;
  struct  qplc_shv_t *caratteristiche;
  char 	  AppoggioChar;
  int     AppoggioInt;
  float   AppoggioFloat;

  if (plcAperto == 0 )
    {
    interp->result = "There is no attachment with plc";
    return TCL_ERROR;
    }

  if (argc != 4)
    {
    interp->result = "wrong # args: should be WriteSegnaleScambio varName ?Value? ?Type(B/I/F)?";
    return TCL_ERROR;
    }

   /* Trasformo le scritture di vettori fatte con [x,x]
   nella convenzione adottata da isaker */
   strcpy(NomeTemporaneo,argv[1]);
   if ( (PosizioneParentesiChiusa=strchr(NomeTemporaneo,(int)']')) != NULL )
     {
     PosizioneParentesiChiusa[0]='\0';
     if ( (PosizioneVirgola=strchr(NomeTemporaneo,(int)',')) != NULL )
       {
       PosizioneVirgola[0]='\0';
       PosizioneVirgola++;
       }
     if ( (PosizioneParentesiAperta=strchr(NomeTemporaneo,(int)'[')) != NULL )
       {
       PosizioneParentesiAperta[0]='\0';
       PosizioneParentesiAperta++;
       }
     if (PosizioneVirgola!=NULL)
       {
       riga=atoi(PosizioneParentesiAperta);
       col=atoi(PosizioneVirgola);
       }
     else
       col=atoi(PosizioneParentesiAperta);
     } 

  if (dbGetQplcVarDescr(NomeTemporaneo,&caratteristiche) != NULL) 
    {
    if(col==-1 && riga == -1 )
      {
      if(strcmp(argv[3],"B")==0)
        {
        AppoggioChar=(char)atoi(argv[2]);
        plcSetBScal(caratteristiche -> key,AppoggioChar);
        }
      else if(strcmp(argv[3],"I")==0)
        {
        AppoggioInt=atoi(argv[2]);
        plcSetLScal(caratteristiche -> key,AppoggioInt);
        }
      else if(strcmp(argv[3],"F")==0)
	{
        AppoggioFloat=atof(argv[2]);
        plcSetFScal(caratteristiche -> key,AppoggioFloat);
	}
      else
        sprintf(interp->result,"%s %s", argv[3], "Type unknown");
      }
    else if (col!=-1 && riga == -1)
      {
      if(strcmp(argv[3],"B")==0)
        {
        AppoggioChar=atoi(argv[2]);
        plcSetBVect(caratteristiche -> key,col,AppoggioChar);
        }
      else if(strcmp(argv[3],"I")==0)
        {
        AppoggioInt=atoi(argv[2]);
        plcSetLVect(caratteristiche -> key,col,AppoggioInt);
        }
      else if(strcmp(argv[3],"F")==0)
	{
        AppoggioFloat=atof(argv[2]);
        plcSetFVect(caratteristiche -> key,col,AppoggioFloat);
	}
      else
        sprintf(interp->result,"%s %s", argv[3], "Type unknown");
      }
    else if (col!=-1 && riga != -1)
      {
      if(strcmp(argv[3],"B")==0)
        {
        AppoggioChar=atoi(argv[2]);
        plcSetBArr(caratteristiche -> key,riga,col,AppoggioChar);
        }
      else if(strcmp(argv[3],"I")==0)
        {
        AppoggioInt=atoi(argv[2]);
        plcSetLArr(caratteristiche -> key,riga,col,AppoggioInt);
        }
      else if(strcmp(argv[3],"F")==0)
	{
        AppoggioFloat=atof(argv[2]);
        plcSetFArr(caratteristiche -> key,riga,col,AppoggioFloat);
	}
      else
        sprintf(interp->result,"%s %s", argv[3], "Type unknown");
      }
    }
  else 
    {
    sprintf(interp->result,"%s %s", argv[1], "doesn't exist.");
    return TCL_ERROR;
    }
  return TCL_OK;
} 
#ifdef DEBUG_PLC
/* 
 ***********************************************************************************
 * CalcolaIndirizzoCmd --
 * Implementa il calcolo dell'indirizzo assoluto del modulo caricato
 ***********************************************************************************
 */

int CalcolaIndirizzoCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int 	  rv=0;
  struct  modprop_t   prop;

  if ((argc > 2) || (argc == 1))
    {
    interp->result = " Usage: indirizzo_assoluto U_CODE ";
    return TCL_ERROR;
    }

  if (strcmp( argv[1], "U_CODE") != 0)
    {
    interp->result = " Usage: indirizzo_assoluto U_CODE ";
    return TCL_ERROR;
    }

  rv = plcModProp(&prop,"U_CODE");
  if (rv) 
    {
    interp->result = "1";
    return TCL_OK;
    }

  if (prop.file[0])
    {
    sprintf (interp->result, "0x%08lx %s",prop.text,prop.file);
    }  
  return TCL_OK;
}

/* 
 *******************************************************************************
 * CaricaUserCmd --
 * carica il modulo user.oo
 *******************************************************************************
 */
int CaricaUserCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
  int rv=0;
  int contasecondi=0;
  if (argc > 2)
    {
    interp->result = " Usage: CaricaUser FileToLoad ";
    return TCL_ERROR;
    }
  if(plcHalt())
    {
    while ( plcStatus() == 1 && contasecondi <=10 ) 
      {
      sleep(1);
      contasecondi++;
      }
    /* caricamento plc      */
    rv=plcLoadProg(argv[1]);
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
      interp->result = "1";
      }

    sleep(2);

    if(!plcGo())
      {
      interp->result = "1";
      }
    }
  else 
    {
    interp->result = "1";
    }
  interp->result = "0";
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * SetStartCycleCmd --
 * Con questo comando memorizzo il ciclo plc di partenza
 * per la sessione di debug corrente
 ***********************************************************************************
 */

int SetStartCycleCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  if (argc == 1)
    {
    interp->result = " Usage: SetStartCycle <StartCycle>";
    return TCL_ERROR;
    }
  StartCycle = atoi(argv[1]);
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * cm_showbrk --
 ***********************************************************************************
 */
static int cm_showbrk(Tcl_Interp *interp)
{
brk_history *hi;
brk_event_list *el;
brk_event_list *elnew;
brk_event *ev;
char *p;
unsigned int n;
int id,size;
int idOld=-1;
struct brk *brp;
char temp_buffer[100];
int flags = 0;
int nev;
int UltimoEvento=0;
int PrimoEvento=0;

  hi = plcLoadDebugHistory(&elnew);

  if (!hi || !elnew)
    return 1;

  nev = hi->bh_list_count - 1;
  PrimoEvento=nev;
  /* Percorro la storia degli eventi fino al ciclo 
     plc memorizzato */
  if ( StartCycle != -1 ) 
    {
    UltimoEvento=hi->bh_last_cycle-StartCycle;
    if ( UltimoEvento < 0)
      return 1;
    else
      UltimoEvento=nev-UltimoEvento;
    StartCycle=hi->bh_last_cycle;
    }
  while ( (ev = plcGetDebugEvent())
       || ((--nev >= UltimoEvento) && plcIndexDebugEventList(nev) &&
       (ev = plcGetDebugEvent())))
    {
    id = ev -> be_id;
    if (id >= MAX_WATCH) 
      return 1;
    else if ((flags & (1 << id)) == 0)
      {
      if (nev==PrimoEvento)
        sprintf(temp_buffer,"Lista%d", id); 
      else if (nev < PrimoEvento)
        sprintf(temp_buffer,"Lista%d_o", id); 
      Tcl_AppendElement(interp,temp_buffer);
      idOld = id;
      flags |= (1 << id);
      }
    else
      continue;
    for (p = ev -> be_data, n = ev -> be_size - 3, brp = bp[id], size=0; n && brp; n -= size, brp = brp -> next) 
      {
      if (brp -> bp.operand.scale == 0) 
        {
        if (brp -> bp.operand.r1) 
          {
          sprintf(temp_buffer,"0x%08lx", *(unsigned long *)p);
          Tcl_AppendElement(interp,temp_buffer);
          size = 4;
          p += 4;
          }
        if (brp -> bp.operand.r2) 
          {
          sprintf(temp_buffer,"0x%08lx", *(unsigned long *)p);
          Tcl_AppendElement(interp,temp_buffer);
          size = 4;
          p += 4;
          }
        }
      else 
        {
        switch (brp -> bp.operand.size)
          {
	  case 1:
		sprintf(temp_buffer,"0x%02x", *(unsigned char*)p);
                Tcl_AppendElement(interp,temp_buffer);
		break;
	  case 2:
		sprintf(temp_buffer,"0x%04x", *(unsigned short*)p);
                Tcl_AppendElement(interp,temp_buffer);
		break;
	  case 4:
		sprintf(temp_buffer,"0x%08lx", *(unsigned long*)p);
                Tcl_AppendElement(interp,temp_buffer);
		break;
	  default:
		sprintf(temp_buffer,"0x%08lx%08lx", *(unsigned long*)(p+4), *(unsigned long*)p);
                Tcl_AppendElement(interp,temp_buffer);
	 	break;
	  }
	  size = brp -> bp.operand.size;
	  p += brp -> bp.operand.size;
	}
      if (brp -> bp.operand.ar) 
        {
        sprintf(temp_buffer,"0x%08lx", *(unsigned long *)p);
        Tcl_AppendElement(interp,temp_buffer);
        size = 4;
        p += 4;
	}
      }
    }
return 0;
}

/* 
 ***********************************************************************************
 * cm_addbrk --
 ***********************************************************************************
 */
static int cm_addbrk(int id,Tcl_Interp *interp,char *argv[])
{
watch_t w;
unsigned long v;
struct brk *b,*q,*r;
int rv;

w.addr = (char *) 0;
w.operand.r1 = 0;
w.operand.r2 = 0;
w.operand.scale = 0;
w.operand.size = 0;
w.operand.off = 0;
w.operand.ar = 0;

  if (sscanf(argv[3],"0x%lx",&v) != 1 && sscanf(argv[3],"%lu",&v) != 1)
    return 0;
  w.addr = (unsigned char *) v;
  if (sscanf(argv[4],"0x%lx",&v) != 1 && sscanf(argv[4],"%lu",&v) != 1)
    return 0;
  w.operand.r1 = v;
  if (sscanf(argv[5],"0x%lx",&v) != 1 && sscanf(argv[5],"%lu",&v) != 1)
    return 0;
  w.operand.r2 = v;
  if (sscanf(argv[6],"0x%lx",&v) != 1 && sscanf(argv[6],"%lu",&v) != 1)
    return 0;
  w.operand.scale = v;
  if (sscanf(argv[7],"0x%lx",&v) != 1 && sscanf(argv[7],"%lu",&v) != 1)
    return 0;
  w.operand.size = v;
  if (sscanf(argv[8],"0x%lx",&v) != 1 && sscanf(argv[8],"%lu",&v) != 1)
    return 0;
  w.operand.off = v;
  if (sscanf(argv[9],"0x%lx",&v) != 1 && sscanf(argv[9],"%lu",&v) != 1)
    return 0;
  w.operand.ar = v;

  if (!(_brk_tst_bit(watch_id,id))) 
    return 1;
  b = br_free;
  if (! b)
    {
    return 1;
    }
  else 
    {
    br_free = br_free -> next;
    b -> bp = w;
    for (q = bp[id], r = (struct brk *) 0; q; r = q, q = q -> next)
      ;
    if (! r)
      bp[id] = b;
    else
      r -> next = b;
    b -> next = (struct brk *) 0;
    rv = plcAddBreak(id,&w);
    if (rv)
      {
      return 1;
      }
    }
  return 0;
}

/* 
 ***********************************************************************************
 * cm_delwatch --
 ***********************************************************************************
 */
static void cm_delwatch(int id)
{
struct brk *p,*q;

  for (p = bp[id]; p; p = q) 
    {
    q = p -> next;
    p -> next = br_free;
    br_free = p;
    }
  _brk_res_bit(watch_id,id) ;
  bp[id] = (struct brk *) 0;
}

/* 
 ***********************************************************************************
 * cm_resetwatches --
 ***********************************************************************************
 */
static void cm_resetwatches(void)
{
 int i;

  i = sizeof(br)/sizeof(br[0]);
  br_free = &br[i - 1];
  while (--i)
    br[i].next = &br[i - 1];
  br[0].next = (struct brk *) 0;
  _brk_clr_bittab(watch_id);
  for (i = 0; i < MAX_WATCH; ++i) 
    {
    bp[i] = (struct brk *) 0;
    }
}

/* 
 ***********************************************************************************
 * cm_wparam --
 ***********************************************************************************
 */
static int cm_wparam(watch_descr_t *param,int argc, char *argv[])
{
unsigned long v;
int indice=0;

param -> act = WATCH_A_INC | WATCH_A_CYC | WATCH_A_LOG;
param -> cond = WATCH_C_TRUE;
param -> apar = 0;
param -> condact = 0;
param -> opc = 0;
param -> opa1 = 0;
param -> opa2 = 0;

  for (indice=3; indice<argc; indice++)
    {
    /* Parametri che agiscono su act */
    if (strcmp(argv[indice],"+set") == 0)
      param -> act |= WATCH_A_SET;
    else if (strcmp(argv[indice],"?set") == 0) 
      {
      param -> act |= WATCH_A_SET;
      param -> condact |= WATCH_A_SET; 
      }
    else if (strcmp(argv[indice],"-set") == 0)
      param -> act &= ~WATCH_A_SET;
    else if (strcmp(argv[indice],"+reset") == 0)
      param -> act |= WATCH_A_RESET;
    else if (strcmp(argv[indice],"?reset") == 0) 
      {
      param -> act |= WATCH_A_RESET;
      param -> condact |= WATCH_A_RESET; 
      }
    else if (strcmp(argv[indice],"-reset") == 0)
      param -> act &= ~WATCH_A_RESET;
    else if (strcmp(argv[indice],"+tooggle") == 0)
      param -> act |= WATCH_A_TOOGGLE;
    else if (strcmp(argv[indice],"?tooggle") == 0) 
      {
      param -> act |= WATCH_A_TOOGGLE;
      param -> condact |= WATCH_A_TOOGGLE; 
      }
    else if (strcmp(argv[indice],"-tooggle") == 0)
      param -> act &= ~WATCH_A_TOOGGLE;
    else if (strcmp(argv[indice],"+inc") == 0)
      param -> act |= WATCH_A_INC;
    else if (strcmp(argv[indice],"?inc") == 0) 
      {
      param -> act |= WATCH_A_INC;
      param -> condact |= WATCH_A_INC; 
      }
    else if (strcmp(argv[indice],"-inc") == 0)
      param -> act &= ~WATCH_A_INC;
    else if (strcmp(argv[indice],"+cyc") == 0)
      param -> act |= WATCH_A_CYC;
    else if (strcmp(argv[indice],"?cyc") == 0) 
      {
      param -> act |= WATCH_A_CYC;
      param -> condact |= WATCH_A_CYC; 
      }
    else if (strcmp(argv[indice],"-cyc") == 0)
      param -> act &= ~WATCH_A_CYC;
    else if (strcmp(argv[indice],"+logfl") == 0)
      param -> act |= WATCH_A_LOGFL;
    else if (strcmp(argv[indice],"?logfl") == 0) 
      {
      param -> act |= WATCH_A_LOGFL;
      param -> condact |= WATCH_A_LOGFL; 
      }
    else if (strcmp(argv[indice],"-logfl") == 0)
      param -> act &= ~WATCH_A_LOGFL;
    else if (strcmp(argv[indice],"+log") == 0)
      param -> act |= WATCH_A_LOG;
    else if (strcmp(argv[indice],"?log") == 0) 
      {
      param -> act |= WATCH_A_LOG;
      param -> condact |= WATCH_A_LOG; 
      }
    else if (strcmp(argv[indice],"-log") == 0)
      param -> act &= ~WATCH_A_LOG;
    else if (strcmp(argv[indice],"+brk") == 0)
      param -> act |= WATCH_A_BRK;
    else if (strcmp(argv[indice],"?brk") == 0) 
      {
      param -> act |= WATCH_A_BRK;
      param -> condact |= WATCH_A_BRK; 
      }
    else if (strcmp(argv[indice],"-brk") == 0)
      param -> act &= ~WATCH_A_BRK;
    else if (strcmp(argv[indice],"false") == 0)		/* Parametri che agiscono su cond */
      param -> cond = WATCH_C_FALSE;
    else if (strcmp(argv[indice],"true") == 0)
      param -> cond = WATCH_C_TRUE;
    else if (strcmp(argv[indice],"and") == 0)
      param -> cond = WATCH_C_AND;
    else if (strcmp(argv[indice],"or") == 0)
      param -> cond = WATCH_C_OR;
    else if (strcmp(argv[indice],"nor") == 0)
      param -> cond = WATCH_C_NOR;
    else if (strcmp(argv[indice],"tgt") == 0)
      param -> cond = WATCH_C_TGT;
    else if (strcmp(argv[indice],"tle") == 0)
      param -> cond = WATCH_C_TLE;
    else if (strcmp(argv[indice],"teq") == 0)
      param -> cond = WATCH_C_TEQ;
    else if (strcmp(argv[indice],"cgt") == 0)
      param -> cond = WATCH_C_CGT;
    else if (strcmp(argv[indice],"cle") == 0)
      param -> cond = WATCH_C_CLE;
    else if (strcmp(argv[indice],"ceq") == 0)
      param -> cond = WATCH_C_CEQ;
    else if (strcmp(argv[indice],"rz") == 0)
      param -> cond = WATCH_C_RZ;
    else if (strcmp(argv[indice],"nrz") == 0)
      param -> cond = WATCH_C_NRZ;
    else if (strcmp(argv[indice],"mz") == 0)
      param -> cond = WATCH_C_MZ;
    else if (strcmp(argv[indice],"nmz") == 0)
      param -> cond = WATCH_C_NMZ;
    else if (strncmp(argv[indice],"apar=",5) == 0)    /* Settaggio di apar */ 
      {
      if (sscanf(argv[indice],"apar=0x%lx",&v) != 1 && sscanf(argv[indice],"apar=%lu",&v) != 1)
	return 0;
      param -> apar = v;
      }
    else if (strncmp(argv[indice],"opc=",4) == 0)    /* Settaggio di opc */ 
      {
      if (sscanf(argv[indice],"opc=0x%lx",&v) != 1 && sscanf(argv[indice],"opc=%lu",&v) != 1)
	return 0;
      param -> opc = v;
      }
    else if (strncmp(argv[indice],"opa1=",5) == 0)   /* Settaggio di opa1 */ 
      {
      if (sscanf(argv[indice],"opa1=0x%lx",&v) != 1 && sscanf(argv[indice],"opa1=%lu",&v) != 1)
	return 0;
      param -> opa1 = v;
      }
    else if (strncmp(argv[indice],"opa2=",5) == 0)   /* Settaggio di opa2 */ 
      {
      if (sscanf(argv[indice],"opa2=0x%lx",&v) != 1 && sscanf(argv[indice],"opa2=%lu",&v) != 1)
	return 0;
      param -> opa2 = v;
      }
   }
return 1;
}

/* 
 ***********************************************************************************
 * WatchCmd --
 * Con questo eseguo tutte le funzioni di debug
 ***********************************************************************************
 */

int WatchCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int id,wid,rv;
  if (argc == 1)
    {
    interp->result = " Usage: watch {reset,create,delete,add,enable,disable,show} <wid>";
    return TCL_ERROR;
    }

  if (strcmp(argv[1],"reset")==0)
    {
    if (argc > 2 )
      {
      interp->result = " Usage: watch reset";
      return TCL_ERROR;
      }
    rv=plcClearAllWatches();
    if (rv != 0)
      {
      interp->result = "1";
      return TCL_OK;
      }
    cm_resetwatches();
    interp->result = "0";
    }
  else if (strcmp(argv[1],"create")==0)
    { 
    id = atoi(argv[2]);
    if (! cm_wparam(&w_param,argc,argv))
      {
      interp->result = "-1";
      return TCL_OK;
      }
    if (_brk_tst_bit(watch_id,id)) 
      {
      interp->result = "-1";
      return TCL_OK;
      }
    if ( (rv = plcCreateWatchList(&w_param,&wid)) )
      {
      interp->result = "-1";
      return TCL_OK;
      }
    else 
      {
      _brk_set_bit(watch_id,wid);
      bp[wid] = (struct brk *) 0;
      sprintf (interp->result, "%d",wid);
      }
    }
  else if (strcmp(argv[1],"delete") == 0) 
    {
    if (argc != 3 )
      {
      interp->result = " Usage: watch delete uid";
      return TCL_ERROR;
      }
    id = atoi(argv[2]);
    if (!(_brk_tst_bit(watch_id,id))) 
      {
      interp->result = "1";
      return TCL_OK;
      }
    else 
      {
      if ( (rv = plcDeleteWatchList(id)) )
        {
        interp->result = "1";
        return TCL_OK;
        }
      else
        cm_delwatch(id);
      }
    interp->result = "0";
    }
  else if (strcmp(argv[1],"add") == 0) 
    {
    if (argc != 10 )
      {
      interp->result = " Usage: watch add uid addr r1 r2 scale size off ar";
      return TCL_ERROR;
      }
    id = atoi(argv[2]);
    if (cm_addbrk(id,interp,argv))
      interp->result = "1";
    else 
      interp->result = "0";
    }
  else if (strcmp(argv[1],"enable") == 0) 
    {
    if (argc != 3 )
      {
      interp->result = " Usage: watch enable uid";
      return TCL_ERROR;
      }
    id = atoi(argv[2]);
    if (!(_brk_tst_bit(watch_id,id))) 
      {
      interp->result = "1";
      return TCL_OK;
      }
    else
      {
      if ( (rv = plcEnableWatchList(id)) )
        {
        interp->result = "1";
        return TCL_OK;
        }
      }
      interp->result = "0";
    }
  else if (strcmp(argv[1],"disable") == 0) 
    {
    if (argc != 3 )
      {
      interp->result = " Usage: watch disable uid";
      return TCL_ERROR;
      }
    id = atoi(argv[2]);
    if (!(_brk_tst_bit(watch_id,id))) 
      {
        interp->result = "1";
        return TCL_OK;
      }
    else
      {
      if ( (rv = plcDisableWatchList(id)) )
        {
        interp->result = "1";
        return TCL_OK;
        }
      }
      interp->result = "0";
    }
  else if (strcmp(argv[1],"show") == 0) 
   {
   if (argc > 2 )
    {
    interp->result = " Usage: watch show";
    return TCL_ERROR;
    }
   if (cm_showbrk(interp))
     interp->result = "1";
   }
  return TCL_OK;
}

/* 
 *******************************************************************************
 * LeggiCRCCmd --
 * Legge il CRC dal modulo
 *******************************************************************************
 */
int LeggiCRCCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  char    temp[4];
  int 	  rv;
  if (argc > 2)
    {
    interp->result = " Usage: LeggiCRC address ";
    return TCL_ERROR;
    }

  rv=plcMemCopy(temp,(char *)atoi(argv[1]),4);
  if (rv == 0)
    {
    sprintf(interp->result,"%ld",*(unsigned long int *)temp);
    return TCL_OK;
    }
  else 
    {
    interp->result = "1";
    return TCL_OK;
    }
}
#endif

#ifdef XNC

/* 
 ***********************************************************************************
 * OpenPLCNoBlockCmd --
 * Apre la connessione con il PLC non bloccante
 ***********************************************************************************
 */
int OpenPLCNoBlockCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv;

  if ((argc > 1))
    {
    interp->result = " Usage: OpenPLCNoBlock";
    return TCL_ERROR;
    }

  rv=plcTryOpen();
  if ( rv == 0 ) 
    {
    plcAperto=1;
    interp->result = "1";
    } 
  else
    {
    interp->result = "0";
    }
  return TCL_OK;
}


/* 
 ***********************************************************************************
 * AttachServerNoBlockCmd --
 * Apre la connessione con il Server Dati Macchina non bloccante
 ***********************************************************************************
 */
int AttachServerNoBlockCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

  if ((argc > 1))
    {
    interp->result = " Usage: AttachServerNoBlock";
    return TCL_ERROR;
    }

  ptRamDM=GetServerDmRam(ID_SDM_CN);
  if( ptRamDM==NULL )
    {
    interp->result = "0";
    }
  else 
    {
    interp->result = "1";
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * AttachServerCmd --
 * Apre la connessione con il Server Dati Macchina bloccante
 ***********************************************************************************
 */
int AttachServerCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

  if ((argc > 1))
    {
    interp->result = " Usage: AttachServer";
    return TCL_ERROR;
    }

  ptRamDM=AttachServerDm(ID_SDM_CN);
  if( ptRamDM==NULL )
    {
    interp->result = "0";
    }
  else 
    {
    interp->result = "1";
    }

  return TCL_OK;
}

/* 
 ***********************************************************************************
 * DetachServerCmd --
 * Chiude la connessione con il Server Dati Macchina
 ***********************************************************************************
 */
int DetachServerCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

  if ((argc > 1))
    {
    interp->result = " Usage: DetachServer";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if (DetachServerDm(ptRamDM) != 0)
      {
      interp->result = "Detachment with Server failed";
      return TCL_ERROR;
      }
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * MD_STATUSCmd --
 * Ritorna lo stato del server
 ***********************************************************************************
 */
int MD_STATUSCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv;
  if ((argc > 1))
    {
    interp->result = " Usage: MD_STATUS";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    rv=GetStatoDm(ptRamDM);
    sprintf(interp->result,"%d",rv);
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Read_AxesNumber --
 * Legge il numero degli assi
 ***********************************************************************************
 */
int Read_AxesNumberCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv;
  if ((argc != 1))
    {
    interp->result = " wrong # args: should be Read_AxesNumber";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    rv=GetGenericItem(ptRamDM,ES_PAR_ASSI);
    if (rv < 0)
      {
      sprintf(interp->result, "Read machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    else
      {
      sprintf(interp->result, "%d",rv); 
      }
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * AxesServerInitialize --
 * Inizializzazione server assi
 ***********************************************************************************
 */
int AxesServerInitializeCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv;
  if ((argc != 1))
    {
    interp->result = " wrong # args: should be AxesServerInitialize";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
      shmem.shdatima=ptRamDM;
      if ((rv=init_col(&shmem))) 
        {
        interp->result = "Axes server initialization failed";
        return TCL_ERROR;
        } 
      ServerAssi=1; 
    } else {
      interp->result = "There is no attachment with Machine Data Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ReadAxesValue --
 * Lettura della quota dell' asse specificato
 ***********************************************************************************
 */
int ReadAxesValueCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  float quota;
  if ((argc != 2))
    {
    interp->result = " wrong # args: should be ReadAxesValue NumAxes";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
      if (ServerAssi == 1)
        {
        read_parm(&shmem,SAX_QUOTA_REALE,atoi(argv[1]),-1,-1,(int *)&quota);
        sprintf(interp->result,"%3.3f",quota);
        }
      else 
      interp->result = "There is no attachment with Axes Server";
    } else {
      interp->result = "There is no attachment with Machine Data Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Read_mdata --
 * Legge (read_mdata) i dati macchina di un qualsiasi
 * oggetto.
 ***********************************************************************************
 */
int Read_mdataCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  DATOTIPO DatoTipo;
  int rv;
  if ((argc != 5))
    {
    interp->result = " wrong # args: should be Read_mdata Table NumCenter Object DataName";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    rv=read_mdata(ptRamDM,argv[1], atoi(argv[2]),argv[3],argv[4],&DatoTipo);
    if (rv < 0)
      {
      sprintf(interp->result, "Read machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    else
      {
      switch (DatoTipo.nTipoDato)
	{
	case ID_STRING:
		sprintf(interp->result, "%s",DatoTipo.Dato.szString);
                break;

	case ID_LONG:
		sprintf(interp->result, "%ld",DatoTipo.Dato.lLong); 
                break;

	case ID_ULONG:
		sprintf(interp->result, "%lu",DatoTipo.Dato.ulLong); 
                break;

	case ID_INT:
		sprintf(interp->result, "%d",DatoTipo.Dato.nInt); 
                break;

	case ID_UNSIGNED:
		sprintf(interp->result, "%ud",DatoTipo.Dato.uUnsigned); 
                break;

	case ID_FLOAT:
		sprintf(interp->result, "%g",DatoTipo.Dato.fFloat); 
                break;

	case ID_DOUBLE:
		sprintf(interp->result, "%g",DatoTipo.Dato.dDouble); 
                break;
        }
      }
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Write_mdataWCmd --
 * Scrive (Write_mdata) i dati macchina di una qualsiasi
 * tabella avendo cura di avvisare gli altri processi.
 ***********************************************************************************
 */
int Write_mdataWCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  DATOTIPO DatoTipo;
  int rv;

  if ((argc != 7))
    {
    interp->result = " wrong # args: should be Write_mdataW Table NumCenter Object DataName Type Value";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if (!strcmp(argv[5],"STRING")) 
	{
        DatoTipo.nTipoDato=ID_STRING;
        memset(DatoTipo.Dato.szString,0,sizeof(DatoTipo.Dato.szString));
        strncpy(DatoTipo.Dato.szString,argv[6],sizeof(DatoTipo.Dato.szString));  
        }
    else if (!strcmp(argv[5],"FLOAT")) 
	{
        DatoTipo.nTipoDato=ID_FLOAT;
        DatoTipo.Dato.fFloat=atof(argv[6]);
        }
    else 
        {
        sprintf(interp->result, "Type = (STRING or FLOAT)");
        return TCL_ERROR;
        }
    SetStatoDm(ptRamDM);
    rv=write_mdata(ptRamDM,argv[1], atoi(argv[2]),argv[3],argv[4],&DatoTipo);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    SetStartSave(ptRamDM);
    rv=SaveGenericData(ptRamDM,argv[1],atoi(argv[3]),argv[4]);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    SetEndSave(ptRamDM);
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Write_mdataCmd --
 * Scrive (Write_mdata) i dati macchina di una qualsiasi
 * tabella non avendo cura di avvisare gli altri processi.
 ***********************************************************************************
 */
int Write_mdataCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  DATOTIPO DatoTipo;
  int rv;

  if ((argc != 7))
    {
    interp->result = " wrong # args: should be Write_mdata Table NumCenter Object DataName Type Value";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if (!strcmp(argv[5],"STRING")) 
	{
        DatoTipo.nTipoDato=ID_STRING;
        memset(DatoTipo.Dato.szString,0,sizeof(DatoTipo.Dato.szString));
        strncpy(DatoTipo.Dato.szString,argv[6],sizeof(DatoTipo.Dato.szString));  
        }
    else if (!strcmp(argv[5],"FLOAT")) 
	{
        DatoTipo.nTipoDato=ID_FLOAT;
        DatoTipo.Dato.fFloat=atof(argv[6]);
        }
    else 
        {
        sprintf(interp->result, "Type = (STRING or FLOAT)");
        return TCL_ERROR;
        }
    rv=write_mdata(ptRamDM,argv[1], atoi(argv[2]),argv[3],argv[4],&DatoTipo);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    rv=SaveGenericData(ptRamDM,argv[1],atoi(argv[3]),argv[4]);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Write_axesdataWCmd --
 * Scrive (Write_axesdataW) i dati macchina degli assi
 * avendo cura di avvisare gli altri processi
 ***********************************************************************************
 */
int Write_axesdataWCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  DATOTIPO DatoTipo;
  int rv;
  int NumAsse;

  if ((argc != 7))
    {
    interp->result = " wrong # args: should be Write_axesdataW Table NumCenter Object DataName Type Value";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if (!strcmp(argv[5],"STRING")) 
	{
        DatoTipo.nTipoDato=ID_STRING;
        memset(DatoTipo.Dato.szString,0,sizeof(DatoTipo.Dato.szString));
        strncpy(DatoTipo.Dato.szString,argv[6],sizeof(DatoTipo.Dato.szString));  
        }
    else if (!strcmp(argv[5],"FLOAT")) 
	{
        DatoTipo.nTipoDato=ID_FLOAT;
        DatoTipo.Dato.fFloat=atof(argv[6]);
        }
    else 
        {
        sprintf(interp->result, "Type = (STRING or FLOAT)");
        return TCL_ERROR;
        }
    SetStatoDm(ptRamDM);
    rv=write_mdata(ptRamDM,argv[1], atoi(argv[2]),argv[3],argv[4],&DatoTipo);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    if((NumAsse=GetAxesNumByName(ptRamDM,atoi(argv[2]),argv[3]))<0)
      {
      sprintf(interp->result, "Get axes number failed with error %d",rv);
      return TCL_ERROR;
      }
    SetStartSave(ptRamDM);
    rv=SaveAxesData(ptRamDM,atoi(argv[2]),NumAsse,argv[4]);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    SetEndSave(ptRamDM);
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * Write_axesdataCmd --
 * Scrive (Write_axesdata) i dati macchina degli assi
 * non avendo cura di avvisare gli altri processi
 ***********************************************************************************
 */
int Write_axesdataCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  DATOTIPO DatoTipo;
  int rv;
  int NumAsse;

  if ((argc != 7))
    {
    interp->result = " wrong # args: should be Write_axesdata Table NumCenter Object DataName Type Value";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if (!strcmp(argv[5],"STRING")) 
	{
        DatoTipo.nTipoDato=ID_STRING;
        memset(DatoTipo.Dato.szString,0,sizeof(DatoTipo.Dato.szString));
        strncpy(DatoTipo.Dato.szString,argv[6],sizeof(DatoTipo.Dato.szString));  
        }
    else if (!strcmp(argv[5],"FLOAT")) 
	{
        DatoTipo.nTipoDato=ID_FLOAT;
        DatoTipo.Dato.fFloat=atof(argv[6]);
        }
    else 
        {
        sprintf(interp->result, "Type = (STRING or FLOAT)");
        return TCL_ERROR;
        }
    rv=write_mdata(ptRamDM,argv[1], atoi(argv[2]),argv[3],argv[4],&DatoTipo);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    if((NumAsse=GetAxesNumByName(ptRamDM,atoi(argv[2]),argv[3]))<0)
      {
      sprintf(interp->result, "Get axes number failed with error %d",rv);
      return TCL_ERROR;
      }
    rv=SaveAxesData(ptRamDM,atoi(argv[2]),NumAsse,argv[4]);
    if (rv < 0)
      {
      sprintf(interp->result, "Write machine data failed with error %d",rv);
      return TCL_ERROR;
      }
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * RescanDirCmd --
 * Segnale al File Selection del CN la necessita' di riscannare il direttorio 
 * dei programmi.
 ***********************************************************************************
 */
int RescanDirCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  float vf;
  if (argc != 1)
    {
    interp->result = " wrong # args: should be RescanDir";
    return TCL_ERROR;
    }

  if ( ptRamDM != NULL )
    {
    if(GetGenericData(ptRamDM,ES_PAR_FLAG,OFFSET_EE,ES_DIRSCAN,&vf)>=0)
      {
      vf++;
      SetGenericData(ptRamDM,ES_PAR_FLAG,OFFSET_EE,ES_DIRSCAN,&vf);
      }
    } else {
      interp->result = "There is no attachment with Server";
      return TCL_ERROR;
    }
  return TCL_OK;
}

/*
 *  ****************************************************************************
 *
 *  Funzione macchinaIsStart
 *
 *  ****************************************************************************
 */
Boolean	macchinaIsStart(void)
{
Boolean dato;
float	vf;

dato=False;
if(GetGenericData(ptRamDM,ES_PAR_FLAG,OFFSET_EE,ES_START,&vf)>=0)
	dato=(vf) ? True : False;
return(dato);
}

/*
 *  ****************************************************************************
 *
 *  Funzione getNumeroCentri
 *
 *  ****************************************************************************
 */
int	getNumeroCentri(void)
{
int dato;
float	vf;

dato=1;
if(GetMaccInfo(ptRamDM,ES_NUMCEN,&vf)>=0)
	dato=(int)vf;
return(dato);
} /* Fine getNumeroCentri */

/*
 *  ****************************************************************************
 *
 *  Funzione programmaIsStart
 *
 *  ****************************************************************************
 */
Boolean programmaIsStart(char *NomeFile) 
{
int 	CountCen,CountDat,TotItem,TotCentri;
char	LocalName[LUNGH_FILE];
char	NomeDato[50];

TotCentri=getNumeroCentri();
TotItem=GetGenericConfItem(ptRamDM,ES_PAR_PRGSTART);
for(CountCen=0;CountCen<TotCentri;CountCen++)
	{
	for(CountDat=0;CountDat<TotItem;CountDat++)
		{
		sprintf(NomeDato,"%s%d",ES_LIV,CountDat);
		GetGenericData(	ptRamDM, ES_PAR_PRGSTART, OFFSET_EE+CountCen, NomeDato, LocalName);
        if(strstr(LocalName,NomeFile)!=NULL)
         	return(True);
		}
	}
return(False);
} /* Fine programmaIsStart */

/*
 *  ****************************************************************************
 *
 *  Funzione SaveFileIsPossible
 *
 *  ****************************************************************************
 */
int  SaveFileIsPossibleCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
  if ((argc != 2))
    {
    interp->result = " wrong # args: should be SaveFileIsPossible FileName";
    return TCL_ERROR;
    }

  if (macchinaIsStart() == True)
    {
    if (programmaIsStart(argv[1]) == True)
      {
      interp->result = "0";
      return TCL_OK;
      }
    }
  interp->result = "1";
  return TCL_OK;
} /* Fine SaveFileIsPossible */

/* 
 ***********************************************************************************
 * initErrorCmd --
 * Funzione che inizializza il colloquio con i canali degli errori.
 ***********************************************************************************
 */
int InitErrorCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv,j;
  int StandardOutput=0;
  int StandardError=0;

  /* if ((argc > 1))
    {
    interp->result = " Usage: InitError";
    return TCL_ERROR;
    } */

  for(j=1;j<argc;j++)
    {
    if(!strcmp(argv[j],"-stdout"))
      StandardOutput=1;
    else if(!strcmp(argv[j],"-stderr"))
      StandardError=1;
    }

  rv=initXerror();
  if ( rv != -1 ) 
    {
    CanaleErrori=1;
    interp->result = "1";
    if ( StandardOutput ) 
      {
      if(initStdout()==-1)
        interp->result = "0";
      }
    if ( StandardError ) 
      {
      if(initStderr()==-1)
        interp->result = "0";
      }
    
    } 
  else
    {
    interp->result = "0";
    }
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * InvErrorCmd --
 * Invia il messaggio di errore
 * Ingressi : Sigla, Codice, Tipo, Parametro
 ***********************************************************************************
 */
int InvErrorCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  if (CanaleErrori == 0 )
    {
    interp->result = "There is no attachment with error channel";
    return TCL_ERROR;
    }

  if (argc != 4 && argc != 5)
    {
    interp->result = "wrong # args: should be InvError ID Code Type Parameter";
    return TCL_ERROR;
    }

  if ( strcmp (argv[3],"NULL") == 0 ) 
    {
    invXerror(argv[1],(long)atoi(argv[2]),NULL);
    }
  if ( strcmp (argv[3],"d") == 0 ) 
    {
    invXerror(argv[1],(long)atoi(argv[2]),"%d");
    }
  else if ( strcmp (argv[3],"v") == 0 )
    {
    if ( argv[4] != NULL )
      invXerror(argv[1],(long)atoi(argv[2]),"%v",(long)atoi(argv[4]));
    else 
      {
      interp->result = "Parameter must be an integer";
      return TCL_ERROR;
      }
    }
  else if ( strcmp (argv[3],"n") == 0 )
    {
    if ( argv[4] != NULL )
      invXerror(argv[1],(long)atoi(argv[2]),"%n",argv[4]);
    else 
      {
      interp->result = "Parameter must be a string";
      return TCL_ERROR;
      }
    } 
  else
    interp->result = "Type unkown";

  return TCL_OK;
}
#endif

/*
	parte compilata SOLO se ifndef WIN32 */
#else

/*      PARTE COMPILATA SOLO PER WINDOWS  */



/******************************************************************************\
**      InitPrtCmd
**
**              inizializza le operazioni di stampa
**              INPUT:  pathname relativo alla finestra
**              OUTPUT: device context su cui operare
**
\******************************************************************************/

int
InitPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
int tipo;
BOOL res;
WNDCLASSEX appClasse;
char nomeClasse[50];
char szDevString[120];	// array di stringhe da win.ini file
char *szPrinter ;	// nome stampante
char *szDriver ;	// nome driver
char *szPort ;		// nome porta per stampa

// test sui parametri di ingresso

 if (argc > 2 || (argc == 2 && strcmp(argv[1],"-d") != 0 && strcmp(argv[1],"-D") != 0 )) {
    interp->result = "Usage : InitPrt [-d]";
    return TCL_ERROR;
  }

//  test su precedenti processi di stampa aperti

 if (printHDC) {
    interp->result =  CNI_ERR;
    return TCL_OK;
  }

// Tutto OK : test se apertura dialog oppure diretto su stampante predefinita ...

 tipo = TK_PRINT;
 if (argc == 2) {
    if ((strcmp(argv[1],"-d")==0 ) || (strcmp(argv[1], "-D")==0))
       tipo = TK_DLGPRINT;
  }

/* carico l' handle del deskTop */
 
 printHwnd = (HWND)GetDesktopWindow();

// prepara la parte comune della strutti\ura dei dati per la stampante

       memset((void *) &pd, 0, sizeof(PRINTDLG));

       pd.lStructSize = sizeof(PRINTDLG); 
       pd.hwndOwner   = (HWND)printHwnd; 
       pd.Flags       = PD_RETURNDC; 
       pd.hInstance   = NULL; 
       pd.Flags       = PD_RETURNDC; 
       pd.hInstance   = NULL; 


// testa il tipo di stampa richiesto : se con print dialog oppure no

 switch ( tipo )
  {
    case TK_PRINT :  // stampa senza apertura Dialog Box (stampante predefinita)

//	Carico dal file win.ini la stampante di default !!

       GetProfileString("windows", "device", ",,,", szDevString, 120);
       szPrinter = strtok(szDevString, (const char *) ",") ;
       szDriver = strtok( (char *) NULL, (const char *) ",");
       szPort   = strtok( (char *) NULL, (const char *) ",");
       printHDC = CreateDC (szDriver, szPrinter, szPort, pDevMode /*&pd*/);
       break;

    case TK_DLGPRINT :  // stampa con apertura Dialog Box

       res = PrintDlg(&pd); // apro dialog box di selezione stampante
       printHDC = pd.hDC;
       break;

    default :
       interp->result = "Errore interno : seguita via di 'default' in PrtInit";
       return TCL_ERROR;
       break;

  }  // fine switch

       InitScale( &datiConv );

       if ( !printHDC )  {
	 		//usciti dalla printdlg con " ANNULLA "
		if (pd.hDevMode)
			GlobalFree( pd.hDevMode);
		if (pd.hDevNames)
			GlobalFree( pd.hDevNames);

		printHDC = NULL;	// NULL = via di errore
  		interp->result =  CNI_ERR;
		return TCL_OK;
	}
 
       pDevMode = GlobalLock(pd.hDevMode);    //carica caratt. device stampa

 
 GetClassName ( (HWND)printHwnd , nomeClasse , 50);
 GetClassInfoEx ( NULL , nomeClasse , &appClasse );

 SetMapMode(printHDC, MM_TEXT);

 di.cbSize       = sizeof(DOCINFO);
 di.lpszOutput   = NULL;
 di.lpszDocName  ="EDIT_PLC";	// nome documento in stampa (nel print manager)
 
 salvaInst = (HINSTANCE)appClasse.hInstance;

 StartDoc(printHDC, &di);	// start stampa
 StartPage(printHDC);
/*
 sprintf(interp->result,
	 "1 %d %d %d",
	  (int) ((float) datiConv.horRes / (float) datiConv.prtFVidX),
	  (int) ((float) datiConv.verRes / (float) datiConv.prtFVidY),
	  (int) ((float) datiConv.horRes / (float) datiConv.horSize ));
*/
 sprintf(interp->result,
	 "1 %d %d %d",
	  (int) ((float) datiConv.horRes / (float) datiConv.prtFVidX),
	  (int) ((float) datiConv.verRes / (float) datiConv.prtFVidY),
	  (int) ((float) datiConv.horResV / (float) datiConv.horSizeV ));

  return TCL_OK;
}

/******************************************************************************\
**      GetNPrtCmd
**
**		Ritorna : tcl_PrtName = Nome stampante
**			  tcl_PrtDev  = tipo device
**			  tcl_PrtPort = porta collegata
**			  della stampante di default!!
**
**			  0 = errore nessun dato caricato
**			  1 = ok , variabili globali caricate
**			  
**
**		NON occorre avere prima aperto un processo di stampa !!
**
\******************************************************************************/

int
GetNPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
char szDevString[120];	// array di stringhe da win.ini file
char *szPrinter ;	// nome stampante
char *szDriver ;	// nome driver
char *szPort ;		// nome porta per stampa

// test sui parametri di ingresso

 if ( argc > 1 ) {
    interp->result = "Usage : PrtGetN ";
    return TCL_ERROR;
  }


//	Carico dal file win.ini la stampante di default !!

       GetProfileString("windows", "device", ",,,", szDevString, 120);
       szPrinter = strtok(szDevString, (const char *) ",") ;
       szDriver = strtok( (char *) NULL, (const char *) ",");
       szPort   = strtok( (char *) NULL, (const char *) ",");

 if ( !szPrinter) {
    interp->result = CNI_ERR ;
    return TCL_OK;
  }

// tutto corretto : setto le variabili globali di TCL

 Tcl_VarEval(interp, "set tcl_PrtName {", (char *) szPrinter, " }",
	      (char *) NULL);
 Tcl_VarEval(interp, "set tcl_PrtDev {", (char *) szDriver, " }",
	      (char *) NULL);
 Tcl_VarEval(interp, "set tcl_PrtPort {", (char *) szPort, " }",
	      (char *) NULL);

 interp->result = CNI_OK ;
  return TCL_OK;
}


/******************************************************************************\
**	Salto pagina (FormFeed)                                               **
**                                                                            **
**		Esegue un salto pagina                                        **
**		INPUT:  NESSUNO						      **
**		        (DEVE esserci un processo di stampa aperto !!)	      **
**                                                                            **
\******************************************************************************/

int FFeedPrtCmd  (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

 if (! printHDC ) {
   interp->result = " Processo di stampa NON aperto : Form Feed impossibile !!";
   return TCL_ERROR;
   }

/*
	salto pagina
*/

 EndPage  ( printHDC );	// fine sezione (salto pagina)

// EndDoc  ( printHDC );		// fine stampa 
 
// ResetDC ( printHDC );

// StartDoc(printHDC, &di);	// start stampa

 StartPage(printHDC);   // riparti con pagina

 interp->result = CNI_OK ;
 return TCL_OK;

}

/******************************************************************************\
**	AbortStampa                                                           **
**                                                                            **
**		abort operazioni di stampa                                    **
**		INPUT:  NESSUNO						      **
**		        (DEVE esserci un processo di stampa aperto !!)	      **
**                                                                            **
\******************************************************************************/

int AbortPrtCmd  (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

 if (! printHDC ) {
   interp->result = CNI_ERR ;
   return TCL_OK;
   }

/*
	chiusura processo di stampa ...
*/

 AbortDoc ( printHDC ); // abort processo di stampa

 if (pd.hDevMode)
	GlobalFree( pd.hDevMode);
 if (pd.hDevNames)
	GlobalFree( pd.hDevNames);
 printHDC= NULL;	// reset HDC di stampa

 interp->result = CNI_OK ;
 return TCL_OK;

}

/******************************************************************************\
**	FineStampa                                                            **
**                                                                            **
**		conclude le operazioni di stampa                              **
**		INPUT:  NESSUNO						      **
**		        (DEVE esserci un processo di stampa aperto !!)	      **
**                                                                            **
\******************************************************************************/

int StopPrtCmd  (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

 if (! printHDC ) {
   interp->result = " Processo di stampa NON aperto ... Impossibile chiuderlo";
   return TCL_ERROR;
   }

/*
	chiusura processo di stampa ...
*/

 EndPage  ( printHDC );	// fine sezione (salto pagina)
 EndDoc   ( printHDC );	// fine stampa 
 DeleteDC ( printHDC );	// rilascio HDC
 //ReleaseDC ( printHDC,pd.hwndOwner );
 //MessageBox ( NULL,"gallo","GALLO",MB_OK);
 
 if (pd.hDevMode)
	GlobalFree( pd.hDevMode);
 if (pd.hDevNames)
	GlobalFree( pd.hDevNames);
 printHDC= NULL;	// reset HDC di stampa

 interp->result = CNI_OK;
 return TCL_OK;

}


/*
 * InitScale --
 *
 *      Calcola i dati necessari ad eseguire i "rescale" per le operazioni
 *	grafiche
 *
 *	INPUT: 	 Punt. alla struttura tipo RESCALEDAT su cui riversare i dati
 *
 *	OUTPUT: 	0 --> tutto okay
 *              -1 --> errore
 */
int
InitScale( RESCALEDAT *rd )
{
    HDC     appoDC;

    /* azzera tutta la struttura su cui riversare l'output */

    memset((void *) rd, 0, sizeof(RESCALEDAT));

    if(!printHDC)
	return -1;

    rd->prtX    = GetDeviceCaps(printHDC, LOGPIXELSX) ;
    rd->prtY    = GetDeviceCaps(printHDC, LOGPIXELSY) ;
    rd->horRes  = GetDeviceCaps(printHDC, HORZRES) ;
    rd->verRes  = GetDeviceCaps(printHDC, VERTRES) ;
    rd->horSize = GetDeviceCaps(printHDC, HORZSIZE) ;
    rd->verSize = GetDeviceCaps(printHDC, VERTSIZE) ;

    appoDC = (HWND)GetWindowDC(printHwnd);

    if(!appoDC)
	return -1;

    rd->vidX     = GetDeviceCaps(appoDC, LOGPIXELSX);
    rd->vidY     = GetDeviceCaps(appoDC, LOGPIXELSY);
    rd->horResV  = GetDeviceCaps(appoDC, HORZRES) ;
    rd->horSizeV = GetDeviceCaps(appoDC, HORZSIZE) ;

    if(!ReleaseDC((HWND)printHwnd, appoDC))
	                       return -1;

    rd->resFSizeX = (float)rd->horRes/rd->horSize ;
    rd->resFSizeY = (float)rd->verRes/rd->verSize ;
    rd->prtFVidX  = (float)rd->prtX/rd->vidX      ;
    rd->prtFVidY  = (float)rd->prtY/rd->vidY      ;
}



/*
 * ########################################################
 *                      Funzione ConvCoord
 * ########################################################
 *
 *
 *	Converte coordinate da mm >> punti di stampa (pixel stampante)
 *
 *  Input : int mmDim       Valore in mm da convertire
 *          boolean xY      Asse di conversione (0 -> ORIZZ. / 1 -> VERT.)
 *          RESCALEDAT *rs  Pointer struttura dati conversione
 *
 *	Output:	int VALORE CONVERTITO IN PUNTI STAMPANTE
 */
int
ConvCoord(int mmDim, boolean xY, RESCALEDAT *rs)
{
 int datConv;

   switch(xY) {
	case 0:
		datConv = (int)((float)mmDim * (float)rs->resFSizeX);
		break;
	default:
		datConv = (int)((float)mmDim * (float)rs->resFSizeY);
		break;
   };

 return datConv;
}


/*
 * ########################################################
 *                      Funzione ConvPrint
 * ########################################################
 *
 *
 *	Converte punti di stampa video --> punti stampante
 *
 *  Input : int nump        Numero punti da convertire
 *          boolean xY      Asse di conversione (CX -> ORIZZ. / CY -> VERT.)
 *          RESCALEDAT *rs  Pointer struttura dati conversione
 *
 *	Output:	int VALORE CONVERTITO IN PUNTI STAMPANTE
 */
int
ConvPrint(int nump, boolean xY, RESCALEDAT *rs)
{
 int datConv;

   switch(xY) {
	case CX:
		datConv = (int)((float)nump * (float)rs->prtFVidX);
		break;
	case CY:
	default:
		datConv = (int)((float)nump * (float)rs->prtFVidY);
		break;
   };

 return datConv;
}



/*
 *****************************************************************************
 *      BmpPrtCmd
 *
 *              disegna un Bitmap coi vertici indicati in stampa
 *
 *****************************************************************************
 */

int
BmpPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
  BITMAP   bm;
  HDC      hdcMem;
  HANDLE   oldObj;
  int      dimX,dimY,resBmp;
  HINSTANCE hinst;
  PRTINPUT ps;			/* struct dati in input */

  if (argc!=3) {
	interp->result = "Usa : PrtBmp -Cx1,y1[,x2,y2] ";
	return TCL_ERROR;
  }

  if( ReadInput(argc, argv, &ps) !=0 ) {
	interp->result = "Usa : PrtBmp -Cx1,y1[,x2,y2] ";
	return TCL_ERROR;
    }

  if ( ps.coordXCentro[0] == 0 || ps.coordYCentro[0] == 0 ) {
	interp->result = " Usa : PrtBmp -Cx1,y1[,x2,y2] ";
	return TCL_ERROR;
    }

  if ( ps.coordXCentro[1] != 0 && ps.coordXCentro[0] > ps.coordXCentro[1] ) {
	interp->result = " x1 > x2 !! Usa : PrtBmp -Cx1,y1[,x2,y2] ";
	return TCL_ERROR;
    }
  if ( ps.coordYCentro[1] != 0 && ps.coordYCentro[0] > ps.coordYCentro[1] ) {
	interp->result = " y1 > y2 !! Usa : PrtBmp -Cx1,y1[,x2,y2] ";
	return TCL_ERROR;
    }

  if (!printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
  }

  hinst = (HINSTANCE)GetWindowLong(NULL, GWL_HINSTANCE);

  resBmp = LoadBmpFile( argv[1], printHDC, &hbm );	//carico il file ...

  switch( resBmp ) {
		case ERR_FILE:  // file NON trovato
        	sprintf(interp->result, "File %s NON trovato !!", argv[1]); 
		return TCL_ERROR;
		break;

		case ERR_MEM:   // Memoria piena : impossibile allocare
        	interp->result = "Memoria insufficiente "; 
		return TCL_ERROR;
		break;

		case ERR_TYPE:  // File NON di tipo BMP
        	sprintf(interp->result, "File %s NON di tipo BMP !!", argv[1]); 
		return TCL_ERROR;
		break;

		case ERR_OBJ:   // File NON di tipo BMP
        	interp->result = "Errore Sulla creazione del bitmap richiesti"; 
		return TCL_ERROR;
		break;

		default:
		break;
    }

  if( hbm==NULL ) {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

  hdcMem =  CreateCompatibleDC (printHDC);
  GetObject (hbm, sizeof(BITMAP), (LPSTR)&bm);
  oldObj=   SelectObject (hdcMem, hbm);
	 
/* set dimensione rettangolo destinazione (definita dall' utente o del bitmap
*/

  dimX = bm.bmWidth  ;
  dimY = bm.bmHeight ;

  if( ps.coordXCentro[1] != 0)
      dimX = ps.coordXCentro[1] - ps.coordXCentro[0] ;

  if( ps.coordYCentro[1] != 0)
      dimY = ps.coordYCentro[1] - ps.coordYCentro[0] ;

  StretchBlt(printHDC ,
	     ConvPrint(ps.coordXCentro[0], CX, &datiConv),
	     ConvPrint(ps.coordYCentro[0], CY, &datiConv),
 	     ConvPrint(dimX, CX, &datiConv),
 	     ConvPrint(dimY, CY, &datiConv),
	     hdcMem,
	     0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

  SelectObject (hdcMem, oldObj);
  DeleteDC (hdcMem);

  interp->result = CNI_OK;
  return TCL_OK;
}

/*
	 ##########################################
		  Funzione LoadBmpFile
	 ##########################################


	Carica un file di tipo BMP in memoria e lo trasforma in un "oggetto" grafico

  Input : char    *nomeFile 	 : nome file da caricare (path compreso)
		  HDC      hDBmp : device context in cui caricare il bitmap
		  HBITMAP *pHBmp : puntatore oggetto bitmap
 

	Output:	NULL	 = File caricato ; TUTTO OK
		ERR_FILE = File NON aperto
		ERR_MEM  = File troppo grande
		ERR_TYPE = File NON di tipo BMP
		ERR_OBJ  = Errore sulla creazione dell'oggetto grafico

*/

int
LoadBmpFile( char *nomeFile, HDC hDBmp, HBITMAP *pHBmp )
{
HANDLE file ;			  // handle al file aperto
HANDLE memAll ;			  // handle alla memoria allocata
HANDLE memAll2 ;		  // handle alla memoria allocata
DWORD bytesR ;		  // bytes letti
HBITMAP hBmpF ;

// strutture per il BitMap

BITMAPFILEHEADER bmpFH ;
BITMAPINFOHEADER bmpIH ;
BITMAPINFO       *bmpInf;
RGBQUAD          *bmpDat;

// apro il file in lettura

 file = CreateFile(nomeFile, GENERIC_READ, FILE_SHARE_READ,
				(LPSECURITY_ATTRIBUTES) NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
				(HANDLE) NULL );

 // leggo e carico la struttura BITMAPFILEHEADER

 ReadFile (file, &bmpFH, sizeof(BITMAPFILEHEADER), (LPDWORD)&bytesR, (LPOVERLAPPED) NULL );

 // leggo e carico la struttura BITMAPINFOHEADER

 ReadFile (file, &bmpIH, sizeof(BITMAPINFOHEADER), (LPDWORD)&bytesR, (LPOVERLAPPED) NULL );

//  alloco la memoria per leggere tutto il resto dei dati

 memAll = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 
						((1<<bmpIH.biBitCount) * sizeof(RGBQUAD)));

 bmpInf = GlobalLock(memAll);

/*
 * Carica la BITMAPINFOHEADER nella struttura BITMAPINFO 
 */ 

bmpInf->bmiHeader.biSize = bmpIH.biSize; 
bmpInf->bmiHeader.biWidth = bmpIH.biWidth; 
bmpInf->bmiHeader.biHeight = bmpIH.biHeight; 
bmpInf->bmiHeader.biPlanes = bmpIH.biPlanes; 
bmpInf->bmiHeader.biBitCount = bmpIH.biBitCount; 
bmpInf->bmiHeader.biCompression = bmpIH.biCompression; 
bmpInf->bmiHeader.biSizeImage = bmpIH.biSizeImage; 
bmpInf->bmiHeader.biXPelsPerMeter = bmpIH.biXPelsPerMeter; 
bmpInf->bmiHeader.biYPelsPerMeter = bmpIH.biYPelsPerMeter; 
bmpInf->bmiHeader.biClrUsed = bmpIH.biClrUsed; 
bmpInf->bmiHeader.biClrImportant = bmpIH.biClrImportant;

 /*
 * Carico la color table. 
 * 1 << bmih.biBitCount == 2 ^ bmih.biBitCount 
 */ 

 ReadFile(file, bmpInf->bmiColors,
	((1<<bmpIH.biBitCount) * sizeof(RGBQUAD)),
	(LPDWORD)&bytesR, (LPOVERLAPPED) NULL);

 /*
 * Allocate memory for the required number of 
 * bytes. 
 */ 

 memAll2 = GlobalAlloc(GHND, (bmpFH.bfSize - bmpFH.bfOffBits));

 bmpDat = GlobalLock(memAll2);

// leggo i dati del bitmap
 
 ReadFile(file, bmpDat, (bmpFH.bfSize - bmpFH.bfOffBits),
			(LPDWORD)&bytesR, (LPOVERLAPPED) NULL);

// creo il bitmap dal file letto

 hBmpF =CreateDIBitmap(hDBmp, &bmpIH, CBM_INIT, bmpDat, bmpInf, DIB_RGB_COLORS);

 
/*
 *	Libero la memoria e chiudo il file
 */ 

 GlobalUnlock(memAll);
 GlobalUnlock(memAll2);
 CloseHandle(file);

 // test sul tipo di errori ... se ce ne sono stati !!
 
 *pHBmp = NULL ;   // resetto oggetto in uscita (se tutto ok lo setto alla fine)

 if(!file)
	 return ERR_FILE;
 if(bmpFH.bfType != 0x4d42)
	 return ERR_TYPE;
 if(!memAll || !memAll2 || !bmpInf || !bmpDat )
	 return ERR_MEM;
 if(!hBmpF)
	 return ERR_OBJ;

//	Tutto OK !!

 *pHBmp = hBmpF ;	// set oggetto grafico in input
 return FALSE;

}


/*
 *****************************************************************************
 *      CirclePrtCmd
 *
 *              disegna un ellisse (cerchio) fornendo in input i due
 *		vertici opposti del rettangolo (quadrato) in cui e` inscritto
 *
 *****************************************************************************
 */

int
CirclePrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
    PRTINPUT ps;			/* struct dati in input */
    HPEN     hpen,hpenOld;
    HBRUSH   hbrush,hbrushOld;
    int      hatchStyle;
    boolean  app;

    if(ReadInput(argc, argv, &ps)!=0) {
	interp->result = "Usa : PrtCircle [-Sx -Tx -Fx] -Px1,y1,..x15,y15 ";
	return TCL_ERROR;
    }

    if(ps.numPoints<2) {
	interp->result = "Usa : PrtCircle [-Sx -Tx -Fx] -Px1,y1,..x15,y15] ";
	return TCL_ERROR;
    }

    if (!printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    if(ps.fillArea!=0) {
	if(ps.fillArea==1) {
    		hbrush    = CreateSolidBrush( 0);
	} else {
		hatchStyle = GimmeHatch( ps.fillArea);
    		hbrush    = CreateHatchBrush(hatchStyle, 0);
	}
    	hbrushOld = SelectObject (printHDC, hbrush );
    }

    hpen      = CreatePen( (int)GimmePenStyle(ps.tipoLinea) , ps.spesLinea, 0);
    hpenOld   = SelectObject (printHDC, hpen );

    app=Ellipse(printHDC,
	 ConvPrint(ps.coordX[0], CX, &datiConv),
	 ConvPrint(ps.coordY[0], CY, &datiConv),
	 ConvPrint(ps.coordX[1], CX, &datiConv),
	 ConvPrint(ps.coordY[1], CY, &datiConv) );

    SelectObject (printHDC, hpenOld );

    if(ps.fillArea!=0)
    	SelectObject (printHDC, hbrushOld );
	
	DeleteObject(hpen);

    if (!app )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    interp->result = CNI_OK;
    return TCL_OK;
}

/*
 *****************************************************************************
 *      ArcPrtCmd
 *
 *              disegna un arco di cerchio/ellissi fornendo in input
 *		il punto di partenza , il punto di arrivo e gli angoli
 *		oppsti del parallelogramma in cui inscrivere l'arco
 *
 *****************************************************************************
 */

int
ArcPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
PRTINPUT ps;			/* struct dati in input */
HPEN     hpen,hpenOld;

    if(ReadInput(argc, argv, &ps)!=0) {
	interp->result = "Usa : PrtArc [-Sx -Tx] -Cx1,y1,x2,y2 -Px1,y1,x2,y2";
	return TCL_ERROR;
    }

    if(ps.numPoints<2) {
	interp->result = "Usa : PrtArc [-Sx -Tx] -Cx1,y1,x2,y2 -Px1,y1,x2,y2";
	return TCL_ERROR;
    }

    if (!printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    hpen      = CreatePen( (int)GimmePenStyle(ps.tipoLinea) , ps.spesLinea, 0);
    hpenOld   = SelectObject (printHDC, hpen );

    if ( ! Arc(printHDC,
	 ConvPrint(ps.coordXCentro[0], CX, &datiConv),
	 ConvPrint(ps.coordYCentro[0], CY, &datiConv),
	 ConvPrint(ps.coordXCentro[1], CX, &datiConv),
	 ConvPrint(ps.coordYCentro[1], CY, &datiConv),
	 ConvPrint(ps.coordX[0], CX, &datiConv),
	 ConvPrint(ps.coordY[0], CY, &datiConv),
	 ConvPrint(ps.coordX[1], CX, &datiConv),
	 ConvPrint(ps.coordY[1], CY, &datiConv) ) ) {

//       arco NON tracciato

        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    SelectObject (printHDC, hpenOld );
	DeleteObject (hpen);

    interp->result = CNI_OK;
    return TCL_OK;
}


/*
 *****************************************************************************
 *      LinePrtCmd
 *
 *              traccia Linee tra i punti indicati in stampa
 *
 *****************************************************************************
 */

int
LinePrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
    int      i;
    PRTINPUT ps;			/* struct dati in input */
    HPEN     hpen,hpenOld;
    LOGPEN   myPen;

	//return(TCL_OK);	//gallogallo

    if(ReadInput(argc, argv, &ps)!=0) {
	interp->result = "Usa : PrtLine [-Sx -Tx] -Px1,y1,..x15,y15 ";
	return TCL_ERROR;
    }

    if(ps.numPoints<2) {
	interp->result = "Usa : PrtLine [-Sx -Tx] -Px1,y1,..x15,y15 ";
	return TCL_ERROR;
    }

    if (!printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    myPen.lopnStyle= GimmePenStyle( ps.tipoLinea );
    myPen.lopnWidth.x = ps.spesLinea;
    myPen.lopnWidth.y = 1;
    myPen.lopnColor= 0;

//  hpen    = CreatePen( (int)GimmePenStyle(ps.tipoLinea) , ps.spesLinea, 0);
    hpen    = CreatePenIndirect( &myPen );
    hpenOld = SelectObject (printHDC, hpen );

    MoveToEx( printHDC, ConvPrint(ps.coordX[0], CX, &datiConv),
			ConvPrint(ps.coordY[0], CY, &datiConv),
			NULL);

    for(i=1;i<ps.numPoints;i++)
	LineTo( printHDC, ConvPrint(ps.coordX[i], CX, &datiConv),
			  ConvPrint(ps.coordY[i], CY, &datiConv));

    SelectObject (printHDC, hpenOld );
	DeleteObject(hpen);

    interp->result = CNI_OK;
    return TCL_OK;
}


/*
 * ReadInput
 *
 *	Esamina i parametri in input e scarica tutti i valori
 *	"interessanti" nella struttura di deposito
 *
 *	Output: 0 --> tutto okay
 *		1 --> errore
 *
 *    int argc;       Numero di Argomenti in input
 *    char **argv;    Array degli Argomenti
 *    PRTINPUT *ps;   Punta Struct Deposito
 */
int
ReadInput(int argc, char **argv, PRTINPUT *ps )
{
 UCHAR    appval[128];		/* buffer appoggio per traduzioni */
 int      i, j, k, c, app;
 int      indCoo;
 int      indCoo2;
 boolean  escLoop;

 /* AZZERA TUTTA LA STRUTTURA SU CUI RIVERSARE L'OUTPUT */

 memset((void *) ps, 0, sizeof(PRTINPUT));
 ps->spesLinea=1;

 if(argc<2)
	return -1;		/* esci: troppo pochi argomenti! */

 for(i=1;i<argc;i++) {

	c = argv[i][0];

	if(c=='-') {

		c = argv[i][1];

		switch (c) {

		case 'c':
		case 'C':	/* coordinate centro */
		case 'p':
		case 'P':	/* coordinate punti */
		case 'h':
		case 'H':	/* altezza , larghezza carattere */
			j = 2;
			indCoo  = 0;
			indCoo2 = 0;
			escLoop = 0;

			while(escLoop == 0 && indCoo<15) {
			    k=0;
			    while( isdigit((UCHAR) argv[i][j]) )
					appval[k++]= (UCHAR) argv[i][j++];

			    if(k!=0) {
						/* c'e` qualcosa da copiare */
				appval[k]=(UCHAR)0;
				if(indCoo2==1) {
				    if(c=='p' || c=='P') {
					sscanf(appval,"%d",&ps->coordY[indCoo]);
				    } else {
					if(indCoo<2)
					    sscanf(appval,"%d",
						&ps->coordYCentro[indCoo]);
				    }
				    indCoo2=0;
				    indCoo++;
				} else {
				    if(c=='p' || c=='P') {
				        sscanf(appval,"%d",&ps->coordX[indCoo]);
				    } else {
					if(indCoo<2)
				            sscanf(appval,"%d",
						&ps->coordXCentro[indCoo]);
				    }
				    indCoo2=1;
				}
			   }

			   if(argv[i][j]!=',')
				escLoop=1;
			   else
				j++;
			}

			if(c=='p' || c=='P') {
				ps->numPoints=indCoo;
			}

		break;  /* end case "P" */

		case 'a':
		case 'A':			/* "allineamento" */
			if( isdigit((UCHAR) argv[i][2]) )
				ps->allinText=(int)((UCHAR)argv[i][2])-0x30;
		break;

		case 'f':
		case 'F':			/* "filled" */
			if( isdigit((UCHAR) argv[i][2]) )
				ps->fillArea=(int)((UCHAR)argv[i][2])-0x30;
		break;

		case 't':
		case 'T':			/* "type" */
			if( isdigit((UCHAR) argv[i][2]) )
				ps->tipoLinea=(int)((UCHAR)argv[i][2])-0x30;
		break;

		case 'S':
		case 's':			/* "spessore" */
			if(isdigit((UCHAR)argv[i][2]) ) {
				app =(int)((UCHAR)argv[i][2])-0x30;
				ps->spesLinea= ConvPrint( app ,CX ,&datiConv );
			}
		break;

		case 'B':
		case 'b':			/* "Bold/singolo" */
				ps->fontBold=(BOOL) 1;
		break;

		};

	} /* endif meno */
 }    /* endloop argomenti input */

 return 0;
}



/*
 *****************************************************************************
 *      PoliPrtCmd
 *
 *              disegna un poligono coi vertici indicati in stampa
 *
 *****************************************************************************
 */

int
PoliPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
    int      i;
    PRTINPUT ps;			/* struct dati in input */
    HPEN     hpen,hpenOld;
    HBRUSH   hbrush,hbrushOld;
    int      hatchStyle;
    POINT    punti[15];

    if(ReadInput(argc, argv, &ps)!=0) {
		interp->result = "Usa : PrtPoli [-Sx -Tx] -Px1,y1,..x15,y15 ";
		return TCL_ERROR;
    }

    if(ps.numPoints<2) {
		interp->result = "Usa : PrtPoli [-Sx -Tx] -Px1,y1,..x15,y15] ";
		return TCL_ERROR;
    }

    if (!printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    if(ps.fillArea!=0) {
		if(ps.fillArea==1) {
    		hbrush     = CreateSolidBrush( 0);
		} else {
			hatchStyle = GimmeHatch( ps.fillArea);
    		hbrush     = CreateHatchBrush(hatchStyle, 0);
		}
		hbrushOld = SelectObject (printHDC, hbrush );
    }

    hpen      = CreatePen( (int)GimmePenStyle(ps.tipoLinea) , ps.spesLinea, 0);
    hpenOld   = SelectObject (printHDC, hpen );

    for(i=0;i<ps.numPoints;i++) {
		punti[i].x= (long) ConvPrint(ps.coordX[i], CX, &datiConv);
		punti[i].y= (long) ConvPrint(ps.coordY[i], CY, &datiConv);
	}

    Polygon(printHDC, &punti[0], ps.numPoints );

    SelectObject (printHDC, hpenOld );

    if(ps.fillArea!=0)
    	SelectObject (printHDC, hbrushOld );

	DeleteObject (hpen);
	DeleteObject (hbrush);

    interp->result = CNI_OK;
    return TCL_OK;
}



/*
 *	EZCREATEFONT
*/
HFONT EzCreateFont(HDC hdc, char *szFName, int iHeight, int iWidth, BOOL bold )
{
HFONT hFont;
LOGFONT lf;

SaveDC(hdc );

    lf.lfHeight= ConvPrint(iHeight ,CY ,&datiConv ) ;
    lf.lfWidth=	 ConvPrint(iWidth ,CX ,&datiConv ) ;
    lf.lfEscapement=0;
    lf.lfOrientation=0;
    lf.lfWeight= 700 * (int) bold;
    lf.lfItalic= 0;
    lf.lfUnderline= 0;
    lf.lfStrikeOut= 0;
    lf.lfCharSet= 0;
    lf.lfOutPrecision= 0;
    lf.lfClipPrecision= 0;
    lf.lfQuality= 0;
    lf.lfPitchAndFamily= 0;
    strcpy (lf.lfFaceName, szFName);

hFont = CreateFontIndirect (&lf);

RestoreDC( hdc, -1);

return hFont;
}

/*
 *****************************************************************************
 *      TextPrtCmd
 *
 *              scrive testo utilizzando la draw text
 *
 *****************************************************************************
 */

int
TextPrtCmd (ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
    PRTINPUT ps;			/* struct dati in input */
    RECT     rect;
    UINT     format;
    int      length;
    int      app;
    HFONT    hFont,oldFont;


    if(ReadInput(argc, argv, &ps)!=0) {
	interp->result = "Usa : PrtText stringa [ -Ax -Hx,y -B ] -Px1,y1,x2,y2 ";
	return TCL_ERROR;
    }

    if(ps.numPoints!=2) {
	interp->result = "Usa : PrtText stringa [ -Ax -Hx,y -B ] -Px1,y1,x2,y2 ";
	return TCL_ERROR;
    }

    if ( !printHDC )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    if (( ps.coordXCentro[0] && !ps.coordYCentro[0] ) ||
	( !ps.coordXCentro[0] && ps.coordYCentro[0] )) {
        interp->result = CNI_ERR ;    // solo una delle 2 dimensioni presenti
        return TCL_OK;
    }


// se dimensioni carattere assenti , carico la dimensione di default

    if ( !ps.coordXCentro[0] && !ps.coordYCentro[0] ) {

      ps.coordXCentro[0] = (int)( 4 * (float) datiConv.horResV / (float) datiConv.horSizeV );
      ps.coordYCentro[0] = (int)( 9 * (float) datiConv.horResV / (float) datiConv.horSizeV );
    }

    length = strlen(argv[1]);

    switch (ps.allinText ) {
	case 1:   format= DT_SINGLELINE | DT_CENTER | DT_TOP      ; break;
	case 2:   format= DT_SINGLELINE | DT_RIGHT  | DT_TOP      ; break;
	case 3:   format= DT_SINGLELINE | DT_LEFT   | DT_VCENTER  ; break;
	case 4:   format= DT_SINGLELINE | DT_CENTER | DT_VCENTER  ; break;
	case 5:   format= DT_SINGLELINE | DT_RIGHT  | DT_VCENTER  ; break;
	case 6:   format= DT_SINGLELINE | DT_LEFT   | DT_BOTTOM   ; break;
	case 7:   format= DT_SINGLELINE | DT_CENTER | DT_BOTTOM   ; break;
	case 8:   format= DT_SINGLELINE | DT_RIGHT  | DT_BOTTOM   ; break;
	default:  format= DT_SINGLELINE | DT_LEFT   | DT_TOP      ; break;
	};

// Creazione e selezione del font richiesto

    hFont = EzCreateFont ( printHDC, "Courier New",
			ps.coordYCentro[0], ps.coordXCentro[0], ps.fontBold);

    oldFont = SelectObject (printHDC, hFont);

// scrittura testo

    rect.left =	     	ConvPrint(ps.coordX[0] ,CX ,&datiConv ) ;
    rect.top =	     	ConvPrint(ps.coordY[0] ,CY ,&datiConv ) ;
    rect.right =     	ConvPrint(ps.coordX[1] ,CX ,&datiConv ) ;
    rect.bottom =     	ConvPrint(ps.coordY[1] ,CY ,&datiConv ) ;

    app = DrawText ( printHDC, argv[1], length, &rect, format );

    SelectObject (printHDC ,oldFont );
    DeleteObject (hFont );

    if (!app )  {
        interp->result = CNI_ERR ;
        return TCL_OK;
    }

    interp->result = CNI_OK;
    return TCL_OK;
}


/*
 * GimmePenStyle
 *
 *		Ritorna il valore di "PEN STYLE" da usare
 */
int
GimmePenStyle (int tipo)
{
 switch (tipo) {
	case 1: return PS_DASH;		break;
	case 2: return PS_DOT;		break;
	case 3: return PS_DASHDOT;	break;
	case 4: return PS_DASHDOTDOT;	break;
	default: return PS_SOLID;	break;
 };
}

/*
 * GimmeHatch
 *
 *		Ritorna il valore di "HATCH STYLE" da utilizzare o -1
 */
int
GimmeHatch (int tipo)
{
 switch (tipo) {
	case 2: return HS_FDIAGONAL;	break;
	case 3: return HS_BDIAGONAL;	break;
	case 4: return HS_HORIZONTAL;	break;
	case 5: return HS_VERTICAL;	break;
	case 6: return HS_DIAGCROSS;	break;
	case 7: return HS_CROSS;	break;
	default: return -1;		break;
 };
}

/*
 *****************************************************************************
 *      QualeRegistroCmd
 *
 *              Funzione che ritorna il registro in cui andare a pescare
 *              macchina remota e service
 *
 *****************************************************************************
 */
int QualeRegistroCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
int  LungString=0;
char dirett[64];

 	memset(dirett,0,sizeof(dirett));
 	if ((LungString=LeggeStringa ("PRE",&dirett[0],sizeof(dirett))) > 0 )
       		sprintf(interp->result,"%s",dirett);
	else
       		sprintf(interp->result,"errore");

  return TCL_OK;
}

#endif

/*#############  PARTE COMPILATA PER UNIX / WINDOWS  #############*/

/* 
 ***********************************************************************************
 * ConvRealCmd --
 * Converte un long in un real
 ***********************************************************************************
 */
int ConvRealCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
union real conv;

  if ((argc != 2))
    {
    interp->result = " Usage: ConvReal number";
    return TCL_ERROR;
    }

  conv.temp=atoi(argv[1]);

  sprintf(interp->result,"%g",conv.temp1);
  return TCL_OK;
}



/* 
 ***********************************************************************************
 * ReadRegCmd --
 * Legge il contenuto di un registro windows
 * Ritorna:	"unix" se il sistema e' unix
 *		"errore_registro" se il sistema e' windows e fallisce la lettura
 *		se il sistema e' windows il valore del registro
 ***********************************************************************************
 */
int ReadRegCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

#ifdef WIN32

char *chiaveReg ;
char Registro[1024] ;
char Path[1024] ;	/* ritorno valore registro */
DWORD cbData, dwType ;
HKEY NomeRegistro ;

#endif

  if ((argc != 3))
    {
    interp->result = " Usage: ReadReg key reg_name";
    return TCL_ERROR;
    }

#ifdef WIN32
 dwType = REG_SZ ;   /* ritorna una stringa NUll TERM. */
 cbData= 1024 * sizeof(char) ;

 chiaveReg="Software\\Cni\\" ;   /* chiave generica */
 sprintf(Registro, "%s%s", chiaveReg, argv[1]);
 
 RegCreateKey(HKEY_CURRENT_USER, Registro, &(NomeRegistro));

 if ( NomeRegistro ) {
/* leggo valore registro richiesto  */
	if ( RegQueryValueEx(NomeRegistro, argv[2], NULL, &dwType, (LPVOID)Path,
			&cbData) != ERROR_SUCCESS ) 
  		sprintf(interp->result,"errore_registro");
	else
  		sprintf(interp->result,"%s", Path);

	RegCloseKey(NomeRegistro);
 }
 else
  	sprintf(interp->result,"errore_registro");


#else
  sprintf(interp->result,"unix");
#endif

  return TCL_OK;
}


#ifdef DEBUG_PLC

/*
 *****************************************************************************
 *
 *   Funzione InitializeDataBuffer
 *   
 *   char *DataString	-	Stringa dei dati provenienti dal socket
 *   int event_size	-	Lunghezza del buffer dati
 *   char *data_buffer	-	buffer di deposito dell'area di memoria
 *
 *   Ritorna:
 *	    NULL in caso di errore
 *	    una struttura contenente i dati di una singola lista di watch
 *
 *****************************************************************************
 */
char *InitializeDataBuffer(char *DataString,int event_size,char *data_buffer)
{
int 	DataStringIndex=0;
char	byte[3];
int 	RestoreByte=0;

  for (DataStringIndex=0; DataStringIndex<event_size; DataStringIndex++)
    { 
    byte[0]=DataString[DataStringIndex*2];
    byte[1]=DataString[DataStringIndex*2+1];
    byte[2]='\0';
    if(sscanf(byte,"%x",&RestoreByte)!= 1)
      {
      return NULL;
      }
    data_buffer[DataStringIndex]=*(char*)&RestoreByte;
    }
  return data_buffer;
}

/*
 *****************************************************************************
 *
 *   Funzione plcGetDebugSocketEvent
 *   
 *   char *buffer	-	buffer che contiene l'area dei dati
 *   int *read_pos	-	Puntatore all'interno del buffer di dati
 *   int event_size	-	Dimensione dell'area dati
 *
 *   Ritorna:
 *          NULL in coso di errore
 *	    una struttura contenente i dati di una singola lista di watch
 *
 *****************************************************************************
 */

brk_event * plcGetDebugSocketEvent(char *buffer,int *read_pos,int event_size)
{
brk_event	*Event;

  if (! buffer)
    return (brk_event *) NULL;
        
  if (*read_pos >= event_size)
    return (brk_event *) NULL;

  Event = (brk_event *) (buffer + *read_pos);
     *read_pos += Event -> be_size;
        
  return Event;
}

/* 
 ***********************************************************************************
 * WatchShowRemoteCmd --
 * Con questo eseguo tutte la funzione di debug remoto
 ***********************************************************************************
 */

int WatchShowRemoteCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int event_size=0;
  int SizeData;
  int IndexDataDimension=3;
  int IndexDataDimension2=3;
  int OldNumLista=-1;
  int ReadPointer=0;
  int ByteLista=0;
  int SizeEventData=0;
  char *data_buffer;
  char *PointerEventData;
  char temp_buffer[1024];
  brk_event_list *EventList;
  brk_event *Event;
  if (argc == 1)
    {
    interp->result = " Usage: watchShowRemote NumByte Buffer ByteType";
    return TCL_ERROR;
    }

  event_size=atoi(argv[1]);
  if (!(data_buffer=(char *)malloc(sizeof(char)*event_size)))
    {
    interp->result = "1";
    return TCL_OK;
    }
  if (InitializeDataBuffer(argv[2],event_size,data_buffer) == NULL)
    {
    free(data_buffer);
    interp->result = "1";
    return TCL_OK;
    }

  EventList= (brk_event_list *) data_buffer;

  ReadPointer=8;
  IndexDataDimension=3;
  IndexDataDimension2=3;

  while ( (Event = plcGetDebugSocketEvent(data_buffer,&ReadPointer,event_size)) ) 
    {
    sprintf(temp_buffer,"Lista%d", Event -> be_id);
    Tcl_AppendElement(interp,temp_buffer);
    OldNumLista++;
    /* Verifico se ci sono delle liste vuote */
    while (OldNumLista != Event -> be_id && OldNumLista < MAX_WATCH)
      {
      while (strcmp(argv[IndexDataDimension],"#") != 0 )
        {
      	if ( IndexDataDimension == argc-1 )
	  {
    	  free(data_buffer);
    	  interp->result = "3";
    	  return TCL_OK;
  	  }
        IndexDataDimension++;
        IndexDataDimension2++;
        }
      IndexDataDimension++;
      IndexDataDimension2++;
      OldNumLista++;
      }
    ByteLista=0;
    /* Verifico se i byte della lista sono quelli che mi aspetto */
    while (IndexDataDimension < argc && strcmp(argv[IndexDataDimension],"#") != 0)
	{
	ByteLista+=atoi(argv[IndexDataDimension]);
        IndexDataDimension++;
	}
    if (IndexDataDimension < argc)
    	IndexDataDimension++;
    if (Event -> be_size - 3 != ByteLista)
	{
    	free(data_buffer);
    	interp->result = "3";
    	return TCL_OK;
	}
    for (PointerEventData = Event -> be_data, SizeEventData = Event -> be_size - 3, SizeData=0; 
           SizeEventData > 0 ; SizeEventData -= SizeData) 
      {
      if (IndexDataDimension2 < argc)
        {
        switch (atoi(argv[IndexDataDimension2]))
	  {
	  case 1:
		sprintf(temp_buffer,"0x%02x", *(unsigned char*)PointerEventData);
                Tcl_AppendElement(interp,temp_buffer);
		break;
	  case 2:
		sprintf(temp_buffer,"0x%04x", *(unsigned short*)PointerEventData);
                Tcl_AppendElement(interp,temp_buffer);
		break;
          case 4:
		sprintf(temp_buffer,"0x%08lx", *(unsigned long*)PointerEventData);
                Tcl_AppendElement(interp,temp_buffer);
		break;
	  }
	  SizeData = atoi(argv[IndexDataDimension2]);
	  PointerEventData += SizeData;
          IndexDataDimension2++;
        }
      }
      if (IndexDataDimension2 < argc)
        {
        if (strcmp(argv[IndexDataDimension2],"#") == 0)
          IndexDataDimension2++;
        }
      if (IndexDataDimension != IndexDataDimension2)
	{
    	free(data_buffer);
    	interp->result = "3";
    	return TCL_OK;
	}
    }
    free(data_buffer); 
return TCL_OK;
}

/* 
 ***********************************************************************************
 * WatchHistoryRemoteCmd --
 * Con questo eseguo tutte la funzione di debug remoto con storia
 ***********************************************************************************
 */
int WatchHistoryRemoteCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int event_size=0;
  int ByteAnalizzati=0;
  int SizeData;
  int IndexDataDimension=3;
  int IndexDataDimension2=3;
  int OldNumLista=-1;
  int ReadPointer=0;
  int ByteLista=0;
  int OffsetLista[MAX_WATCH];
  int IndexLst=0,i;
  int SizeEventData=0;
  int flags = 0;
  char *data_buffer;
  char *PointerEventData;
  char temp_buffer[1024];
  brk_event_list *EventList;
  brk_event *Event;
  if (argc == 1)
    {
    interp->result = " Usage: watchHistoryRemote NumByte Buffer ByteType";
    return TCL_ERROR;
    }

  event_size=atoi(argv[1]);
  if (!(data_buffer=(char *)malloc(sizeof(char)*event_size)))
    {
    interp->result = "1";
    return TCL_OK;
    }
  if (InitializeDataBuffer(argv[2],event_size,data_buffer) == NULL)
    {
    free(data_buffer);
    interp->result = "1";
    return TCL_OK;
    }

  while (ByteAnalizzati != event_size)
    {
    OffsetLista[IndexLst]=ByteAnalizzati;
    EventList= (brk_event_list *) (data_buffer + ByteAnalizzati);
    ByteAnalizzati+=EventList->bl_size;
    IndexLst++;
    }
  for ( i=(IndexLst-1); i>=0; i-- )
    {
    EventList= (brk_event_list *) (data_buffer + OffsetLista[i]);

    ReadPointer=8;
    IndexDataDimension=3;
    IndexDataDimension2=3;
    OldNumLista=-1;
  
    while ((Event=plcGetDebugSocketEvent(data_buffer+OffsetLista[i],&ReadPointer,EventList->bl_size))) 
      {
      if ((flags & (1 << Event -> be_id)) == 0)
        {
        if (i==(IndexLst-1))
          sprintf(temp_buffer,"Lista%d", Event -> be_id); 
        else if (i < (IndexLst-1))
          sprintf(temp_buffer,"Lista%d_o", Event -> be_id); 
        Tcl_AppendElement(interp,temp_buffer);
        flags |= (1 << Event -> be_id);
        OldNumLista++;
        }
      else
        continue;
      /* Verifico se ci sono delle liste vuote */
      while (OldNumLista != Event -> be_id && OldNumLista < MAX_WATCH)
        {
        while (strcmp(argv[IndexDataDimension],"#") != 0 )
          {
      	  if ( IndexDataDimension == argc-1 )
	    {
    	    free(data_buffer);
    	    interp->result = "3";
    	    return TCL_OK;
  	    }
          IndexDataDimension++;
          IndexDataDimension2++;
          }
        IndexDataDimension++;
        IndexDataDimension2++;
        OldNumLista++;
        }
      ByteLista=0;
      /* Verifico se i byte della lista sono quelli che mi aspetto */
      while (IndexDataDimension < argc && strcmp(argv[IndexDataDimension],"#") != 0)
	  {
	  ByteLista+=atoi(argv[IndexDataDimension]);
          IndexDataDimension++;
	  }
      if (IndexDataDimension < argc)
    	  IndexDataDimension++;
      if (Event -> be_size - 3 != ByteLista)
	  {
    	  free(data_buffer);
    	  interp->result = "3";
    	  return TCL_OK;
	  }
      for (PointerEventData = Event -> be_data, SizeEventData = Event -> be_size - 3, SizeData=0; 
             SizeEventData > 0 ; SizeEventData -= SizeData) 
        {
        if (IndexDataDimension2 < argc)
          {
          switch (atoi(argv[IndexDataDimension2]))
	    {
	    case 1:
		  sprintf(temp_buffer,"0x%02x", *(unsigned char*)PointerEventData);
                  Tcl_AppendElement(interp,temp_buffer);
		  break;
	    case 2:
		  sprintf(temp_buffer,"0x%04x", *(unsigned short*)PointerEventData);
                  Tcl_AppendElement(interp,temp_buffer);
		  break;
            case 4:
		  sprintf(temp_buffer,"0x%08lx", *(unsigned long*)PointerEventData);
                  Tcl_AppendElement(interp,temp_buffer);
		  break;
	    }
	    SizeData = atoi(argv[IndexDataDimension2]);
	    PointerEventData += SizeData;
            IndexDataDimension2++;
          }
        }
        if (IndexDataDimension2 < argc)
          {
          if (strcmp(argv[IndexDataDimension2],"#") == 0)
            IndexDataDimension2++;
          }
        if (IndexDataDimension != IndexDataDimension2)
	  {
    	  free(data_buffer);
    	  interp->result = "3";
    	  return TCL_OK;
	  }
      }
    }
    free(data_buffer); 
return TCL_OK;
}

/* 
 *******************************************************************************
 * CalcolaCRCCmd --
 * Implementa il calcolo del CRC del file in input
 *******************************************************************************
 */

int CalcolaCRCCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int sh = 0;
  int CarattereShiftato=0;
  unsigned long int CarattereShiftatoOri=0;
  int RisultatoDivisione=0;
  int CRC=0;
  char cChar = 0;
  FILE    *fp;

  if (argc > 2)
    {
    interp->result = " Usage: CalcolaCRC filename ";
    return TCL_ERROR;
    }

  if( !(fp=fopen(argv[1], "r")))
     {
     sprintf(interp->result,"File doesn't exist");
     return TCL_ERROR;
     }

  while(fread(&cChar,sizeof (char), 1,fp)) 
    {
    CarattereShiftato=(int)cChar;
    CarattereShiftato <<= GRADO_POLINOMIO_GENERATORE;
    CarattereShiftatoOri=CarattereShiftato;
    for (sh=1; sh <= DIMENSIONE_BYTE; sh++)
      {
      RisultatoDivisione=(CarattereShiftato>>(BIT_TOTALI-sh)&1);
      if (RisultatoDivisione == 0 ) 
        {
        CarattereShiftato^=0;
        }
      else if (RisultatoDivisione == 1)
        {
        CarattereShiftato^=(POLINOMIO_GENERATORE_SHIFTATO>>(sh-1));
        }
      }
      CRC+=(CarattereShiftatoOri|CarattereShiftato);
    }

  fclose(fp);
  sprintf(interp->result,"%d",CRC);
  return TCL_OK;
}

/* 
 *******************************************************************************
 * strtab_len --
 * Effettua la Lettura della lunghezza della string table. 
 *
 * INPUT:
 *	FILE * src		: descrittore del file	
 *	struct exec *header 	: header di descrizione del file
 *	long base		: eventuale offset nel file
 *
 * Ritorna:
 *      -1	: in caso di errore
 *	Lunghezza della string table
 *******************************************************************************
 */
static long strtab_len(FILE * src, struct exec *header, long base)
{
  long st_pos,val;

  /* Naturalmente, qualche simbolo ci vuole... */

  if (! header -> a_syms) 
    {
    /* ERRORE : neanche un simbolo. */
    return -1;
    }

  /* Calcolo della posizione del campo contenente la lunghezza
     della string table. */

  st_pos = header -> a_text + header -> a_data +
           header -> a_trsize + header -> a_drsize +
       	   header -> a_syms + sizeof(*header) + base;

  if (fseek(src,st_pos,0)) 
    {
    /* ERRORE : qualcosa e` andato storto. */
    return -1;
    }

  /* Lettura della lunghezza della tabella. */

  if (fread((char *)&val, sizeof(val), 1, src) != 1) 
    {
    /* ERRORE : qualcosa e` andato storto. */
    return -1;
    }

  return val;
}

/* 
 ***********************************************************************************
 * TclNmCmd --
 * Con questo comando estraggo i simboli di debug del PLC
 ***********************************************************************************
 */
int TclNmCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
FILE *ifile;
int i;
int TipoNm=0;
char Risultato[100];
long pos,val;
char *p;
struct exec header;
struct nlist *SymbolTable = (struct nlist *) 0;
char *TabellaStringhe = (char *) 0;
long NumeroSimboli = 0;

  if (argc > 3)
    {
    interp->result = "Usage: tclnm TipoNm Nomefile";
    return TCL_ERROR;
    }


  TipoNm=atoi(argv[1]);

  if (! argv[2]) 
    {
    interp->result = "1";
    return TCL_OK;
    }

  ifile = fopen(argv[2],"r");

  if (! ifile) 
    {
    interp->result = "1";
    return TCL_OK;
    }

  /* Lettura dell'header a.out. */
  if (fread((char *)&header, sizeof(header), 1, ifile) != 1) 
    {
    /* ERRORE : sembra che il file sia troppo corto... */
    interp->result = "1";
    return TCL_OK;
    }

  /* Controlla il formato. */
  if (header.a_magic != OMAGIC) 
    {
    /* ERRORE. Formato dell'oggetto imprevisto. */
    interp->result = "1";
    return TCL_OK;
    }

  if (TipoNm)
    {
    pos = strtab_len(ifile,&header,0);
    pos += header.a_text + header.a_data +
           header.a_trsize + header.a_drsize +
           header.a_syms + sizeof(header);

    /* Posizionamento all'inizio dell'estensione di debug. */
    if (fseek(ifile,pos,0)) 
      {
      /* ERRORE : qualcosa e` andato storto. */
      interp->result = "1";
      return TCL_OK;
      }

    /* BENE, SIAMO ALL'INIZIO DEL BLOCCO DI DEBUG. */
    /* Lettura dell'header a.out secondario. */
    if (fread((char *)&header, sizeof(header), 1, ifile) != 1) 
      {
      /* ERRORE : sembra che il file sia troppo corto... */
      interp->result = "1";
      return TCL_OK;
      }
    }
  else 
    {
    pos=0;
    }

  /* Lettura della lunghezza della string table secondaria. */
  val = strtab_len(ifile,&header,pos);
  if (val == -1) 
    {
    interp->result = "1";
    return TCL_OK;
    }

  /* Allocazione dell'area necessaria a contenere simboli e nomi. */
  val += header.a_syms;
  p = malloc((size_t)val);
  if (! p) 
    {
    /* ERRORE : non c'e` abbastanza memoria. */
    interp->result = "1";
    return TCL_OK;
    }

  /* Riposizionamento e lettura del blocco di dati. */
  if (TipoNm)
    pos += sizeof(header);
  else 
    pos = header.a_text + header.a_data +
          header.a_trsize + header.a_drsize +
          sizeof(header);


  if (fseek(ifile,pos,0)) {
    /* ERRORE : qualcosa e` andato storto. */
    free(p);
    interp->result = "1";
    return TCL_OK;
  }

  /* Lettura della symbol tabel + string table. */
  if (fread(p, (int) val, 1, ifile) != 1) 
    {
    /* ERRORE : sembra che il file sia troppo corto... */
    free(p);
    interp->result = "1";
    return TCL_OK;
    }

  /* Risultato : */
  NumeroSimboli = header.a_syms / sizeof(struct nlist);
  SymbolTable = (struct nlist *) p;
  TabellaStringhe = p + header.a_syms;

  fclose(ifile);

  for (i = 0; i < NumeroSimboli; ++i) 
    {
    sprintf(Risultato,"%08lx %02x %s",
          SymbolTable[i].n_value,
          SymbolTable[i].n_type,
          &TabellaStringhe[SymbolTable[i].n_un.n_strx]);
    Tcl_AppendElement(interp,Risultato);
    }

  free(p);

  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ReceiveFileCmd --
 * Con questo comando si richiede un file remoto
 *
 * Ritorna
 * 	0	: tutto OK
 *	<0	: errore
 ***********************************************************************************
 */
int ReceiveFileCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv=0;

  if (argc > 7)
    {
    interp->result = "Usage: ReceiveFile FileName RemoteDir LocalDir Hostname Login Password";
    return TCL_ERROR;
    }

  rv=receive_file(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
  sprintf(interp->result,"%d",rv);
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * TransmitFileCmd --
 * Con questo comando si trasmette un file remoto
 *
 * Ritorna
 * 	0	: tutto OK
 *	<0	: errore
 ***********************************************************************************
 */
int TransmitFileCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  int rv=0;

  if (argc > 7)
    {
    interp->result = "Usage: TransmitFile FileName RemoteDir LocalDir Hostname Login Password";
    return TCL_ERROR;
    }

  rv=transmit_file(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
  sprintf(interp->result,"%d",rv);
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ListDirCmd --
 * Con questo comando si visualizza un direttorio remoto
 *
 * Ritorna
 * 	0	: tutto OK
 *	<0	: errore
 ***********************************************************************************
 */
int ListDirCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
int	rv=0;
FILE	*fId;

  if (argc > 5)
    {
    interp->result = "Usage: ListDir RemoteDir Hostname Login Password";
    return TCL_ERROR;
    }

  if (!(fId = fopen ("/tmp/RemoteDir", "w"))) 
    {
    sprintf(interp->result,"%d",-1);
    return TCL_OK;
    }

  rv=remote_dir(argv[1],fId,argv[2],argv[3],argv[4]);
  if ( rv == 0 ) 
    sprintf(interp->result,"/tmp/RemoteDir");
  else
    sprintf(interp->result,"%d",rv);
  fclose(fId);
  return TCL_OK;
}

/* 
 ***********************************************************************************
 * ListDir2Cmd --
 * Con questo comando si visualizza un direttorio remoto
 *
 * Ritorna
 * 	0	: tutto OK
 *	<0	: errore
 ***********************************************************************************
 */
int ListDir2Cmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
char 	Risultato[100];
int	rv=0;
FILE	*fId;

  if (argc > 6)
    {
    interp->result = "Usage: ListDir TmpDir RemoteDir Hostname Login Password";
    return TCL_ERROR;
    }

  sprintf(Risultato,"%s/RemoreDir",argv[1]);
  if (!(fId = fopen (Risultato, "w"))) 
    {
    sprintf(interp->result,"%d",-1);
    return TCL_OK;
    }

  rv=remote_dir(argv[2],fId,argv[3],argv[4],argv[5]);
  if ( rv == 0 ) 
    sprintf(interp->result,"%s",Risultato);
  else
    sprintf(interp->result,"%d",rv);
  fclose(fId);
  return TCL_OK;
}

#endif

/* 
 ***********************************************************************************
 * CtclcniVerCmd --
 * Con questo comando si richiede la versione del file ctclcni
 *
 * Ritorna
 * 	0	: tutto OK
 *	<0	: errore
 ***********************************************************************************
 */
int CtclcniVerCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) 
{
  sprintf(interp->result,"1.0.0.3");
  return TCL_OK;
}

/*
#######################################################
 		REGISTRAZIONE COMANDI
#######################################################
*/

void RegistraComandi(Tcl_Interp *interp)
{
/*  Comandi per UNIX */
#ifndef WIN32
    /* 
     * Register application-specific commands.
     */
    Tcl_CreateCommand (interp, "OpenPLC", OpenPLCCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ClosePLC", ClosePLCCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ReadSegnaleScambio", ReadSegnaleScambioCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "WriteSegnaleScambio", WriteSegnaleScambioCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#ifdef DEBUG_PLC
    Tcl_CreateCommand (interp, "indirizzo_assoluto", CalcolaIndirizzoCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "CaricaUser", CaricaUserCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "watch", WatchCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "SetStartCycle", SetStartCycleCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "LeggiCRC", LeggiCRCCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif

#ifdef XNC
    Tcl_CreateCommand (interp, "OpenPLCNoBlock", OpenPLCNoBlockCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "AttachServerNoBlock", AttachServerNoBlockCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "AttachServer", AttachServerCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "DetachServer", DetachServerCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "MD_STATUS", MD_STATUSCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Read_AxesNumber", Read_AxesNumberCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "AxesServerInitialize", AxesServerInitializeCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ReadAxesValue", ReadAxesValueCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Read_mdata", Read_mdataCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Write_mdataW", Write_mdataWCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Write_mdata", Write_mdataCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Write_axesdataW", Write_axesdataWCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "Write_axesdata", Write_axesdataCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "RescanDir", RescanDirCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "SaveFileIsPossible", SaveFileIsPossibleCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "InitError", InitErrorCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "InvError", InvErrorCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif

#else

/*
	Comandi per WINDOWS
*/
    Tcl_CreateCommand (interp, "PrtInit", InitPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtFFeed", FFeedPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtStop", StopPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtAbort", AbortPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtLine", LinePrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtPoli", PoliPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtCircle", CirclePrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtArc", ArcPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtText", TextPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtBmp", BmpPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "PrtGetN", GetNPrtCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "QualeRegistro", QualeRegistroCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif

/*
	Comandi per UNIX / WINDOWS
*/

    Tcl_CreateCommand (interp, "ConvReal", ConvRealCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ReadReg", ReadRegCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#ifdef DEBUG_PLC
    Tcl_CreateCommand (interp, "watchShowRemote", WatchShowRemoteCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "watchHistoryRemote", WatchHistoryRemoteCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "CalcolaCRC", CalcolaCRCCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "tclnm", TclNmCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ReceiveFile", ReceiveFileCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "TransmitFile", TransmitFileCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ListDir", ListDirCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ListDir2", ListDir2Cmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif
    Tcl_CreateCommand (interp, "CtclcniVer", CtclcniVerCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#include "custom.c"
}
