/* params.h - header file for params.c */
#ifndef _PARAMS_H
#define _PARAMS_H

#define	CONFIG_FILE			"pmx_conf.cfg"

#define PARAM_INT		0
#define PARAM_STRING	1
#define PARAM_FLOAT		2
#define STD_PARAM		"M%2.2d_%s"		/* format of standard param */

/* Used to easily access parameters */
typedef struct {
	char *name;
	void *value;
	int type;
} PARAM_LIST;

int ReadParams(char *);
int WriteParams(char *);
int FreeParams(void);
char *GetParam(char *);
int GetParamList(int, PARAM_LIST *);
int GetIntParam(char *, int *);
char *GetParamByIndex(int, char **);
int SetParam(char *, char *);
int DeleteParam(char *);
int AppendParam(char *, char *);

#endif /* _PARAMS_H */
