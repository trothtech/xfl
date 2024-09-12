/*
 *        Name: filew.c (C program source)
 *              POSIX Pipelines FILEW stage (file write)
 *        Date: 2024-05-28 (Tue) taking a break from Rexx development
 *              This stage reads records and writes a file.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'filew'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'filew' main()";
    int rc, fd, buflen;
    char *args, *fn, buffer[4096], *p, *q, *msgv[16];
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) {
      printf("filew: xfl_stagestart() returned %d\n",rc); return 1; }

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

    /* take the first blank-delimited word as the name of the file    */
    p = args;
    while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
    q = p;
    while (*q != ' ' && *q != '\t' && *q != 0x00) q++;
    if (*q != 0x00) *q = 0x00;
    fn = p;

    /* open the named file for writing                                */
#ifdef XFL_STAGE_FILEAPPEND
    rc = fd = open(fn,O_WRONLY|O_CREAT|O_APPEND,0666);
#else
    rc = fd = open(fn,O_WRONLY|O_CREAT|O_TRUNC,0666);
#endif
    if (rc < 0)
      { char em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("filew(): open()");   /* provide standard Unix report */
        /* 0699 E Return code &1 from &2 (file: &3) */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        msgv[2] = "open()";
        msgv[3] = fn;
        xfl_error(699,4,msgv,"FIO");       /* provide specific report */
        return -1;
      }

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }


    /* 0127 E This stage cannot be first in a pipeline                */
    if (pi == NULL) { xfl_error(127,0,NULL,"FIO"); return 1; }

    while (1)
      {
        buflen = sizeof(buffer);        /* start with max record size */
        rc = xfl_peekto(pi,buffer,buflen-1);      /* sip on the input */
        if (rc < 0) break; /* else */ buflen = rc;
        buffer[buflen++] = '\n';  /* mark it with a newline character */
        buffer[buflen] = 0x00;                /* terminate the string */
//printf("filew: got a record %d\n",buflen);

        /* write this record to the file */
        rc = write(fd,buffer,buflen);
//      if (rc < buflen) break;
        if (rc < 0) break;
//      write(fd,"\n",1);     /* and mark it with a newline character */

        /* write the record to our primary output stream              */
        if (po != NULL) rc = xfl_output(po,buffer,buflen);
//      if (rc < 0) break;

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);             /* consume the record */
        if (rc < 0) break;
      }

    close(fd);

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* filew (file write), aliased as "&gt;"
//MD
//MDUse `>` to write to a file.
//MD
 */


