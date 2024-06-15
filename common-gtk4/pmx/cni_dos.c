/* 
* CNI_DOS.C
* Funzioni di utilita' per portabilita' UNIX
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#ifdef DOS_VERSION
#define	PATH_SEPARATOR		'\\'
#define	PATH TO_REPLACE		'/'
#define	SWITCH_SEPARATOR	'/'
#else
#define	PATH_SEPARATOR		'/'
#define	PATH_TO_REPLACE		'\\'
#define	SWITCH_SEPARATOR	'-'
#endif

/*
* strupr
* Converte tutti i caratteri minuscoli in maiuscoli
*/
char *strupr(char *pszStr)
{
int		nI=0;

	if (pszStr!=NULL)
	{
		while(pszStr[nI]!='\0')
		{
			pszStr[nI]=(char)toupper(pszStr[nI]);
			nI++;
		}
	}
	return(pszStr);
}

/*
* strlwr
* Converte tutti i caratteri maiuscoli in minuscoli
*/
char *strlwr(char *pszStr)
{
int		nI=0;

	if (pszStr!=NULL)
	{
		while(pszStr[nI]!='\0')
		{
			pszStr[nI]=(char)tolower(pszStr[nI]);
			nI++;
		}
	}
	return(pszStr);
}

/*
* strset
* Sostituisce i caratteri di una stringa con il carattere specificato
*/
char *strset(char *pszStr,int nCar)
{
int		nLungStr;

	if (pszStr!=NULL)
	{
		nLungStr=strlen(pszStr);
		memset(pszStr,nCar,nLungStr);
		*(pszStr+nLungStr)='\0';
	}
	return(pszStr);
}

/*
* strcmpi
* Esegua una string compare case insesitive
*/
int strcmpi(char *pszStr1,char *pszStr2)
{
int		nC1;
int		nC2;
int		nRit;

	nRit=0;
	if (pszStr1!=NULL)
	{
		if (pszStr2!=NULL)
		{
			do
			{
				nC1=tolower(*pszStr1);
				nC2=tolower(*pszStr2);
				
				if (nC1>nC2) nRit=1;
				if (nC1<nC2) nRit=-1;

				pszStr1++;
				pszStr2++;
			} while (*pszStr1!='\0' && nRit==0);

			if (*pszStr2!='\0' && nRit==0) nRit=-1;
		}
		else
			nRit=1;
	}

	return(nRit);
}

/*
* CD_StrNoBlank
* Elimina tutti blank dalla stringa
*/
char *CD_StrNoBlank(char *pszStr)
{
char	*pszLast;
int		nI=0;

	if (pszStr!=NULL)
	{
		pszLast=pszStr;
		while(pszStr[nI]!='\0')
		{
			if (pszStr[nI]!=32)
			{
				*pszLast=pszStr[nI];
				pszLast++;
			}
			nI++;
		}
		*pszLast='\0';
	}
	return(pszStr);
}

/*
* CD_SetPathSeparator
* Sostituisce i separatori path con quelli per la versione UNIX o DOS
*/
char *CD_SetPathSeparator(char *pszStr)
{
int		nI=0;

	if (pszStr!=NULL)
	{
		while(pszStr[nI]!='\0')
		{
			if (pszStr[nI]==PATH_TO_REPLACE)	 pszStr[nI]=PATH_SEPARATOR;
			nI++;
		}
	}
	return(pszStr);
}

/*
* CD_DosFileName
* Elimina eventuali blank nel nome file e rende tutti i caratteri minuscoli
* Sostituisce la '\' con '/'
*/
char *CD_DosFileName(char *pszPathName)
{
	if (pszPathName!=NULL)
	{
		strlwr(pszPathName);
		CD_StrNoBlank(pszPathName);
		CD_SetPathSeparator(pszPathName);
	}
	return(pszPathName);
}

/*
* CD_DosFopen 
* Elimina eventuali blank nel nome file e rende tutti i caratteri minuscoli
* Sostituisce la '\' con '/' 
*/
FILE *CD_DosFopen(char *pszPathName,char *pszMode)
{
char	*pszTmpName;
FILE	*fpRit=NULL;

	pszTmpName=malloc(strlen(pszPathName)+1);
	if (pszTmpName!=NULL)
	{
		strcpy(pszTmpName,pszPathName);
		strlwr(pszTmpName);
		CD_StrNoBlank(pszTmpName);
		CD_SetPathSeparator(pszTmpName);
		fpRit=fopen(pszTmpName,pszMode);
		free(pszTmpName);
	}
	return(fpRit);
}

