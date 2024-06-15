/* trace.c - trace file routines */
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include "trace.h"

#ifndef NO_CHSIZE
int chsize(int, long);
#endif    /* NO_CHSIZE */

/* Struct controlling a trace file */
typedef struct tracefile {
    char filename[LEN_TRACE_FILENAME];
    int break_off;
    FILE *fp;
    int fd;
    long file_size;
    long char_count;
} TRACEFILE;

/* Names of ASCII control characters and space characters */
static char *ascii_names[] = {
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
    "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
    "DLE", "XON", "DC2", "XOFF", "DC4", "NAK", "SYN", "ETB",
    "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US", "SP"
};
static int num_ctl_chars = sizeof(ascii_names) / sizeof(char *);

//int errno;

static char err_msg[121];
static TRACEFILE trace;
static char sbuf[4096];
/* Indicates max. length in bytes of trace */
static long trace_file_maxsize;

/* Module-local function prototypes */
#ifdef ANSIPROTO
static void trace_out_a_ch(int, int);
#else    /* ANSIPROTO */
static void trace_out_a_ch();
#endif    /* ANSIPROTO */

static struct tm *_current_time(void)
{
#ifdef __linux__
    time_t curr_time;
    
    time( &curr_time );
    return localtime(&curr_time);
#else
    __time64_t curr_time;

    _time64( &curr_time );
    return _localtime64( &curr_time );
#endif
}


/*****************************************************************************
  FUNCTION: int open_trace(char *prog_name, char *filename, int trace_len)
  PURPOSE: Open a trace file
  PARAMETERS: prog_name - name of program
              filename - name of trace file
              trace_len - 0: file grows without limit
                        < 0: file grows to -trace_len and is then closed
                        > 0: file grows to trace_len and is then restarted
  COMMENTS: (source trace.c, library: pmx_lib.a)
  RETURNS:   0 - OK
            !0 - UNIX error number
 ****************************************************************************/
int open_trace(char *prog_name, char *filename, long trace_len)
{
    char cch;
    char *trace_filename;
    int trace_type;

    trace_filename = filename;
    trace_type = UNLIMITED_LEN;
    if (trace_len < 0) {
        trace_file_maxsize = -trace_len;
        trace_type = BREAK_OFF_AFTER_LEN;
    }
    else if (trace_len) {
        trace_file_maxsize = trace_len;
        trace_type = TRUNCATE_AFTER_LEN;
    }
    trace.break_off = trace_type;
    if ((trace.fp = fopen(trace_filename, "a+")) == (FILE *)NULL) {
        sprintf(err_msg, "%s: cannot open trace file '%s' (errno: %d)",
            prog_name, trace_filename, errno);    
        trace.break_off = NO_TRACE;
        return(errno);
    }

    strcpy(trace.filename, trace_filename);
    trace.fd = fileno(trace.fp);
    fseek(trace.fp,0L,SEEK_END);
    trace.file_size = ftell(trace.fp) + 1;
    cch = '\n';
    fwrite(&cch, 1, 1, trace.fp);
    trace.char_count = 0;
    return(0);
}

/*****************************************************************************
  FUNCTION: void truncate_trace(void)
  PURPOSE: Truncate opened trace file
  COMMENTS: Usually activated by an interrupt
    17-10-2000 rm : il file non viene cancellato ma rinominato .old
 ****************************************************************************/
void truncate_trace ()
{
    int lent;
    char szNewName[256];

    if (trace.fp) {
#ifdef DELETE_TRACE
#ifdef NO_CHSIZE
        close_trace();
        unlink(trace.filename);
        trace.fp = fopen(trace.filename, "a+");
#else    /* NO_CHSIZE */
        rewind(trace.fp);
        chsize(trace.fd, 0L);
#endif    /* NO_CHSIZE */
#else 
        close_trace();
        strcpy(szNewName,trace.filename);
        strcat(szNewName,".old");
        rename(trace.filename,szNewName);
        trace.fp = fopen(trace.filename, "a+");
#endif
        lent = strlen(FILE_TRUNC);
        fwrite(FILE_TRUNC, lent, 1, trace.fp);
        trace.file_size = lent;
        trace.char_count = 0;
    }
}

/*****************************************************************************
  FUNCTION: void close_trace(void)
  PURPOSE: Close the trace file
 ****************************************************************************/
void close_trace ()
{
    if (trace.fp)
        fclose(trace.fp);
}

/*****************************************************************************
  FUNCTION: void trace_out_str(char *str, int nl)
  PURPOSE: Write a trace string to the trace file
  PARAMETERS: str - string to be written
              nl - if >0, write newline (\n) before string
 ****************************************************************************/
void trace_out_str (str, nl)
    char *str;
    int nl;
{
    int l, lent;
    char cch;

    if (trace.break_off == NO_TRACE)
        return;
    if (trace.fp != (FILE *)NULL) {
        if (nl) {
            cch = '\n';
            fwrite(&cch, 1, 1, trace.fp);
            trace.char_count = 0;
            trace.file_size++;
        }
        l = strlen(str);
        trace.file_size += l;
        if (trace.file_size > trace_file_maxsize) {
            switch (trace.break_off) {
            case TRUNCATE_AFTER_LEN:
                truncate_trace();
                break;
            case BREAK_OFF_AFTER_LEN:
                lent = strlen(FILE_BREAK);
                fwrite(FILE_BREAK, lent, 1, trace.fp);
                fclose(trace.fp);
                trace.fp = (FILE *)NULL;
                return;
            }
        }
        if (l) {
            trace.char_count += l;
#ifdef USE_NEW_LINE
            if (trace.char_count > MAX_TRACE_LINE_LENGTH) { /* go to next line */
                cch = '\n';
                fwrite(&cch, 1, 1, trace.fp);
                trace.char_count = 0;
            }
#endif
            fwrite(str, l, 1, trace.fp);
            fflush(trace.fp);
        }
    }
}

/*****************************************************************************
  FUNCTION: void trace_out_errstr(char *str, int nl)
  PURPOSE: Write a title and the errno string to the trace file
  PARAMETERS: str - title
              nl - if >0, write newline (\n) before string
 ****************************************************************************/
void trace_out_errstr (str, nl)
    char *str;
    int nl;
{
    char buf[121];

    sprintf(sbuf, "%s%s", str, strerror(errno));
    trace_out_str(buf, nl);
}

/*****************************************************************************
  FUNCTION: void trace_out_seq(char *seq, int len, int nl)
  PURPOSE: write a sequence of characters to trace file
  PARAMETERS: seq - sequence to be written
              len - length of sequence
              nl - if >0, write newline (\n) before sequence
 ****************************************************************************/
void trace_out_seq (seq, len, nl)
    char *seq;
    int len, nl;
{
    int i;

    if (nl)
        trace_out_str("", 1);
    if (len == 0)
        return;
    for (i=0; i<len; i++)
        trace_out_ch(seq[i]);
}

/*****************************************************************************
  FUNCTION: void trace_out_hexseq(char *seq, int len, int nl)
  PURPOSE: write a sequence of hexadecimal characters to trace file
  PARAMETERS: seq - sequence to be written
              len - length of sequence
              nl - if >0, write newline (\n) before sequence
 ****************************************************************************/
void trace_out_hexseq (seq, len, nl)
    char *seq;
    int len, nl;
{
    int i;

    if (nl)
        trace_out_str("", 1);
    if (len == 0)
        return;
    for (i=0; i<len; i++)
        trace_out_hexch(seq[i]);
}

/*****************************************************************************
  FUNCTION: void TraceDateFormatted(int nl, char *fmt, va_list)
  PURPOSE: write a formatted variable args sequence to trace file
  PARAMETERS: nl - newline
              fmt - format string
              va_list - variable argument list
 ****************************************************************************/
void trace_out_vstr_date (int nl,char *fmt,...)
{
    va_list args;
    char tbuf[80];
    char dbuf[80];

    va_start(args,fmt);
    if (nl)
        trace_out_str("", 1);
    sprintf(sbuf, "[%s] [%s] : ", trace_getdate(dbuf), trace_gettime_msec(tbuf));
    trace_out_str(sbuf, 0);
    (void)vsprintf(sbuf, fmt, args);
    va_end(args);
    trace_out_str(sbuf, 0);
}

/*****************************************************************************
  FUNCTION: void trace_out_vstr(int nl, char *fmt, va_list)
  PURPOSE: write a formatted variable args sequence to trace file
  PARAMETERS: nl - newline
              fmt - format string
              va_list - variable argument list
 ****************************************************************************/
void trace_out_vstr (int nl,char *fmt,...)
{
    va_list args;

    va_start(args,fmt);
    if (nl)
        trace_out_str("", 1);
    (void)vsprintf(sbuf, fmt, args);
    va_end(args);
    trace_out_str(sbuf, 0);
}

/*****************************************************************************
  FUNCTION: void trace_out_vseq(int nl, char *fmt, va_list)
  PURPOSE: write a formatted variable args sequence to trace file as
           separate characters
  COMMENTS: Watch out for overflow of sbuf!!
  PARAMETERS: nl - newline
              fmt - format string
              va_list - variable argument list
 ****************************************************************************/
void trace_out_vseq (int nl,char *fmt,...)
{
    va_list args;
    int l;

    va_start(args,fmt);
    if (nl)
        trace_out_str("", 1);
    (void)vsprintf(sbuf, fmt, args);
    va_end(args);
    l = strlen(sbuf);
    trace_out_seq(sbuf, l, 0);
}

/*****************************************************************************
  FUNCTION: void trace_out_buf(char *buf_to, char *buf_fr, int len)
  PURPOSE: Convert a buffer into trace form in another buffer
  PARAMETERS: buf_to - target buffer
              buf_fr - source buffer
              len - length of source buffer
  RETURNS: length of data in target buffer
 ****************************************************************************/
int trace_out_buf (buf_to, buf_fr, len)
    char *buf_to;
    char *buf_fr;
    int len;
{
    int i, l;
    char ch;

    for (i=0, l=0; i<len; i++) {
        ch = buf_fr[i];
        if (isgraph(ch))
            buf_to[l++] = ch;
        else
            l += get_ascii(ch, &buf_to[l]);
        buf_to[l++] = ' ';
    }
    return(l);
}

/*****************************************************************************
  FUNCTION: void trace_out_ch(int ch)
  PURPOSE: write a character to trace file
  PARAMETER: ch - character to be written
 ****************************************************************************/
void trace_out_ch (ch)
    int ch;
{
    trace_out_a_ch(ch, 0);
}

/*****************************************************************************
  FUNCTION: void trace_out_hexch(int ch)
  PURPOSE: write a hex character to trace file
  PARAMETER: ch - character to be written
 ****************************************************************************/
void trace_out_hexch (ch)
    int ch;
{
    trace_out_a_ch(ch, 1);
}

/*****************************************************************************
  FUNCTION: static void trace_out_a_ch(int ch, int hex)
  PURPOSE: write a character to trace file
  PARAMETER: ch - character to be written
 ****************************************************************************/
static void trace_out_a_ch (ch, hex)
    int ch;
    int hex;
{
    char buf[6], cch;
    unsigned char chr;
    int l, lent;

    if (trace.break_off == NO_TRACE)
        return;
    if (trace.fp != (FILE *)NULL) {
        trace.char_count++;
        if (trace.char_count > MAX_TRACE_LINE_LENGTH) { /* go to next line */
            cch = '\n';
            fwrite(&cch, 1, 1, trace.fp);
            trace.char_count = 0;
        }
        else {
            cch = ' ';
            fwrite(&cch, 1, 1, trace.fp);
        }
        trace.file_size++;

        chr = (unsigned char)(ch & LSBYTE); /* make sure it is a char */
        if (!hex && isgraph(chr)) {
            trace.char_count++;
            if (trace.char_count > MAX_TRACE_LINE_LENGTH) { /* go to next line */
                trace.file_size++;
                cch = '\n';
                fwrite(&cch, 1, 1, trace.fp);
                trace.char_count = 0;
            }
            trace.file_size++;
            fwrite((char *)&chr, 1, 1, trace.fp);
        }
        else {
            if (hex) {
                sprintf(buf, "x%2.2x", chr);
                l = 4;
            }
            else
                l = get_ascii(chr, buf);
            trace.char_count += l;
            trace.file_size += l;
            if (trace.char_count > MAX_TRACE_LINE_LENGTH) { /* go to next line */
                trace.file_size++;
                cch = '\n';
                fwrite(&cch, 1, 1, trace.fp);
                trace.char_count = l;
            }
            if (trace.file_size > trace_file_maxsize) {
                switch (trace.break_off) {
                case TRUNCATE_AFTER_LEN:
                    truncate_trace();
                    break;
                case BREAK_OFF_AFTER_LEN:
                    lent = strlen(FILE_BREAK);
                    fwrite(FILE_BREAK, lent, 1, trace.fp);
                    fclose(trace.fp);
                    trace.fp = (FILE *)NULL;
                    return;
                }
            }
            fwrite(buf, l, 1, trace.fp);
        }
        fflush(trace.fp);
    }
}

/*****************************************************************************
  FUNCTION: int get_ascii(char ch, char *tbuf)
  PURPOSE: Get ASCII name of character
  PARAMETERS: ch - character to be translated
              tbuf - returned string giving ASCII name or hexadecimal value
  RETURNS: length of string
 ****************************************************************************/
int get_ascii (ch, tbuf)
    char ch;
    char *tbuf;
{
    unsigned char chr;
    int len;

    chr = (unsigned char)(ch & LSBYTE); /* make sure it is a char */

    if ((int)chr < num_ctl_chars) {    /* ASCII control char or space */
        strcpy(tbuf, ascii_names[chr]);
        len = strlen(ascii_names[chr]);
    }
    else {
        sprintf(tbuf, "x%2.2x", chr);
        len = 4;
    }
    return(len);
}

/*****************************************************************************
  FUNCTION: void endtime(void)
  PURPOSE: Write current time in trace file
 ****************************************************************************/
void endtime ()
{
    char tbuf[14];

    sprintf(sbuf, " [%s]", trace_gettime_msec(time((time_t *)0), tbuf));
    trace_out_str(sbuf, 0);
}

/*****************************************************************************
  FUNCTION: void trace_gettime_msec(long source, char *target)
  PURPOSE: Get time of day as a string with milliseconds precision
  PARAMETERS: source - time in seconds from time() call
              target - returned time in format (hh:mm:ss)
 ****************************************************************************/
char *trace_gettime_msec(char *target)
{
    struct tm *ltime = _current_time();
    struct timeval tv;

    gettimeofday(&tv,NULL);

    sprintf(target, "%.2d:%.2d:%.2d.%3d", ltime->tm_hour, ltime->tm_min, ltime->tm_sec,(int)tv.tv_usec/1000);

    return target;
}

/*****************************************************************************
  FUNCTION: void gettime(long source, char *target)
  PURPOSE: Get time of day as a string
  PARAMETERS: source - time in seconds from time() call
              target - returned time in format (hh:mm:ss)
 ****************************************************************************/
char *gettime(char *target)
{
    struct tm *ltime = _current_time();

    sprintf(target, "%.2d:%.2d:%.2d", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

    return target;
}

/*
* trace_getdate()
* get date a string
*/
char *trace_getdate(char *szDateString)
{
    struct tm *ltime = _current_time();

    sprintf(szDateString, "%.2d-%.2d-%.2d", ltime->tm_mday, ltime->tm_mon+1, ltime->tm_year%100);

    return szDateString;
}
