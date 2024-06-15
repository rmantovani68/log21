/** 
* timeout.c - timeout list routines
*
* Mode of operation:
*   These routines allow the user to specify simply a number of
*   concurrently operating timeouts, which are held transparently
*   in a timeout list, each timeout being identified by a timeout id.
*   The timeouts can then be used for instance to unblock blocking
*   read, message read or semaphore operations.
* Instructions for use:
*   The simplest form of use is as follows. The user sets his timeouts
*   using SetTimeout(), starts the Timeouts with StartTimeout() and then
*   goes into a blocking read.
*   On a timeout being activated, the blocking read returns with an
*   error and the user can employ CheckTimeout() to read out successively
*   the id's of those timeouts which have elapsed (there can be more
*   than one), whereby each timeout thereby found is then automatically
*   deleted from the timeout list.
*   The remaining timeouts can afterwards be reactivated with
*   StartTimeout().
*/
#ifdef __linux__
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include "timeout.h"

/* Timeout list */
static TIMEOUT_LIST	timeout_list[MAXNUM_TIMEOUTS];
/* Number of timeouts */
static int num_timeouts;
/* Time for next timeout */
static time_t next_timeout;
/* Id of next timeout */
static int next_timeout_id;
/* -> environment for longjmp() */
static jmp_buf *env;

/* Timeout callback */
static void TimeoutCallback();
/* Dummy function prototype */
unsigned int alarm(unsigned int);

/****************************************************************************
  FUNCTION : int SetTimeout(int timeout, int id)
  PURPOSE : Put a timeout into the timeout list
  COMMENTS: (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: timeout - timeout in seconds
              id - integer id of timeout
  RETURNS: 0: OK
          -1: timeout <= 0 or timeout too large (> MAX_TIMEOUT)
          -2: id <= 0
          -3: no space left in list
          -4: one or more timeouts have already elapsed
****************************************************************************/
int SetTimeout(int timeout, int id)
{
	int		i;
	int		id_found = 0;
	int		nearest_timeout_id;
	time_t	t;
	time_t	this_time;
	time_t	nearest_timeout;

	/* Test timeout > 0 */
	if (timeout <= 0 || timeout > MAX_TIMEOUT)
		/* Timeout out of bounds */
		return(-1);
	if (id <= 0)
		/* Id not allowed */
		return(-2);
	nearest_timeout_id = 0;
	this_time = time(NULL);
	for (i=0;; i++) {
		if (i >= MAXNUM_TIMEOUTS)
			/* No space left in list */
			return(-3);
		if (!timeout_list[i].id || timeout_list[i].id == id) {
			if (!timeout_list[i].id)	/* at end of timeout list */
				num_timeouts++;
			timeout_list[i].id = id;
			timeout_list[i].timeout = this_time + (time_t)timeout;
			break;
		}
	}
	nearest_timeout = this_time + (time_t)(MAX_TIMEOUT + 1);
	for (i=0; i<num_timeouts; i++) {
		/* Find the next timeout which will elapse, if any */
		t = timeout_list[i].timeout;
		if (t < nearest_timeout) {
			/* Get next timeout to elapse */
			nearest_timeout = t;
			nearest_timeout_id = id;
		}
	}
	next_timeout = nearest_timeout;
	next_timeout_id = nearest_timeout_id;
	if (next_timeout <= this_time) {
		/* One or more timeouts have already elapsed. The user must call
		 * StartTimeout() after dealing with them.
		 */
		return(-4);
	}
	return(0);
}

/****************************************************************************
  FUNCTION : int DeleteTimeout(int id)
  PURPOSE : Remove a timeout from the timeout list
  COMMENTS: (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: id - integer id of timeout
  RETURNS: 0: OK, id was deleted
          -1: timeout list empty
          -2: id not found
****************************************************************************/
int DeleteTimeout(int id)
{
	int		i;
	int		id_found = 0;

	if (!num_timeouts)
		return(-1);	/* timeout list empty */
	for (i=0; i<num_timeouts; i++) {
		if (!id_found) {
			if (timeout_list[i].id == id)
				id_found++;
		}
		else
			memcpy((char *)(timeout_list+i-1), (char *)(timeout_list + i),
				sizeof(TIMEOUT_LIST));
	}
	if (id_found) {
		num_timeouts--;
		timeout_list[num_timeouts].id = 0;
		return(0);
	}
	return(-2);
}

/****************************************************************************
  FUNCTION : int CheckTimeout(int interval)
  PURPOSE : Check the timeout list for elapsed timeouts
  COMMENTS: Find timeout which has elapsed or would elapse first
            If the interval is set to 0 then only elapsed timeouts
            are selected and the timeout is also deleted from the list.
            (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: interval - interval within which timeout may elapse
                         The nearest timeout which would elapse within
                         this interval is selected. Normally set to 0,
                         in which case only elapsed timeouts are considered.
  RETURNS:  0: no elapsed timeout found
           >0: id of an elapsed timeout (there may be others)
           -1: timeout list empty or interval < 0
****************************************************************************/
int CheckTimeout(int interval)
{
	time_t	this_time = time(NULL);
	time_t	nearest_timeout;
	int		i;
	int		id_found = 0;

	if (!num_timeouts || interval < 0)
		return(-1);
	nearest_timeout = this_time + (time_t)(MAX_TIMEOUT + 1);
	for (i=0; i<num_timeouts; i++) {
		if (timeout_list[i].timeout <= this_time + interval &&
				timeout_list[i].timeout < nearest_timeout) {
			id_found = timeout_list[i].id;	/* this timeout elapsed most */
			nearest_timeout = timeout_list[i].timeout;
		}
	}
	if (id_found && !interval)
		DeleteTimeout(id_found);	/* remove it from list */
	return(id_found);
}

/****************************************************************************
  FUNCTION : int GetTimeout(int id)
  PURPOSE : Get the time yet to elapse for the specified id of timeout
  COMMENTS: (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: id - id of timeout
  RETURNS:  >0: OK, time left to timeout
             0: timeout already elapsed
            -1: timeout list empty
            -2: id not found
****************************************************************************/
int GetTimeout(int id)
{
	time_t	this_time = time(NULL);
	int		i;

	if (!num_timeouts)
		return(-1);
	for (i=0; i<num_timeouts; i++) {
		if (timeout_list[i].id == id) {
			if (this_time >= timeout_list[i].timeout)
				return(0);
			return(timeout_list[i].timeout - this_time);
		}
	}
	return(-2);
}

/****************************************************************************
  FUNCTION : int StartTimeout();
  PURPOSE : Start the nearest timeout in the timeout list
  COMMENTS: The nearest timeout in the list is found and activated
            (source: timeout.c, library: pmx_msq.a)
  RETURNS: >0: OK, id of timeout being started
           -1: timeout list empty
           -2: timeout already elapsed, NB: use CheckTimeout() to find it
****************************************************************************/
int StartTimeout()
{
	time_t	t;
	time_t	this_time = time(NULL);
	time_t	nearest_timeout;
	int		i;
	int		id_found;

	if (!num_timeouts)
		return(-1);
	/* Find nearest timeout */
	nearest_timeout = this_time + (time_t)(MAX_TIMEOUT + 1);
	for (i=0; i<num_timeouts; i++) {
		t = timeout_list[i].timeout;
		if (t < nearest_timeout) {
			nearest_timeout = t;
			id_found = timeout_list[i].id;
		}
	}
	/* Check if it has already elapsed */
	if (nearest_timeout > this_time) {
		/* Timeout has still time to run */
		signal(SIGALRM, TimeoutCallback);
		alarm(t - this_time);
		return(id_found);
	}
	/* One or more timeouts have already elapsed.
	 * Firstly, CheckTimeout() can be called to find these timeouts.
	 * After this, call StartTimeout() again to reactivate any timeout.
	 */
	return(-2);
}

/****************************************************************************
  FUNCTION : int StopTimeout();
  PURPOSE : Stop the timeout
  COMMENTS: The nearest timeout in the list is found and activated
            (source: timeout.c, library: pmx_msq.a)
  RETURNS: >0: OK, id of nearest timeout which has been stopped
           -1: OK, timeout list empty
           -2: OK, timeout already elapsed, NB: use CheckTimeout() to find it
****************************************************************************/
int StopTimeout()
{
	time_t	t;
	time_t	this_time = time(NULL);
	time_t	nearest_timeout;
	int		i;
	int		id_found;

	/* Stop alarm */
	alarm(0);
	signal(SIGALRM, SIG_IGN);

	if (!num_timeouts)
		return(-1);

	/* Find nearest timeout */
	nearest_timeout = this_time + (time_t)(MAX_TIMEOUT + 1);
	for (i=0; i<num_timeouts; i++) {
		t = timeout_list[i].timeout;
		if (t < nearest_timeout) {
			nearest_timeout = t;
			id_found = timeout_list[i].id;
		}
	}
	/* Check if it has already elapsed */
	if (nearest_timeout > this_time)
		return(id_found);
	else
		/* Already elapsed */
		return(-2);
}

/****************************************************************************
  FUNCTION : void SetTimeoutEnv(int(*func)(void))
  PURPOSE : Set the timeout env for longjmp() function
  COMMENTS: (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: func - callback function, takes void param and returns int
****************************************************************************/
void SetTimeoutEnv(jmp_buf *jmp_env)
{
	/* Save environment pointer */
	env = jmp_env;
}

/****************************************************************************
  FUNCTION : static void TimeoutCallback()
  PURPOSE : Callback for SIGALRM signal
  COMMENTS: (source: timeout.c, library: pmx_msq.a)
  PARAMETERS: sig - signal passed to routine by operating system
****************************************************************************/
static void TimeoutCallback()
{
	/* Disable SIGALRM */
	signal(SIGALRM, SIG_IGN);
	alarm(0);
}
#endif