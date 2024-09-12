/*
 *        Name: var.c (C program source)
 *              POSIX Pipelines VAR stage
 *        Date: 2024-05-15 (Wednesday)
 *
 * (from CMS HELP PIPE VAR)
 * "var" connects a variable to the pipeline.
 * When "var" is first in the pipeline, the contents of the specified
 * variable are written to the pipeline. When "var" is not first in a
 * pipeline, it sets the specified variable to the contents of the first
 * input record and then passes all input to the output.
 *
 * This stage runs as a program.
 * Setting the variable has no effect unless there are child processes
 * or there is some way to communicate with the parent/calling process.
 *
 * Unlike the CMS counterpart, this stage only recognizes one argument
 * the name of the variable.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'var'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'var' main()";
    int rc, l, buflen;
    char *p, *q, *args, var[256], *val, *msgv[4], em[16], buffer[4096];
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0)
      { /* 0303 E Return code &1 from &2                              */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[1] = "xfl_stagestart()"; /* identify the failing routine */
        xfl_error(303,4,msgv,"VMC");   /* provide XFL specific report */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* string-up the command line arguments .. remember free() below  */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    /* find the variable name in the input argument string            */
    p = args; while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;

    /* now mark the END of the variable name in the arg string        */
    q = p; while (*q != ' ' && *q != '\t' && *p != 0x00) q++; *q = 0x00;

    /* now copy the variable name to its own buffer                   */
    strncpy(var,p,sizeof(var)-1); var[sizeof(var)-1] = 0x00;

    /* remember to free the strung-up arguments buffer                */
    free(args);

    /* if first stage then read the variable and write its value      */
    if (pi == NULL)
      {
        /* get the value of this variable */
        val = getenv(var);

        /* if failed then upcase the variable name and try again      */
        if (val == NULL || *val == 0x00)
          { for (p = var; *p != 0x00; p++) *p = toupper(*p);
            val = getenv(var); }

        /* write the literal string to our primary output stream      */
        if (val == NULL) val = "";
        l = strlen(val);
        rc = xfl_output(po,val,l);
      }

    /* proper pipeline: once we have written the literal to the       *
     * output, we then copy all input records, if any, to the output  */
    else if (po != NULL) while (1)
      {
/*      ideally we would set the variable based on the first record   */
/*      e.g., setenv(), but talking to an interpreter                 */
/*      but that can't happen without special arrangements            */

        /* perform a PEEKTO and see if there is a record ready        */
        buflen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;

        /* write the record to our primary output stream              */
        rc = xfl_output(po,buffer,buflen);      /* send it downstream */
        if (rc < 0) break;

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);             /* consume the record */
        if (rc < 0) break;

      }

//  if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

//  int i, buflen, rc, argl;
//  char buffer[4096], *p, *q, *args;


