/*
 *
 *        Name: xmitmsgx.h (C header), "transmit message" for POSIX
 *              read and emit enumerated messages from the repository
 *      Author: Rick Troth, rogue programmer
 *        Date: 2014-May-01 (Thu)
 *              2017-Nov-23 (Thu) Thanksgiving 2017
 *              2023-03-16 (Thu)
 */

#ifndef _XMITMSGX_H
#define _XMITMSGX_H

/* xmitmsgx-2.3.2            v2            r3            m2           */
#define  XMITMSGX_VERSION  (((2) << 24) + ((3) << 16) + ((2) << 8) + (0))
/* version 2.3.1 adds initial support for 'XMITMSG' on VM/CMS         */
/* version 2.2.9 adds xm_findfile() internal locale search function   */
/* version 2.2.8 removes "extern" from prototype statements (below)   */
/* version 2.2.7 includes a built-in SYSLOG place holder for Windows  */
/* version 2.2.5 removes "unsigned char" for to build on IBM VM/CMS   */
/*  ... but characters are NOT "signed" in practice anyway ... WCGW?  */

/* priorities (these are ordered)                                     */
/*      MSGLEVEL_DEBUG           LOG_DEBUG   7 debug-level messages, not used here */
#define MSGLEVEL_INFO     'I' /* LOG_INFO    6 informational */
#define MSGLEVEL_REPLY    'R' /* LOG_NOTICE  5 normal but significant condition, ATTENTION */
#define MSGLEVEL_WARNING  'W' /* LOG_WARNING 4 warning conditions */
#define MSGLEVEL_ERROR    'E' /* LOG_ERR     3 error conditions */
#define MSGLEVEL_SEVERE   'S' /* LOG_CRIT    2 critical conditions */
#define MSGLEVEL_TERMINAL 'T' /* LOG_ALERT   1 action must be taken immediately */
/*      MSGLEVEL_EMERG           LOG_EMERG   0 reserved */
/*      MSGLEVEL_STDOUT   '*'                  print to stdout        */
/*      MSGLEVEL_STDERR   '!'                  print to stderr        */

#define MSGROUTE_DEFAULT     (1<<3)  /* user-generated messages */
#define MSGROUTE_USER        (1<<3)  /* user-generated messages */
#define MSGROUTE_FILE        (8<<3)  /* file transfer subsystem */
#define MSGROUTE_PRINT       (6<<3)  /* printing subsystem */

/* the following are used by derivative functions, not by xmmake() itself */
#define  MSGFLAG_SYSLOG   0x01   /* used by xmopen() to enable logging */
#define  MSGFLAG_NOLOG    0x02   /* used by xmprint() and xmwrite() to skip logging */
#define  MSGFLAG_NOCODE   0x04   /* means message text only, good for decorations */
#define  MSGFLAG_NOPRINT  0x08   /* implies log only */
/* what about time stamp? logging automtically has time stamping */

#define  MSGERR_NOLIB     813
#define  MSGERR_NOMSG     814

static int xmitmsgx_version = XMITMSGX_VERSION;

typedef struct MSGSTRUCT
  {
    int  msgnum;                /* message number */
    int  msgc;                  /* count of replacement tokens */
    char **msgv;       /* vector of replacement tokens */
    char *msgbuf;      /* buffer supplied by caller */
    int  msglen;                /* buffer size on input, msg size on return */
    char *msgtext;     /* offset past msg code/header */
/* ints still only 4 bytes on 64-bit systems, though ptrs are 8 bytes */

    int  msgfmt;        /* message format number (for future use) */
    int  msgline;       /* message line number (for future use, zero means all lines) */
    int  msglevel;      /* message level/serverity (zero means use the letter in the file) */
    int  msgopts;       /* set by xmopen(), sometimes overridden for xmmake() */

    /* the following are probably not for external use */
    char *caller;       /* default is getenv("LOGNAME") roughly, msgu */
    char *prefix;       /* default is applid[0..2]||caller[0..2] */
    char *letter;       /* default taken from message file */

    /* the following are filled in by xmopen() not for external use */
    int  msgmax;                /* highest message number in table */
    char **msgtable;   /* array of messages (allocated memory) */
    char *msgdata;     /* messages file content (allocated memory) */
    char *msgfile;     /* name of message file found (for reference and debugging) */
    char *escape;      /* the escape character (for reference and debugging) */

    char  pfxmaj[4];   /* truncated up-cased applid/major */
    char  pfxmin[4];   /* truncated up-cased caller/minor */
    char  locale[32];  /* possibly truncated to match the nearest file found */
    char  applid[32];  /* default is basename of messages file, used as SYSLOG identity */

    int  version;               /* version of the library when struct is initialized */
    void *next;                 /* pointer to next in chain or NULL */
    void *prev;                 /* pointer to previous in chain (first is NULL) */
    char  msgmagic[16];  /* filled-in with "MSGSTRUCT" when initialized */
  } MSGSTRUCT;        /* we will expand this struct for release 2.2.x */

/* Open the messages file, read it, get ready for service. */
int xmopen(char*,int,struct MSGSTRUCT*);
/* args: filename, opts, MSGSTRUCT */
/* calls xminit()                                                     */
/* may call openlog(char*ident,int option,int facility)               */
/* Specify a syslog ident via applid in MSGSTRUCT. */
/* specify a syslog facility via optional MSGSTRUCT */

/* This is the heart of the utility. */
int xmmake(struct MSGSTRUCT*);
/* args: MSGSTRUCT - provide all details via this DSECT */

/* Print to stdout or stderr depending on level, optionally syslog. */
int xmprint(int,int,char**,int,struct MSGSTRUCT*);
/* args: msgnum, msgc, msgv, opts, MSGSTRUCT */

/* Write to file descriptor, optionally syslog. */
int xmwrite(int,int,int,char**,int,struct MSGSTRUCT*);
/* args: fd, msgnum, msgc, msgv, opts, MSGSTRUCT */

/* Generate a message and store it as a string. */
int xmstring(char*,int,int,int,char**,struct MSGSTRUCT*);
/* args: output, outlen, msgnum, msgc, msgv, MSGSTRUCT */

/* Clear the message repository struct. */
int xmclose(struct MSGSTRUCT*);
/* args: MSGSTRUCT - allocations performed by xmopen() will be undone */
/* calls xmquit()                                                     */
/* may call closelog()                                                */

/* internal functions */
int xm_lev2pri(char*);
int xm_negative(int);
int xm_deliver(char*,int);
int xm_make_cms(struct MSGSTRUCT*);

#endif

/* some example formats (for future use)
01 standard message
02 explanation
03 user action
04 mnemonics or symbolics
 */


