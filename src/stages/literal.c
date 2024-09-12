/*
 *        Name: literal.c (C program source)
 *              POSIX Pipelines LITERAL stage
 *        Date: 2023-06-09 (Friday) Gallatin
 *              2023-06-27 (Tuesday) Belle Pre
 *              sends the argument string to next stage via xfl_output()
 *              and then passes all input records to the output
 *              looping over xfl_peekto(), xfl_output(), xfl_readto(),
 *              to not delay records
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'literal'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'literal' main()";
    int i, buflen, rc, argl;
    char buffer[4096], *p, *q, *args;
    struct PIPECONN *pc, *pi, *po, *pn;

/*
 * init/start                                                      works
 * get args                                                        works
 * write string to primary output
 * quit
 */

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    /* FIXME: provide an error message "no output stream */
    if (po == NULL)
      { xfl_error(61,0,NULL,"LIT");        /* provide specific report */
        return 1; }

    /* write the literal string to our primary output stream          */
    argl = strlen(args);
    rc = xfl_output(po,args,argl);
    if (rc < 0) return 1;

    /* proper pipeline: once we have written the literal to the       *
     * output, we then copy all input records, if any, to the output  */
    if (pi != NULL) while (1)
      {
        /* perform a PEEKTO and see if there is a record ready        */
        buflen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;

        /* write the record to our primary output stream              */
        rc = xfl_output(po,buffer,buflen);      /* send it downstream */
        if (rc < 0) break;

        /* now consume the record from the input stream               */
        xfl_readto(pi,NULL,0);                  /* consume the record */
      }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* literal
//MD
//MDUse the `literal` stage to insert a line of literal text into a stream.
//MD
 */


