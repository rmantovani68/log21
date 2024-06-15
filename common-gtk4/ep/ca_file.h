/*
* ca_file.h
* include per gestione configuration files
* rm : varie modifiche per de-izzarlo
*/
#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(1)
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


char	*GetFileBuffer	(char *, FILE *);
char	*GetBufferParagraph	(char *, char *, char *);
bool	GetBufferString	(char *, char *, char *, char *, int, char *);
int		GetBufferInt		(char *, char *, int, char *);

bool	GetFileString	(char *, char *, char *, char *, int, char *, FILE *);
int		GetFileInt		(char *, char *, int, char *, FILE *);

bool	PutFileString	(char *, char *, char *, char *, FILE *);
bool	PutFileInt		(char *, char *, int, char *, FILE *);

bool	DelFileString	(char *, char *, char *, FILE *);
bool	DelFileParagraph	(char *, char *, FILE *);

bool	PutFileBuffer	(char *, char *, long);

bool	BackupFile		(char *);


