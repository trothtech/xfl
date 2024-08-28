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
#include <syslog.h>
#include <errno.h>

#include <libgen.h>
#include <ctype.h>

#include "xmitmsgx.h"

extern char *xmmprefix; /* installation prefix not application prefix */

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
                ""          /* empty string marks the end of the list */
                     };

/* These are the directories where we might find locale support:

                 /usr/share/locale/%s/%s.msgs
                 /usr/lib/nls/msg/%s/%s.msgs
                 /usr/lib/locale/%s/%s.msgs
                 /usr/share/nls/%s/%s.msgs

   The xmopen() routine will search all of the above. We do not use
   a per-platform single path because any given platform might have
   multiple locale directories and paths.

   X11 locale directories are not searched because their content is different.

   Examples:
      /usr/share/locale/en_US/%s.msgs           locale=en_US
      /usr/lib/locale/en_US.UTF-8/%s.msgs       locale=en_US.UTF-8
      /usr/lib/locale/en_US.ISO8859-1/%s.msgs   locale=en_US.ISO8859-1

 */

static struct MSGSTRUCT *msglobal = NULL, msstatic;

/* ---------------------------------------------------------------- OPEN
 * Open the messages file, read it, get ready for service.
 * Returns: zero upon successful operation, or 813 if cannot open the repository file
 * The VM/CMS counterpart does 'SET LANG' to load the messages file.
 * See also the catopen() call on many POSIX systems.
 *
 * The first thing we must do is find and open the message repository.
 * This routine looks in several places using a variety of names.
 * If we cannot find the messages file then we cannot proceed.
 */
int xmopen(unsigned char*file,int opts,struct MSGSTRUCT*ms)
  {
    int rc, fd;
    struct stat statbuf;
    unsigned char filename[256]; int filesize;
    int memsize, i;
    unsigned char *p, *q, *escape, *locale;

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

    /* try the file directly as if full name was supplied (sans ext)  */
    (void) snprintf(filename,sizeof(filename)-1,
                "%s.msgs",file);
    filename[sizeof(filename)-1] = 0x00;
    rc = stat(filename,&statbuf);

    i = 0;                                     /* localevars loop top */
    while (*localevars[i] != 0x00) {

    /* if that didn't work then try filename plus locale variables    */
    if (rc != 0) {
        locale = getenv(localevars[i]);
        if (locale != NULL && *locale != 0x00) {
            (void) strncpy(ms->locale,locale,sizeof(ms->locale)-1);

            (void) snprintf(filename,sizeof(filename)-1,
                "%s.%s.msgs",file,ms->locale);
            filename[sizeof(filename)-1] = 0x00;
            rc = stat(filename,&statbuf); }

            /* if that didn't work then try removing locale dot qual  */
            if (rc != 0) {
                for (p = ms->locale; *p != 0x00 && *p != '.'; p++);
                if (*p != 0x00) { *p = 0x00;

            (void) snprintf(filename,sizeof(filename)-1,
                "%s.%s.msgs",file,ms->locale);
            filename[sizeof(filename)-1] = 0x00;
            rc = stat(filename,&statbuf); } }
                 }

        i++; }                                 /* localevars loop end */

    /* beyond this point, ignore any prepended path info              */
    file = basename(file);

    /* NOTE: in the following several stanzas, we attempt variations  *
     *       with several standard locations for locale content       */

    i = 0;                                     /* localevars loop top */
    while (*localevars[i] != 0x00) {

    /* if that didn't work then try finding locale in standard places */
    if (rc != 0) {
        locale = getenv(localevars[i]);
        if (locale != NULL && *locale != 0x00) {
            (void) strncpy(ms->locale,locale,sizeof(ms->locale)-1);

            (void) snprintf(filename,sizeof(filename)-1,
                "%s/share/locale/%s/%s.msgs",xmmprefix,ms->locale,file);
            rc = stat(filename,&statbuf);
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/nls/msg/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/nls/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }

            /* if that didn't work then try removing locale dot qual  */
            if (rc != 0) {
                for (p = ms->locale; *p != 0x00 && *p != '.'; p++);
                if (*p != 0x00) { *p = 0x00;

            (void) snprintf(filename,sizeof(filename)-1,
                "%s/share/locale/%s/%s.msgs",xmmprefix,ms->locale,file);
            rc = stat(filename,&statbuf);
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/nls/msg/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/nls/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }

                                    }
                         }
                                        } }
        i++; }                                 /* localevars loop end */

    if (rc != 0) {
        locale = "C";
            (void) strncpy(ms->locale,locale,sizeof(ms->locale)-1);

            (void) snprintf(filename,sizeof(filename)-1,
                "%s/share/locale/%s/%s.msgs",xmmprefix,ms->locale,file);
            rc = stat(filename,&statbuf);
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/nls/msg/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/lib/locale/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }
            if (rc != 0) {
            (void) snprintf(filename,sizeof(filename)-1,
                "/usr/share/nls/%s/%s.msgs",ms->locale,file);
            rc = stat(filename,&statbuf); }

            /* If this had been an environmentally supplied locale    */
            /* then we would remove any dotted qualifier here         */
            /* and re-drive the myriad stat() calls.                  */

                 }

    /* if we can't find the file then return the best error we know   */
    if (rc != 0)
      { if (errno != 0) return errno; else return rc; }
    /* There happens to be message number 813 for this condition.     */

    /* allocate memory to hold the message repository source file     */
    filesize = statbuf.st_size;          /* total file size, in bytes */
    memsize = filesize + sizeof(filename) + 16;        /* add and pad */
    ms->msgdata = malloc(memsize);
    if (ms->msgdata == NULL)
      { if (errno != 0) return errno; else return ENOMEM; }

    /* open the message repository */
    rc = fd = open(filename,O_RDONLY);
    if (rc < 0)
      { (void) free(ms->msgdata); ms->msgdata = NULL;
        if (errno != 0) return errno; else return EBADF; }

    /* read the file into the buffer */
    rc = read(fd,ms->msgdata,filesize);
    (void) close(fd);
    if (rc < 0)
      { (void) free(ms->msgdata); ms->msgdata = NULL;
        if (errno != 0) return errno; else return EBADF; }

    /* put filename at end of buffer */
    p = &ms->msgdata[rc]; *p++ = 0x00;
    (void) strncpy(p,filename,sizeof(filename)-1);
    ms->msgfile = p;

    /* allocate the message array - sizing needs work */
    ms->msgtable = malloc(163840);
    if (ms->msgtable == NULL)
      { (void) free(ms->msgdata); ms->msgdata = NULL;
        if (errno != 0) return errno; else return ENOMEM; }
    /* make sure we have clean pointers (all NULLs) */
    (void) memset(ms->msgtable,0x00,163840);

    /* parse the file */
    p = ms->msgdata;
    ms->msgmax = 0;
    while (*p != 0x00)
      {

        /* mark off and measure this line */
        q = p; i = 0;
        while (*p != 0x00 && *p != '\n') { p++; i++; }
        if (*p == '\n') *p++ = 0x00;

        /* skip comments */
        if (*q == '*' || *q == '#') continue;

        /* look for escape character */
        if (*q != ' ' && (*q < '0' || *q > '9')) { ms->escape = q; continue; }

        /* ignore short lines */
        if (i < 10) continue;

        /* parse this line */
        q[4] = 0x00;
        i = atoi(q);
        ms->msgtable[i] = &q[8];

        /* keep track of the highest message number in the file */
        if (i > ms->msgmax) ms->msgmax = i;

      }

    /* use basename of the file as the applic */
    p = (unsigned char*) basename(ms->msgfile);
    (void) strncpy(ms->applid,p,sizeof(ms->applid)-1);
    p = ms->applid;
    while (*p != 0x00 && *p != '.') p++; *p = 0x00;

    /* establish major and minor prefix area */
    /* if (ms->prefix == NULL || *ms->prefix == 0x00) */ ms->prefix = ms->applid;
    p = ms->prefix;     /* application prefix not installation prefix */
    for (i = 0; i < 3 && *p != 0x00; i++) ms->pfxmaj[i] = toupper((int)*p++);
    ms->pfxmaj[i] = 0x00;
    for (i = 0; i < 3 && *p != 0x00; i++) ms->pfxmin[i] = toupper((int)*p++);
    ms->pfxmin[i] = 0x00;

    /* handle SYSLOG and record other options */
    ms->msgopts = opts;
    if (ms->msgopts & MSGFLAG_SYSLOG) {
      /* figure out syslog identity */
      openlog(ms->applid,LOG_PID,LOG_USER); }

    /* default "caller" is the user, but is better as a function name */

    /* force clear other elements of the struct */
    ms->msgnum = 0;
    ms->msgc = 0;   ms->msgv = NULL;
    ms->msgbuf = NULL;   ms->msglen = 0;   ms->msgtext = NULL;

    ms->msglevel = 0;
    ms->msgfmt = 0;   ms->msgline = 0;  /* neither is yet implemented */
    ms->letter = NULL;

    /* return success */
    return 0;
  }

/* ---------------------------------------------------------------- MAKE
 * This is the central function: make a message.
 * All other print, string, and write functions are derivatives.
 * Returns: zero upon successful operation, ENOENT or 814 if no message
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 */
int xmmake(struct MSGSTRUCT*ms)
  {
    int  rc, i, j;
    unsigned char *p, *q;

    if (ms == NULL) return EINVAL; /* invalid argument */
    if (ms->msgnum <= 0) return EINVAL; /* invalid argument */
    if (ms->msgnum > ms->msgmax) return EINVAL; /* invalid argument */

    /* NULL pointer indicates an undefined message */
    if (ms->msgtable[ms->msgnum] == NULL) return 814;     /* no entry */

    p = ms->letter = ms->msgtable[ms->msgnum];

    i = rc = snprintf(ms->msgbuf,ms->msglen,"%s%s%03d%c ",
      ms->pfxmaj,ms->pfxmin,ms->msgnum,*p);

    p++; if (*p == ' ') p++;
    ms->msgtext = p;

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
  }

/* --------------------------------------------------------------- PRINT
 * Print a message, stdout or stderr depending on level/letter.
 * Newline automatically appended. Optionally SYSLOG the message.
 * Returns: number of characters printed, negative indicates error
 * Calls: xmmake()
 * Return value does not reflect SYSLOG effects or errors.
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 */
int xmprint(int msgnum,int msgc,unsigned char*msgv[],int msgopts,struct MSGSTRUCT*ms)
  {
    int  rc;
    struct MSGSTRUCT ts;
    unsigned char buffer[256];

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

    if (ms->msgopts & MSGFLAG_NOPRINT) ; else
    if (ms->msglevel > 5)
    rc = fprintf(stdout,"%s\n",ms->msgbuf);   /* 5 and 6 are "normal" */
    else                                      /* (and 7 is "debug")   */
    rc = fprintf(stderr,"%s\n",ms->msgbuf);   /* 4, 3, 2, 1 "errors"  */

    return rc;
  }

/* --------------------------------------------------------------- WRITE
 * Write a message to the indicated file descriptor.
 * Newline automatically appended. Optionally SYSLOG the message.
 * Returns: number of bytes written, negative indicates error
 * Calls: xmmake()
 * The return value does not reflect SYSLOG effects or errors.
 * The VM/CMS counterpart is the APPLMSG macro (high level assembler).
 */
int xmwrite(int fd,int msgnum,int msgc,unsigned char*msgv[],int msgopts,struct MSGSTRUCT*ms)
  {
    int  rc;
    struct MSGSTRUCT ts;
    unsigned char buffer[256];

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

/* -------------------------------------------------------------- STRING
 * Build the message and put it into a string buffer. No newline.
 * Returns: number of bytes in string, negative indicates error
 * Calls: xmmake()
 * The VM/CMS counterpart (for Rexx variables) is the XMITMSG command.
 */
int xmstring(unsigned char*output,int outlen,int msgnum,int msgc,unsigned char*msgv[],struct MSGSTRUCT*ms)
  {
    int  rc;
    struct MSGSTRUCT ts;

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

/* --------------------------------------------------------------- CLOSE
 * Close (figuratively): free common storage and reset static variables.
 * Returns: zero upon successful operation
 */
int xmclose(struct MSGSTRUCT*ms)
  {
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

/* ------------------------------------------------------------- LEV2PRI
 *  Return an integer priority for a given severity level letter.
 *  This routine is not presently used because xmmake() handles it.
 */
int xm_lev2pri(unsigned char*l)
  {
    switch (*l) {
      case 'I': case 'i':       /* MSGLEVEL_INFO */
        return LOG_INFO;        /* 6 */
      case 'R': case 'r': case 'N': case 'n':
        return LOG_NOTICE;      /* 5 */
      case 'W': case 'w':       /* MSGLEVEL_WARNING */
        return LOG_WARNING;     /* 4 */
      case 'E': case 'e':       /* MSGLEVEL_ERROR */
        return LOG_ERR;         /* 3 */
      case 'S': case 's': case 'C': case 'c':
        return LOG_CRIT;        /* 2 */
      case 'T': case 't':       /* MSGLEVEL_TERMINAL */
        return LOG_ALERT;       /* 1 */
      default:
        return 0;
                }
    return 0;
  }

/* ------------------------------------------------------------ NEGATIVE
 *  Force the supplied integer to be negative. Good for error indications.
 *  Yeah, yeah, ... it's cheezy. But it works.
 */
int xm_negative(int n)
  { if (n < 0) return n;
          else return 0 - n;
  }


