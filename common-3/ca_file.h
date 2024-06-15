/*
* ca_file.h
* include per gestione configuration files
* rm : varie modifiche per de-xncizzarlo
*/
#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define BOOL_TYPE_DEFINED
#endif

#ifndef UINT_TYPE_DEFINED
typedef unsigned int UINT;
#define UINT_TYPE_DEFINED
#endif

#ifndef MIN_MAX_DEFINED
#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))
#endif

#define True 	1
#define False 0

#define L_F					0x0A
#define C_R					0x0D

#define IDS_XNC_BACKUP		".bak"


/*
 *	****************************************************************************
 *	PROTOTIPI
 *	****************************************************************************
 */


char	*xncGetFileBuffer	(char *, FILE *);
char	*xncGetBufferParagraph	(char *, char *, char *);
BOOL	xncGetBufferString	(char *, char *, char *, char *, int, char *);
int		xncGetBufferInt		(char *, char *, int, char *);

BOOL	xncGetFileString	(char *, char *, char *, char *, int, char *, FILE *);
int		xncGetFileInt		(char *, char *, int, char *, FILE *);

BOOL	xncPutFileString	(char *, char *, char *, char *, FILE *);
BOOL	xncPutFileInt		(char *, char *, int, char *, FILE *);

BOOL	xncDelFileString	(char *, char *, char *, FILE *);
BOOL	xncDelFileParagraph	(char *, char *, FILE *);

BOOL	xncPutFileBuffer	(char *, char *, long);

BOOL	xncBackupFile		(char *);


