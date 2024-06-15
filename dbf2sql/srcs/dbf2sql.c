/* 
* Modulo : dbf2sql.c
* ------------------
* - Creazione di tabelle e indici postgreSQL utilizzando come modello file .cfg
* - Inserimento, aggiornamento, sostituzione di dati in tabelle, importandoli da file .dbf
*
* Progetto: Easy Picking 4.0
*
* Data creazione Maggio 2000
*
* Autore : Roberto Mantovani - Stefano Tarroni
*
* Copyright A&L srl 2003-2021
*
* 27-09-2000 rm+st : Aggiunto parametro -o per sostituzione di righe gia' esistenti.
*                    L'indice di ricerca e' il primo della lista (vedi .cfg)
* st -  Wed May 16 19:52:31 CEST 2001 : impostato il default dei campi numerici a 0
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <libpq-fe.h>
#include "dbf2sql.h"        /* struttura delle tabelle del database (file.cfg) */
#include "ca_file.h"    /* include per la gestione di un file.cfg */

#define MAX_FIELDS    512

typedef struct config { 
    bool bVerbose;    
    bool bReplaceInTable;
    bool bAddToTable;
    bool bUserID;
    bool bUserName;
    bool bPassword;
    bool bConfigFile;
    bool bDbfDataFile;
    bool bTableName;
    bool bPGHost;
    bool bPGPort;
    bool bDBName;
    bool bOverwrite;
    bool bTriggerOnInsertUpdate;
    int nUserID;
    char szConfigFile[128];
    char szDbfDataFile[128];
    char szTableName[128];
    char szPGHost[128];
    char szPGPort[128];
    char pszdbName[128];
    char pszUserName[128];
    char pszPassword[128];
    char szUserName[128];
} CFGSTRUCT, *PCFGSTRUCT;

extern char *optarg;
char szSQLCmd[4096];        /* Query da passare a postgreSQL */
PGconn *conn;                /* Connessione al DataBase */
PGresult *res;                    /* Risultato della query */
DBF_HEAD dbf_head;


/*--------------------------------------------------------*/
/*-------------- function declaration --------------------*/
/*--------------------------------------------------------*/

int main(int argc, char **argv);
void PrintUsage (char **argv);
void exit_nicely (PGconn *conn);
void set_cfg_str (PCFGSTRUCT pCfg, int argc, char **argv); 
void cfg_create_table (PCFGSTRUCT pCfg, PDBSTRUCT pDB);
void cfg_create_index (PCFGSTRUCT pCfg, PDBSTRUCT pDB);
void cfg_create_trigger (PCFGSTRUCT pCfg, PDBSTRUCT pDB);
void dbf_insert_in_table (int fd, PCFGSTRUCT pCfg, PDBSTRUCT pDB);
bool ReadFieldsStructFromDBF (int fd, PDBSTRUCT pDB);


/*--------------------------------------------------------*/
/*--------------- function definition --------------------*/
/*--------------------------------------------------------*/

void PrintUsage(char **argv)
{
    printf("\nUsage:\n");
    printf("dbf2sql [-d <database> -c <cfg_file> -f <dbf_file> -t <table_name> ...\n");
    printf("\t ... -u <user_id> -h <hostname> -p <port> -a -r -v -?]\n\n");
    printf("\t-d <Name of the destination DataBase> \n");
    printf("\t-c <Source Configuration_File> \n");
    printf("\t-f <dbf Data_File> \n");
    printf("\t-t <Name of the destination Table> - default is the name of the .CFG - \n");
    printf("\t-u <User ID> \n");
    printf("\t-U <User Name> \n");
    printf("\t-P <password> \n");
    printf("\t-h <Hostname> \n");
    printf("\t-p <Port> \n");
    printf("\t-a : Add data in existing table \n");
    printf("\t-r : Replace data in existing table \n");
    printf("\t-o : Overwrite identical rows\n");
    printf("\t-i : create additional user and time columns for insert and update triggers\n");
    printf("\t-v : Verbose sql query, date and time\n");
    printf("\t-? : Print This Help Message \n");
    printf("\nOption need for:\n");
    printf("\n\t - Only create a table        : -d -c    [-t]");
    printf("\n\t - Create and insert in table : -d -c -f [-t]");
    printf("\n\t - Add data in a table        : -d -c -f [-t] -a");
    printf("\n\t - Add data  with overwrite)  : -d -c -f [-t] -a -o");
    printf("\n\t - Replace data in table      : -d -c -f [-t] -r");
    printf("\n\n");
}

/*--------------------------------------------------------*/

void exit_nicely (PGconn *conn)    
{
    PQfinish(conn);
    exit(1);
}
    
/*--------------------------------------------------------*/
bool ReadFieldsStructFromDBF(int fd, PDBSTRUCT pDB)
{
    bool bmore_fields = TRUE;
    bool bRetValue = TRUE;
    FIELD_REC field_rec[MAX_FIELDS];
    int nFieldIndex;
    int status;
    PEP_FIELD pField;

    /* 
    * lista dei campi 
    */
    if(lseek(fd,(long)sizeof(dbf_head),0)==-1){
        fprintf(stderr,"Error seeking in database file\n");
        bRetValue = FALSE;
    }

    /* 
    * conteggio campi del record (nFieldNumber) 
    */
    nFieldIndex=0;
    while(bRetValue && bmore_fields){
        status=read(fd,(char *)&field_rec[nFieldIndex],sizeof(FIELD_REC));
        bmore_fields=(status==sizeof(FIELD_REC));
        if (bmore_fields){
            bmore_fields=(field_rec[nFieldIndex].field_name[0] != 0x0D);
            if (bmore_fields){
                pDB->pFields[nFieldIndex]=(PEP_FIELD)malloc(sizeof(FIELD));
                memset(pDB->pFields[nFieldIndex],0,sizeof(FIELD));

                pField=pDB->pFields[nFieldIndex];

                strcpy(pField->szFieldName,field_rec[nFieldIndex].field_name);
                pField->cFieldType=field_rec[nFieldIndex].field_type;
                switch(pField->cFieldType){
                    case 'D':
                        pField->nFieldLen=0;
                        pField->nFieldDec=0;
                    break;
                    case 'P':
                        pField->nFieldLen=field_rec[nFieldIndex].len_info.num_size.len;
                        pField->nFieldDec=0;
                    break;
                    case 'N':
                        pField->nFieldLen=field_rec[nFieldIndex].len_info.num_size.len;
                        pField->nFieldDec=field_rec[nFieldIndex].len_info.num_size.dec;
                    break;
                    case 'L':
                        pField->nFieldLen=0;
                        pField->nFieldDec=0;
                    break;
                    case 'C':
                        pField->nFieldLen=field_rec[nFieldIndex].len_info.char_len;
                        pField->nFieldDec=0;
                    break;
                    default:
                        fprintf(stderr,"Campo [%s] - Tipo [%c] non gestito\n",pField->szFieldName,pField->cFieldType);
                        bRetValue = FALSE;
                    break;

                }
                nFieldIndex++;
            }
        }
    }
    pDB->nFieldsNumber=nFieldIndex;

    return bRetValue;;
}

/*--------------------------------------------------------*/

/*
*    void dbf_insert_in_table (int fd, PCFGSTRUCT pCfg, PDBSTRUCT pDB)
* inserisce i dati in tabella utilizzando un file dbf
*/
void dbf_insert_in_table (int fd, PCFGSTRUCT pCfg, PDBSTRUCT pDB)
{
    int nFieldOffset;
    int nFieldIndex;
    unsigned rec_num;
    char *record;
    char szFieldData[256];
    char *pPtr;
    time_t t;
    PEP_FIELD pField;
    int nKeyFieldIndex=-1;
    char szKeyFieldData[256];

    fprintf (stdout, "\nINSERT IN TABLE %s: ",pCfg->szTableName);
    if(pCfg->bOverwrite){
        fprintf (stdout, "(WITH OVERWRITE)");
    }
    /* 
    * lista dei campi 
    */
    if(lseek(fd,(long)sizeof(dbf_head),0)==-1){
        fprintf(stderr,"Error seeking in database file\n");
        exit_nicely (conn);
    }

    if(lseek(fd,(long)dbf_head.data_offset,0)==-1){
        fprintf(stderr,"Error seeking in database file\n");
        exit_nicely (conn);
    }
    record=(char *)malloc(dbf_head.rec_size+1);
    record[dbf_head.rec_size]='\0';

    if(pCfg->bOverwrite){
        if(pDB->pIndex[0]==(PINDEX)NULL){
            fprintf(stderr,"Errore : Overwrite e Indice [0] non presente !!!\n");
            exit_nicely (conn);
        }
        for(nFieldIndex=0; nKeyFieldIndex==-1 || nFieldIndex<pDB->nFieldsNumber; nFieldIndex++){
            if(!strcmp(pDB->pFields[nFieldIndex]->szFieldName,pDB->pIndex[0]->szExpression)){
                nKeyFieldIndex=nFieldIndex;
            }
        }
        if(nKeyFieldIndex==-1){
            fprintf(stderr,"Errore : Overwrite attivo e campo indice non presente [%s]\n",pDB->pIndex[0]->szExpression);
            exit_nicely (conn);
        }
    }
    /*
    * generazione della stringa di inserimento tupla
    */    
    for(rec_num=1;rec_num<=dbf_head.last_rec;rec_num++){
        if(pCfg->bOverwrite){
            strcpy (szSQLCmd,"UPDATE " );
            strcat (szSQLCmd, pCfg->szTableName);
            strcat (szSQLCmd," SET "); 
        } else {
            strcpy (szSQLCmd,"INSERT INTO " );
            strcat (szSQLCmd, pCfg->szTableName);
            strcat (szSQLCmd," VALUES ( "); 
        }
        
        read(fd,record,dbf_head.rec_size);
        nFieldOffset=1;
        for(nFieldIndex=0; nFieldIndex<pDB->nFieldsNumber; nFieldIndex++){
            pField=pDB->pFields[nFieldIndex];
            strncpy(szFieldData, record+nFieldOffset, pField->nFieldLen);
            szFieldData[pField->nFieldLen]='\0';
            if(pCfg->bOverwrite && nFieldIndex==nKeyFieldIndex) {
                strcpy(szKeyFieldData,szFieldData);
            }
            /* sostituzione dell'apostrofo (') */
            while((pPtr=strchr(szFieldData,'\''))!=(char *)NULL){
                *pPtr = '"';
            }
            /* 
            * aggiorno il field offset 
            */
            nFieldOffset+=pField->nFieldLen;
            /*
            * inserimento della ','
            */
            strcat (szSQLCmd, nFieldIndex?", ":" ");

            /*
            * Se sono in overwrite inserisco il nome del campo prima 
            * della assegnazione a valore
            */
            if(pCfg->bOverwrite){
                strcat (szSQLCmd, pField->szFieldName);
                strcat (szSQLCmd, "=");
            }

            /* 
            * inserimento valore attributo
            */
            if(pField->cFieldType=='C'){
                strcat (szSQLCmd, "'");
            }
            /*
            * Conversione per il valori numerici
            */
            if(pField->cFieldType=='N' || pField->cFieldType=='P'){
                sprintf(szFieldData,"%0d",atoi(szFieldData));
            }
            strcat (szSQLCmd, szFieldData);

            if(pField->cFieldType=='C'){
                strcat (szSQLCmd, "'");
            }
        }
    
        /*
        * Se sono in overwrite inserisco la condizione di ricerca
        */
        if(pCfg->bOverwrite){
            strcat (szSQLCmd, " WHERE ");
            strcat (szSQLCmd, pDB->pIndex[0]->szExpression);
            strcat (szSQLCmd, " = ");

            if(pDB->pFields[nKeyFieldIndex]->cFieldType=='C'){
                strcat (szSQLCmd, "'");
            }
            /*
            * Conversione per il valori numerici
            */
            if(pDB->pFields[nKeyFieldIndex]->cFieldType=='N' || pDB->pFields[nKeyFieldIndex]->cFieldType=='P'){
                sprintf(szKeyFieldData,"%0d",atoi(szKeyFieldData));
            }
            strcat (szSQLCmd, szKeyFieldData);

            if(pDB->pFields[nKeyFieldIndex]->cFieldType=='C'){
                strcat (szSQLCmd, "'");
            }
            strcat (szSQLCmd, ";");
        } else {
            strcat (szSQLCmd, ");");
        }
    
        if (pCfg->bVerbose){
            t = time(NULL);
            fprintf (stdout, "\n%s\t- '%s'", ctime (&t), szSQLCmd);
        } else {
            /*
            * visualizzo sullo stdout un '.' ogni 100 record inseriti in tabella
            */
            if (((rec_num-1) % 100) == 0){
                fprintf(stdout,".");
                fflush(stdout);
            }
        }
        /*
        * eseguo il comando
        */
        res = PQexec(conn, szSQLCmd);
        /* 
        * controllo l'esecuzione di PQexec() 
        */
        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf (stderr, "\nINSERT IN TABLE %s: COMMAND FAILED\n",pCfg->szTableName);
            fprintf (stderr, "RECORD NUMBER[%d]\n", rec_num);
            fprintf (stderr, "SQL CMD      [%s]\n", szSQLCmd);
            fprintf (stderr, PQerrorMessage(conn));
            /*
            exit_nicely (conn);
            */
        }
        PQclear(res);
    }
    fprintf (stdout, "\n DONE : LETTI E INSERITI %d RECORDS \n\n", rec_num );
    free(record);
}



/*--------------------------------------------------------*/

/*
* void cfg_create_table (PCFGSTRUCT Cfg, PDBSTRUCT pDB)
* crea la tabella utilizzando i dati contenuti in un file .cfg
*/ 
void cfg_create_table (PCFGSTRUCT pCfg, PDBSTRUCT pDB)
{
    time_t t;
    int nFieldIndex;
    bool bOK=TRUE;

    fprintf (stdout, "\nCREATE TABLE %s:",pCfg->szTableName);
    /*
    * inserimento nome tabella
    */
    strcpy (szSQLCmd, "CREATE TABLE ");
    strcat (szSQLCmd, pCfg->szTableName);
    strcat (szSQLCmd, " (");
    
    if (pCfg->bConfigFile){
        for (nFieldIndex=0; nFieldIndex < pDB->nFieldsNumber; nFieldIndex++){
            /*
            * inserimento nome dell'attributo
            */
            strcat (szSQLCmd, (nFieldIndex==pDB->nFieldsNumber || nFieldIndex==0)?" ":", ");
            strcat (szSQLCmd, pDB->pFields[nFieldIndex]->szFieldName);
            /* 
            * inserimento tipo dell'attributo 
            */
            switch(pDB->pFields[nFieldIndex]->cFieldType){
                case 'P': /* progressivo - uso serial  */
                        strcat (szSQLCmd, " serial");
                break;
                case 'N': /* numero  */
                    if(pDB->pFields[nFieldIndex]->nFieldDec == 0){
                        strcat (szSQLCmd, " integer");
                    } else {
                        strcat (szSQLCmd, " real");
                    }
                    /*
                    * st 16-05-2001 imposto il default dei campi numerici a 0
                    */
                    strcat (szSQLCmd, " DEFAULT 0");    
                break;
                case 'C': /* carattere  */
                    strcat (szSQLCmd, " TEXT");
                break;
                case 'L': /* logico */
                    strcat (szSQLCmd, " BOOL");
                break;
                case 'D': /* data */
                    strcat (szSQLCmd, " TIMESTAMP");
                break;
                default:
                    fprintf(stderr,"Campo [%s] - Tipo [%c] non gestito\n",pDB->pFields[nFieldIndex]->szFieldName,pDB->pFields[nFieldIndex]->cFieldType);
                    bOK=FALSE;
                break;
            }
        } 

        if(pCfg->bTriggerOnInsertUpdate){
            strcat (szSQLCmd, ", ");
            strcat (szSQLCmd, "insert_user TEXT, ");
            strcat (szSQLCmd, "insert_time TIMESTAMP, ");
            strcat (szSQLCmd, "update_user TEXT, ");
            strcat (szSQLCmd, "update_time TIMESTAMP );");
        } else {
            strcat (szSQLCmd, " );");
        }
        
        if(bOK){
            if (pCfg->bVerbose){
                t = time(NULL);
                fprintf (stdout, "%s\t- '%s'\n", ctime (&t), szSQLCmd);
            }
    
            res = PQexec(conn, szSQLCmd);
            
            /* 
            * controllo l'esecuzione di PQexec() 
            */ 
            if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
                fprintf (stderr, "\nCREATE TABLE %s: COMMAND FAILED\n",pCfg->szTableName);
                fprintf (stderr, PQerrorMessage(conn));
                exit_nicely (conn);
            } else {
                fprintf (stdout, "\n DONE\n");
            }
            PQclear(res);
        }
    }
}

/*--------------------------------------------------------*/

/*
* void cfg_create_index (PCFGSTRUCT Cfg, PDBSTRUCT pDB)
* crea gli indici della tabella utilizzando un file cfg
*/ 
void cfg_create_index (PCFGSTRUCT pCfg, PDBSTRUCT pDB)
{
    time_t t;
    int i;
    char *pPtr;
    char szIndexField[512];

    /*
    *                   ! ! !   N.B.   ! ! !
    * creazione indici: al nome dell'indice viene anteposto il nome della
    * tabella per evitare indici aventi lo stesso nome all'interno del DataBase
    */
    fprintf (stdout, "\nCREATE INDEXES ON %s:\n",pCfg->szTableName);
    for (i=0; i<pDB->nTagsNumber; i++){
        strcpy (szSQLCmd, "CREATE ");
        if(pDB->pIndex[i]->nUniqueFlag){
            strcat (szSQLCmd, "UNIQUE ");
        }
        strcat (szSQLCmd, "INDEX ");
        strcat (szSQLCmd, pCfg->szTableName);
        fprintf (stdout, pCfg->szTableName);
        strcat (szSQLCmd, "_");
        fprintf (stdout, "_");
        strcat (szSQLCmd, pDB->pIndex[i]->szIndexName);
        fprintf (stdout, "%s  ", pDB->pIndex[i]->szIndexName);
        strcat (szSQLCmd, " ON ");
        strcat (szSQLCmd, pCfg->szTableName);
        strcat (szSQLCmd, " ( ");
        strcpy (szIndexField, pDB->pIndex[i]->szExpression);
        while ((pPtr=strchr(szIndexField, '+')) != (char *)NULL){
            *pPtr = ',';                        /* sostituzione del '+' con la ','*/ 
        }        
        strcat (szSQLCmd, szIndexField);
        strcat (szSQLCmd, " );");
        if(pDB->pIndex[i]->nUniqueFlag){
            fprintf (stdout, "UNIQUE");
        }
        fprintf (stdout, "\n");
        
        if (pCfg->bVerbose){
            t = time(NULL);
            fprintf (stdout, "\n%s\t- '%s'", ctime (&t), szSQLCmd);
        }
        
        fprintf (stdout, "%s\n", szSQLCmd);

        res = PQexec(conn, szSQLCmd);
        /* 
        * controllo l'esecuzione di PQexec() 
        */ 
        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf (stderr, "CREATE INDEX %s: COMMAND FAILED\n",pDB->pIndex[i]->szIndexName);
            fprintf (stderr, PQerrorMessage(conn));
            exit_nicely (conn) ;
        }
        PQclear(res);
    }
    fprintf (stdout, "DONE\n");
}

/*
* void cfg_create_trigger (PCFGSTRUCT Cfg, PDBSTRUCT pDB)
* crea il trigger della tabella utilizzando un file cfg
*/ 
void cfg_create_trigger (PCFGSTRUCT pCfg, PDBSTRUCT pDB)
{
    time_t t;
    int i;
    char *pPtr;
    char szIndexField[512];

    /*
    *                   ! ! !   N.B.   ! ! !
    * creazione trigger: al nome del trigger viene anteposto il nome della
    * tabella per evitare indici aventi lo stesso nome all'interno del DataBase
    */
    fprintf (stdout, "\nCREATE %s_update_trigger TRIGGER ON %s:\n",pCfg->szTableName,pCfg->szTableName);
    sprintf (szSQLCmd, "CREATE TRIGGER %s_update_trigger BEFORE UPDATE ON %s FOR EACH ROW EXECUTE PROCEDURE fn_update_trigger();",pCfg->szTableName,pCfg->szTableName);
        
    if (pCfg->bVerbose){
        t = time(NULL);
        fprintf (stdout, "\n%s\t- '%s'", ctime (&t), szSQLCmd);
    }
        
    res = PQexec(conn, szSQLCmd);
    /* 
    * controllo l'esecuzione di PQexec() 
    */ 
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf (stderr, "CREATE TRIGGER %s_update_trigger COMMAND FAILED\n",pCfg->szTableName);
        fprintf (stderr, PQerrorMessage(conn));
        exit_nicely (conn) ;
    }
    PQclear(res);

    fprintf (stdout, "\nCREATE %s_insert_trigger TRIGGER ON %s:\n",pCfg->szTableName,pCfg->szTableName);
    sprintf (szSQLCmd, "CREATE TRIGGER %s_insert_trigger BEFORE INSERT ON %s FOR EACH ROW EXECUTE PROCEDURE fn_insert_trigger();",pCfg->szTableName,pCfg->szTableName);
        
    if (pCfg->bVerbose){
        t = time(NULL);
        fprintf (stdout, "\n%s\t- '%s'", ctime (&t), szSQLCmd);
    }
        
    res = PQexec(conn, szSQLCmd);
    /* 
    * controllo l'esecuzione di PQexec() 
    */ 
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf (stderr, "CREATE TRIGGER %s_insert_trigger COMMAND FAILED\n",pCfg->szTableName);
        fprintf (stderr, PQerrorMessage(conn));
        exit_nicely (conn) ;
    }
    PQclear(res);

    fprintf (stdout, "DONE\n");
}

/*--------------------------------------------------------*/

/*
* settaggio dei parametri per la connessione
*/
void set_cfg_str(PCFGSTRUCT pCfg, int argc, char **argv) 
{
    int nOpt;
    char *pPtr;
    char szTableName_aux[128];    /* nome della tabella ricavato dal nome del file cfg o dbf */
    
    /*
    * Inizializzazione struttura CFG
    */
    memset (pCfg, '\0', sizeof(CFGSTRUCT) );    

    while ((nOpt = getopt(argc, argv, "U:P:c:d:f:h:p:t:u:aiorv?")) != EOF) {
        switch(nOpt){
            case 'U': /* username */
                pCfg->bUserName=TRUE;
                strcpy(pCfg->pszUserName ,optarg);
                break;
            case 'P': /* password */
                pCfg->bPassword=TRUE;
                strcpy(pCfg->pszPassword ,optarg);
                break;
            case 'a': /* add to table */
                pCfg->bAddToTable=TRUE;
                break;
            case 'c': /* Configuration File */
                pCfg->bConfigFile=TRUE;
                strcpy(pCfg->szConfigFile ,optarg);
                break;
            case 'd': /* database */
                pCfg->bDBName=TRUE;
                strcpy(pCfg->pszdbName ,optarg);
                break;
            case 'f': /* dbf data */
                pCfg->bDbfDataFile=TRUE;
                strcpy(pCfg->szDbfDataFile ,optarg);
                break;
            case 'h': /* hostname */
                pCfg->bPGHost=TRUE;
                strcpy(pCfg->szPGHost,optarg);
                break;
            case 'p': /* port */
                pCfg->bPGPort=TRUE;
                strcpy(pCfg->szPGPort, optarg);
                break;
            case 'r': /* replace data in table */
                pCfg->bReplaceInTable=TRUE;
                break;
            case 't': /* table name */
                pCfg->bTableName=TRUE;
                strcpy(pCfg->szTableName ,optarg);
                break;
            case 'u': /* User ID */
                pCfg->bUserID=TRUE;
                pCfg->nUserID = atoi(optarg);
                break;
            case 'o': /* Sostituzione righe gia' presenti */
                pCfg->bOverwrite=TRUE;
                break;
            case 'v': /* visualizza a video i comandi */
                pCfg->bVerbose=TRUE;
                break;
            case 'i': /* aggiungi campi e trigger per update insert */
                pCfg->bTriggerOnInsertUpdate=TRUE;
                break;
            case '?':
            default:
                PrintUsage(argv);
                exit(1);
                break;
        }
    }
    
    /*
    * controllo presenza delle opzioni fondamentali per
    * la connessione ((DbfData o ConfigFile) e DataBase)
    */
    if (!(pCfg->bDBName && pCfg->bConfigFile)) {
        PrintUsage(argv);
        exit(1);
    }
    
    /* 
    * se non � stato inserito, al nome della tabella
    * viene assegnato al nome del file .cfg
    */    
    if (!pCfg->bTableName){
        strcpy (szTableName_aux, pCfg->szConfigFile);
        if ((pPtr=strrchr(szTableName_aux, '.')) != (char *)NULL){
            *pPtr = '\0';    /* eliminazione dell'estensione (.dbf o .cfg)  */
        }    
        if ((pPtr=(strrchr(szTableName_aux, '/' ))) != (char *)NULL){
            strcpy(szTableName_aux, ++pPtr);    /* eliminazione del path */ 
        }
        strcpy (pCfg->szTableName, szTableName_aux);
    }
    
    /*
    * settaggio user-id
    */
    if(pCfg->bUserID){
#ifdef __linux__
        seteuid (pCfg->nUserID);
#endif
    }
}

/*--------------------------------------------------------*/
/*------------------------ MAIN --------------------------*/
/*--------------------------------------------------------*/

int main(int argc,char **argv)
{
    CFGSTRUCT Cfg;    /* The Cfg Data Struct */
    DBSTRUCT DB;
    time_t t;
    int dbf_fd=-1;                        /* file descriptor del file.dbf */
    
    if(argc==1){
        PrintUsage(argv);
        exit(1);
    }

    set_cfg_str(&Cfg, argc, argv); 

    /*
    * Gestione Configuration File
    */
    if(Cfg.bConfigFile){
        if((DB.nTagsNumber=ReadTagInfo(Cfg.szConfigFile,&DB))==-1){
            fprintf(stderr,"Error reading tags structure %s", Cfg.szConfigFile);
            exit(1);
        }

        if((DB.nFieldsNumber=ReadFieldStruct(Cfg.szConfigFile,&DB))==-1){
            fprintf(stderr,"Error reading field structure %s", Cfg.szConfigFile);
            exit(1);
        }
    }
    
    /*
    * apertura del file .dbf
    */
     if (Cfg.bDbfDataFile){
        dbf_fd=open(Cfg.szDbfDataFile, O_RDONLY);
        if (dbf_fd == -1){
            printf("Error opening %s\n", Cfg.szDbfDataFile);
            exit(1);
        }
        if (read(dbf_fd,(char *)&dbf_head,sizeof(dbf_head))!=sizeof(DBF_HEAD)) {
            printf("Read error\n");
            exit_nicely (conn);
        }
    }
    
    /*
    * connessione al database
    */
    conn = PQsetdbLogin(Cfg.szPGHost, Cfg.szPGPort, NULL, NULL, Cfg.pszdbName, Cfg.bUserName ? Cfg.pszUserName : NULL, Cfg.bPassword ? Cfg.pszPassword : NULL);
    
    /*
    * controllo correttezza della connessione
    */
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database '%s' failed.\n", Cfg.pszdbName);
        fprintf(stderr, "%s", PQerrorMessage(conn));
        exit_nicely (conn);
    } else {
        fprintf(stderr, "Connection to database '%s' OK.\n", Cfg.pszdbName);
    }

    /*
    * creazione della tabella
    */ 
    if (!(Cfg.bAddToTable || Cfg.bReplaceInTable)){
        cfg_create_table(&Cfg, &DB);
    }
    
    if (Cfg.bReplaceInTable) {
        /*
        * eliminazione delle tuple presenti
        */
        printf ("\nELIMINAZIONE RECORD PRESENTI IN TABELLA :");
        strcpy (szSQLCmd, "DELETE FROM ");
        strcat (szSQLCmd, Cfg.szTableName);
        strcat (szSQLCmd, ";");
        if (Cfg.bVerbose){
            t = time(NULL);
            fprintf (stdout, "\n%s\t- '%s'", ctime (&t), szSQLCmd);
            }
        res = PQexec(conn, szSQLCmd);
        /* 
        * controllo l'esecuzione di PQexec() 
        */ 
        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf (stderr, "DELETE FROM %s: COMMAND FAILED\n",Cfg.szTableName);
            fprintf (stderr, PQerrorMessage(conn));
            exit_nicely (conn);
        }
        PQclear(res);
        printf ("\n DONE\n");
    }
    
    /*
    * inserimento dati
    */
    if (Cfg.bDbfDataFile) {
        dbf_insert_in_table(dbf_fd, &Cfg, &DB);
        close(dbf_fd);
    }    

    /*
    * creazione degli indici
    */ 
    if (!(Cfg.bAddToTable || Cfg.bReplaceInTable)){
        cfg_create_index(&Cfg, &DB);
        printf("\n");
    }
    if(Cfg.bTriggerOnInsertUpdate){
        cfg_create_trigger(&Cfg, &DB);
        printf("\n");
    }

    PQfinish(conn);
    return 0;
}
