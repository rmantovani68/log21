/* params.c - routines for reading and writing parameter file */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "params.h"

#define MAX_PARAMS		500
#define SEPSTR			"= "
#define COMMENTCHAR		'#'
#define SPACE			' '

typedef struct {
	char *name;		/* name == NULL for comment */
	char *value;
} PARAM;

/* Globals */
/* Set of parameters:
 * 1. Represented in text file as name = value.
 * 2. Array terminated by a name value of NULL
 * 3. Where there is no value given after the equals sign in the
 *    text file (or only blanks), the value is set to ZEROLENSTR.
 * 3. Comments have a name value of ZEROLENSTR and a value
 *    indicating the text of the coment.
 * 4. Deleted entries have a value of ZEROLENSTR in both name and value.
 */
static PARAM *params;

static char *ZEROLENSTR = "";

/****************************************************************************
  FUNCTION : int ReadParams(char *filename)
  PURPOSE : Read in a set of parameters from a file
  COMMENTS: A line starting with a blank or a COMMENTCHAR is ignored
            (source code: params.c, library: pmx_msq.a)
  PARAMETER: filename - name of file 
  RETURNS: 0: OK
		  -1: incorrect number of parameters in file
          -2: memory allocation fault
          -3: unable to open file for read
          -4: name format error (the name field contains spaces)
          >0: line number of incorrect format
****************************************************************************/
int ReadParams(char *filename)
{
	FILE	*fp;
	char	*token;
	char	*ptr;
	char	*ptr_eq;
	char	buf[81];
	char	*name;
	char	*value;
	void	*p;
	int		i = 0;
	int		len;

	if (!(fp = fopen(filename, "r")))
		return(-3);
	/* Determine no. of lines in file */
	while (fgets(buf, 80, fp)) {
		if (buf[0] != COMMENTCHAR)	/* not a comment line */
			i++;
	}
	if (i == 0 || i > MAX_PARAMS)
		return(-1);
	/* Allocate space for parameters + 0 terminator */
	if (!(p = malloc((i + 2) * sizeof(PARAM))))
		return(-2);
	params = (PARAM *)p;
	rewind(fp);
	i = 0;
	while (fgets(buf, 80, fp)) {
		if (buf[0] == COMMENTCHAR)
			continue;	/* comment line */
		len = strlen(buf);
		buf[len - 1] = '\0';
		if (!(ptr_eq = strchr(buf, SEPSTR[0]))) {
			/* A heading line */
			name = ZEROLENSTR;
			token = buf;	/* heading line */
		}
		else {
			if (!(token = strtok(buf, SEPSTR))) {	/* name */
				fclose(fp);
				return(i+1);	/* format error */
			}
			/* Check that the name does not contain spaces */
			ptr = token + strlen(token) + 1;	/* after zero at end of token */
			/* Test that there are no chars other than
			 * spaces between end of name and the equals sign
			 */
			while ((ptr < ptr_eq)) {
				if (isascii(*ptr) && isspace(*ptr))
					ptr++;
				else
					return(-4);	/* name contains embedded spaces */
			}
			if (!(name = strdup(token))) {
				fclose(fp);
				return(-2);
			}
			/* Go beyond the equals sign, ignoring any leading spaces
			 * Note that any other spaces beyond this in
			 * the value are retained (including trailing spaces)
			 */
			ptr = ++ptr_eq;		/* -> character after equals sign */
			while (*ptr == SPACE)
				ptr++;
			token = (*ptr) ? ptr : NULL;	/* NULL if end of string reached */
		}
		/* can be NULL */
		if (token) 
			value = strdup(token);
		else 
			value = ZEROLENSTR;
		
		params[i].name = name;
		params[i].value = value;
		i++;
	}
	params[i].name = params[i].value = NULL;	/* denotes end of array */
	fclose(fp);
	return(0);
}

/****************************************************************************
  FUNCTION : int WriteParams(char *filename)
  PURPOSE : Write a set of parameters to a file
  COMMENTS: (source code: params.c, library: pmx_msq.a)
  PARAMTER: filename - name of file
  RETURNS: 0: OK
          -1: parameters non-existent
          -3: unable to open file for write
****************************************************************************/
int WriteParams(char *filename)
{
	FILE	*fp;
	char	*name;
	char	*value;
	int		i;

	if (!params)
		return(-1);
	if (!(fp = fopen(filename, "w")))
		return(-3);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR)
			fprintf(fp, "%s %c", name, SEPSTR[0]);
		else if (value == ZEROLENSTR)
			continue;	/* deleted entry */
		if ((value = params[i].value))
			fprintf(fp, " %s\n", value);
	}
	fclose(fp);
	return(0);
}

/****************************************************************************
  FUNCTION : int FreeParams()
  PURPOSE : Free a set of already read-in parameters
  PARAMETERS: None
  COMMENTS: (source code: params.c, library: pmx_msq.a)
  RETURNS: 0: OK
          -1: parameters non-existent
****************************************************************************/
int FreeParams()
{
	char	*name;
	int		i;

	if (!params)
		return(-1);
	for (i=0; params[i].name; i++) {
		name = params[i].name;
		if (name != ZEROLENSTR)
			free((void *)name);
		if (params[i].value != ZEROLENSTR)
			free((void *)(params[i].value));
	}
	free((void *)params);
	params = NULL;
	return(0);
}

/****************************************************************************
  FUNCTION : char *GetParam(char *paramname)
  PURPOSE : Get a parameter from a set of already read-in parameters
  PARAMETER: paramname - name of parameter to be found
  COMMENTS: (source code: params.c, library: pmx_msq.a)
  RETURNS: !NULL: OK - -> parameter string value
            NULL: parameter not found or parameters not read in
****************************************************************************/
char *GetParam(char *paramname)
{
	int		i;
	char	*name;

	if (!params)
		return(NULL);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR) {
			if (!strcmp(name, paramname))
				return(params[i].value);
		}
	}
	return(NULL);
}

/****************************************************************************
  FUNCTION : int GetParamList(int id, PARAM_LIST *param_list)
  PURPOSE : Get a set of parameters corresponding to a parameter list
  COMMENTS: If the id is not zero then the parameter names in the
            parameter list are prefixed with the standard parameter
            prefix (see file params.h) corresponding to this id.
            (source code: params.c, library: pmx_msq.a)
  PARAMETERS: id - Greenline id of process reading parameters
              param_list - NULL-terminated list of parameters, each
                 list element contains:
                 (i)   char *: name of parameter (e.g. "baudrate")
                 (ii)  void *: -> parameter (e.g. (void *)&baudrate)
                 (iii) int: type of parameter (e.g. PARAM_INT)
  RETURNS: 0: OK,
          -1: one or more parameters not found or parameters not read in
          >0: index + 1 of first (if any) parameter not found or incorrect
****************************************************************************/
int GetParamList(int id, PARAM_LIST *param_list)
{
	int		i;
	int		error = 0;
	char	*ptr;
	char	paramlist_name[81];

	/* Check that parameters already read in */
	if (!params || !param_list)
		return(-1);
	for (i=0; param_list[i].name!=NULL; i++) {
		if (id)
			/* Insert standard parameter prefix */
			sprintf(paramlist_name, STD_PARAM, id, param_list[i].name);
		else
			/* No prefix */
			strcpy(paramlist_name, param_list[i].name);
		if ((ptr = GetParam(paramlist_name))) {
			/* Name found amongst parameters */
			switch (param_list[i].type) {
			case PARAM_INT:	/* integer parameter */
				if (sscanf(ptr, "%d", (int *)(param_list[i].value)) != 1) {
					if (!error)
						error = i + 1;
				}
				break;
			case PARAM_STRING:	/* string/char array parameter */
				strcpy((char *)(param_list[i].value), ptr);
				break;
			case PARAM_FLOAT:	/* floating point parameter */
				if (sscanf(ptr, "%f", (float *)(param_list[i].value)) != 1) {
					if (!error)
						error = i + 1;
				}
				break;
			}
		}
		else if (!error)
			error = i + 1;
	}
	return(error);
}

/****************************************************************************
  FUNCTION : int GetIntParam(char *paramname, int *intparam)
  PURPOSE : Get a decimal integer parameter from a set of read-in parameters
  COMMENTS: (source code: params.c, library: pmx_msq.a)
  PARAMETERS: paramname - name of parameter
              intparam - -> integer to receive parameter value
  RETURNS: 0: OK
          -1: parameter not found
          -2: parameters not read in
          -3: not an integer
****************************************************************************/
int GetIntParam(char *paramname, int *intparam)
{
	int		i;
	int		j;
	char	*name;

	if (!params)
		return(-2);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR) {
			if (!strcmp(name, paramname)) {
				if (sscanf(params[i].value, "%d", &j) != 1)
					return(-3);
				*intparam = j;
				return(0);
			}
		}
	}
	return(-1);
}

/****************************************************************************
  FUNCTION : int GetFloatParam(char *paramname, float *floatparam)
  PURPOSE : Get a floating point parameter from a set of read-in parameters
  COMMENTS: (source code: params.c, library: pmx_msq.a)
  PARAMETERS: paramname - name of parameter
              floatparam - -> float to receive parameter value
  RETURNS: 0: OK
          -1: parameter not found
          -2: parameters not read in
          -3: not floating point
****************************************************************************/
int GetFloatParam(char *paramname, float *floatparam)
{
	int		i;
	float	f;
	char	*name;

	if (!params)
		return(-2);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR) {
			if (!strcmp(name, paramname)) {
				if (sscanf(params[i].value, "%f", &f) != 1)
					return(-3);
				*floatparam = f;
				return(0);
			}
		}
	}
	return(-1);
}

/****************************************************************************
  FUNCTION : char *GetParamByIndex(int index, char **paramname)
  PURPOSE : Get a parameter by its index >=0
                 from a set of read-in parameters
  COMMENTS: This routine can be used to read the parameters in the order
            in which they are read from a file
            (source code: params.c, library: pmx_msq.a)
  PARAMETERS: index - index of parameter, i.e. in order of reading in
              paramname - returned -> parameter name
  RETURNS: !NULL: OK -> value string of parameter
            NULL: parameter not found or parameters not read in
****************************************************************************/
char *GetParamByIndex(int index, char **paramname)
{
	int		i;
	char	*name;

	if (!params)
		return(NULL);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (i == index) {
			*paramname = name;
			return(params[i].value);
		}
	}
	return(NULL);
}

/****************************************************************************
  FUNCTION : int SetParam(char *paramname, char *paramvalue)
  PURPOSE : Set a parameter in a set of read-in parameters
  COMMENTS: This routine used to set a parameter, which can afterwards
            be written to a file			
            (source code: params.c, library: pmx_msq.a)
  PARAMETERS: paramname - name of parameter to be set
              paramvalue - string giving value of parameter
  RETURNS: 0: OK
          -1: parameter not found
          -2: memory allocation error
          -4: name contains white spaces
****************************************************************************/
int SetParam(char *paramname, char *paramvalue)
{
	int		i;
	char	*name;
	char	*value;

	if (!params)
		return(-1);
	if (!paramname[0])
		return(-3);
	if (strchr(paramname, SPACE))
		return(-4);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR) {
			if (!strcmp(name, paramname)) {
				if ((value = params[i].value))
					free((void *)value);
				if (!(value = strdup(paramvalue)))
					return(-2);
				strcpy(value,paramvalue);
				params[i].value = value;
				return(0);
			}
		}
	}
	return(-1);
}

/****************************************************************************
  FUNCTION : int DeleteParam(char *paramname)
  PURPOSE : Delete a parameter from a set of read-in parameters.
  COMMENTS: The parameter is then not included on writing the file to disk
            (source code: params.c, library: pmx_msq.a)
  PARAMETERS: paramname - name of parameter
  RETURNS: 0: OK
          -1: parameter not found or parameters not read in
****************************************************************************/
int DeleteParam(char *paramname)
{
	int		i;
	char	*name;
	char	*value;

	if (!params)
		return(-1);
	for (i=0; (name=params[i].name)!= NULL; i++) {
		if (name != ZEROLENSTR) {
			if (!strcmp(name, paramname)) {
				free((void *)name);
				params[i].name = ZEROLENSTR;
				if ((value = params[i].value))
					free((void *)value);
				params[i].value = ZEROLENSTR;
				return(0);
			}
		}
	}
	return(-1);
}

/****************************************************************************
  FUNCTION : int AppendParam(char *paramname, char *paramvalue)
  PURPOSE : Append a parameter to a set of read-in parameters. The
            parameter is then included on writing the file to disk
  COMMENTS: A comment line is appended by making its name (paramname)
            equal to NULL or a zero-length string
            (source code: params.c, library: pmx_msq.a)
  RETURNS: 0: OK
          -1: parameter not found or parameters not read in
          -2: memory allocation error
          -3: parameter value error
          -4: parameter name contains space
          -5: parameter already exists
****************************************************************************/
int AppendParam(char *paramname, char *paramvalue)
{
	int		i;
	char	*name;
	char	*value;
	void	*p;

	if (!paramvalue)
		return(-3);
	if (strchr(paramname, SPACE))
		return(-4);
	if (params) {
		i = 0;
		while ((name = params[i].name)) {
			if (strlen(name) && !strcmp(name, paramname))
				return(-5);	/* parameter already exists */
			i++;
		}
		/* Reallocate extra memory for 1 parameter + plus NULL
		 * terminator at end of array
		 */
		if (!(p = realloc((void *)params, (i + 2) * sizeof(PARAM))))
			return(-2);
	}
	if (!params || i == 0) {
		i = 0;
		/* No params: allocate memory for 1 param + NULL terminator */
		if (!(p = malloc(2 * sizeof(PARAM))))
			return(-2);
	}
	params = (PARAM *)p;
	if (paramname && strlen(paramname)) {
		if (!(name = strdup(paramname)))
			return(-2);
	}
	else
		/* Comment line */
		name = ZEROLENSTR;
	if (!(value = strdup(paramvalue)))
		return(-2);
	strcpy(value,paramvalue);

	params[i].name = name;
	params[i].value = value;
	params[i+1].name = NULL;

	return(0);
}
