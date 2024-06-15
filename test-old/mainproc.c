/*
* mainproc.c
* work procedures 
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "interface.h"
#include "support.h"

#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <ca_file.h>
#include <pmx_msq.h>
#include <dbfun.h>

#include "picking.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"
#include "callbacks.h"

/*
* Gestione Eventi 
*/
gint ProcessMsgs( gpointer data )
{
	int nAllMsg = 0;
	int nCarIn;
	char szInMsg[INFO_LEN];
	char szText[INFO_LEN];
	MSGHDRINT InMsgStruct;
	static BOOL bFirstTime=TRUE;


	/*
	* Messaggi da altri processi
	*/
	while((nCarIn = ReceiveMsgNoWait(ProcList[PROC_MAIN].nQNumber, &nAllMsg, szInMsg))>=0){
		UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

		trace_out_vstr_date(1, "Msg [%d] [%s] from [%s]",InMsgStruct.code,szText?szText:"",ProcList[InMsgStruct.srce-1].szProcName);

		/*
		* notifica di Start/Stop Processi
		*/
		switch (InMsgStruct.code){
			case PROGRAM_STARTED:
				trace_out_vstr_date(1, "%s Started", ProcList[InMsgStruct.srce-1].szProcName);
			break;
			case PROGRAM_STOPPED:
				trace_out_vstr_date(1, "%s Stopped", ProcList[InMsgStruct.srce-1].szProcName);
			break;
			case DISPLAY_MSG:
			{
				GdkColor *pcColor=&cBLACK;

				gtk_text_printf(NULL,txt_msgs,szText);
			}
			break;
		}

		switch (InMsgStruct.srce-1){
		}
	}

	if(bFirstTime){
		bFirstTime=FALSE;
	}
	/* 
	* As this is a timeout function, return TRUE so that it
	* continues to get called 
	*/
	return(TRUE);
}
