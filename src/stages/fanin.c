/*
 *        Name: fanin.c (C program source)
 *              POSIX Pipelines FANIN stage
 *        Date: 2024-05-05
 *              reads all input records, holds them in memory,
 *              then writes them all out after input is severed
 *
 * (from CMS HELP PIPE FANIN)
 * "fanin" passes all records on the primary input stream to the
 * primary output stream, then all records on the secondary input
 * stream to the primary output stream, and so on.
 */

#include <stdio.h>

/* development: the following is for sleep() */
#include <unistd.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'fanin'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'fanin' main()";
    int buflen, rc;
    char buffer[4096];
    struct PIPECONN *pc, *pi, *po, *pn;

#ifdef DEVELOPMENT
printf("fanin: (starting)\n");
#endif

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

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
/* CMS Pipelines 'fanin' does not complain about this condition */
#ifdef DEVELOPMENT
printf("fanin: YES output is connected\n");
#endif

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = pc; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

#ifdef DEVELOPMENT
if (pi != NULL)
printf("fanin: YES input is connected\n");
#endif

    while (pi != NULL)
      {
//printf("fanin: top of loop\n");
        /* "Do Forever" until we break out otherwise */
        while (1)
          {
            /* perform a PEEKTO and see if there is a record ready        */
            buflen = sizeof(buffer) - 1;
            rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
            if (rc < 0) break; /* else */ buflen = rc;
#ifdef DEVELOPMENT
printf("fanin: got a record\n");
#endif

            /* write the record to our primary output stream              */
            rc = xfl_output(po,buffer,buflen);      /* send it downstream */
            if (rc < 0) break;
#ifdef DEVELOPMENT
printf("fanin: sent that record downstream\n");
#endif

            /* now consume the record from the input stream               */
            rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
            if (rc < 0) break;
#ifdef DEVELOPMENT
printf("fanin: consumed the record\n");
#endif
          }

//printf("fanin: looking for next input\n");
        /* snag the next input, if any */
        pn = pi->next;
        for (pi = NULL; pi == NULL && pn != NULL; pn = pn->next)
          if (pn->flag & XFL_F_INPUT) pi = pn;
      }

//printf("fanin: shutdown??\n");
    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

#ifdef DEVELOPMENT
printf("fanin: (normal exit)\n");
#endif

//sleep(7);

    /* pass a "success" return code to our caller */
    return 0;
  }

/*
//MD
//MD* fanin
//MD
//MDUse the `fanin` stage to collect multiple input streams
//MDinto a single output.
//MD
 */


