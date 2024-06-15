/* 	CNI_DOS.H*/
/* 	Funzioni di utilita' per portabilita' UNIX*/
/* */
#ifndef _CNI_DOS_H_
#define _CNI_DOS_H_

char 	*strupr					(char *);
char 	*strlwr					(char *);
char	*strset					(char *,int);
int		strcmpi					(char *,char *);
char 	*CD_StrNoBlank			(char *);
char 	*CD_SetPathSeparator	(char *);
char	*CD_DosFileName			(char *);
FILE 	*CD_DosFopen			(char *,char *);

#endif /* _CNI_DOS_H_ */
