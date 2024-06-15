/**
* @file socket_server.c 
* Gestione Comunicazione via Socket
* 
* Progetto Easy Picking 4.0
*
* @author Roberto Mantovani
*
* @copyright A&L srl 2000
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
    #include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <proc_list.h>
#include <sockets.h>
#include <picking.h>

#include "cmp_time.h"
#include "socket_server.h"

int nAllMsg;
int nCarIn;
MSGHDRINT InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
char szBufTrace[80];
char *pszNomeModulo;

char szTimeBuffer[80];
char szDateBuffer[80];


CFGSTRUCT Cfg;                            /* struttura per la configurazione */
PTSCKTSRVR ptScktSrvr=(PTSCKTSRVR)NULL;   /* socket di ricezione */
int nPort = -1;                           /* TCP port */
int nMainProcess=-1;                      /* requester ID */
int nPID=0;                               /* Process ID */

CFGITEM CFGItems[] =  {
    {"General Settings", "PathTrace",  CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},  {szString:"../trace"},  CFG_NO_WIDGET, ""},
    {"socket_server_#",    "Delay",    CFG_TYPE_INT,    {pnInt:&Cfg.nDelay},          {nInt:100},             CFG_NO_WIDGET, ""},
    {"socket_server_#",    "MainID",   CFG_TYPE_INT,    {pnInt:&Cfg.nMainID},         {nInt:PROC_FINE_LINEA}, CFG_NO_WIDGET, ""},
    {NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}},
};

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
ep_bool_t ReadConfiguration(char *szCfgFileName,ep_bool_t bReadProcInfo)
{
    char szCurrentDirectory[128];

    /*
    * leggo il file di configurazione dalla directory corrente
    */
    getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
    strcpy(Cfg.szCfgFileName,szCurrentDirectory);
    strcat(Cfg.szCfgFileName,"/");
    strcat(Cfg.szCfgFileName,szCfgFileName);

    if(!FileExists(Cfg.szCfgFileName)){
        trace_debug(TRUE, TRUE, "Read Configuration : file %s not exists",szCfgFileName);
        return FALSE;
    }

    /* CFGItems viene dichiarato e inizializzato in mainvar.c */
    ReadCfgItems(CFGItems,Cfg.szCfgFileName, nPID);

    /*
    * Lettura dati processi
    */
    if(bReadProcInfo){
        ReadProcInfo(Cfg.szCfgFileName);
    }
    return TRUE;
}

int main(int argc, char **argv)
{
    pszNomeModulo = argv[0];
    GError *error = NULL;
    gint arg_pid=0;
    GOptionContext *option_context;
    GOptionEntry option_entries[] = {
        { "pid", 'p',  0, G_OPTION_ARG_INT, &arg_pid, NULL, NULL },
        { NULL }
    };

    option_context = g_option_context_new ("socket_server");

    g_option_context_add_main_entries (option_context, option_entries, NULL);

    g_option_context_add_group (option_context, NULL);
    if (!g_option_context_parse (option_context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        exit (1);
    }

    nPID = arg_pid;

    /*
    * lettura del file di configurazione
    */
    if(ReadConfiguration(__configuration_file__,TRUE)==FALSE){
        fprintf(stderr, "Cannot read configuration file [%s] ... exiting",__configuration_file__);
        exit(1);
    }

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace, "%s/%s_%d%s", Cfg.szPathTrace, g_path_get_basename (pszNomeModulo), nPID, TRACE_FILE_SUFFIX);
    open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);
#endif
    trace_debug(TRUE, TRUE, "Started");
    trace_debug(TRUE, TRUE, "Module Name     : %s",pszNomeModulo);

    /*
    * Apro la coda messaggi principale 
    */
    if((OpenProcessMsgQ(Cfg.nMainID))<0){
        trace_debug(TRUE, TRUE, "Apertura coda messaggi principale [%d] fallita", Cfg.nMainID);
    }
    /* Creo la coda messaggi locale*/
    if(CreateProcessMsgQ(nPID, 1)<0){
        trace_debug(TRUE, TRUE, "Creazione coda messaggi locale [%d] fallita", nPID);
    }


    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);
    signal(SIGINT,  SafeIntFunc);
    signal(SIGQUIT, SafeIntFunc);
    signal(SIGILL,  SafeIntFunc);
    signal(SIGKILL, SafeIntFunc);

    /*
    * avverto MAIN dello start
    */
    SendMessage(Cfg.nMainID, nPID,  PROGRAM_STARTED, NULL);

    /*
    * loop di ricezione messaggi
    */
    for(;;){
        nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsgNoWait(nPID, &nAllMsg, szInMsg))>=0){
            UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

            switch(InMsgStruct.code){
                case SOCK_OPEN_SERVICE:
                    nMainProcess=InMsgStruct.srce;

                    /*
                    * Apro la coda di chi ha chiamato
                    */
                    if((OpenProcessMsgQ(nMainProcess))<0){
                        trace_debug(TRUE, TRUE, "Apertura coda messaggi [%d] fallita", nMainProcess);
                    }

                    trace_debug(TRUE, TRUE, "Received Message SOCK_OPEN_SERVICE [%s] da [%d]", szText, nMainProcess);
                    if(ptScktSrvr==(PTSCKTSRVR)NULL){
                        nPort = atoi(szText);
                        if((ptScktSrvr = ScktSrvrOpen("", "tcp", nPort))==(PTSCKTSRVR)NULL){
                            trace_debug(TRUE, TRUE, "Error in open service : [%d]",nPort);
                            SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_OPEN_SERVICE_ERROR , szText);
                        } else {
                            SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_OPEN_SERVICE_OK , szText);
                        }
                    } else {
                        trace_debug(TRUE, TRUE, "cannot open service : already open [%s]",szText);
                        SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_OPEN_SERVICE_ALREADY_OPEN , szText);
                    }
                break;
                case SOCK_CLOSE_SERVICE:
                    nMainProcess=InMsgStruct.srce;
                    /*
                    * Apro la coda di chi ha chiamato
                    */
                    if((OpenProcessMsgQ(nMainProcess))<0){
                        trace_debug(TRUE, TRUE, "Apertura coda messaggi [%d] fallita", nMainProcess);
                    }
                    if(ptScktSrvr!=(PTSCKTSRVR)NULL){
                        if((ScktClose(&(ptScktSrvr->nSocket)))==FALSE){
                            trace_debug(TRUE, TRUE, "Error in close service [%d]", nPort);
                            SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_CLOSE_SERVICE_ERROR , szText);
                        } else {
                            trace_debug(TRUE, TRUE, "Closed service : [%d]", nPort);
                            SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_CLOSE_SERVICE_OK , szText);
                        }
                        free((void *) ptScktSrvr);
                    } else {
                        trace_debug(TRUE, TRUE, "cannot close service : not open");
                        SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_CLOSE_SERVICE_NOT_OPEN, NULL);
                    }
                break;
            }
        }
        /*
        * Loop di ricezione messaggi da socket
        */
        if(ptScktSrvr && ptScktSrvr->nSocket!=ID_SOCK_NO_FD){
            ScktSrvrGest(ptScktSrvr, GestSocket, (void *)0, 0);
        }
        usleep(Cfg.nDelay*1000);
    }     /* fine loop */
    return 0;
}

/*
* GestSocket()
* gestione dei messaggi che arrivano sul socket
*/
ep_bool_t GestSocket(int *pnFd, void *ClientData, void *CallData)
{
    char szMessage[512];
    char szCommand[256];
    char szParams[512];
    char *pPtr;
    ep_bool_t bRC;
    int nCmdIndex;


    if( *pnFd==ID_SOCK_NO_FD )
        return(FALSE);

    strcpy(szMessage, "");
    if((bRC=ScktRecv(pnFd, szMessage, sizeof(szMessage)-1, 0))){
    
        trace_debug(TRUE, TRUE, "Arrivato [%s]",szMessage);
        SendMessage(nMainProcess != -1 ? nMainProcess : Cfg.nMainID, nPID, SOCK_RECEIVED_MSG , szMessage);
    }
        
    return(TRUE);
}

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
    trace_debug(TRUE, TRUE, "Stopped");
    if(ptScktSrvr!=(PTSCKTSRVR)NULL && ptScktSrvr->nSocket!=ID_SOCK_NO_FD){
        if((ScktClose(&(ptScktSrvr->nSocket)))==FALSE){
            trace_debug(TRUE, TRUE, "Error in close service : [%s]",szText);
        }
        free((void *) ptScktSrvr);
    }

    /*
    * avverto MAIN dello stop
    */
    SendMessage(PROC_FINE_LINEA, nPID,  PROGRAM_STOPPED, NULL);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(nPID);

    exit(1);
}

