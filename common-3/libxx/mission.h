/* mission.h - header file for mission.c */
#ifndef _MISSION_H
#define _MISSION_H

/* Types of mission move */
#define MOVE_LINE			0
#define MOVE_ALL			1

/* Mission states common to all missions. The states defined by
 * the user must be numbered from 2 onwards
 */
#define MISSION_NOTEXIST	0	/* mission does not exist */
#define MISSION_INIT		1	/* mission has just been initialized */

/* Mission constants */
#define MISS_NUM_LINES		16		/* maximum no. of lines per mission */
#define MISS_NUM			4		/* no. of missions */
#define MISS_LEN_PATH		60		/* max. length of path to mission file */

/* Mission header fields: length constants */
#define MISS_LEN_NUM		2
#define MISS_LEN_NUM_LINES	2
#define MISS_LEN_SIZE_LINE	4

/* Mission status structure */
typedef struct {
	int status;			/* status of mission (its progress) */
	int mod;			/* mission changed (reupdate display) */
	int save;			/* mission changed (save it to disk file) */
	int num_lines;		/* current number of lines in mission */
	int orig_lines;		/* original number of lines when mission created */
} MISS_STAT;

/* structure of missions 0, 1, 2, 3 */
typedef struct {
	MISS_STAT mission_stat;
	char mission_line;	/* start of user defined area */
} MISSION;

/* Function prototypes */
int SetMissionParams(int, int, int, char *, char *, void (*)(int));
int SetMissionStatus(int, int);
int TouchMission(int);
int SetMissionNumLines(int, int);
int GetMissionStatus(int);
int GetMissionNumLines(int);
char *GetMissionLine(int, int);
int AccessMission(int);
void ResetMissions(void);
int ResetMissionState(int);
int DeleteMission(int);
int ReadMission(int);
int WriteMission(int);
int MoveMission(int, int, int);
int ShowMission(int);
void ShowMissions(void);
int SaveMission(int);
void SaveMissions(void);
int DelMissionLines (int, int);
int MoveMissionWild(int, int, int, int);
int DelMissionWild(int, int, int);
int VerExistMissions(void);

#endif /* _MISSION_H */
