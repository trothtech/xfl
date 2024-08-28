/*
 *        Name: command.c (C program source)
 *              POSIX Pipelines COMMAND stage
 *        Date: 2024-05-12
 *              runs commands on the host system,
 *              much like the COMMAND stage in CMS Pipelines.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <xfl.h>

#define DEVELOPMENT

/* ---------------------------------------------------------- DO1COMMAND
 */
int do1command(PIPECONN*pp,char*cmd)
  { static char _eyecatcher[] = "pipeline stage 'command' do1command()";
    int buflen, rc;
    char buffer[4096], *s;
    FILE *cf;

#ifdef DEVELOPMENT
printf("command: do1command()\n");
#endif

    /* use popen() to instantiate the command process                 */
    cf = popen(cmd,"r");
    if (cf == NULL)
      { rc = errno; if (rc > 0) rc = 0 - rc;
perror("do1command(): popen():");
        return rc; }
printf("command: shell pipe opened\n");

    /* loop, converting lines of output into records and send along   */
    while (1)
      {

        /* read one line from cf */
        s = fgets(buffer,buflen,cf);
        if (s == NULL)
          {
perror("do1command(): fgets():");
            // FIXME: check errno
            // end-of-file is okay
            break; }
#ifdef DEVELOPMENT
printf("command: got a line %s\n",buffer);
#endif

buflen = strlen(buffer);
if (buffer[buflen-1] == '\n') buflen = buflen - 1;

        /* write the record to our primary output stream              */
        rc = xfl_output(pp,buffer,buflen);      /* send it downstream */
        if (rc < 0) break;
#ifdef DEVELOPMENT
//printf("fanin: sent a record downstream\n");
printf("%s\n",buffer);
#endif
      }

    /* close the FILE handle and try to process the condition code    */
    rc = pclose(cf);
    if (rc < 0) return rc;              /* Error reported by pclose() */
    /* Use macros described under wait() to inspect 'status' in order
          to determine success/failure of command executed by popen() */
    return rc;
  }

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "pipeline stage 'command' main()";

    int buflen, rc;
    char *args, buffer[4096];
    struct PIPECONN *pc, *pi, *po, *pn;

#ifdef DEVELOPMENT
printf("command: (starting)\n");
#endif

    /* initialize this stage and populate the PIPECONN structs        */
    rc = xfl_stagestart(&pc);  /* must be pointer to PIPECONN pointer */
    if (rc < 0) return 1;

    /* string-up all arguments into one - must be freed later         */
    args = xfl_argcat(argc,argv);             /* must eventually free */
    if (args == NULL) /* error, then */ return 1;

    /* snag the primary output stream (scanning all connectors)       */
    po = NULL;
    for (pn = pc; po == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_OUTPUT) po = pn;

    /* if no output then stop now */
    if (po == NULL)
      {
        xfl_stagequit(pc);
        return 0;
      }
#ifdef DEVELOPMENT
printf("command: YES output is connected\n");
#endif


do1command(po,args);


    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = pc; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

#ifdef DEVELOPMENT
if (pi != NULL)
printf("command: YES input is connected\n");
#endif

    while (1)
      {
        /* perform a PEEKTO and see if there is a record ready        */
        buflen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
#ifdef DEVELOPMENT
printf("command: got a record\n");
#endif

        /* write the record to our primary output stream              */
        rc = do1command(po,buffer);
        if (rc < 0) break;
#ifdef DEVELOPMENT
printf("command: sent that output downstream\n");
#endif

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
        if (rc < 0) break;
#ifdef DEVELOPMENT
printf("command: consumed the record\n");
#endif
      }

    /* terminate this stage cleanly - free the PIPECONN structs       */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

#ifdef DEVELOPMENT
printf("command: (normal exit)\n");
#endif

    /* pass a "success" return code to our caller */
    return 0;
  }


