/* timeout.h - header file for timeout.c */
#ifndef _TIMEOUT_H
#define _TIMEOUT_H
#include <sys/types.h>
#include <time.h>

#define MAX_TIMEOUT		1000	/* maximum possibile permitted timeout */
#define MAXNUM_TIMEOUTS	30		/* maximum no. of simultaneous timeouts */

/* Structure of timeout list */
typedef struct {
	time_t timeout;
	int id;
} TIMEOUT_LIST;

/* Function prototypes */
extern int SetTimeout(int, int);
extern int DeleteTimeout(int);
extern int CheckTimeout(int);
extern int GetTimeout(int);
extern int StartTimeout(void);
extern int StopTimeout(void);
extern void SetTimeoutCallback(void(*)());

#endif	/* _TIMEOUT_H */
