/*
* printstruct.h
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani / Luca Guerrini
*
* Copyright A&L srl 2003-2021
*/

typedef struct tagCfgStruct{
	char szCfgFileName[128];
	char szPathData[128];
	char szPathExe[128];
	char szPathTrace[128];
	char szPathLabel[128];
	char szPathStampe[128];

	char szTipoUbicazioni[80];
	char szTipoOrdini[80];

	int nDebugLevel;              /* versione di debug */
	int nTipoStampe;                /* usa a2ps nelle stampe */
	int nHtml2ps;                   /* usa html2ps nelle stampe */

	/*
	* Database settings
	*/
	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

	char szConnectionName[128]; 

	/*
	* Dati specifici del processo (printlabel)
	*/
	int nMainID;								/* Main ID */

} CFGSTRUCT, *PCFGSTRUCT;

