/*
* modulo : dbfun.c
* -----------------
* Libreria di gestione database
*
* Progetto Picking - DB Management
*
* Data creazione 13/12/1995
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 15-05-2000 : Versione per dbf2sql (senza CB)
* -------------   -------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dbf2sql.h"
#include "ca_file.h"

/*
* ReadFieldStruct()
*
* Fields structure inizialization
* legge i dati contenuti nel file di inizializzazione e
* riempie la struttura preposta alla loro memorizzazione
*
* ritorna:
* >=0 : numero di campi allocati
*  -1 : errore in allocazione campi
*/
int ReadFieldStruct(char *szCfgFile,PDBSTRUCT pDB)
{
    PEP_FIELD pField;
    int   nFieldIndex;
    char  szBuffer[256];
    char  szFieldNumber[80];
    char  szParagraph[80];
    char  *pPtr;
    char  *szSeparator=" ,\t\n";
    char  *szDescrSeparator=",\n";

    /*
    * lettura del file di configurazione
    */
    strcpy(szParagraph,"General Settings");

    pDB->nFieldsNumber   = GetFileInt(szParagraph,"FieldsNumber", 0,szCfgFile,NULL);
    pDB->nRecordLen      = GetFileInt(szParagraph,"RecordLen",    0,szCfgFile,NULL);
    pDB->nRecordTypePos  = GetFileInt(szParagraph,"RecordTypePos",1,szCfgFile,NULL);
    pDB->nRecordTypeLen  = GetFileInt(szParagraph,"RecordTypeLen",0,szCfgFile,NULL);
    pDB->nLineNumberPos  = GetFileInt(szParagraph,"LineNumberPos",1,szCfgFile,NULL);
    pDB->nDataStart      = GetFileInt(szParagraph,"DataStart",    1,szCfgFile,NULL);
    pDB->nLinesNumber    = GetFileInt(szParagraph,"LinesNumber",  0,szCfgFile,NULL);

    GetFileString(szParagraph,"RecordType","",pDB->szRecordType,sizeof(pDB->szRecordType),szCfgFile,NULL);

    /*
    * normalizzazione dati (offset 0)
    */
    pDB->nRecordTypePos -=1;
    pDB->nLineNumberPos -=1;
    pDB->nDataStart -=1;

    nFieldIndex=0;
    strcpy(szParagraph,"Fields");
    while(nFieldIndex<pDB->nFieldsNumber){
        sprintf(szFieldNumber,"Field_%02d",nFieldIndex);
        GetFileString(szParagraph,szFieldNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
        if(strlen(szBuffer)){
            pDB->pFields[nFieldIndex]=(PEP_FIELD)malloc(sizeof(FIELD));
            memset(pDB->pFields[nFieldIndex],0,sizeof(FIELD));
            pField=pDB->pFields[nFieldIndex];
            /* 
            * Field Name 
            */
            if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
                strcpy(pField->szFieldName,pPtr);
            } else {return(-1);}
            /* 
            * Field Type
            */
            if((pPtr=strtok(NULL,szSeparator))!=NULL){
                pField->cFieldType=*pPtr;
            } else {return(-1);}
            /*
            * Field Len
            */
            if((pPtr=strtok(NULL,szSeparator))!=NULL){
                pField->nFieldLen=atoi(pPtr);
            } else {return(-1);}
            /*
            * Field Dec
            */
            if((pPtr=strtok(NULL,szSeparator))!=NULL){
                pField->nFieldDec=atoi(pPtr);
            } else {return(-1);}
            /*
            * rm 15-01-1999 : Dati per multilinea solo se LinesNumber>0
            */
            if(pDB->nLinesNumber){
                /*
                * Line Number : Numero di riga se record multilinea
                */
                if((pPtr=strtok(NULL,szSeparator))!=NULL){
                    pField->nLineNumber=atoi(pPtr);
                } else {return(-1);}
                /*
                * Field Offset : offset del campo nel file ascii (offset 1)
                */
                if((pPtr=strtok(NULL,szSeparator))!=NULL){
                    pField->nFieldOffset=atoi(pPtr);
                } else {return(-1);}
                /*
                * normalizzazione dati (offset 0)
                */
                pField->nFieldOffset -=1;
            }
            /* Field Flag  */
            if((pPtr=strtok(NULL,szSeparator))!=NULL){
                pField->nFieldFlag=atoi(pPtr);
            } else {return(-1);}
            /*
            * Field Title
            */
            if((pPtr=strtok(NULL,szDescrSeparator))!=NULL){
                strncpy(pField->szFieldTitle,pPtr,FIELD_TITLE_LEN-1);
            } else {return(-1);}
            /*
            * Field Description
            */
            if((pPtr=strtok(NULL,szDescrSeparator))!=NULL){
                strncpy(pField->szFieldDescr,pPtr,FIELD_DESCR_LEN-1);
            } else {return(-1);}
        }
        nFieldIndex++;
    }

    return nFieldIndex;
}

/*
* ReadTagInfo()
* alloca e legge i dati relativi ai tag per gli indici
*/
int ReadTagInfo(char *szCfgFile,PDBSTRUCT pDB)
{
    PINDEX pIndex;
    int   nTagIndex;
    int   nTagsNumber;
    char  szBuffer[128];
    char  szTagNumber[80];
    char  szParagraph[80];
    char  *pPtr;
    char  *szSeparator=" ,\n";
    char  *szExprSeparator="\n";
    /*
    * lettura del file di configurazione
    */
    strcpy(szParagraph,"General Settings");
    nTagsNumber=GetFileInt(szParagraph,"TagsNumber",0,szCfgFile,NULL);

    nTagIndex=0;
    strcpy(szParagraph,"Tags");
    while(nTagIndex<nTagsNumber){
        sprintf(szTagNumber,"Tag_%02d",nTagIndex);
        GetFileString(szParagraph,szTagNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
        if(strlen(szBuffer)){
            pDB->pIndex[nTagIndex]=(PINDEX)malloc(sizeof(INDEX));
            memset(pDB->pIndex[nTagIndex],0,sizeof(INDEX));
            pIndex=pDB->pIndex[nTagIndex];
            /* Nome tag */
            if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
                strcpy(pIndex->szIndexName,pPtr);
            } else {return(-1);}
            /* Order (facoltativo) */
            if((pPtr=strtok(NULL,szSeparator))!=NULL){
                pIndex->nUniqueFlag=atoi(pPtr);
            } else {return(-1);}
            /* Field Name */
            if((pPtr=strtok(NULL,szExprSeparator))!=NULL){
                strcpy(pIndex->szExpression,pPtr);
            } else {return(-1);}
        }
        nTagIndex++;
    }
    return nTagsNumber;
}


