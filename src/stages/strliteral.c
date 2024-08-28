/*
 *        Name: strliteral.c (C program source)
 *              POSIX Pipelines STRLITERAL stage
 *        Date: 2024-01-15 (Monday, MLK) based on literal.c
 *              sends the argument string to next stage via xpl_output()
 *              and then passes all input records to the output
 *              looping over xpl_peekto(), xpl_output(), xpl_readto(),
 *              to not delay records
 */

#include <stddef.h>
#include <string.h>

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "pipeline stage 'strliteral' main()";
    int i, buflen, rc, argl;
    char buffer[4096], *args, *argp, delim, *argq;
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) /* there was an error, then */ return 1;

    /* skip leading white space and look for a delimiter character    */
    argp = args;
    while (*argp == ' ' || *argp == '\t') argp++;

    /* use everything between the delimiters as the literal string    */
    delim = *argp++; argq = argp;
    while (*argp != delim && *argp != 0x00) argp++;
    *argp++ = 0x00;

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
    argl = strlen(argq);
    rc = xfl_output(po,argq,argl);
    if (rc < 0) return 1;

    /* proper behavior: once we have written the literal to the       *
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
        rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
        if (rc < 0) break;
      }
    if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* strliteral
//MD
//MDUse the `strliteral` stage to insert a line of literal text into a stream.
//MD
 */


