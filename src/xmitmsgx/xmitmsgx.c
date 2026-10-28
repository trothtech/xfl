/*
 *
 *        Name: xmitmsgx.c (C program source)
 *              library of functions for the XMITMSGX package
 *      Author: Rick Troth, rogue programmer
 *        Date: 2017-Nov-25 (Sat) Thanksgiving 2017
 *              2023-April/May
 *
 *              This is a re-do after some time ... a very long time.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
 #include "xmmwin.c"
#else
 #ifdef OECS
  #define __UU
 #endif
 #include "syslog.h"
#endif

#include <libgen.h>
#include <ctype.h>

#include "xmitmsgx.h"
extern char *xmmprefix;       /* install prefix not appliction prefix */

/* These are the locale environment variables we will interrogate:    */
char *localevars[] = {
                "LANG",
                "LC_CTYPE",
                "LC_MESSAGES",
                "LC_ALL",
/*              "LC_COLLATE",         */
/*              "LC_TIME",            */
/*              "LC_NUMERIC",         */
/*              "LC_MONETARY",        */
                "LOCALE",
/*              "LC_PAPER",           */
/*              "LC_NAME",            */
/*              "LC_ADDRESS",         */
/*              "LC_TELEPHONE",       */
/*              "LC_MEASUREMENT",     */
/*              "LC_IDENTIFICATION",  */
                "LANGUAGE",
                ""   };     /* empty string marks the end of the list */
/* the syntax of the locale is                                        */
/*              <LANGUAGE>_<TERRITORY>.<CODESET>[@<MODIFIERS>]        */
/*   on Windoze <LANGUAGE>-<TERRITORY>.<CODESET>[@<MODIFIERS>]        */

/* These are the directories where we might find locale support:      */
char *localeoptd[] = {     /* opt (package) rooted locale directories */
                "%s/share/locale/%s/%s.msgs",
                "%s/lib/nls/msg/%s/%s.msgs",
                "%s/lib/locale/%s/%s.msgs",
                "%s/share/nls/%s/%s.msgs",
                ""   };     /* empty string marks the end of the list */
char *localedirs[] = {                   /* system locale directories */
                "/usr/share/locale/%s/%s.msgs",
/*               /usr/share/locale/en_US/uft.msgs                     */
                "/usr/lib/nls/msg/%s/%s.msgs",
                "/usr/lib/locale/%s/%s.msgs",
                "/usr/share/nls/%s/%s.msgs",
/*               /usr/share/nls/C/uft.msgs                            */
#if defined(_WIN32) || defined(_WIN64)
                "C:/Program Files/Common Files/System/%s/%s.msgs",
                "C:/Program Files (x86)/Common Files/System/%s/%s.msgs",
                "C:/Windows/System32/%s/%s.msgs",
                "C:/Windows/%s/%s.msgs",
#endif
                ""   };     /* empty string marks the end of the list */
/*               prefix, locale, applid                               */
/*
      The xmopen() routine will search all of the above. We do not use
      a per-platform single path because any given platform might have
      multiple locale directories and paths. X11 locale directories are
      not searched because their content is completely different.

   Examples:
      /usr/share/locale/en_US/%s.msgs           locale=en_US
      /usr/lib/locale/en_US.UTF-8/%s.msgs       locale=en_US.UTF-8
      /usr/lib/locale/en_US.ISO8859-1/%s.msgs   locale=en_US.ISO8859-1
      /usr/lib/nls/msg/En_US/%s.msgs            locale=En_US     (case!)

   On MS Windows:
      C:/Program Files/Common Files/System/en-US
      C:/Program Files (x86)/Common Files/System/en-US
      C:/Windows/System32/en-US
      C:/Windows/System32/en-GB
      C:/Windows/System32/en
      C:/Windows/en-US
 */

static struct MSGSTRUCT *msglobal = NULL, msstatic;

/* --------------------------------------------------------- XM_BASENAME
 *    This routine exists purely because of a segmentation fault
 *    when invoking stock basename() on a recent FreeBSD installation.
 *
 *    NOTE: this routine is NOT prototyped and not called externally
 */
char*xm_basename(char*path)
  { char *p, *q;
    p = q = path;               /* do not modify path in this routine */
    while (*p != 0x00)                             /* scan the string */
      { while (*p != 0x00 && *p != '/') p++;   /* looking for slashes */
        if (*p == '/') q = p; }           /* pointing to the last one */
    return q;
  }

/* --------------------------------------------------------- XM_FINDFILE
 *    This routine searches combinations of locale values plus
 *    file locations to find and open the desired message repository.
 *    If we cannot find the messages file then we cannot proceed.
 *    Returns: an open file descriptor on success or negative on error
 *    The underlying system calls will have set errno appropriately.
 *    This routine will have set ms->msgfile and ms->applid if success.
 *
 *    NOTE: this routine is NOT prototyped and not called externally
 */
int xm_findfile(char*fn0,struct MSGSTRUCT*ms)
  {
    int rc, fd, i, j;
    char *fn, *bn, *lc, *p;
    static char filename[256];           /* one time use for filename */

    /* isolate supplied arguments and treat them as read-only         */
    fn = fn0;

    /* prepare to search for the message repository                   */
    ms->msgdata = NULL;
    ms->msgtable = NULL;
    ms->msgfile = filename;
    (void) memset(ms->locale,0x00,sizeof(ms->locale));
    (void) memset(ms->applid,0x00,sizeof(ms->applid));
    ms->msgmax = 0;

#ifdef _DIRECT_NAME
    /* try the named file directly */
    strncpy(filename,fn,sizeof(filename)-1);
    rc = fd = open(filename,O_RDONLY);
    if (rc >= 0)
      { fn = xm_basename(fn);
        snprintf(ms->applid,sizeof(ms->applid),"%s",fn);
        /* NOTE: in this case the locale is not set                   */
        return fd; }
#endif

    /* try the named file + ".msgs" */
    snprintf(filename,sizeof(filename)-1,"%s.msgs",fn);
    rc = fd = open(filename,O_RDONLY);
    if (rc >= 0)
      { fn = xm_basename(fn);
        strncpy(ms->applid,fn,sizeof(ms->applid));
        /* NOTE: in this case the locale is not set                   */
        return fd; }

    fn = xm_basename(fn);     /* beyond this point fn *is* the applid */
    strncpy(ms->applid,fn,sizeof(ms->applid));    /* set it in struct */

    /* try the named file with any locale value */
    i = 0; while (*localevars[i] != 0x00)      /* localevars loop top */
      {
        lc = getenv(localevars[i++]);
        if (lc == NULL) continue;       /* skip all unset locale vars */
        if (*lc == 0x00) continue;       /* skip empty locale strings */
        strncpy(ms->locale,lc,sizeof(ms->locale));        /* pass one */

        /* try various combinations of named file + locale + ".msgs"  */
        snprintf(filename,sizeof(filename)-1,"%s.%s.msgs",fn,ms->locale);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd;

        /* if that didn't work try removing any locale "at" qualifier */
        for (p = ms->locale; *p != 0x00 && *p != '@'; p++); *p = 0x00;
        snprintf(filename,sizeof(filename)-1,"%s.%s.msgs",fn,ms->locale);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd;

        /* if that didn't work try removing any locale dot qualifier  */
        for (p = ms->locale; *p != 0x00 && *p != '.'; p++); *p = 0x00;
        snprintf(filename,sizeof(filename)-1,"%s.%s.msgs",fn,ms->locale);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd;

#if defined(_WIN32) || defined(_WIN64)
        /* if that didn't work then try Windows style locale          */
        p = ms->locale; if (p[2] == '_') p[2] = '-';
        snprintf(filename,sizeof(filename)-1,"%s.%s.msgs",fn,ms->locale);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd;
#endif
      }                                        /* localevars loop end */

    /* try using the name in locale search */
    i = 0; while (*localevars[i] != 0x00)      /* localevars loop top */
      {
        lc = getenv(localevars[i++]);
        if (lc == NULL) continue;       /* skip all unset locale vars */
        if (*lc == 0x00) continue;       /* skip empty locale strings */
        strncpy(ms->locale,lc,sizeof(ms->locale));        /* pass two */

        j = 0; while (*localeoptd[j] != 0x00)  /* localeOPTD loop top */
          { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeOPTD loop end */
        j = 0; while (*localedirs[j] != 0x00)  /* localeDIRs loop top */
          { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeDIRs loop end */

        /* if that didn't work try removing the locale "at" qualifier */
        for (p = ms->locale; *p != 0x00 && *p != '@'; p++); *p = 0x00;

        j = 0; while (*localeoptd[j] != 0x00)  /* localeOPTD loop top */
          { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeOPTD loop end */
        j = 0; while (*localedirs[j] != 0x00)  /* localeDIRs loop top */
          { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeDIRs loop end */

        /* if that didn't work try removing the locale dot qualifier  */
        for (p = ms->locale; *p != 0x00 && *p != '.'; p++); *p = 0x00;

        j = 0; while (*localeoptd[j] != 0x00)  /* localeOPTD loop top */
          { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeOPTD loop end */
        j = 0; while (*localedirs[j] != 0x00)  /* localeDIRs loop top */
          { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeDIRs loop end */

#if defined(_WIN32) || defined(_WIN64)
        /* if that didn't work then try Windows style locale          */
        p = ms->locale; if (p[2] == '_') p[2] = '-';

        j = 0; while (*localeoptd[j] != 0x00)  /* localeOPTD loop top */
          { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeOPTD loop end */
        j = 0; while (*localedirs[j] != 0x00)  /* localeDIRs loop top */
          { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
            rc = fd = open(filename,O_RDONLY);
            if (rc >= 0) return fd; }          /* localeDIRs loop end */
#endif
      }                                        /* localevars loop end */

    /* possibly try hard coded "C" or "POSIX" here */
    lc = "C"; strncpy(ms->locale,lc,sizeof(ms->locale));
    j = 0; while (*localeoptd[j] != 0x00)      /* localeOPTD loop top */
      { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd; }              /* localeOPTD loop end */
    j = 0; while (*localedirs[j] != 0x00)      /* localeDIRs loop top */
      { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd; }              /* localeDIRs loop end */

    /* possibly try hard coded "C" or "POSIX" here */
    lc = "POSIX"; strncpy(ms->locale,lc,sizeof(ms->locale));
    j = 0; while (*localeoptd[j] != 0x00)      /* localeOPTD loop top */
      { snprintf(filename,sizeof(filename)-1,localeoptd[j++],xmmprefix,ms->locale,fn);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd; }              /* localeOPTD loop end */
    j = 0; while (*localedirs[j] != 0x00)      /* localeDIRs loop top */
      { snprintf(filename,sizeof(filename)-1,localedirs[j++],ms->locale,fn);
        rc = fd = open(filename,O_RDONLY);
        if (rc >= 0) return fd; }              /* localeDIRs loop end */

    /* if all attempts fail then return an error indication           */
    ms->msgfile = NULL;
    ms->locale[0] = 0x00;
    ms->applid[0] = 0x00;
    if (errno == 0) errno = ENOENT;              /* if no other error */
    return -1;
  }

/* -------------------------------------------------------------- XMOPEN
 * Open the messages file, read it, get ready for service.
 * Returns: zero upon successful operation,
 * or MSGERR_NOLIB (813) if cannot open the repository file.
 * The VM/CMS counterpart does 'SET LANG' to load the messages file.
 * See also: the catopen() call on many POSIX systems.
 *
 * The first thing we must do is find and open the message repository.
 * This routine looks in several places using a variety of names.
 * If we cannot find the messages file then we cannot proceed.
 */
int xmopen(char*fn,int opts,struct MSGSTRUCT*ms)
  { static char _eyecatcher[] = "xmopen()";
    int rc, fd, memsize, i, j, filesize;
    char *p, *q, *locale, filename[256], *file;
    struct stat statbuf;
    static char ampersand[2] = "&";       /* default escape character */

    file = fn;             /* protect arguments from being written to */

    /* NULL struct pointer means to use global static storage         *
     * unless it was already established, in which case "busy".       */
    if (ms == NULL && msglobal != NULL) return EBUSY;
    if (ms == NULL) { rc = xmopen(file,opts,&msstatic);
        if (rc != 0) return rc; msglobal = &msstatic; return 0; }
    /* we can only do this once and it makees things not thread-safe  */

    /* prepare to search for the message repository                   */
    ms->msgdata = NULL;
    ms->msgtable = NULL;
    ms->msgfile = NULL;
    (void) memset(ms->locale,0x00,sizeof(ms->locale));
    (void) memset(ms->applid,0x00,sizeof(ms->applid));

    rc = fd = xm_findfile(file,ms);   /* find and open the repository */

#ifdef __VM__
    /* do stuff for VM/CMS in here - cms 'xmmshim' macro command      */
    ms->msgmax = 9999;         /* CMS handles it so go to four digits */
    ms->escape = NULL;          /* we won't know the escape character */
    ms->msgopts = opts;
#else

    /* if we can't find the file then return the best error we know   */
    if (rc < 0) { if (errno != 0) perror("xmopen(): xm_findfile()");
                  fprintf(stderr,"message repository '%s' not found.\n",file);
                  return 813; }
    /* There happens to be message number 813 for this condition.     */

    /* allocate memory to hold the message repository source file     */
    rc = fstat(fd,&statbuf);            /* stat the already-open file */
    if (rc < 0) { if (errno != 0) perror("xmopen(): fstat()"); return rc; }
    filesize = statbuf.st_size;          /* total file size, in bytes */
    memsize = filesize + sizeof(filename) + 16;        /* add and pad */
    ms->msgdata = malloc(memsize);
    if (ms->msgdata == NULL)
      { if (errno != 0) return errno; else return ENOMEM; }

    /* read the messages file content into the buffer */
    rc = read(fd,ms->msgdata,filesize);
    if (rc < 0)
      { rc = errno; close(fd);
        free(ms->msgdata); ms->msgdata = NULL;
        if (rc != 0) return rc; else return EBADF; }
    close(fd); fd = -1;

    /* append filename after end of messages buffer */
    p = &ms->msgdata[rc]; *p++ = 0x00;
    strncpy(p,filename,strlen(filename));
    ms->msgfile = p;      /* a safer ref than what xm_findfile() gave */

    /* allocate the message array - FIXME: sizing needs work */
    ms->msgtable = malloc(163840);  /* FIXME: should not be arbitrary */
    if (ms->msgtable == NULL)
      { (void) free(ms->msgdata); ms->msgdata = NULL;
        if (errno != 0) return errno; else return ENOMEM; }
    /* make sure we have clean pointers (all NULLs) */
    (void) memset(ms->msgtable,0x00,163840);             /* arbitrary */

    /* parse the file */
    p = ms->msgdata;
    ms->msgmax = 0;
    ms->escape = NULL;
    while (*p != 0x00)
      {
        /* mark off and measure this line */
        q = p; i = 0;
        while (*p != 0x00 && *p != '\n') { p++; i++; }
/* FIXME: need to strip CR                                            */
/*      if (i > 0) if (q[i-1] == '\r') q[i-1] = 0x00;                 */
        if (*p == '\n') *p++ = 0x00;

        /* skip comments */
        if (*q == '*' || *q == '#') continue;

        /* look for escape character */
        if (ms->escape == NULL)
        if (*q != ' ' && (*q < '0' || *q > '9') && *q != 0x00)
                                           { ms->escape = q; continue; }

        /* ignore short lines */
        if (i < 10) continue;

        /* parse this line */
        q[4] = 0x00;
        i = atoi(q);
        ms->msgtable[i] = &q[8];

        /* keep track of the highest message number in the file */
        if (i > ms->msgmax) ms->msgmax = i;
      }

    /* handle SYSLOG and record other options */
    ms->msgopts = opts;
    if (ms->msgopts & MSGFLAG_SYSLOG) {
      /* figure out syslog identity */
      openlog(ms->applid,LOG_PID,MSGROUTE_DEFAULT); }

#endif

    /* establish major and minor prefix area */
    /* if (ms->prefix == NULL || *ms->prefix == 0x00) */ ms->prefix = ms->applid;
    p = ms->prefix;     /* application prefix not installation prefix */
    for (i = 0; i < 3 && *p != 0x00; i++) ms->pfxmaj[i] = toupper((int)*p++);
    ms->pfxmaj[i] = 0x00;
    for (i = 0; i < 3 && *p != 0x00; i++) ms->pfxmin[i] = toupper((int)*p++);
    ms->pfxmin[i] = 0x00;

    /* default "caller" is the user, but is better as a function name */

    /* force clear other elements of the struct */
    ms->msgnum = 0;
    ms->msgc = 0;   ms->msgv = NULL;
    ms->msgbuf = NULL;   ms->msglen = 0;   ms->msgtext = NULL;

    ms->msglevel = 0;
    ms->msgfmt = 0;   ms->msgline = 0;  /* neither is yet implemented */
    ms->letter = NULL;

    /* if no escape character set then make it an ampersand           */
    if (ms->escape == NULL) ms->escape = ampersand;

    /* return success */
    return 0;
  }

/* -------------------------------------------------------------- XMMAKE
 * This is the central function: make a message.
 * All other print, string, and write functions are derivatives.
 * Returns: zero upon successful operation, ENOENT or 814 if no message
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 */
int xmmake(struct MSGSTRUCT*ms)
  { static char _eyecatcher[] = "xmmake()";
    int  rc, i, j;
    char *p, *q;

    if (ms == NULL) return EINVAL; /* invalid argument */
    if (ms->msgnum <= 0) return EINVAL; /* invalid argument */
    if (ms->msgnum > ms->msgmax) return EINVAL; /* invalid argument */

#ifdef __VM__
    return xm_make_cms(ms);
#else

    /* NULL pointer indicates an undefined message */
    if (ms->msgtable[ms->msgnum] == NULL) return 814;     /* no entry */

    p = ms->letter = ms->msgtable[ms->msgnum];

    i = rc = snprintf(ms->msgbuf,ms->msglen,"%s%s%03d%c ",
      ms->pfxmaj,ms->pfxmin,ms->msgnum,*p);

    p++; if (*p == ' ') p++;
    ms->msgtext = p;

    /* perform token replacement, the main purpose of this library    */
    while (i < ms->msglen)
      { if (*p == *ms->escape)
          { p++;
            j = 0;
            while ('0' <= *p && *p <= '9')
              { j = j * 10;
                j = j + (*p & 0x0f);
                p++; }
            if (j < ms->msgc) q = ms->msgv[j];
                         else q = "";
            while (*q != 0x00 && i < ms->msglen)
              { ms->msgbuf[i] = *q;
                i++; q++; }
            ms->msgbuf[i] = *p;
            if (*p == 0x00) break;
          } else if (*p == '\\') {
            p++; switch (*p) {
              case 'n': ms->msgbuf[i] = '\n';
                        break;
              case 't': ms->msgbuf[i] = '\t';
                        break;
              default: ms->msgbuf[i] = '*';
                        break;
                             }
            if (*p == 0x00) break;
            i++; p++;
          } else {
            ms->msgbuf[i] = *p;
            if (*p == 0x00) break;
            i++; p++;
                 }
      }
    ms->msglen = i;

    /* optional syslogging */
    if (ms->msgopts & MSGFLAG_SYSLOG) {
      if (ms->msglevel == 0) {
        switch (*ms->letter) {
/*           MSGLEVEL_DEBUG:                    LOG_DEBUG         7 */
        case MSGLEVEL_INFO:      ms->msglevel = LOG_INFO;     /* I6 */ break;
        case MSGLEVEL_REPLY:     ms->msglevel = LOG_NOTICE;   /* R5 */ break;
        case MSGLEVEL_WARNING:   ms->msglevel = LOG_WARNING;  /* W4 */ break;
        case MSGLEVEL_ERROR:     ms->msglevel = LOG_ERR;      /* E3 */ break;
        case MSGLEVEL_SEVERE:    ms->msglevel = LOG_CRIT;     /* S2 */ break;
        case MSGLEVEL_TERMINAL:  ms->msglevel = LOG_ALERT;    /* T1 */ break;
/*           MSGLEVEL_EMERG:                    LOG_EMERG         0 */
/*                                              INTERNAL_NOPRI      */
        default:                 ms->msglevel = LOG_INFO;              break;
                           } }
                                      }

    return 0;
#endif
  }

/* ------------------------------------------------------------- XMPRINT
 * Print a message to stdout or stderr depending on level/letter.
 * Newline automatically appended. Optionally SYSLOG the message.
 * Returns: number of characters printed, negative indicates error
 * Calls: xmmake()
 * Return value does not reflect SYSLOG effects or SYSLOG errors.
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 *
 *    NOTE: as of 2026-06-30 (2.2.9) there is some kind of bug
 *    where xmprint() may fail even if xmstring() would work.
 */
int xmprint(int msgnum,int msgc,char*msgv[],int msgopts,struct MSGSTRUCT*ms0)
  { static char _eyecatcher[] = "xmprint()";
    int  rc;
    struct MSGSTRUCT ts, *ms;
    char buffer[1024];

    ms = ms0;                /* treat supplied arguments as read-only */

    /* NULL message struct means use the static common struct */
    if (ms == NULL) ms = msglobal;
    if (ms == NULL) return xm_negative(EINVAL);
    (void) memcpy(&ts,ms,sizeof(ts));    /* make a copy of the struct */
    ms = &ts;

    ms->msgbuf = buffer;    /* output buffer supplied by this routine */
    ms->msglen = sizeof(buffer) - 1;     /* size of the output buffer */
    ms->msgnum = msgnum;    /* message number specified by the caller */
    ms->msgc = msgc;               /* count of tokens from the caller */
    ms->msgv = msgv;                   /* token array from the caller */
    ms->msglevel = 0;             /* zero means set level from letter */
    ms->msgopts |= msgopts;

    rc = xmmake(ms);                              /* make the message */
    if (rc != 0) return xm_negative(rc);    /* if error then negative */

    /* optionally route to SYSLOG */
    if (ms->msgopts & MSGFLAG_SYSLOG) syslog(ms->msglevel,"%s",ms->msgbuf);

    if (ms->msgopts & MSGFLAG_NOPRINT) return 0;      /* non-printing */

    if (ms->msglevel > 5)
    rc = fprintf(stdout,"%s\n",ms->msgbuf);   /* 5 and 6 are "normal" */
    else                                      /* (and 7 is "debug")   */
    rc = fprintf(stderr,"%s\n",ms->msgbuf);   /* 4, 3, 2, 1 "errors"  */

    return rc;            /* normal return is number of bytes printed */
  }

/* ------------------------------------------------------------- XMWRITE
 * Write a message to the indicated file descriptor.
 * Newline automatically appended. Optionally SYSLOG the message.
 * Returns: number of bytes written, negative indicates error
 * Calls: xmmake()
 * The return value does not reflect SYSLOG effects or errors.
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 */
int xmwrite(int fd,int msgnum,int msgc,char*msgv[],int msgopts,struct MSGSTRUCT*ms0)
  { static char _eyecatcher[] = "xmwrite()";
    int  rc;
    struct MSGSTRUCT ts, *ms;
    char buffer[1024];

    ms = ms0;                /* treat supplied arguments as read-only */

    /* NULL message struct means use the static common struct */
    if (ms == NULL) ms = msglobal;
    if (ms == NULL) return xm_negative(EINVAL);
    (void) memcpy(&ts,ms,sizeof(ts));    /* make a copy of the struct */
    ms = &ts;

    ms->msgbuf = buffer;    /* output buffer supplied by this routine */
    ms->msglen = sizeof(buffer) - 1;     /* size of the output buffer */
    ms->msgnum = msgnum;    /* message number specified by the caller */
    ms->msgc = msgc;               /* count of tokens from the caller */
    ms->msgv = msgv;                   /* token array from the caller */
    ms->msglevel = 0;             /* zero means set level from letter */
    ms->msgopts |= msgopts;

    rc = xmmake(ms);                              /* make the message */
    if (rc != 0) return xm_negative(rc);    /* if error then negative */

    /* optionally route to SYSLOG */
    if (ms->msgopts & MSGFLAG_SYSLOG) syslog(ms->msglevel,"%s",ms->msgbuf);

    ms->msgbuf[ms->msglen++] = '\n';
    rc = write(fd,ms->msgbuf,ms->msglen);

    return rc;
  }

/* ------------------------------------------------------------ XMSTRING
 * Build the message and put it into a string buffer. No newline.
 * Returns: number of bytes in string, negative indicates error
 * Calls: xmmake()
 * The VM/CMS counterpart (for Rexx variables) is the XMITMSG command.
 */
int xmstring(char*output,int outlen,
            int msgnum,int msgc,char*msgv[],            struct MSGSTRUCT*ms0)
  { static char _eyecatcher[] = "xmstring()";
    int  rc;
    struct MSGSTRUCT ts, *ms;

    ms = ms0;                /* treat supplied arguments as read-only */

    /* NULL message struct means use the static common struct */
    if (ms == NULL) ms = msglobal;
    if (ms == NULL) return xm_negative(EINVAL);
    (void) memcpy(&ts,ms,sizeof(ts));    /* make a copy of the struct */
    ms = &ts;

    ms->msgbuf = output;      /* output buffer supplied by the caller */
    ms->msglen = outlen;                 /* size of the output buffer */
    ms->msgnum = msgnum;    /* message number specified by the caller */
    ms->msgc = msgc;               /* count of tokens from the caller */
    ms->msgv = msgv;                   /* token array from the caller */
    ms->msglevel = 0;             /* zero means set level from letter */

    rc = xmmake(ms);                              /* make the message */
    if (rc != 0) return xm_negative(rc);    /* if error then negative */

    return ms->msglen;   /* normal return is length of message string */
  }

/* ------------------------------------------------------------- XMCLOSE
 * Close (figuratively): free common storage and reset static variables.
 * Returns: zero upon successful operation
 */
int xmclose(struct MSGSTRUCT*ms0)
  { static char _eyecatcher[] = "xmclose()";
    struct MSGSTRUCT *ms;

    ms = ms0;

    /* NULL struct pointer means to use global static storage */
    if (ms == NULL && msglobal == NULL) return EINVAL;
    if (ms == NULL) { ms = msglobal; msglobal = NULL; }

    /* release any allocated storage for this MSGSTRUCT */
    if (ms->msgdata != NULL) { (void) free(ms->msgdata); ms->msgdata = NULL; }
    if (ms->msgtable != NULL) { (void) free(ms->msgtable); ms->msgtable = NULL; }
    if (ms->msgopts & MSGFLAG_SYSLOG) closelog();
    ms->msgopts = 0;

    /* clear character fields */
    (void) memset(ms->pfxmaj,0x00,sizeof(ms->pfxmaj));
    (void) memset(ms->pfxmin,0x00,sizeof(ms->pfxmin));
    (void) memset(ms->locale,0x00,sizeof(ms->locale));
    (void) memset(ms->applid,0x00,sizeof(ms->applid));

    /* force clear other elements of the struct */
    ms->msgnum = ms->msgmax = 0;
    ms->msgc = 0;   ms->msgv = NULL;
    ms->msgbuf = NULL;   ms->msglen = 0;   ms->msgtext = NULL;

    ms->msglevel = 0;
    ms->msgfmt = 0;   ms->msgline = 0;  /* neither is yet implemented */
    ms->caller = ms->prefix = ms->letter = ms->escape = NULL;
    ms->msgfile = NULL;

    return 0;
  }

/* ---------------------------------------------------------- XM_LEV2PRI
 *  Return an integer priority for a given severity level letter.
 *  This routine is not presently used because xmmake() handles it.
 */
int xm_lev2pri(char*l)
  { static char _eyecatcher[] = "xm_lev2pri()";
    switch (*l) {
      case 'I': case 'i':       /* MSGLEVEL_INFO */
        return LOG_INFO;        /* 6 */                           break;
      case 'R': case 'r': case 'N': case 'n':
        return LOG_NOTICE;      /* 5 */                           break;
      case 'W': case 'w':       /* MSGLEVEL_WARNING */
        return LOG_WARNING;     /* 4 */                           break;
      case 'E': case 'e':       /* MSGLEVEL_ERROR */
        return LOG_ERR;         /* 3 */                           break;
      case 'S': case 's': case 'C': case 'c':
        return LOG_CRIT;        /* 2 */                           break;
      case 'T': case 't':       /* MSGLEVEL_TERMINAL */
        return LOG_ALERT;       /* 1 */                           break;
      default:
        return 0;                                                 break;
                }
    return 0;
  }

/* --------------------------------------------------------- XM_NEGATIVE
 *  Force the supplied integer to be negative. Good for error indications.
 *  Yeah, yeah, ... it's cheezy. But it works.
 */
int xm_negative(int n)
  { static char _eyecatcher[] = "xm_negative()";
    if (n < 0) return n; else return 0 - n; }

/* ---------------------------------------------------------- XM_DELIVER
 *    Deliver a message in a buffer to stdout, stderr, or SYSLOG.
 *    Buffer is passed as a pointer to a NULL-terminated string.
 *    Main reason this even exists is to hide SYSLOG variances.
 */
int xm_deliver(char*buff,int type)
  { static char _eyecatcher[] = "xm_deliver()";
    switch (type) {
      case 'I': case 'i':       /* MSGLEVEL_INFO */
        syslog(LOG_INFO,"%s",buff);                               break;
      case 'R': case 'r': case 'N': case 'n':
        syslog(LOG_NOTICE,"%s",buff);                             break;
      case 'W': case 'w':       /* MSGLEVEL_WARNING */
        syslog(LOG_WARNING,"%s",buff);                            break;
      case 'E': case 'e':       /* MSGLEVEL_ERROR */
        syslog(LOG_ERR,"%s",buff);                                break;
      case 'S': case 's': case 'C': case 'c':
        syslog(LOG_CRIT,"%s",buff);                               break;
      case 'T': case 't':       /* MSGLEVEL_TERMINAL */
        syslog(LOG_ALERT,"%s",buff);                              break;
      case '*':
        return fprintf(stdout,"%s\n",buff);                       break;
      case '!':
        return fprintf(stderr,"%s\n",buff);                       break;
      default:
        return 0;                                                 break;
                  }
    return 0;
  }

/* ------------------------------------------------------------- XM_SAFE
 *    Use this routine to confirm that the first character in the
 *    provided string is not one of the remaining in the string.
 *    NOTE: this function returns zero for false and one for true
 */
int xm_safe(char*s)
  { char *q, *p;
    if (*s == 0x00) return 0;       /* if candidate is NULL then fail */
    q = p = s;          /* start at the start reserving to-be-checked */
    p++;             /* list-o-bad-chars begins second char in string */
    while (*p != 0x00) { if (*q == *p++) return 0; }
    return 1; }

/* --------------------------------------------------------- XM_MAKE_CMS
 * This routine replaces xmmake() when we are running on VM/CMS.
 * This function assumes that we are in a CMS or OpenVM environment.
 */
int xm_make_cms(struct MSGSTRUCT*ms)
  {
    int i, j, rc;
    char x1[256], x2[384], *p, checkchars[16];
    char xd = '/';

    /* ms->msgopts is ignored here                                    */
    checkchars[0] = 0x00;        /* this to be replaced with examinee */
    checkchars[1] = xd;           /* delimiter to be used by the shim */
    checkchars[2] = '"';         /* double quote is special for shell */
    checkchars[3] = '\'';        /* single quote is special for shell */
    checkchars[4] = '\\';           /* backslash is special for shell */
    checkchars[5] = '(';           /* open paren introduces sub-shell */
    checkchars[6] = ')';                /* close paren ends sub-shell */
    checkchars[7] = '*';             /* asterisk may lead to globbing */
    checkchars[8] = '&';              /* ampersand means "background" */
    checkchars[9] = 0x00;                   /* NULL marks end of list */

/*  char *prefix;       // default is applid[0..2]||caller[0..2]      */
/*  ms->prefix                                                     // */
/*  ms->applid                                                     // */
/*  ms->caller                                                     // */
/*  ms->pfxmaj                                                     // */
/*  ms->pfxmin                                                     // */

    /* build a concatenation of all supplied replacement tokens       */
    j = 0;
    for (i = 1; i < ms->msgc; i++)
      { x1[j++] = xd;                /* slip in the delimiter at each */
        if (j >= sizeof(x1)-1) break;                /* stay in range */
        p = ms->msgv[i];           /* pointing to this specific token */
fprintf(stderr,"looping on '%s' %d\n",p,i);
        while (*p != 0x00)         /* tack it onto the growing string */
          { checkchars[0] = *p;        /* load the char to be checked */
            if (xm_safe(checkchars)) x1[j++] = *p++; else p++;
            if (j >= sizeof(x1)-1) break; }
      }
    x1[j] = 0x00;                   /* terminate the resulting string */

    /* now drive our shim to invoke XMITMSG command in CMS            */
    p = getenv("SHELL");
    if (p != NULL && *p != 0x00)    /* if we have a shell then use it */
    sprintf(x2,"cms 'xmmshim %d %s %s %x %d %s'",
            ms->msgnum,ms->pfxmaj,ms->pfxmin,ms->msgbuf,ms->msglen,x1);
    else          /* otherwise go directly to the CMS command handler */
    sprintf(x2,"xmmshim %d %s %s %x %d %s",
            ms->msgnum,ms->pfxmaj,ms->pfxmin,ms->msgbuf,ms->msglen,x1);
fprintf(stderr,"%s\n",x2);
    rc = system(x2);
    if (rc != 0) return rc;

    ms->msglen = strlen(ms->msgbuf);

    return 0;
  }


