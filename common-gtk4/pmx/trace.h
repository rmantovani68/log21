/* trace.h: header file for trace.c */
#ifndef _TRACE_H
#define _TRACE_H

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include "ascii.h"

/*** Types of trace file ***/
#define TRACE_FILE_SUFFIX		".trc"	/* extension of trace file */
/* Do not perform trace */
#define NO_TRACE			-1
/* Truncate (but continue) trace file after reaching certain length */
#define TRUNCATE_AFTER_LEN	0
/* Stop trace after reaching certain length in trace file */
#define BREAK_OFF_AFTER_LEN	1
/* Unlimited length of trace file */
#define	UNLIMITED_LEN		2

#define TRACE_FILE_MAXSIZE		10000000L	/* sample max. size of trace */
#define FILE_TRUNC				"file truncated\n"
#define FILE_BREAK				"file broken off\n"
#define LSBYTE					0xff
#define LEN_TRACE_FILENAME		60 /* max. len of trace file pathname */
#define MAX_TRACE_LINE_LENGTH   120

/* trace return codes */
#define TRACE_GOOD	0
#define TRACE_BAD	1
#define INFO_GOOD	0
#define INFO_BAD	1

/* function prototypes */
#ifdef ANSIPROTO
extern int open_trace(char *, char *, int);
extern void truncate_trace();
extern void trace_out_str(char *, int);
extern void trace_out_errstr(char *, int);
extern void trace_out_hexseq(char *, int, int);
extern void trace_out_ch(int);
extern void trace_out_hexch(int);
extern int get_ascii(char, char *);
extern char *trace_gettime_msec(long, char *);
extern char *trace_gettime(long, char *);
extern char *trace_getdate(long, char *);
#else	/* ANSIPROTO */
extern int open_trace();
extern void truncate_trace();
extern void trace_out_str();
extern void trace_out_errstr();
extern void trace_out_hexseq();
extern void trace_out_ch();
extern void trace_out_hexch();
extern int get_ascii();
extern char *trace_gettime_msec();
extern char *trace_gettime();
extern char *trace_getdate();
#endif	/* ANSIPROTO */
extern void trace_out_vstr(int nl,char *fmt,...);
extern void trace_out_vstr_date(int nl,char *fmt,...);
extern void trace_out_vseq(int nl,char *fmt,...);
extern void close_trace();
extern void endtime();

#endif /* _TRACE_H */
