/**
*
* @file ca_file.c
*
* Libreria di gestione file per XNC.
*
* ****************************************************************************
* 15.02.95    MF    Corretti alcuni errori tipo free non fatte in caso di errore
*                   e sistemati i controlli sui caratteri.
* 17.02.95    MF    Inseriti test per fread fallite.
* 13.07.95    MF    La DelFileParagraph ritorna False se non trova il
*                   paragrafo.
* 20.09.95    MF    Inserite le funzioni per fare il backup di un file e la
*                   funzione per scrivere un buffer in un file (BackupFile e
*                   PutFileBuffer).
* 23.10.95    MF    Riunificate tutte le librerie.
* ****************************************************************************
*/


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "ca_file.h"


/*
 *    ****************************************************************************
 *
 *    Funzione GetFileBuffer
 *
 *    Legge il file in ingresso e ne deposita il contenuto in un buffer allocato
 *    dinamicamente.
 *
 *    char    *pszFile    -    nome del file,
 *    FILE    *pFile        -    puntatore al file.
 *
 *    Se si passa il puntatore al file viene usato questo e quindi non e'
 *    necessario passare anche il nome del file.
 *
 *    Ritorna:
 *        il puntatore al buffer allocato (bisogna fare una free quando non si
 *        usa piu').
 *
 *    ****************************************************************************
 */
char *GetFileBuffer(char *pszFile, FILE *pFile)
{
    int         nRead;
    char       *pszFileBuffer=NULL;
    long        lFileSize;
    FILE       *fp;
    bool        bMyOpen=False;
    struct stat FileStat;


    if( pFile!=NULL )
    {
        /*
         *    Puntatore al file gia' aperto
         */
        fp=pFile;
        bMyOpen=False;

        fseek(fp, 0, 0);
    }
    else
    {
        /*
         *    Apro il file per conto mio
         */
        if( (fp=fopen(pszFile, "r"))==NULL )
        {
            printf("File %s not found\r\n", pszFile);
            return(NULL);
        }
        bMyOpen=True;
    }

    /*
     *    Ricavo la dimensione del file.
     */
    fstat(fileno(fp), &FileStat);
    lFileSize=FileStat.st_size;

    if( lFileSize>0 )
    {
        /*
         *    Alloco il buffer necessario (+1 per il \0)
         */
        if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
        {

            if( bMyOpen )
                fclose(fp);
            return(NULL);
        }

        /*
         *    Leggo il file ed aggiungo un \0 in fondo.
         */
        nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
        pszFileBuffer[lFileSize]='\0';

        if( nRead!=(int)lFileSize )
        {
            free(pszFileBuffer);

            if( bMyOpen )
                fclose(fp);
            return(NULL);
        }
    }

    if( bMyOpen )
        fclose(fp);

    return(pszFileBuffer);
}    /*    Fine GetFileBuffer */


/*
 *    ****************************************************************************
 *
 *    Funzione GetBufferParagraph
 *
 *    Cerca il paragrafo pszParagraph all'interno del buffer del file e ne ritorna
 *    il puntatore.
 *
 *    char    *pszFile        -    nome del file per printf di errore,
 *    char    *pszParagraph    -    [PARAGRAFO],
 *    char    *pszFileBuffer    -    buffer del file.
 *
 *    Ritorna:
 *        puntatore al paragrafo nel buffer del file
 *
 *    ****************************************************************************
 */
char *GetBufferParagraph(
    char    *pszFile,
    char    *pszParagraph,
    char    *pszFileBuffer
)
{
    char    *pszPara;
    char    szPara[128];


    if( pszFileBuffer==NULL )
    {
        /*
         *    ERRORE puntatore a file nullo
         */
        return(NULL);
    }

    sprintf(szPara, "[%s]", pszParagraph);

    if( (pszPara=strstr(pszFileBuffer, szPara))!=NULL )
    {
        /*
         *    Paragrafo trovato.
         *
         *    19.06.95    MF    aggiunta la somma di strlen
         */
        return(pszPara+strlen(szPara));
    }

    /*
     *    Debug solo se necessario
     */
    if( pszFile!=NULL )
        printf("file %s - paragraph %s not found\r\n", pszFile, pszParagraph);

    /*
     *    Paragrafo non trovato
     */
    return(NULL);
}    /*    Fine GetBufferParagraph */


/*
 *    ****************************************************************************
 *
 *    Funzione GetBufferString
 *
 *    Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *    pszParagraph letto da pszFileBuffer.
 *    La ricerca viene iniziata dal puntatore pszParagraph.
 *
 *    char    *pszParagraph    -    puntatore al [PARAGRAFO] nel buffer
 *    char    *pszItem        -    DATO=
 *    char    *pszDefault        -    stringa di default,
 *    char    *pszBufDep        -    deposito della stringa (dato=STRINGA)
 *    int        nLen            -    lunghezza buffer di deposito (escluso \0)
 *    char    *pszFileBuffer    -    buffer del file
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool GetBufferString(
    char    *pszParagraph,
    char    *pszItem,
    char    *pszDefault,
    char    *pszBufDep,
    int        nLen,
    char    *pszFileBuffer
)
{
    int        nLunghString;
    char    *pszTmp=NULL;
    char    *pszNewLine=NULL;
    char    szElem[128];


    /*
     *    Per prima cosa metto il default
     */
    strcpy(pszBufDep, pszDefault);

    if( pszFileBuffer==NULL || pszParagraph==NULL )
        return(False);

    sprintf(szElem, "%s=", pszItem);

    pszTmp=pszParagraph;

    /*
     *    Loop sulla ricerca del LineFeed
     */
    while( *pszTmp!='\0' && (pszNewLine=strchr(pszTmp, L_F))!=NULL )
    {
        ++pszNewLine;

        /*
         *    Salto l'eventuale CarriageReturn
         */
        if( *pszNewLine==C_R )
            ++pszNewLine;

        switch( *pszNewLine )
        {
            case '[':
                /*
                 *    Il paragrafo che volevo era gia' stato trovato e quindi,
                 *    avendone trovato un altro, ho finito la ricerca.
                 */
                return(False);
            break;

            default:
                /*
                 *    Cerco il <dato>=
                 */
                if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
                {
                    /*
                     *    Trovato!
                     */
                    pszTmp=pszNewLine+strlen(szElem);

                    /*
                     *    Cerco l'eventuale 0A e calcolo quanti caratteri copiare
                     */
                    if( (pszNewLine=strchr(pszTmp, L_F))!=NULL )
                        nLunghString=min(pszNewLine-pszTmp, nLen);
                    else
                        nLunghString=min((int)strlen(pszTmp), nLen);

                    memcpy(pszBufDep, pszTmp, nLunghString);
                    pszBufDep[nLunghString]='\0';

                    return(True);
                }
            break;
        }

        pszTmp=pszNewLine;
    }

    return(False);
}    /*    Fine GetBufferString */


/*
 *    ****************************************************************************
 *
 *    Funzione GetBufferInt
 *
 *    Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *    pszParagraph letto da pszFileBuffer.
 *    La ricerca viene iniziata dal puntatore pszParagraph.
 *
 *    char    *pszParagraph    -    puntatore al [PARAGRAFO] nel buffer
 *    char    *pszItem        -    DATO=
 *    int        nDefault        -    valore di default,
 *    char    *pszFileBuffer    -    buffer del file
 *
 *    Ritorna:
 *        il numero desiderato.
 *
 *    ****************************************************************************
 */
int GetBufferInt(
    char    *pszParagraph,
    char    *pszItem,
    int        nDefault,
    char    *pszFileBuffer
)
{
    char    szBuffer[80];
    bool bRC;

    bRC=GetBufferString(
        pszParagraph,
        pszItem,
        "",
        szBuffer,
        sizeof(szBuffer)-1,
        pszFileBuffer
    );

    /*
     *    Con errore ritorno il default
     */
    if( !bRC )
        return(nDefault);

    return(atoi(szBuffer));
}    /*    Fine GetBufferInt */


/*
 *    ****************************************************************************
 *
 *    Funzione GetFileString
 *
 *    Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *    pszParagraph letto da pFile.
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    char    *pszDefault        -    valore di default con cui riempire pszBufDep
 *    char    *pszBufDep        -    deposito della stringa (dato=STRINGA)
 *    int        nLen            -    lunghezza buffer di deposito (escluso \0)
 *    char    *pszFile        -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto (lettura o
 *                                modifica).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il
 *    puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve esser
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool GetFileString(
    char    *pszParagraph,
    char    *pszItem,
    char    *pszDefault,
    char    *pszBufDep,
    int        nLen,
    char    *pszFile,
    FILE    *pFile
)
{
    int        nLunghString;
    char    *pszTmp;
    char    szPara[128];
    char    szElem[128];
    char    szBuffer[128];
    FILE    *fp;
    bool bPara=False;
    bool bMyOpen=False;


    sprintf(szPara, "[%s]", pszParagraph);
    sprintf(szElem, "%s=", pszItem);
    strcpy(pszBufDep, pszDefault);

#ifdef CASE_INSENSITIVE
    SdmStrlwr(szPara);
    SdmStrlwr(szElem);
#endif

    if( pFile!=NULL )
    {
        /*
         *    Puntatore a file gia' aperto
         */
        fp=pFile;
        bMyOpen=False;

        fseek(fp, 0, 0);
    }
    else
    {
        /*
         *    Apro il file per conto mio
         */
        if( (fp=fopen(pszFile, "r"))==NULL )
        {
            printf("File %s not found\r\n", pszFile);
            return(False);
        }
        bMyOpen=True;
    }

    /*
     *    Loop di lettura riga x riga del file
     */
    while( fgets(szBuffer, sizeof(szBuffer), fp)!=NULL )
    {
        if(
            (pszTmp=strchr(szBuffer, C_R))!=NULL ||
            (pszTmp=strchr(szBuffer, L_F))!=NULL
        )
        {
            *pszTmp='\0';
        }

        if( *szBuffer=='[' )
        {
            /*
             *    E' un paragrafo
             */

#ifdef CASE_INSENSITIVE
            /*
             *    Controllo il minuscolo del paragrafo
             */
            SdmStrlwr(szBuffer);
#endif

            if( strcmp(szPara, szBuffer)==0 )
            {
                /*
                 *    Trovato il paragrafo ricercato.
                 */
                bPara=True;
            }
            else
            {
                if( bPara )
                {
                    /*
                     *    Il paragrafo che volevo era gia' stato trovato e quindi,
                     *    avendone trovato un altro, ho finito la ricerca.
                     */
                    if( bMyOpen )
                        fclose(fp);
                    return(False);
                }
            }
        }
        else
        {
            /*
             *    Non e' un paragrafo e quindi sara' un elemento
             */
            if( bPara && (pszTmp=strchr(szBuffer, '='))!=NULL )
            {
#ifdef CASE_INSENSITIVE
                /*
                 *    I controlli vengono fatti tutti sul minuscolo
                 */
                SdmStrlwr(szBuffer);
#endif

                /*
                 *    Il paragrafo corrisponde, l'uguale e' stato trovato,
                 *    controllo che sia effettivamente quello che cerco.
                 */
                if( memcmp(szElem, szBuffer, strlen(szElem))==0 )
                {
                    /*
                     *    Trovato!!
                     */
                    ++pszTmp;
                    nLunghString=min((int)strlen(pszTmp), nLen);
                    memcpy(pszBufDep, pszTmp, nLunghString);
                    pszBufDep[nLunghString]='\0';

                    if( bMyOpen )
                        fclose(fp);
                    return(True);
                }
            }
        }
    }

    if( bMyOpen )
        fclose(fp);

    return(False);
}    /*    Fine GetFileString */


/*
 *    ****************************************************************************
 *
 *    Funzione GetFileInt
 *
 *    Ritorna l'intero a destra dell'= dell'elemento letto da file nel
 *    paragrafo specificato.
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    int        nDefault        -    valore di default da ritornare
 *    char    *szFile            -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto (lettura o
 *                                modifica).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        il valore letto
 *
 *    ****************************************************************************
 */
int GetFileInt(
    char    *pszParagraph,
    char    *pszItem,
    int        nDefault,
    char    *pszFile,
    FILE    *pFile
)
{
    char    szBuffer[80];
    bool bRC;

    bRC=GetFileString(
        pszParagraph,
        pszItem,
        "",
        szBuffer,
        sizeof(szBuffer)-1,
        pszFile,
        pFile
    );

    /*
     *    Con errore ritorno il default
     */
    if( !bRC )
        return(nDefault);

    return(atoi(szBuffer));
}    /*    Fine GetFileInt */

/*
 *    ****************************************************************************
 *
 *    Funzione PutFileString
 *
 *    Scrive nel paragrafo specificato elemento=valore
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    char    *pszValue        -    dato=VALORE
 *    char    *szFile            -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto in modifica (r+).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool PutFileString(
    char    *pszParagraph,
    char    *pszItem,
    char    *pszValue,
    char    *pszFile,
    FILE    *pFile
)
{
    int            nPos;
    int            nRead;
    int            nLenElemVal;
    char        *pszFileBuffer=NULL;
    char        *pszTmp=NULL;
    char        *pszNewLine=NULL;
    char        szPara[128];
    char        szElem[128];
    char        szElemVal[128];
    long        lFileSize;
    FILE        *fp=NULL;
    bool bMyOpen=False;
    bool bFound=False;
    struct stat    FileStat;


    sprintf(szPara, "[%s]", pszParagraph);
    sprintf(szElem, "%s=", pszItem);
    sprintf(szElemVal, "%s%s", szElem, pszValue);

    nLenElemVal=strlen(szElemVal);

    if( pFile!=NULL )
    {
        /*
         *    Utilizzo il puntatore al file gia' aperto
         */
        fp=pFile;
        bMyOpen=False;

        fseek(fp, 0, 0);
    }
    else
    {
        /*
         *    Apro il file per conto mio
         */
        if( (fp=fopen(pszFile, "r+"))==NULL )
        {
            /*
             *    File non trovato, lo apro in scrittura
             */
            if( (fp=fopen(pszFile, "w"))==NULL )
            {
                return(False);
            }
        }
        bMyOpen=True;
    }

    /*
     *    Ricavo la dimensione del file
     */
    fstat(fileno(fp), &FileStat);
    lFileSize=FileStat.st_size;

    if( lFileSize>0 )
    {
        /*
         *    Alloco il buffer (+1 per il \0)
         */
        if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
        {
            if( bMyOpen )
                fclose(fp);
            return(False);
        }

        /*
         *    Leggo il file e metto il \0 finale
         */
        nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
        pszFileBuffer[lFileSize]='\0';

        if( nRead!=(int)lFileSize )
        {
            free(pszFileBuffer);

            if( bMyOpen )
                fclose(fp);
            return(False);
        }
    }

    /*
     *    La dimensione effettiva del file non e' quella letta ma la lunghezza
     *    del buffer perche' in caso di cancellazione vengono messi degli zeri
     *    in fondo che fanno dei casini.
     */
    lFileSize=strlen(pszFileBuffer);

    if( lFileSize>0 && (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
    {
        /*
         *    Paragrafo trovato.
         */

        /*
         *    Loop sulla ricerca del LineFeed
         */
        while(
            !bFound &&
            pszTmp-pszFileBuffer<=lFileSize &&
            (pszNewLine=strchr(pszTmp, L_F))!=NULL
        )
        {
            ++pszNewLine;

            /*
             *    Salto l'eventuale CarriageReturn
             */
            if( *pszNewLine==C_R )
                ++pszNewLine;

            switch( *pszNewLine )
            {
                case '[':
                    /*
                     *    Nuovo paragrafo -> ho finito la ricerca
                     */
                    bFound=True;
                break;

                default:
                    if( isprint(*pszNewLine) )
                    {
                        /*
                         *    Cerco il <dato>=
                         */
                        if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
                        {
                            bFound=True;

                            /*
                             *    Controllo <dato>=<value>
                             */
                            if(
                                memcmp(pszNewLine, szElemVal, nLenElemVal)==0 &&
                                (*(pszNewLine+nLenElemVal)==C_R ||
                                *(pszNewLine+nLenElemVal)==L_F)
                            )
                            {
                                /*
                                 *    Trovato dato e valore uguali nello stesso
                                 *    paragrafo (DATO=VAL).
                                 */
                                if( bMyOpen )
                                    fclose(fp);
                                free(pszFileBuffer);

                                return(True);
                            }
                        }
                    }
                break;
            }

            pszTmp=pszNewLine;
        }

        if( pszNewLine==NULL )
            pszNewLine=pszFileBuffer+lFileSize;

        /*
         *    Calcolo l'offset per FSEEK
         */
        nPos=min((int)(pszNewLine-pszFileBuffer), (int)lFileSize);

        /*
         *    Mi posiziono e scrivo il dato
         */
        fseek(fp, nPos, 0);
        fputs(szElemVal, fp);
        fputs("\n", fp);

        if( nPos<(int)lFileSize )
        {
            /*
             *    Ci sono altri caratteri da accodare
             */
            if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
            {
                /*
                 *    Sono fermo sul vecchio dato
                 */
                pszNewLine=strchr(pszNewLine, L_F);
                if( pszNewLine!=NULL )
                    ++pszNewLine;
            }

            if( pszNewLine!=NULL )
            {
                fwrite(
                    pszNewLine,
                    sizeof(char),
                    lFileSize-(pszNewLine-pszFileBuffer),
                    fp
                );
            }
        }
    }
    else
    {
        /*
         *    Paragrafo non trovato, accodo il tutto.
         */
        fseek(fp, lFileSize, 0);

        fputs("\n", fp);
        fputs(szPara, fp);
        fputs("\n", fp);
        fputs(szElemVal, fp);
        fputs("\n", fp);
    }

    if( bMyOpen )
        fclose(fp);
    else
        fflush(fp);

    free(pszFileBuffer);

    return(True);
}    /*    Fine PutFileString */


/*
 *    ****************************************************************************
 *
 *    Funzione PutFileInt
 *
 *    Scrive nel paragrafo specificato elemento=numero intero.
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    int        nValue            -    dato=VALORE
 *    char    *szFile            -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto in modifica (r+).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool PutFileInt(
    char    *pszParagraph,
    char    *pszItem,
    int        nValue,
    char    *pszFile,
    FILE    *pFile
)
{
    char    szBuffer[80];
    bool bRC;

    sprintf(szBuffer, "%d", nValue);

    bRC=PutFileString(
        pszParagraph,
        pszItem,
        szBuffer,
        pszFile,
        pFile
    );

    return(bRC);
}    /*    Fine PutFileInt */


/*
 *    ****************************************************************************
 *
 *    Funzione DelFileString
 *
 *    Cancella dal paragrafo specificato la stringa elemento=valore.
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    char    *szFile            -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto in modifica (r+).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool DelFileString(
    char    *pszParagraph,
    char    *pszItem,
    char    *pszFile,
    FILE    *pFile
)
{
    int            nPos;
    int            nRead;
    int            nIndex;
    char        *pszFileBuffer=NULL;
    char        *pszTmp=NULL;
    char        *pszNewLine=NULL;
    char        szPara[128];
    char        szElem[128];
    long        lFileSize;
    FILE        *fp=NULL;
    bool bMyOpen=False;
    bool bFound=False;
    struct stat    FileStat;


    sprintf(szPara, "[%s]", pszParagraph);
    sprintf(szElem, "%s=", pszItem);

    if( pFile!=NULL )
    {
        /*
         *    Utilizzo il puntatore al file gia' aperto
         */
        fp=pFile;
        bMyOpen=False;

        fseek(fp, 0, 0);
    }
    else
    {
        /*
         *    Apro il file per conto mio
         */
        if( (fp=fopen(pszFile, "r+"))==NULL )
        {
            printf("File %s not found\r\n", pszFile);
            return(False);
        }
        bMyOpen=True;
    }

    /*
     *    Ricavo la dimensione del file
     */
    fstat(fileno(fp), &FileStat);
    lFileSize=FileStat.st_size;

    if( lFileSize<=0 )
    {
        /*
         *    Il file e' gia' vuoto, cosa cancello?
         */
        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
    {
        /*
         *    Non ottengo la memoria che mi serve.
         */
        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    /*
     *    Leggo tutto il file e metto il barra-zero di sicurezza
     */
    nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
    pszFileBuffer[lFileSize]='\0';

    if( nRead!=(int)lFileSize )
    {
        free(pszFileBuffer);

        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    if( (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
    {
        /*
         *    Paragrafo trovato.
         */

        /*
         *    Cerco il LineFeed
         */
        while(
            !bFound &&
            pszTmp-pszFileBuffer<=lFileSize &&
            (pszNewLine=strchr(pszTmp, L_F))!=NULL
        )
        {
            ++pszNewLine;

            /*
             *    Salto l'eventuale CarriageReturn
             */
            if( *pszNewLine==C_R )
                ++pszNewLine;

            switch( *pszNewLine )
            {
                case '[':
                    /*
                     *    E' finito il paragrafo perche' ne e' iniziato uno nuovo
                     *    e percio' ho finito la ricerca
                     */
                    if( bMyOpen )
                        fclose(fp);
                    free(pszFileBuffer);

                    return(False);
                break;

                default:
                    /*
                     *    Cerco il <DATO>=
                     */
                    if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
                        bFound=True;
                break;
            }

            pszTmp=pszNewLine;
        }

        if( pszNewLine==NULL )
            pszNewLine=pszFileBuffer+lFileSize;

        nPos=min((int)(pszNewLine-pszFileBuffer), (int)lFileSize);

        if( nPos<(int)lFileSize )
        {
            /*
             *    Ci sono altri caratteri da accodare
             */
            fseek(fp, nPos, 0);

            /*
             *    Sono fermo sul <dato>= e percio' vado al prossimo LF
             */
            pszNewLine=strchr(pszNewLine, L_F);

            if( pszNewLine!=NULL )
            {
                ++pszNewLine;

                /*
                 *    Salto l'eventuale CarriageReturn
                 */
                if( *pszNewLine==C_R )
                    ++pszNewLine;

                fwrite(
                    pszNewLine,
                    sizeof(char),
                    lFileSize-(pszNewLine-pszFileBuffer)-1,
                    fp
                );

                for(
                    nIndex=0;
                    nIndex<(pszNewLine-pszFileBuffer)-nPos;
                    nIndex++
                )
                {
                    /*
                     *    Scrivo zeri perche' non interferiscono con il file se
                     *    viene editato con un editor (e a me non da fastidio).
                     */
                    fputc(0x00, fp);
                }
            }
        }
    }

    if( bMyOpen )
        fclose(fp);
    else
        fflush(fp);

    free(pszFileBuffer);

    return(True);
}    /*    Fine DelFileString */


/*
 *    ****************************************************************************
 *
 *    Funzione DelFileParagraph
 *
 *    Cancella il paragrafo specificato.
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *szFile            -    nomefile
 *    FILE    *pFile            -    puntatore a file gia' aperto in modifica (r+).
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool DelFileParagraph(char *pszParagraph, char *pszFile, FILE *pFile)
{
    int            nPos;
    int            nRead;
    int            nIndex;
    char        *pszStartPara=NULL;
    char        *pszFileBuffer=NULL;
    char        *pszTmp=NULL;
    char        *pszNewLine=NULL;
    char        szPara[128];
    long        lFileSize;
    FILE        *fp=NULL;
    bool bMyOpen=False;
    bool bFound=False;
    struct stat    FileStat;


    sprintf(szPara, "[%s]", pszParagraph);

    if( pFile!=NULL )
    {
        /*
         *    Utilizzo il puntatore al file gia' aperto
         */
        fp=pFile;
        bMyOpen=False;

        fseek(fp, 0, 0);
    }
    else
    {
        /*
         *    Apro il file per conto mio
         */
        if( (fp=fopen(pszFile, "r+"))==NULL )
        {
            printf("File %s not found\r\n", pszFile);
            return(False);
        }
        bMyOpen=True;
    }

    /*
     *    Ricavo la dimensione del file
     */
    fstat(fileno(fp), &FileStat);
    lFileSize=FileStat.st_size;

    if( lFileSize<=0 )
    {
        /*
         *    Il file e' gia' vuoto, cosa cancello?
         */
        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
    {
        /*
         *    Non ottengo la memoria che mi serve.
         */
        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    /*
     *    Leggo tutto il file e metto il barra-zero di sicurezza
     */
    nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
    pszFileBuffer[lFileSize]='\0';

    if( nRead!=(int)lFileSize )
    {
        free(pszFileBuffer);

        if( bMyOpen )
            fclose(fp);
        return(False);
    }

    if( (pszStartPara=strstr(pszFileBuffer, szPara))!=NULL )
    {
        /*
         *    Paragrafo trovato.
         */
        pszTmp=pszStartPara;

        /*
         *    Cerco il LineFeed
         */
        while(
            !bFound &&
            pszTmp-pszFileBuffer<=lFileSize &&
            (pszNewLine=strchr(pszTmp, L_F))!=NULL
        )
        {
            ++pszNewLine;

            /*
             *    Salto l'eventuale CarriageReturn
             */
            if( *pszNewLine==C_R )
                ++pszNewLine;

            switch( *pszNewLine )
            {
                case '[':
                    /*
                     *    E' finito il paragrafo perche' ne e' iniziato uno nuovo
                     *    e percio' ho finito la ricerca
                     */
                    bFound=True;
                break;
            }

            pszTmp=pszNewLine;
        }

        if( pszNewLine==NULL )
            pszNewLine=pszFileBuffer+lFileSize;

        nPos=min((int)(pszStartPara-pszFileBuffer), (int)lFileSize);

        if( nPos<(int)lFileSize )
        {
            /*
             *    Ci sono altri caratteri da accodare
             */
            fseek(fp, nPos, 0);

            fwrite(
                pszNewLine,
                sizeof(char),
                lFileSize-(pszNewLine-pszFileBuffer)-1,
                fp
            );

            for(
                nIndex=0;
                nIndex<(pszNewLine-pszFileBuffer)-nPos;
                nIndex++
            )
            {
                /*
                 *    Scrivo zeri perche' non interferiscono con il file se
                 *    viene editato con un editor (e a me non da fastidio).
                 */
                fputc(0x00, fp);
            }
        }
    }

    if( bMyOpen )
        fclose(fp);
    else
        fflush(fp);

    free(pszFileBuffer);

    if( pszStartPara==NULL )
        return(False);

    return(True);
}    /*    Fine DelFileParagraph */


#ifdef PROVE_PER_SCRITTURA_SU_BUFFER
/*
 *    ****************************************************************************
 *
 *    Funzione PutBufferString
 *
 *    Scrive nel paragrafo specificato elemento=valore
 *
 *    char    *pszParagraph    -    [PARAGRAFO]
 *    char    *pszItem        -    DATO=
 *    char    *pszValue        -    dato=VALORE
 *    char    *pszFileBuffer    -    buffer del file
 *
 *    E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *    al file aperto.
 *    Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *    NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *    NULL.
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
char *PutBufferString(
    char    *pszParagraph,
    char    *pszItem,
    char    *pszValue,
    char    *pszFileBuffer
)
{
    char    *pszTmp=NULL;
    char    *pszNewLine=NULL;
    char    szPara[128];
    char    szElem[128];
    char    szElemVal[128];
    char    *pszNewFileBuffer=NULL;


    if( pszFileBuffer==NULL || pszParagraph==NULL || strlen(pszParagraph)<=0 )
    {
        return(NULL);
    }

    sprintf(szPara, "[%s]", pszParagraph);
    sprintf(szElem, "%s=", pszItem);
    sprintf(szElemVal, "%s%s", szElem, pszValue);

    if( (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
    {
        /*
         *    Paragrafo trovato.
         */

        /*
         *    Loop sulla ricerca del LineFeed
         */
        while( *pszTmp!='\0' && (pszNewLine=strchr(pszTmp, L_F))!=NULL )
        {
            ++pszNewLine;
            /*
             *    Salto l'eventuale CarriageReturn
             */
            if( *pszNewLine==C_R )
                ++pszNewLine;

            switch( *pszNewLine )
            {
                /*
                 *    Nuovo paragrafo -> ho finito la ricerca
                 */
                case '[':
                {
                    char    *pszDove;

                    pszDove=pszNewLine;

                    /*
                     *    La realloc e' da modificare
                     */
                    pszNewFileBuffer=realloc(pszFileBuffer, strlen(szElemVal));
                    if( pszNewFileBuffer==NULL )
                        return(NULL);

                    pszFileBuffer=pszNewFileBuffer;

                    memmove(
                        pszDove+strlen(szElemVal),
                        pszDove,
                        strlen(szElemVal)
                    );
                    memcpy(pszDove, szElemVal, strlen(szElemVal));
                }
                break;

                default:
                    /*
                     *    Linea vuota? (se non e' un carattere ascii)
                     */
                    if( isprint(*pszNewLine) )
                    {
                        /*
                         *    Cerco il <dato>=
                         */
                        if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
                        {
                            /*
                             *    Controllo <dato>=<value>
                             */
                            if( memcmp(pszNewLine, szElemVal, strlen(szElemVal))!=0 )
                            {
                                int        nStart=strlen(szElem);
                                int        nEnd=strlen(pszValue);
                                int        nIndex;


                                for(
                                    nIndex=0;
                                    *(pszNewLine+nStart+nIndex)!=BLANK &&
                                        isprint(*(pszNewLine+nStart+nIndex)) &&
                                            nIndex<nEnd;
                                    nIndex++
                                )
                                {
                                    *(pszNewLine+nStart+nIndex)=*(pszValue+nIndex);
                                }

                                if( nIndex<nEnd )
                                {
                                    /*
                                     *    La realloc e' da modificare
                                     */
                                    pszNewFileBuffer=realloc(
                                        pszFileBuffer,
                                        nEnd-nIndex
                                    );
                                    if( pszNewFileBuffer==NULL )
                                        return(NULL);

                                    pszFileBuffer=pszNewFileBuffer;

                                    memmove(
                                        pszNewLine+nStart+nEnd,
                                        pszNewLine+nStart+nIndex,
                                        nEnd-nIndex
                                    );

                                    memcpy(
                                        pszNewLine+nStart+nIndex,
                                        pszValue+nIndex,
                                        nEnd-nIndex
                                    );
                                }
                            }

                            return(pszFileBuffer);
                        }
                    }
                break;
            }

            pszTmp=pszNewLine;
        }
    }

    pszTmp=pszFileBuffer;
    while( *pszTmp!='\0' )
        ++pszTmp;

    /*
     *    La realloc e' da modificare
     */
    pszNewFileBuffer=realloc(pszFileBuffer, strlen(szPara)+strlen(szElemVal)+3);
    if( pszNewFileBuffer==NULL )
        return(NULL);

    pszFileBuffer=pszNewFileBuffer;

    /*
     *    Paragrafo non trovato, accodo il tutto.
     */
    strcpy(pszTmp, "\n");
    strcpy(pszTmp+strlen(pszTmp), szPara);
    strcpy(pszTmp+strlen(pszTmp), "\n");
    strcpy(pszTmp+strlen(pszTmp), szElemVal);
    strcpy(pszTmp+strlen(pszTmp), "\n");

    return(pszFileBuffer);
}    /*    Fine PutBufferString */
#endif


/*
 *    ****************************************************************************
 *
 *    Funzione PutFileBuffer
 *
 *    Scrive il file con il contenuto passato in ingresso.
 *
 *    char    *pszFile        -    nome del file
 *    char    *pszFileBuffer    -    buffer del file
 *    long    lSize            -    dimensione del buffer
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool PutFileBuffer(char *pszFile, char *pszFileBuffer, long lSize)
{
    UINT    uWrite;
    FILE    *fp;

    /*
     *    Apro il file per conto mio
     */
    if( (fp=fopen(pszFile, "w"))==NULL )
    {
        return(False);
    }

    uWrite=(UINT)fwrite(pszFileBuffer, sizeof(char), (UINT)lSize, fp);

    fclose(fp);

    if( uWrite!=(UINT)lSize )
        return(False);

    return(True);
}    /*    Fine PutFileBuffer */


/*
 *    ****************************************************************************
 *
 *    Funzione BackupFile
 *
 *    Copia il file con l'estensione in piu' ".bak".
 *
 *    char    *pszFile        -    nome del file
 *
 *    Ritorna:
 *        True    -    OK
 *        False    -    errore.
 *
 *    ****************************************************************************
 */
bool BackupFile(char *pszFile)
{
    char        *pszFileBuffer;
    char        *pszFileBak;
    bool bRC;
    struct stat    FileStat;


    pszFileBak=malloc(strlen(pszFile)+strlen(IDS_XNC_BACKUP));
    if( pszFileBak==NULL )
    {
        return(False);
    }

    /*
     *    Compongo il nome file .bak
     */
    sprintf(pszFileBak, "%s%s", pszFile, IDS_XNC_BACKUP);

    /*
     *    Ottengo il buffer del file
     */
    pszFileBuffer=GetFileBuffer(pszFile, NULL);
    if( pszFileBuffer==NULL )
    {
        free(pszFileBak);
        return(False);
    }

    /*
     *    Ricavo la dimensione del file.
     */
    stat(pszFile, &FileStat);

    /*
     *    Scrivo il nuovo file
     */
    bRC=PutFileBuffer(pszFileBak, pszFileBuffer, (long)FileStat.st_size);

    free(pszFileBak);
    free(pszFileBuffer);

    return(bRC);
}    /*    Fine BackupFile */


/*    Fine CA_FILE.C */
