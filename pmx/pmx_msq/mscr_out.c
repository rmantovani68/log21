/* mscreen_out.c - routines for managing output to ANSI multiscreens */
#include <string.h>
#include <stdio.h>
#include "msq_lib.h"
#include "colours.h"
#include "mscr_out.h"

#define NUM_MSCREEN		16
#define MAX_COLOUR		15
/* Clear screen for ANSI screen */
#define CLR_SEQ			"\033[2J"
/* Position cursor for ANSI screen */
#define CUP_SEQ			"\033[%d;%dH"

/* Structure for writing on an ANSI multiscreen */
typedef struct {
	char screen[SCR_LEN];
	char row[POS_LEN];
	char col[POS_LEN];
	char fg[CLR_LEN];
	char bg[CLR_LEN];
	char str[STR_LEN];
} MSCREEN_MSG;

/* Globals */
static FILE *mscreen_list[NUM_MSCREEN+1];

/* Module-local functions */
static void SetColor(FILE *, int, int);

/*****************************************************************************
  FUNCTION: int OpenMscreen(int screen, char *dev_name, int clear)
  PURPOSE: Open an ANSI multiscreen and note its file pointer in the list
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - screen number assigned to this multiscreen
              dev_name - device name ("/dev/tty*") of multiscreen
              clear - clear screen on opening
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int OpenMscreen(int screen, char *dev_name, int clear)
{
	if (screen < 1 || screen > NUM_MSCREEN)
		return(-1);
	/* Close any screen already opened at this position */
	if (mscreen_list[screen])
		fclose(mscreen_list[screen]);
	if (!(mscreen_list[screen] = fopen(dev_name, "w")))	/* error */
		return(-2);	/* screen could not be opened */
	if (clear)	/* clear screen after opening */
		ClearMscreen(screen);
	return(0);
}

/*****************************************************************************
  FUNCTION: int CloseMscreen(int screen)
  PURPOSE: Close an ANSI screen in the current list of multiscreens. If
           the screen 0 is given then all open screens are closed.
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - opened multiscreen to be closed
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int CloseMscreen(int screen)
{
	int		i = 0;

	if (screen == 0) {
		while (mscreen_list[i])
		{
			fclose(mscreen_list[i]);
			mscreen_list[i] = (FILE *)NULL;
			i++;
		}
		return(0);
	} else {
		if (screen < 1 || screen > NUM_MSCREEN) {
			return(-1);
		}
	}

	if (!mscreen_list[screen]) {
		return(-2);
	}

	fclose(mscreen_list[screen]);
	mscreen_list[screen] = (FILE *)NULL;
	return(0);
}

/*****************************************************************************
  FUNCTION: int ClearMscreen(int screen)
  PURPOSE: Clear an open ANSI multiscreen
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - opened multiscreen to be cleared
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int ClearMscreen(int screen)
{
	return(WriteMscreenStr(screen, CLR_SEQ));
}

/*****************************************************************************
  FUNCTION: int WriteMscreen(int screen, int row, int col,
                                     int fg, int bg, char *str)
  PURPOSE: Write to an ANSI multiscreen with position and colour
  RETURNS: 0: OK, -1: screen number incorrect, -2: screen not opened
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - multiscreen to be opened
              row - row position (1 - 80)
              col - column position (1 - 25)
              fg - foreground colour (0 - 15)
              bg - background colour (0 - 15)
              str - string to be written
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int WriteMscreen(int screen, int row, int col, int fg, int bg, char *str)
{
	int		i = 0;
	FILE	*fp;
	char	seq[20];

	if (screen == 0)
		fp = stdout;
	else if (screen < 1 || screen > NUM_MSCREEN)
		return(-1);
	if (!(fp = mscreen_list[screen]))
		return(-2);	/* multiscreen not opened */

	if (fg >= 0)	/* valid colours */
		SetColor(fp, fg, bg);
	if (col > 0) {		/* valid coordinates */
		sprintf(seq, CUP_SEQ, col, row);
		fprintf(fp, seq);
	}
	if (str && str[0])		/* valid string */
		fprintf(fp, str);
	fflush(fp);
	return(0);
}

/*****************************************************************************
  FUNCTION: int WriteMscreenStr(int screen, char *str)
  PURPOSE: Write a string to an ANSI multiscreen
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - multiscreen to be written to
              str - string to be written
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int WriteMscreenStr(int screen, char *str)
{
	return(WriteMscreen(screen, 0, 0, -1, -1, str));
}

/*****************************************************************************
  FUNCTION: int WriteMscreenColour(int screen, int fg, int bg)
  PURPOSE: Write foreground and background colours to a multiscreen
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: screen - opened multiscreen to be closed
              fg - foreground colour (0 - 15)
              bg - background colour (0 - 15)
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int WriteMscreenColour(int screen, int fg, int bg)
{
	return(WriteMscreen(screen, 0, 0, fg, bg, NULL));
}

/*****************************************************************************
  FUNCTION: int WriteMscreenMsg(char *ms_info)
  PURPOSE: Write the contents of a multiscreen message to a multiscreen
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: ms_info - buffer containing all data for multiscreen
  RETURNS: 0: OK
          -1: screen number incorrect
          -2: screen not opened
*****************************************************************************/
int WriteMscreenMsg(char *ms_info)
{
	MSCREEN_MSG	*ptr = (MSCREEN_MSG *)ms_info;
	int			screen;
	int			row;
	int			col;
	int			fg;
	int			bg;

	/* Insert screen */
	screen = GetField(ptr->screen, SCR_LEN);
	if (screen < 1 || screen > NUM_MSCREEN)
		screen = 0;	/* use stdout */

	/* Insert column */
	col = GetField(ptr->col, POS_LEN);
	/* Could get number of columns (80) from termcap here */
	if (col < 1 || col > 80)
		col = 0;	/* do not set position */
	else {
		/* Insert row */
		row = GetField(ptr->row, POS_LEN);
		if (row < 1 || row > 25)
			col = 0;	/* do not set position */
	}
	/* Insert foreground colour */
	fg = GetField(ptr->fg, CLR_LEN);
	if (fg < 0 || fg > 15)
		fg = -1;	/* do not set colour */
	else {
		/* Insert background colour */
		bg = GetField(ptr->bg, CLR_LEN);
		if (bg < 0 || bg > 15)
			fg = -1;	/* do not set colour */
	}

	return(WriteMscreen(screen, row, col, fg, bg, (char *)ptr->str));
}

/*****************************************************************************
  FUNCTION: int PackMscreenMsg(char *msg_info, int screen, int row, int col,
                                     int fg, int bg, char *str)
  PURPOSE: Pack multiscreen information into a multiscreen
           message structure
  COMMENTS: To actually send a message, do the following:
            PackMscreenMsg() to pack the mscreen info
            PackMsg() to pack the mscreen info with the message header
            SendMsg() to send the message
            (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: msg_info - buffer to receive packed message
              screen - multiscreen to be written to
              row - row position (1 - 80)
              col - column position (1 - 25)
              fg - foreground colour (0 - 15)
              bg - background colour (0 - 15)
              str - string to be written
  RETURNS: length of message information to be sent
*****************************************************************************/
int PackMscreenMsg(char *ms_info, int screen, int row, int col, int fg, int bg,
					char *str)
{
	MSCREEN_MSG	*ptr = (MSCREEN_MSG *)ms_info;
	int			len;

	/* Insert screen */
	PutField(ptr->screen, screen, SCR_LEN);

	/* Insert row */
	PutField(ptr->row, row, POS_LEN);

	/* Insert column */
	PutField(ptr->col, col, POS_LEN);

	/* Insert foreground colour */
	PutField(ptr->fg, fg, CLR_LEN);

	/* Insert background colour */
	PutField(ptr->bg, bg, CLR_LEN);

	/* Copy string into structure */
	strcpy(ptr->str, str);	/* zero-terminated */
	len = strlen(str);

	return(len+SCRHDR_LEN);	/* length of message information to be sent */
}

/*****************************************************************************
  FUNCTION: int UnpackMscreenMsg(char *msg_info, int *screen,
                  int *row, int *col, int *fg, int *bg, char *str)
  PURPOSE: Unpack multiscreen information into a parameters
  COMMENTS: (source mscr_out.c, library: pmx_lib.a)
  PARAMETERS: msg_info - buffer to receive packed message
              screen - multiscreen to be written to
              row - row position (1 - 80)
              col - column position (1 - 25)
              fg - foreground colour (0 - 15)
              bg - background colour (0 - 15)
              str - string to be written
  RETURNS: length of string to be printed
*****************************************************************************/
int UnpackMscreenMsg(char *ms_info, int *screen, int *row, int *col, int *fg,
						int *bg, char *str, int len)
{
	MSCREEN_MSG	*ptr = (MSCREEN_MSG *)ms_info;
	int			str_len;

	/* Insert screen */
	*screen = GetField(ptr->screen, SCR_LEN);

	/* Insert row */
	*row = GetField(ptr->row, POS_LEN);

	/* Insert column */
	*col = GetField(ptr->col, POS_LEN);

	/* Insert foreground colour */
	*fg = GetField(ptr->fg, CLR_LEN);

	/* Insert background colour */
	*bg = GetField(ptr->bg, CLR_LEN);

	/* Copy string into structure */
	str_len = len - SCRHDR_LEN;
	if (str_len > 0)
		memcpy(str, ptr->str, str_len);
	else
		str_len = 0;
	str[str_len] = '\0';

	return(str_len);	/* length of string to be printed */
}

/*****************************************************************************
   FUNCTION: static void SetColor(FILE *stream, int fg, int bg)
   PURPOSE: Set the colour of an ANSI multiscreen through its file pointer
   COMMENTS: The foreground and background colours are each set for
			 normal, reverse and graphic modes.
   PARAMETERS: stream - stream pointer to multiscreen
               fg - foreground colour (0 - 15)
               bg - background colour (0 - 15)
*****************************************************************************/
static void SetColor(FILE *stream, int fg, int bg)
{
	fprintf(stream, SET_SEQ, fg, SET_FGROUND);
	fprintf(stream, SET_SEQ, bg, SET_BGROUND);
	fprintf(stream, SET_SEQ, fg, SET_REV_FGROUND);
	fprintf(stream, SET_SEQ, bg, SET_REV_BGROUND);
	fprintf(stream, SET_SEQ, fg, SET_GRA_FGROUND);
	fprintf(stream, SET_SEQ, bg, SET_GRA_BGROUND);
	fflush(stream);
}
