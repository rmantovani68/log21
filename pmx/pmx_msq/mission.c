/* mission.c - mission handling routines */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>	/* contains prototype of access() */

#include "mission.h"

/* Extension for mission file names (e.g. "filename.ext") */
#define MISSION_EXT			"mis"
#define MAX_NUM_MISSIONS	40

/* general buffer */
static char sbuf[121];

/* +++++ Mission Variables +++++++ */
/* Number of missions */
static int num_missions;
/* Max. number of lines */
static int mission_num_lines;
/* Size of line */
static int mission_size_line;
/* Array of mission names */
static char *mission_name[MAX_NUM_MISSIONS];
/* Array containing missions */
static MISSION *mission[MAX_NUM_MISSIONS];
/* User-supplied mission display function */
void (*DisplayMission)(int);

/****************************************************************************
  FUNCTION: int SetMissionParams(int num, int num_lines, int size_line,
               char *dir, char *base_name, void (*display_function)(int))
  PURPOSE: Setup mission array using dimensions given and prepare
           system for using the missions
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: num - number of missions
             num_lines - max. number of mission lines
             size_line - size in bytes of a line
             dir - directory where missions are to be held
             base_name - base name of missions
             display_function - user-defined function for displaying a
			                    mission, takes the mission index as a
                                parameter
  RETURN_VALUE: 0: OK
               -1: malloc error
               -2: missions already set
               -3: too many missions
****************************************************************************/
int SetMissionParams(int num, int num_lines, int size_line, char *dir,
					char *base_name, void(*display_function)(int))
{
	int		i;
	int		size_mission;
	char	*missions;
	char	*mission_names;

	if (mission[0])
		return(-2);	/* missions have already been set */
	if (num > MAX_NUM_MISSIONS)
		return(-3);	/* too many missions */
	size_mission = sizeof(MISS_STAT) + num_lines * size_line;
	/* Allocate space for all mission names */
	if (!(mission_names = (char *)malloc((MISS_LEN_PATH + 1) * num)))
		return(-1);
	/* Allocate space for all missions */
	if (!(missions = (char *)malloc(size_mission * num)))
		return(-1);
	/* Set global statics */
	num_missions = num;
	mission_num_lines = num_lines;
	mission_size_line = size_line;
	/* Allocate heap space for missions */
	for (i=0; i<num_missions; i++) {
		/* Initialize mission name */
		mission_name[i] = mission_names + (i * MISS_LEN_PATH + 1);
		sprintf(mission_name[i], "%s/%s%d.%s",
				dir, base_name, i, MISSION_EXT);
		/* Initialize mission pointer */
		mission[i] = (MISSION *)(missions + (i * size_mission));
	}
	/* Initialize mission display function pointer */
	DisplayMission = display_function;
	return(0);
}

/****************************************************************************
  FUNCTION: int SetMissionStatus(int index, int status)
  PURPOSE: Set the status of a mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
             status - mission status to be set
  RETURN VALUE: >=0: OK
                 -1: index out of range 
                 -2: incorrect value for status
****************************************************************************/
int SetMissionStatus(int index, int status)
{
	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	if (status > MISSION_INIT || status < MISSION_NOTEXIST)
		return(-2);	/* status out of range */
	(mission[index])->mission_stat.status = status;
	return(0);
}

/****************************************************************************
  FUNCTION: int TouchMission(int index)
  PURPOSE: Set the mission status flags for save and display
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
  RETURN VALUE: >=0: OK
                 -1: index out of range 
****************************************************************************/
int TouchMission(int index)
{
	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	(mission[index])->mission_stat.save =
		(mission[index])->mission_stat.mod = 1;
	return(0);
}

/****************************************************************************
  FUNCTION: int SetMissionNumLines(int index, int num_lines)
  PURPOSE: Set the number of lines in a mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
             num_lines - number of lines to set
  RETURN VALUE: >=0: OK
                 -1: index out of range 
****************************************************************************/
int SetMissionNumLines(int index, int num_lines)
{
	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	(mission[index])->mission_stat.num_lines = num_lines;
	return(0);
}

/****************************************************************************
  FUNCTION: int GetMissionStatus(int index)
  PURPOSE: Obtain the status of a mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
  RETURN VALUE: >=0: OK
                 -1: index out of range 
****************************************************************************/
int GetMissionStatus(int index)
{
	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	return((mission[index])->mission_stat.status);
}

/****************************************************************************
  FUNCTION: int GetMissionNumLines(int index)
  PURPOSE: Obtain the number of lines in a mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
  RETURN VALUE: >=0: OK
                 -1: index out of range 
****************************************************************************/
int GetMissionNumLines(int index)
{
	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	return((mission[index])->mission_stat.num_lines);
}

/****************************************************************************
  FUNCTION: char *GetMissionLine(int index, int line)
  PURPOSE: Obtain a char pointer to a mission line
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission
             line - line number within mission
  RETURN VALUE: !=NULL: OK
                  NULL: index out of range or line out of range
****************************************************************************/
char *GetMissionLine(int index, int line)
{
	if (index < 0 || index >= num_missions)
		return(NULL);	/* index out of range */
	if (line < 0 || line >= (mission[index])->mission_stat.num_lines)
		return(NULL);	/* line index out of range */
	return(&(mission[index]->mission_line) + (line * mission_size_line));
}

/****************************************************************************
  FUNCTION: int AccessMission(int index)
  PURPOSE: Test if mission present
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission in mission list
  RETURN VALUE: 0: OK
                1: index out of range
               -1: access error
****************************************************************************/
int AccessMission(int index)
{
	if (index < 0 || index >= num_missions)
		return(1);
	return(access(mission_name[index], 0));
}

/****************************************************************************
  FUNCTION: void ResetMissions()
  PURPOSE: Reset all missions
  WHERE: (source: mission.c, library: pmx_msq.a)
****************************************************************************/
void ResetMissions()
{
	int i;

	for (i=0; i<num_missions; i++)
		ResetMissionState(i);
}

/****************************************************************************
  FUNCTION: int ResetMissionState(int index)
  PURPOSE: Reset state fields of a mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission in mission list
  RETURNS:  0: OK
           -1: index out of range
****************************************************************************/
int ResetMissionState(int index)
{
	MISSION	*missp;

	if (index < 0 || index >= num_missions)
		return(-1);
	missp = mission[index];
	/* Reset mission status structures */
	SetMissionStatus(index, MISSION_NOTEXIST);
	missp->mission_stat.mod = missp->mission_stat.save =
		missp->mission_stat.num_lines = missp->mission_stat.orig_lines = 0;
}

/****************************************************************************
  FUNCTION: int DeleteMission(int index)
  PURPOSE: Delete mission from system
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission in mission list
  RETURNS:  0: OK
           -1: index out of range
****************************************************************************/
int DeleteMission(int index)
{
	if (index < 0 || index >= num_missions)
		return(-1);
	/* Initialize structure in memory */
	ResetMissionState(index);
	/* Delete file */
	unlink(mission_name[index]);
}

/****************************************************************************
  FUNCTION: int ReadMission(int index)
  PURPOSE: Read a mission from disk into memory
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission in mission list
  RETURN VALUES:  0: OK
                 -1: index out of range
                 -2: file open error
                 -3: mission status read error
                 -4: mission lines read error
                 -5: mission has no lines
****************************************************************************/
int ReadMission(int index)
{
	int		i;
	int		err = 0;
	FILE	*fp;
	MISSION	*missp;

	if (index < 0 || index >= num_missions)
		return(-1);
	missp = mission[index];
	/* Open mission file */
	if ((fp = fopen(mission_name[index], "r")) == (FILE *)NULL)
		err = -2;
	else {
		/* Read mission status */
		if (fread((char *)missp, sizeof(MISS_STAT), 1, fp) != 1) {
			err = -3;
			fclose(fp);
		}
		else {
			/* Read mission lines */
			if ((i = missp->mission_stat.num_lines)) {
				if (fread(&(missp->mission_line),
						i * mission_size_line, 1, fp) != 1)
					err = -4;
			}
			else
				err = -5;
		}
		fclose(fp);
	}
	/* If no error, set to display */
	if (!err)
		missp->mission_stat.mod = 1;
	return(err);
}

/****************************************************************************
  FUNCTION: int WriteMission(int index)
  PURPOSE: Write a mission from memory to disk
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission in mission list
  RETURNS:  0: OK
           -1: index out of range
           -2: file open error
           -3: file write error
****************************************************************************/
int WriteMission(int index)
{
	int		len;
	int		err = 0;
	FILE	*fp;
	MISSION	*missp;

	if (index < 0 || index >= num_missions)
		return(-1);

	missp = mission[index];
	/* Open mission file */
	if ((fp = fopen(mission_name[index], "w")) == (FILE *)NULL)
		return(-2);
	/* Write whole mission structure */
	len = sizeof(MISS_STAT) +
				missp->mission_stat.num_lines * mission_size_line;
	if (fwrite((char *)missp, len, 1, fp) != 1) {
		fclose(fp);
		return(-3);
	}
	fclose(fp);
	return(0);
}

/****************************************************************************
  FUNCTION: int MoveMission(int to, int from, int move_type)
  PURPOSE: Move a mission or a line of it to another mission
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: to - index of mission to move to (mission-to)
             from - index of mission to move from (mission-from)
             move_type - MOVE_ALL:  move complete mission or
                         MOVE_LINE: move only first line
  RETURNS:  0: OK
          |=1: OK, mission-to is a new mission
          |=2: OK, mission-from no longer exists
           -1: index too high
           -2: mission-from has no lines
           -3: (MOVE_ALL): to-mission was not empty
                           (move was still performed)
****************************************************************************/
int MoveMission(int to, int from, int move_type)
{
	int		rc;
	int		len;
	char	*ml_to;
	char	*ml_from;
	MISSION	*missp_to;
	MISSION	*missp_from;

	if (to < 0 || to >= num_missions ||
			from < 0 || from >= num_missions)
		return(-1);	/* index range error */

	missp_from = mission[from];	/* mission to move from */
	if (missp_from->mission_stat.num_lines == 0)
		return(-2); /* there are no lines to be moved */

	rc = 0;
	missp_to = mission[to];	/* mission to move to */
	switch (move_type) {
	case MOVE_ALL:	/* Move whole mission */
		/* Note if to-mission is not empty, but carry on */
		if (GetMissionStatus(to) != MISSION_NOTEXIST)
			rc = -3;	/* mission does not exist */
		/* Determine length of mission to be moved */
		len = sizeof(MISS_STAT) +
				missp_from->mission_stat.num_lines * mission_size_line;
		/* Transfer from-mission to to-mission */
		memcpy((char *)missp_to, (char *)missp_from, len);
		/* Set relevant status fields */
		SetMissionStatus(to, MISSION_INIT);	/* now exists */
		TouchMission(to);
		/* Eliminate original mission, as it has been transferred */
		DeleteMission(from);
		break;

	case MOVE_LINE:
		/* If the to-mission does not exist then we must create
		 * its header as well
		 */
		if (GetMissionStatus(to) == MISSION_NOTEXIST) {
			/* Note new to-mission */
			rc |= 1;
			ResetMissionState(to);
			SetMissionStatus(to, MISSION_INIT);
			missp_to->mission_stat.num_lines = 0;
			missp_to->mission_stat.orig_lines = 1;	/* not important */
		}
		/* To-mission must be saved and redisplayed */
		TouchMission(to);

		/* Move first line of from-mission to end of to-mission */
		ml_to = &(missp_to->mission_line);
		ml_from = &(missp_from->mission_line);
		memcpy(ml_to + (missp_to->mission_stat.num_lines * mission_size_line),
				ml_from, mission_size_line);
		/* Increment number of panels in to-mission */
		missp_to->mission_stat.num_lines++;
		/* Decrement number of panels in from-mission */
		missp_from->mission_stat.num_lines--;
		/* Check if from-mission only has at least one line left */
		if (missp_from->mission_stat.num_lines) {
			/* Shift up from-mission (memmove() allows for overlap) */
			memmove(ml_from, ml_from + mission_size_line,
				missp_from->mission_stat.num_lines * mission_size_line);
			/* Ensure save of from-mission to disk */
			missp_from->mission_stat.save++;
			/* Ensure redisplay of from-mission */
			missp_from->mission_stat.mod++;
		}
		else {
			/* Note no more from-mission */
			rc |= 2;
			DeleteMission(from);
		}
		break;
	}
	/* OK */
	return(rc);
}

/****************************************************************************
  FUNCTION: int ShowMission(int index)
  PURPOSE: Cause a mission to be displayed using the user-defined routine
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission to be shown
  RETURNS:  0: OK or not marked as to be shown
           -1: index out of range
           -3: mission does not exist
****************************************************************************/
int ShowMission(int index)
{
	MISSION	*missp;

	if (index < 0 || index >= num_missions)
		return(-1);
	missp = mission[index];
	if (GetMissionStatus(index) == MISSION_NOTEXIST)
		return(-3);
	if (missp->mission_stat.mod) {
		missp->mission_stat.mod = 0;
		/* User-supplied function */
  		DisplayMission(index);
	}
	return(0);
}

/****************************************************************************
  FUNCTION: void ShowMissions()
  PURPOSE: Cause all missions to be displayed, using the user-defined
           display function.
  WHERE: (source: mission.c, library: pmx_msq.a)
****************************************************************************/
void ShowMissions()
{
	int		nIndex;

	for (nIndex=0; nIndex<num_missions; nIndex++)
		ShowMission(nIndex);
}

/****************************************************************************
  FUNCTION: int SaveMission(int index)
  PURPOSE: Save a marked mission to disk
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: index - index of mission to be shown
  RETURNS:  0: OK or not marked as to be saved
           -1: index out of range
           -2: file open error
           -3: file write error
           -4: (internal error) mission has no lines
****************************************************************************/
int SaveMission(int index)
{
	MISSION	*missp;

	if (index < 0 || index >= num_missions)
		return(-1);	/* index out of range */
	missp = mission[index];
	if (missp->mission_stat.save) {
		if (missp->mission_stat.num_lines == 0)
			return(-2);	/* internal error in mission structure */
		missp->mission_stat.save = 0;
		return(WriteMission(index));
	}
	return(0);
}

/****************************************************************************
  FUNCTION: void SaveMissions(void)
  PURPOSE: Cause all missions to be saved to disk
  WHERE: (source: mission.c, library: pmx_msq.a)
****************************************************************************/
void SaveMissions()
{
	int		nIndex;

	for (nIndex=0; nIndex<num_missions; nIndex++)
		SaveMission(nIndex);
}

/****************************************************************************
  FUNCTION: int DelMissionLines(int from, int num_lines)
  PURPOSE: Cancella n righe iniziali della missione specificata
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: from - indice della missione su cui operare
             num_lines - numero di linee da cancellare
  RETURNS:  0: OK
          |=1: OK, missione cancellata
           -1: indice fuori dal campo permesso
           -2: la missione e' vuota
           -3: num_lines non e' corretto
           -4: la missione non esiste
****************************************************************************/
int DelMissionLines(int from, int num_lines)
{
	int		rc;
	char	*miss_line;
	MISSION	*missp;

	if (from < 0 || from >= num_missions) {
		return(-1);	/* Indice fuori dal campo permesso */
	}

	missp = mission[from];
	if (missp->mission_stat.num_lines == 0) {
		return(-2); /* Non ci sono linee da cancellare */
	}

	if (num_lines>missp->mission_stat.num_lines || num_lines<0) {
		return(-3); /* num_lines non e' corretto */
	}

	if (GetMissionStatus(from) == MISSION_NOTEXIST) {
		return(-4); /* La missione non esiste */
	}

	rc = 0;

	switch (num_lines)
	{
		case 0:
			rc |= 1;
			DeleteMission(from);
		break;

		default:
			miss_line = &(missp->mission_line);

			/* Decremento il numero di righe della missione */
			missp->mission_stat.num_lines -= num_lines;

			/* Controllo che sia rimasta almeno una riga */
			if (missp->mission_stat.num_lines) {
				/* Eseguo lo shift delle righe successive */
				memmove(miss_line, miss_line + (mission_size_line*num_lines),
					missp->mission_stat.num_lines * mission_size_line);

				/* Setto i flags di missione modificata */
				missp->mission_stat.save++;
				missp->mission_stat.mod++;
			} else {
				rc |= 1;
				DeleteMission(from);
			}
		break;
	}

	/* OK */
	return(rc);
}

/****************************************************************************
  FUNCTION: int MoveMissionWild(int to, int from, int start, int num_lines)
  PURPOSE: Sposta num_lines righe a partire dalla riga start della missione
			from in coda alla missione to
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: to - indice della missione destinazione
             from - indice della missione sorgente
			 start - riga di partenza
			 num_lines - numero di righe
  RETURNS:  0: OK
          |=1: OK, missione destinazione creata
          |=2: OK, missione sorgente cancellata
           -1: indice fuori dal campo permesso
           -2: missione vuota
           -3: parametri funzione errati
****************************************************************************/
int MoveMissionWild(int to, int from, int start, int num_lines)
{
	int		rc;
	char	*ml_to;
	char	*ml_from;
	MISSION	*missp_to;
	MISSION	*missp_from;

	if (to<0 || to>=num_missions || from<0 || from>=num_missions) {
		return(-1);	/* Indice fuori dal campo permesso */
	}

	missp_from = mission[from];	/* Missione sorgente */
	if (missp_from->mission_stat.num_lines == 0) {
		return(-2); /* Missione vuota */
	}

	if (start<0 || num_lines<0 ||
		missp_from->mission_stat.num_lines<start+num_lines)
	{
		return(-3);	/* Parametri funzione errati */
	}

	rc = 0;

	missp_to = mission[to];	/* Missione destinazione */

	/* Se la missione destinazione non esiste, va creata */
	if (GetMissionStatus(to) == MISSION_NOTEXIST) {
		rc |= 1;
		ResetMissionState(to);
		SetMissionStatus(to, MISSION_INIT);
		missp_to->mission_stat.num_lines = 0;
		missp_to->mission_stat.orig_lines = 1;
	}

	TouchMission(to);

	/* Copio le righe da una missione in coda all'altra */
	ml_to = &(missp_to->mission_line);
	ml_from = &(missp_from->mission_line);
	memcpy(ml_to + (missp_to->mission_stat.num_lines * mission_size_line),
			ml_from + (start*mission_size_line), mission_size_line*num_lines);

	/* Incrementa il numero di righe della missione destinazione */
	missp_to->mission_stat.num_lines += num_lines;

	/* Decrementa il numero di righe della missione sorgente */
	missp_from->mission_stat.num_lines -= num_lines;

	/* Controllo se la missione sorgente e' rimasta vuota */
	if (missp_from->mission_stat.num_lines) {
		/* Cancello le righe spostate */
		memmove(ml_from + (start*mission_size_line), 
			ml_from + (mission_size_line*(start+num_lines)),
			(missp_from->mission_stat.num_lines-start) * mission_size_line);

		/* Setto i flags di missione modificata */
		missp_from->mission_stat.save++;
		missp_from->mission_stat.mod++;
	} else {
		rc |= 2;
		DeleteMission(from);
	}

	/* OK */
	return(rc);
}

/****************************************************************************
  FUNCTION: int DelMissionWild(int from, int start, int num_lines)
  PURPOSE: Cancella num_lines righe a partire dalla riga start della missione
			from 
  WHERE: (source: mission.c, library: pmx_msq.a)
  PARAMETER: from - indice della missione
			 start - riga di partenza
			 num_lines - numero di righe
  RETURNS:  0: OK
          |=2: OK, missione sorgente cancellata
           -1: indice fuori dal campo permesso
           -2: missione vuota
           -3: parametri funzione errati
****************************************************************************/
int DelMissionWild(int from, int start, int num_lines)
{
	int		rc;
	char	*ml_from;
	MISSION	*missp_from;

	if (from<0 || from>=num_missions) {
		return(-1);	/* Indice fuori dal campo permesso */
	}

	missp_from = mission[from];	/* Missione sorgente */
	if (missp_from->mission_stat.num_lines == 0) {
		return(-2); /* Missione vuota */
	}

	if (start<0 || num_lines<0 ||
		missp_from->mission_stat.num_lines<start+num_lines)
	{
		return(-3);	/* Parametri funzione errati */
	}

	rc = 0;

	ml_from = &(missp_from->mission_line);

	/* Decrementa il numero di righe della missione */
	missp_from->mission_stat.num_lines -= num_lines;

	/* Controllo se la missione sorgente e' rimasta vuota */
	if (missp_from->mission_stat.num_lines) {
		/* Cancello le righe */
		memmove(ml_from + (start*mission_size_line), 
			ml_from + (mission_size_line*(start+num_lines)),
			(missp_from->mission_stat.num_lines-start) * mission_size_line);

		/* Setto i flags di missione modificata */
		missp_from->mission_stat.save++;
		missp_from->mission_stat.mod++;
	} else {
		rc |= 2;
		DeleteMission(from);
	}

	/* OK */
	return(rc);
}


/****************************************************************************
  FUNCTION: int VerExistMissions()
  PURPOSE: Verifica se esiste almeno una missione inizializzata
  RETURN VALUES:	0: missioni tutte vuote
					1: trovata almeno una missione
****************************************************************************/
int VerExistMissions()
{
	int		nIndex;
	FILE	*fpTemp;

	for (nIndex=0; nIndex<=num_missions; nIndex++)
	{
		fpTemp = fopen(mission_name[nIndex], "r");
		if (fpTemp != (FILE *)NULL) {
			fclose(fpTemp);
			return(1);
		}
	}
	return(0);
}
