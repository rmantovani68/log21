/*
* mainproc.c
* work procedures 
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <libpq-fe.h>

#include "interface.h"
#include "support.h"


#include <pmx_msq.h>
#include <dbfun.h>
#include <plcsock.h>
#include <proc_list.h>
#include <picking.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"
#include "callbacks.h"



/*
* Gestione Eventi (Messaggi e PLC)
*/
gint ProcessMsgs( gpointer data )
{
	int nAllMsg = 0;
	int nCarIn;
	char szInMsg[INFO_LEN];
	char szText[INFO_LEN];
	MSGHDRINT InMsgStruct;
	static BOOL bFirstTime=TRUE;

	if(bFirstTime){
		trace_debug(NULL, txt_trace ,"A&L - Easy Picking 3.0\nGestione Scarti\nApplication Ready\n");
		gtk_label_printf(get_widget(main_window,"lb_barcode_title"),"BARCODE");
		gtk_label_printf(get_widget(main_window,"lb_barcode")," ");
	}

	/*
	* Messaggi da altri processi
	*/
	while((nCarIn = ReceiveMsgNoWait(ProcList[PROC_SCARTI].nQNumber, &nAllMsg, szInMsg))>=0){
		UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

		/*
		* notifica di Start/Stop Processi
		*/
		switch (InMsgStruct.code){
			case PROGRAM_STARTED:
				trace_debug(&cGREEN, txt_trace, "%s Started", ProcList[InMsgStruct.srce].szProcName);
			break;
			case PROGRAM_STOPPED:
				trace_debug(&cGREEN, txt_trace, "%s Stopped", ProcList[InMsgStruct.srce].szProcName);
			break;
			case DISPLAY_MSG:
			{
				GdkColor *pcColor;

				switch (InMsgStruct.srce){
					default: pcColor=&cBLACK;break;
				}
				trace_debug(pcColor, txt_trace, szText);
			}
			break;
		}

		switch (InMsgStruct.srce){
			case PROC_BARCODE_SCARTI:
				switch (InMsgStruct.code){

					case BCRD_RX_BARCODE:
					{
						char  szBarcode[128];
						int nRC;

						strcpy(szBarcode,szText);
						DeleteNewLine(szBarcode);

						trace_debug( &cGREEN,txt_trace, "READ Barcode : [%s]",szBarcode);

						/* copio il dato nel buffer di memorizzazione barcode letto */
						strcpy(szReadBarcode,szBarcode);

						do_check_barcode();
					}
					break;

				}
			break;

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

