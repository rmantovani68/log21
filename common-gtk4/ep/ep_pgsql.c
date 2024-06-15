/*--------------------------------------------*/
/* POSTGRESQL Functions                       */
/*--------------------------------------------*/
#include <libpq-fe.h>

#include <gtk-support.h>
#include <trace.h>
#include <ep-db.h>

DBconn *pgsql_connect_db(char *szHost,char *szPort,char *szDataBase)
{
	DBconn *DBconnection=(void *)NULL;
	/*
	* connessione al database
	*/
	DBconnection = (DBconn *)PQsetdb(szHost, szPort, NULL, NULL, szDataBase);
	
	/*
	* controllo correttezza della connessione
	*/
	if (PQstatus((PGconn *)DBconnection) == CONNECTION_BAD) {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' failed.", szDataBase);
		trace_debug(TRUE, TRUE, "Error : %s", PQerrorMessage((PGconn *)DBconnection));
#endif
		DBconnection=(DBconn *)NULL;
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' OK.", szDataBase);
#endif
	}
	return DBconnection;
}

DBconn *pgsql_connect_db_user(char *szHost,char *szPort,char *szDataBase,char *szUserName,char *szPassword)
{
	DBconn *DBconnection=(void *)NULL;
	/*
	* connessione al database
	*/
	DBconnection = (DBconn *)PQsetdbLogin(szHost, szPort, NULL, NULL, szDataBase, szUserName, szPassword);
	
	/*
	* controllo correttezza della connessione
	*/
	if (PQstatus((PGconn *)DBconnection) == CONNECTION_BAD) {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' failed.", szDataBase);
		trace_debug(TRUE, TRUE, "Error : %s", PQerrorMessage((PGconn *)DBconnection));
#endif
		DBconnection=(DBconn *)NULL;
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database '%s' OK.", szDataBase);
#endif
	}
	return DBconnection;
}

void pgsql_disconnect_db(DBconn *DB)
{
	PQfinish((PGconn *)DB);
}
/*
* Wrapper function for executing SQL statements
*/
DBresult *pgsql_exec(DBconn *DB,char *szQuery)
{
	return (DBresult *) PQexec ((PGconn *)DB, szQuery);
}


void pgsql_free_result(DBresult *DBRes)
{
	PQclear((PGresult *)DBRes);
}
