/*
* modulo : dbfun.c
* -----------------
* Libreria di gestione tabelle e configurazioni
*
* Progetto Picking - DB Management - PostgreSQL
*
* Data creazione 13/12/1995
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 07-07-1998 : Aggiunta compilazione condizionale per CodeBase 4.5/5.1
* rm 18-01-1999 : Aggiunta la gestione multi linea per DeAgostini
* rm 16-06-2000 : Eliminati i riferimenti a CodeBase - si usa PGSQL
* -------------   -------------------------------------------------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <libpq-fe.h>

#if defined(TRACE) || defined(TRACE_SQL)
	#include <trace.h>
#endif

#include <ep-db.h>
#include "dbfun.h"

static PGconn *DBConn;


ep_bool_t TestDataBaseConnection(ep_bool_t bReset)
{
	ep_bool_t bOK=TRUE;

	if(CONNECTION_OK!=PQstatus(DBConn)){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database is BAD trying to reconnect");
#endif
		bOK=FALSE;
		if(bReset){
			PQreset(DBConn);
		}
	}
	return bOK;
}

ep_bool_t ConnectDataBase(char *szHost,char *szPort,char *szDataBase)
{
	ep_bool_t bRetValue;
	/*
	* connessione al database
	*/
	/*
	*pDBConn = PQsetdb(Cfg.szPGHost, Cfg.szPGPort, NULL, NULL, Cfg.szPGDataBase);
	*/
	DBConn = PQsetdb(szHost, szPort, NULL, NULL, szDataBase);
	
	/*
	* controllo correttezza della connessione
	*/
	if (PQstatus(DBConn) == CONNECTION_BAD) {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' failed.", szDataBase);
		trace_debug(TRUE, TRUE, "Error : %s", PQerrorMessage(DBConn));
#endif
		bRetValue=FALSE;
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' OK.", szDataBase);
#endif
		bRetValue=TRUE;
	}
	return bRetValue;
}

void DisconnectDataBase(void)
{
	PQfinish(DBConn);
}


/*
* DBResult *DBExecQuery (ep_bool_t bTrace, char *format, ...)
* Wrapper function for SQL statements
*/
DBResult *DBExecQuery (ep_bool_t bTrace,char *format, ...)
{
	va_list ap;
	char command[4096];
	DBResult *retval;

	va_start (ap, format);
	vsprintf (command, format, ap);
	va_end (ap);
	if(bTrace){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "EXEC SQL [%s]", command);
#endif
	}
	retval = PQexec (DBConn, command);

#ifdef TRACE
	/*
	* Controllo ritorno del comando SQL
	* visualizzo comunque l'eventuale errore anche se bTrace e' FALSE
	*/
	switch(PQresultStatus(retval)){
		case PGRES_EMPTY_QUERY:
			trace_debug(TRUE, TRUE, "PGRES_EMPTY_QUERY : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_COMMAND_OK:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_COMMAND_OK : [%5d] tuples",atoi(PQcmdTuples(retval)));
		break;
		case PGRES_SINGLE_TUPLE:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_TUPLES_OK : [%5d] tuples", PQntuples (retval));
		break;
		case PGRES_TUPLES_OK:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_TUPLES_OK : [%5d] tuples", PQntuples (retval));
		break;
		case PGRES_COPY_OUT:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_COPY_OUT");
		break;
		case PGRES_COPY_BOTH:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_COPY_BOTH");
		break;
		case PGRES_COPY_IN:
			if(bTrace)trace_debug(TRUE, TRUE, "PGRES_COPY_IN");
		break;
		case PGRES_BAD_RESPONSE:
			trace_debug(TRUE, TRUE, "PGRES_BAD_RESPONSE : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_NONFATAL_ERROR:
			trace_debug(TRUE, TRUE, "PGRES_NONFATAL_ERROR : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_FATAL_ERROR:
			trace_debug(TRUE, TRUE, "PGRES_FATAL_ERROR : %s",PQresultErrorMessage(retval));
		break;
	}
#endif

	return retval;
}
