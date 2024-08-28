/*
 *
 *        Name: xfllib.c (C program source)
 *              Ductwork library
 *        Date: 2023-06-12 (Monday)
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>

#include "configure.h"
/* defines PREFIX among other things*/

#include <xmitmsgx.h>
/* static */ struct MSGSTRUCT xflmsgs;
char *xmmprefix = PREFIX;

#include "xfl.h"
/* static */ int xfl_version = XFL_VERSION;
/* static */ struct PIPECONN *xfl_pipeconn = NULL;
/* static */ struct PIPESTAGE *xfl_pipestage = NULL;


static int xfl_errno = XFL_E_NONE;
static int xfl_dotrace = 0;

/* ---------------------------------------------------------------------
 *  This is a byte-at-a-time read function which attempts to consume
 *  a single line of text and no more.
 */
ssize_t readl(int fd, void *buf, size_t count)
  { static char _eyecatcher[] = "readl()";
    int i, rc;
    char *p;

    i = 0; p = buf;
    while (i < count)
      {
        /* try to read just one byte from the socket */
        rc = read(fd,p,1);
        /* this is a tight spin while the stack gives us empty reads  */
        while (rc == 0) rc = read(fd,p,1);
        /* if we got an error then return now and indicate the fact   */
        if (rc < 0) return rc;
        /* treat a newline as end-of-string */
        if (*p == '\n') *p = 0x00;
        /* upon finding end-of-string then return */
        if (*p == 0x00) break;
        /* ignore carriage return but for all others bump the pointer */
        if (*p == '\r') { i++; p++; }
      }
    return i;
  }

/* ---------------------------------------------------------------------
 *  NOTE: this routine allocates a string buffer
 *        which the caller must eventually free to avoid memory leaks.
 *  NOTE: this routine ignores argv[0] so that you can call it
 *        directly from main() with supplied argc/argv as-is.
 */
char*xfl_argcat(int argc,char*argv[])
  { static char _eyecatcher[] = "xfl_argcat()";
    int i, buflen;
    char *buffer, *p, *q;

    /* account for intervening blanks between tokens                  */
    buflen = argc;
    /* result is plus one for all args, allowing for the terminator,  */
    /* but also one more for argv[0] which is harmless, but we later  */
    /* have to remember to back-off the count by TWO for real length  */

    /* add-up the sizes of the tokens                                 */
    for (i = 1; i < argc; i++) buflen = buflen + strlen(argv[i]);

    /* allocate a buffer to hold the combined tokens as one string    */
    buffer = malloc(buflen);

    if (buffer == NULL)          /* 0026 E Error &1 obtaining storage */
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("argcat(): malloc()"); /* provide standard Unix report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return NULL; }

    /* insert the first token into the string                         */
    p = buffer;
    if (argc > 1) { q = argv[1]; while (*q) *p++ = *q++; }

    /* now concatenate all remaining tokens delimited with blanks     */
    for (i = 2; i < argc; i++)
      { *p++ = ' ';        /* insert a blank space between the tokens */
                    q = argv[i]; while (*q) *p++ = *q++; }
    *p = 0x00;

    return buffer;
  }

/* --------------------------------------------------------------- ERROR
 *  Report an error (including some non-errors) using formal messages.
 */
int xfl_error(int msgn,int msgc,char*msgv[],char*caller)
  { static char _eyecatcher[] = "xfl_error()";
    char msgbuf[256];
    int rc;

    rc = xmopen("xfl",0,&xflmsgs);

    /* some functions indicate the error with a negative number       */
    if (msgn < 0) msgn = 0 - msgn;   /* force message number positive */

    /* populate the message struct - some of this is outside the API  */
    xflmsgs.msgnum = msgn;
    xflmsgs.msgc = msgc;
    xflmsgs.msgv = (unsigned char**) msgv;
    xflmsgs.msgbuf = msgbuf;
    xflmsgs.msglen = sizeof(msgbuf) - 1;

    /* do we need this? */
    xflmsgs.msglevel = 0;

    /* using pfxmaj and pfxmin is definitely outside the XMITMSGX API */
    strncpy(xflmsgs.pfxmaj,"XFL",4);
    strncpy(xflmsgs.pfxmin,caller,4);
    /* also remember to up-case the latter */

    /* make the message */
    rc = xmmake(&xflmsgs);
    if (rc != 0) return rc;

    /* print it */
    fprintf(stderr,"%s\n",msgbuf);

    return 0;
  }

/* --------------------------------------------------------------- TRACE
 *  Similar to xfl_error() but for debugging and tracing.
 */
int xfl_trace(int msgn,int msgc,char*msgv[],char*caller)
  { static char _eyecatcher[] = "xfl_trace()";
    char msgbuf[256];
    int rc;

    /* consider tracing ... do we want it or not? */
    if (xfl_dotrace == 0)
      { char *p;
        p = getenv("PIPEOPT_TRACE");
        if (p != NULL && *p != 0x00) xfl_dotrace = 1; }
    if (xfl_dotrace == 0) return 0;

    rc = xmopen("xfl",0,&xflmsgs);

    /* some functions indicate the error with a negative number       */
    if (msgn < 0) msgn = 0 - msgn;   /* force message number positive */

    /* populate the message struct - some of this is outside the API  */
    xflmsgs.msgnum = msgn;
    xflmsgs.msgc = msgc;
    xflmsgs.msgv = (unsigned char**) msgv;
    xflmsgs.msgbuf = msgbuf;
    xflmsgs.msglen = sizeof(msgbuf) - 1;

    /* do we need this? */
    xflmsgs.msglevel = 0;

    /* using pfxmaj and pfxmin is definitely outside the XMITMSGX API */
    strncpy(xflmsgs.pfxmaj,"XFL",4);
    strncpy(xflmsgs.pfxmin,caller,4);
    /* also remember to up-case the latter */

    /* make the message */
    rc = xmmake(&xflmsgs);
    if (rc != 0) return rc;

    /* log it */
    syslog(LOG_DEBUG,"%s",msgbuf);

    return 0;
  }

#ifdef DELETE_THIS_PLEASE

/* ----------------------------------------------------------- STAGEEXEC
      DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED
 *       Calls: ...
 *   Called by: ...
 *
 * NOTE: this routine is destructive to the argument string supplied
 */
int xfl_stageexec(char*args,PIPECONN*pc[])
  { static char _eyecatcher[] = "xfl_stageexec()";
    int rc;
    char *verb, *argv[3], pipeconn[256], pipeprog[256], *p;
    PIPECONN *pi, *po;

    /* skip past any leading white space */
    while (*args == ' ' || *args == '\t') args++;
    verb = args;
    while (*args != 0x00 && *args != ' ' && *args != '\t') args++;
    if (*args != 0x00) *args++ = 0x00;

    argv[0] = verb;
    argv[1] = args;
    argv[2] = NULL;

    p = pipeconn;
    pi = pc[0];
    po = pc[1];
    if (pi != NULL)
      {
        rc = sprintf(p,"*.INPUT:%d,%d ",pi->fdf,pi->fdr);
        p = &p[rc];
      }
    if (po != NULL)
      {
        printf("*.OUTPUT:%d,%d\n",po->fdf,po->fdr);
        p = &p[rc];
      }
    setenv("PIPECONN",pipeconn,1);

    sprintf(pipeprog,"stages/%s",verb);                    // DEPRECATED
    execv(pipeprog,argv);
/*  execve(pipeprog,argv,NULL);                                       */

    return 0;
  }

#endif

/* ---------------------------------------------------------- STAGESPAWN
 *       Calls: the stage indicated in argv[0]
 *   Called by: launcher
 */
int xfl_stagespawn(int argc,char*argv[],PIPECONN*pc[],PIPESTAGE*sx)
  /* argc - count of arguments much like Unix/POSIX main()            */
  /* argv - argument array much like Unix/POSIX main()                */
  /* pc   - pipe connector(s) this stage will use (input and output)  */
  { static char _eyecatcher[] = "xfl_stagespawn()";
    int rc, i, ii, io;
    char *p, *q, envbuf[8192], tmpbuf[256], pipepath[8192], *verb;
    PIPECONN *px;
    struct stat sb;

    p = envbuf;
    /* process the supplied array of connectors */
    i = ii = io = 0; while (pc[i] != NULL)
      {
        if (pc[i]->flag & XFL_F_INPUT)
        sprintf(tmpbuf,"*.INPUT.%d:%d,%d",ii++,pc[i]->fdf,pc[i]->fdr);
      else
        if (pc[i]->flag & XFL_F_OUTPUT)
        sprintf(tmpbuf,"*.OUTPUT.%d:%d,%d",io++,pc[i]->fdf,pc[i]->fdr);
      else
// 0100    E Direction "&1" not input or output
{ printf("fail\n");
        xfl_errno = XFL_E_DIRECTION;
 return -1; }

        /* copy this token into the environment variable buffer       */
        q = tmpbuf;
        while (*q != 0x00) *p++ = *q++; *p++ = ' ';

        pc[i]->flag |= XFL_F_KEEP;
        i++;
      }
    *p = 0x00;                                /* terminate the string */

// FIXME: we should do the PIPEPATH scanning before we fork()

    /* fork() is expensive but the most common and reliable way here  */
    rc = fork();
    if (rc < 0) return errno;       /* negative return code: an error */
    if (rc > 0) {             /* positive return code is PID of child */
                          /* process the supplied array of connectors */
                  i = 0; while (pc[i] != NULL) { pc[i]->cpid = rc;
                                                 pc[i]->flag -= XFL_F_KEEP;
//                                     xfl_sever(pc[i]);
                                                 sx->cpid = rc;
                                                 i++;
 }
//                i has the count of connectors, for what that's worth
                  return 0; }
    /* and finally, fork() returning zero means we are the child      */

/* -- at this point we are the child process ------------------------ */

//printf("xfl_stagespawn(%d,%s %s)\n",argc,argv[0],argv[1]);

    /* prepare to pass connector info to the stage when it runs       */
    setenv("PIPECONN",envbuf,1);

    px = xfl_pipeconn;
    while (px != NULL)
      {          /* close the connectors which the child will not use */
        if (px->flag & XFL_F_KEEP); else
// if not previously closed ...
          {
        close(px->fdf);
        close(px->fdr);
// or maybe sever(px) instead?
          }
// step through connectors listed closing all *not* listed
        px = px->next;
      }

if (argc < 2) argv[1] = NULL;

    /* scan PIPEPATH for the stage of interest ($PREFIX/libexec/xfl)  */
    p = getenv("PIPEPATH"); if (p == NULL) p = "";
    if (*p == 0x00) p = PREFIX "/libexec/xfl";
//  if (*p == 0x00) p = "stages"; // FIXME: remember to define PIPEPATH
    strncpy(pipepath,p,sizeof(pipepath)-1);
    p = q = pipepath;
    while (1)
      {
        /* find a searchable directory in the PIPEPATH string         */
        while (*p != 0x00 && *p != ':') p++;
        if (*p != 0x00) *p++ = 0x00;

        verb = argv[0];

        /* magical file syntax fixup */
        if (strcmp(verb,"<") == 0) verb = "filer";
        if (strcmp(verb,">") == 0) verb = "filew";
        if (strcmp(verb,">>") == 0) verb = "filea";
        /* magical file syntax fixup */

        /* abbreviation logic should go near here */

        snprintf(tmpbuf,sizeof(tmpbuf),"%s/%s",q,verb);   /* looking for arg0 */
        rc = stat(tmpbuf,&sb);
        if (rc == 0) break;       /* found it! break out with success */
        if (*p == 0x00) break;  /* if end of string break out failing */
        q = p;         /* otherwise try again with next dir in search */
      }
//  if (rc == 0) printf("xfl_stagespawn(): found '%s'\n",tmpbuf);
//          else printf("xfl_stagespawn(): failed to find '%s'\n",argv[0]);
    if (rc == 0) execv(tmpbuf,argv);
// FIXME: if executable not found we should return to the caller
// FIXME: and that means this needs to be processed before the fork()

#ifdef THIS_WAS_REPLACED

    if (argc > 1) q = argv[1];
             else q = "";
    sprintf(tmpbuf,"stages/%s %s",argv[0],q);
    system(tmpbuf);

    exit(0);
//  return 0;     NO!! we really do *not* want to go back into that code

#endif
  }


/* ------------------------------------------------------------ PIPEPAIR
 * Conceptually similar to POSIX pipe() function, returns two ends.
 */
int xfl_pipepair(PIPECONN*pp[])
  { static char _eyecatcher[] = "xfl_pipepair()";
    struct PIPECONN p0, *pi, *po;
    int fdf[2], fdr[2];

    /* we need *two* traditional POSIX/Unix pipes */
    pipe(fdf);                  /* forward for data */
    pipe(fdr);                  /* reverse for control */
    /* FIXME: need to check for errors after these calls */

    /* establish the side used for input */
    pi = malloc(sizeof(p0));    /* pipeline input */
    if (pi == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("xfl_pipepair(): malloc()");        /* standard report */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return en; }

    pi->fdf /* read  */ = fdf[0]; /* data forward */
    pi->fdr /* write */ = fdr[1]; /* control back */
    pi->flag = XFL_F_INPUT;

    /* establish the side used for output */
    po = malloc(sizeof(p0));    /* pipeline output */
    if (po == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("xfl_pipepair(): malloc()");        /* standard report */
        free(pi);     /* the other malloc() worked so free that block */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return en; }


    po->fdf /* write */ = fdf[1]; /* data forward */
    po->fdr /* read  */ = fdr[0]; /* control back */
    po->flag = XFL_F_OUTPUT;

    /* cross-link these to each other and insert them into the chain  */
    pi->next = po;                   /* input links forward to output */
    pi->prev = NULL;                 /* and becomes new head-of-chain */
    po->next = xfl_pipeconn;    /* output links forward to prior head */
    po->prev = pi;                     /* ... and links back to input */
    xfl_pipeconn = pi;    /* let anchor now point to input (new head) */


    /* follow POSIX pipe() semantics: two plenum connectors           */
    pp[0] = pi;                 /* [0] refers to the read end */
    pp[1] = po;                 /* [1] refers to the write end */
    /* see 'man 2 pipe' on most Unix or Linux systems for the idea    */

    return 0;
  }

/* ------------------------------------------------------------ PIPEPART
 * This routine allocates a stage struct for "part" of this stream.   *
 * The stage might have been previously allocated and labeled.        *
 *       Calls:
 *   Called by: launcher
 */
int xfl_getpipepart(PIPESTAGE**ps,char*l)
  { static char _eyecatcher[] = "xfl_getpipepart()";
    struct PIPESTAGE ps0, *pst;

    /* scan current chain-o-stages looking for the supplied label     */
    if (l != NULL && *l != 0x00)
      {
        pst = xfl_pipestage;
        while (pst != NULL)
          {
            char *m;
            m = pst->label;
            if (m == NULL) m = "";
            /* if this struct has the label then its the one we want  */
            if(strcmp(m,l) == 0)
              {
//printf("xfl_getpipepart(): re-using '%s'\n",l);         // 3047
//printf("xfl_getpipepart(): arg0 '%s' args '%s'\n",pst->arg0,pst->args);   // 3027
                *ps = pst;
                return 0;
              }

            /* or if label does not match then skip to next-in-chain  */
            pst = pst->next;
          }
      }

    /* allocate the struct */
    pst = malloc(sizeof(ps0));
    if (pst == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("xfl_pipepart(): malloc()");        /* standard report */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return en; }

    if (l != NULL && *l != 0x00) pst->label = l; else pst->label = NULL;

    /* insert this stage struct into the chain of stages              */
    pst->next = xfl_pipestage;  /* struct links forward to prior head */
    pst->prev = NULL;                /* and becomes new head-of-chain */

    /* set some defaults */
    pst->arg0 = NULL;                    /* executable name or "verb" */
    pst->args = NULL;                             /* arguments string */
    pst->ipcc = 0;                      /* input pipe connector count */
    pst->opcc = 0;                     /* output pipe connector count */
    pst->xpcc = 0;                     /* COMMON pipe connector count */
    pst->cpid = -1;       /* PID of child process handling this stage */

//  xfl_pipestage->prev = pst;       /* prev head points back to this */
    xfl_pipestage = pst;              /* and this one gets the anchor */
    *ps = pst;
    return 0;
  }

/* ------------------------------------------------------------------ */
/* routines used by the stages follow                                 */
/* ------------------------------------------------------------------ */

/* ---------------------------------------------------------- STAGESTART
 * initialize the internal input and output connectors (two fd each)
 */
int xfl_stagestart(PIPECONN**pc)
  { static char _eyecatcher[] = "xfl_stagestart()";
    char *p, *pipeconn, number[16];
    struct PIPECONN pc0, *pc1, *pcp;
    int i, n;

//  openlog(const char *ident, int option, int facility);

    *pc = NULL;
    pcp = NULL;
n = 0;

    /* connectors are passed to stages as matched file descriptors    */
    pipeconn = getenv("PIPECONN");
    if (pipeconn == NULL) return 0;        /* FIXME: this is an error */
//printf("stagestart: PIPECONN='%s'\n",pipeconn);

    /* parse the connections passed to this stage in the environment  */
    p = pipeconn;
    while (*p != 0x00 && *p != ' ')
      {
        if (*p == '*') p++;        /* skip past "*." to I/O indicator */
        if (*p == '.') p++;            /* else throw error number 191 */

        if (*p == 'I' || *p == 'i') pc0.flag = XFL_F_INPUT;
        if (*p == 'O' || *p == 'o') pc0.flag = XFL_F_OUTPUT;
//0100    E Direction "&1" not input or output

//printf("before '%s'\n",p);
        while (*p != 0x00 && *p != ' ' && *p != '.' && *p != ':') p++;
//printf("after1 '%s'\n",p);

        /* if there is a name and it is numeric then it is a number   */
        if (*p == '.') { p++;
        /* looks like this connector is qualified (a name or number)  */
            number[0] = 0x00;
            for (i = 0; i < sizeof(number) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                number[i] = *p++;
            number[i] = 0x00; pc0.n = atoi(number);
        while (*p != 0x00 && *p != ' ' && *p != '.' && *p != ':') p++;
 }
//printf("after2 '%s'\n",p);

        if (*p == ':')                 /* else throw error number 193 */
          { p++;
            number[0] = 0x00;
            for (i = 0; i < sizeof(number) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                number[i] = *p++;
            number[i] = 0x00; pc0.fdf = atoi(number); }
        if (*p == ',')
          { p++;
            number[0] = 0x00;
            for (i = 0; i < sizeof(number) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                number[i] = *p++;
            number[i] = 0x00; pc0.fdr = atoi(number); }

        pc0.next = NULL;                                /* STAGESTART */
        pc0.prev = *pc;                                 /* STAGESTART */

        /* allocate the connector struct for hand-off                 */
        pc1 = malloc(sizeof(pc0));    // FIXME: check for malloc() error
        if (*pc == NULL) *pc = pc1;
              else pcp->next = pc1;                     /* STAGESTART */
        memcpy(pc1,&pc0,sizeof(pc0));
        pcp = pc1;
n = n + 1;

        while (*p != 0x00 && *p == ' ') p++;
      }

//printf("xfl_stagestart: %d connectors\n",n);      // can discard variable "n"

    /* be sure that stages won't get whacked by SIGPIPE on connectors */
    signal(SIGPIPE,SIG_IGN);

    return 0;
  }

/* ----------------------------------------------------------- STAGEQUIT
 *  do an orderly close of the file descriptors and release of storage
 */
int xfl_stagequit(PIPECONN*pc)
  { static char _eyecatcher[] = "xfl_stagequit()";
    struct PIPECONN *pn;

    while (pc != NULL)
      {
//      /* if an input connector then signal the producer to quit     */
//      if (pc->flag && XFL_F_INPUT)
//        { // printf("xfl_stagequit: signaling consumer to sever the connection\n");
//           write(pc->fdr,"QUIT",sizeof("QUIT")); }
//      /* in any case close the forward and reverse file descriptors */
//      close(pc->fdf); close(pc->fdr);
        xfl_sever(pc);

        /* proceed to next struct in the chain and free this one      */
        pn = pc->next;                                   /* STAGEQUIT */
        free(pc);
        pc = pn;
      }

//  closelog();

    return 0;
  }

/* -------------------------------------------------------------- PEEKTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 *  A return value of zero is not an error if the record was null.
 *  See also: xfl_readto() and xfl_output()
 */
int xfl_peekto(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_peekto()";
    int  rc, reclen;
    char  infobuff[256];

    if (pc == NULL) { xfl_errno = XFL_E_NULLPTR; return -1; }

    /* be sure we are on the input side of the connection             */
    if ((pc->flag & XFL_F_INPUT) == 0)
      { fprintf(stderr,"xfl_peekto: called for a non-input connector\n");
        return -1; }  //FIXME: get a better return code
//printf("peekto: okay but bailing out for development\n");
//return -614;

    /* if the connection was severed then return XFL_E_SEVERED (12)   */
    if (pc->flag & XFL_F_SEVERED) { xfl_errno = XFL_E_SEVERED; return -1; }

/*

"STAT" ** the only meta data at this point in the development
          producer sends number of bytes available

"PEEK" ** think PIPLOCAT to examine a record
          producer sends data

"NEXT" ** think PIPINPUT (sort of) consume the record
          producer advances the sequence count

"QUIT" ** for SEVER operation
          producer closes file descriptors

"FAIL" ** if something went wrong

 */

    /* PROTOCOL:                                                      */
    /* direct the producer to report the size of this record */
    rc = write(pc->fdr,"STAT",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        if (errno == EPIPE) {
 xfl_sever(pc); xfl_errno = XFL_E_SEVERED; return -1; }
        rc = 0 - errno; if (rc == 0) rc = -1;
        perror("peekto(): write():"); /* provide standard Unix report */
//      /* also throw a pipelines/ductwork/plenum error and bail out  */
//      sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
//      xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return rc; }
//printf("xfl_peekto: sent STAT control %d\n",rc);

    /* PROTOCOL:                                                      */
    /* read the response which should simply have an integer string   */
    rc = read(pc->fdf,infobuff,sizeof(infobuff));
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = 0 - errno; if (rc == 0) rc = -1;
        perror("peekto(): read()");        /* provide standard report */
//      /* also throw a pipelines/ductwork/plenum error and bail out  */
//      sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
//      xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
    infobuff[rc] = 0x00;
//printf("xfl_peekto: infobuff = '%s'\n",infobuff);

    /* convert integer string into a binary integer */
//  if (*infobuff is non-digit) then set this connector to close
if (*infobuff == 0x00) return -1;    // FIXME: also set an errno

    if (isdigit(*infobuff))
    reclen = atoi(infobuff);
//  else { /* shutdown */ }
//printf("xfl_peekto: expecting %d bytes\n",reclen);

    /* undocumented feature: zero-length peekto tells the record size */
    if (buflen == 0) return reclen;

    /* does the supplied buffer have room for this record? */
    if (buflen < reclen) return -1;

    /* PROTOCOL:                                                      */
    /* direct the producer to send the record content */
    rc = write(pc->fdr,"PEEK",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        if (errno == EPIPE) {
//printf("xfl_peekto(): got an EPIPE for a PEEK\n");
 xfl_sever(pc); return -XFL_E_SEVERED; }
        rc = 0 - errno; if (rc == 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
//      /* also throw a pipelines/ductwork/plenum error and bail out  */
//      sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
//      xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
//printf("xfl_peekto: sent PEEK; expecting %d bytes\n",reclen);

    /* PROTOCOL:                                                      */
    rc = read(pc->fdf,buffer,reclen);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = 0 - errno; if (rc == 0) rc = -1;
        perror("peekto(): read()");        /* provide standard report */
//      /* also throw a pipelines/ductwork/plenum error and bail out  */
//      sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
//      xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }

    return rc;
  }

/* -------------------------------------------------------------- READTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 *  A return value of zero is not an error if the record was null.
 *  See also: xfl_peekto() and xfl_output()
 */
int xfl_readto(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_readto()";
    int  rc;
    char  infobuff[256];

    if (pc == NULL) { xfl_errno = XFL_E_NULLPTR; return -1; }

    /* be sure we are on the input side of the connection             */
    if ((pc->flag & XFL_F_INPUT) == 0)
      { fprintf(stderr,"xfl_readto: called for a non-input connector\n");
        return -1; } // FIXME: get a better return code

    /* if the connection was severed then return XFL_E_SEVERED (12)   */
    if (pc->flag & XFL_F_SEVERED) { xfl_errno = XFL_E_SEVERED; return -1; }

    /* if buffer supplied and length not zero then try to get data    */
    if (buffer != NULL && buflen > 0)
      { rc = xfl_peekto(pc,buffer,buflen);
        if (rc < 0) return rc; }
    /* this also checks things like which side this connector is for  */

    /* PROTOCOL:                                                      */
    /* direct the producer to proceed with the next record */
    rc = write(pc->fdr,"NEXT",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        if (errno == EPIPE) {
 xfl_sever(pc); xfl_errno = XFL_E_SEVERED; return -1; }
        rc = 0 - errno; if (rc == 0) rc = -1;
        perror("readto(): write():");      /* standard Unix report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return rc; }


    /* increment the record counter */
    pc->rn = pc->rn + 1;

    return 0;
  }

/* -------------------------------------------------------------- OUTPUT
 *  PRODUCER SIDE
 *  Returns: number of bytes written to output or negative for error
 *  A return value of zero is not an error if the record was null.
 *  This routine sits in a loop driven by the consumer.
 *  See also: xfl_peekto() and xfl_readto()
 */
int xfl_output(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_output()";
    int rc, xx;
    char  infobuff[256];
int n;

    if (pc == NULL) { xfl_errno = XFL_E_NULLPTR; return -1; }

    /* be sure we are on the output side of the connection            */
    if ((pc->flag & XFL_F_OUTPUT) == 0)
//    { xfl_error(100,0,NULL,"LIB");       /* provide specific report */
      { fprintf(stderr,"xfl_output: called for a non-output connector\n");
        return -1; } // FIXME: get a better return code

    /* if the connection was severed then return XFL_E_SEVERED (12)   */
    if (pc->flag & XFL_F_SEVERED) { xfl_errno = XFL_E_SEVERED; return -1; }

//printf("xfl_output: '%s' %d %d\n",buffer,buflen,strlen(buffer));

n = 0;
    while (1)
      {
n = n + 1;
        /* the following is a blocking read; this routine waits until *
         * the consumer side signals that it is ready to consume      */
//      rc = read(pc->fdr,infobuff,sizeof(infobuff));
        rc = 0; while (rc == 0)
        rc = read(pc->fdr,infobuff,4);    /* expect 4 bytes by design */
        if (rc < 4)
          { char *msgv[2], em[16];
//          rc = errno; if (rc == 0) rc = -1;
//          perror("output(): read():");   /* provide standard report */
            /* also throw a pipelines/ductwork/plenum error and bail  */
            sprintf(em,"%d",rc); msgv[1] = em;   /* integer to string */
            xfl_error(26,2,msgv,"LIB");    /* provide specific report */
printf("xfl_output: error trying to read the control channel after %d %d\n",n,rc);
            return rc; }
        infobuff[rc] = 0x00;
//printf("xfl_output: infobuff = '%s'\n",infobuff);

        xx = 0;
        switch (*infobuff)
          {
            case 'S': case 's':                               /* STAT */
                /* PROTOCOL: send the size of the record              */
                sprintf(infobuff,"%d",buflen);   /* say # bytes avail */
                rc = write(pc->fdf,infobuff,strlen(infobuff)+1);
                break;

            case 'P': case 'p':                               /* PEEK */
                /* PROTOCOL: send the record downstream               */
                rc = write(pc->fdf,buffer,buflen);   /* send the data */
                break;

            case 'N': case 'n':                               /* NEXT */
                /* PROTOCOL: acknowledge to consumer we unblocked     */
                rc = 0;
                xx = 1;
                break;

            case 'Q': case 'q':                               /* QUIT */
printf("xfl_output: got a quit signal from the consumer\n");
                /* PROTOCOL: consumer has signaled a sever            */
                xfl_sever(pc);
                rc = 0;
                xx = 1;
                break;

            default:
printf("xfl_output: protocol error '%s'\n",infobuff);
// need to indicate a protocol error here
                rc = -1;
                xx = 1;
                break;
          }

        if (rc < 0)
          { char *msgv[2], em[16];
            if (errno == EPIPE) {
 xfl_sever(pc); xfl_errno = XFL_E_SEVERED; return -1; }
            rc = errno; if (rc == 0) rc = -1;
            perror("xfl_output(): write():");   /* Unix system report */
            /* also throw a pipelines/ductwork/plenum error and bail  */
            sprintf(em,"%d",rc); msgv[1] = em;   /* integer to string */
            xfl_error(26,2,msgv,"LIB");    /* provide specific report */
printf("xfl_output: error after protocol\n");
            return rc; }

        if (xx) break;
      }

    /* increment the record counter */
    pc->rn = pc->rn + 1;

//printf("xfl_output: (normal exit)\n");

    return 0;
  }

/* --------------------------------------------------------------- SEVER
 *  Sever a connection: tell the upstream, close FDs, mark it severed
 */
int xfl_sever(PIPECONN*pc)
  { static char _eyecatcher[] = "xfl_sever()";

    if (pc == NULL) { xfl_errno = XFL_E_NULLPTR; return -1; }

    /* if already severed then return no error */
    if (pc->flag & XFL_F_SEVERED) { xfl_errno = XFL_E_NONE; return 0; }

    /* if this is an input then signal upstream to shut it down */
    if (pc->flag & XFL_F_INPUT) write(pc->fdr,"QUIT",4);
    /* close the file descriptors */
    close(pc->fdf); close(pc->fdr);
    /* mark this connection as severed */
    pc->flag |= XFL_F_SEVERED;

    /* clear the global errno and return non error */
    xfl_errno = XFL_E_NONE;
    return 0;
  }

/* -- COBOL support ------------------------------------------------- *
 *    The following routines were added to support COBOL              *
 *    but they facilitate any language using call-by-reference.       *
 * ------------------------------------------------------------------ */

static struct PIPECONN *xfl_pc_common = NULL;

int XFLVERSN(char*b)
  { /* return the version (numbers only) to the caller                */
    sprintf(b,"%d.%d.%d",
       (xfl_version>>24),
      ((xfl_version>>16) & 0xFF),
      ((xfl_version>>8) & 0xFF));
    return 0; }

/* ------------------------------------------------------------------ *
 *    NOTE: these are ham-strung and only do stream zero              *
 * ------------------------------------------------------------------ */

int XFLINIT()
  {
    int rc;
    struct PIPECONN *pc;

    /* if we have done this before then return no error               */
    if (xfl_pc_common != NULL) return 0;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc != 0) return rc;

    xfl_pc_common = pc;
    return 0;
  }

int XFLPEEK(int*sn,char*b,int*bl)
  {
    int rc;
    struct PIPECONN *pi, *pn;

    XFLINIT();

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = xfl_pc_common; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

    rc = xfl_peekto(pi,b,*bl);                        /* sip on input */
    if (rc < 0) return rc;
    *bl = rc;

    return 0;
  }

int XFLOUT(int*sn,char*b,int*bl)
  {
    int rc;
    struct PIPECONN *po, *pn;

    XFLINIT();

    /* snag the first output stream from the chain-o-connectors       */
    po = NULL;
    for (pn = xfl_pc_common; po == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_OUTPUT) po = pn;

    rc = xfl_output(po,b,*bl);                    /* write the record */
    if (rc < 0) return rc;

    return 0;
  }

int XFLREAD(int*sn,char*b,int*bl)
  {
    int rc;
    struct PIPECONN *pi, *pn;

    XFLINIT();

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = xfl_pc_common; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

    rc = xfl_readto(pi,b,*bl);            /* consume the input record */
    if (rc < 0) return rc;
    *bl = rc;

    return 0;
  }


