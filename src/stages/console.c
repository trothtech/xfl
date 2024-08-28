/*
 *        Name: console.c (C program source)
 *              POSIX Pipelines CONSOLE stage
 *        Date: 2010-Mar-24, 2023-07, 2024
 *              reads stdin and writes to primary output
 *               -or-
 *              reads primary input and writes to stdout
 *
 * (from CMS HELP PIPE CONSOLE)
 * When "console" is first in a pipeline it reads lines from the terminal
 * and writes them into the pipeline.  When "console" is not first in a
 * pipeline it copies lines from the pipeline to the terminal.
 */

#include <stdio.h>
#include <stdio.h>
#include <string.h>

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "pipeline stage 'console' main()";
    int i, buflen, rc;
    char buffer[4096], *p;
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* FIXME: ignoring command line arguments during development      */
    /*        we should support EOF (at least) "DARK"                 */
    /*        DIRECT and ASYNCHRON not applicable but accepted        */

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }


    /* if pi is null then we are a first stage so slurp stdin         */
    if (pi == NULL && po == NULL)
    /* 1493 E Too few streams are defined; &1 are present, but &2 are required */
      { char *msgv[3];
        msgv[1] = "0"; msgv[2] = "1";
        xfl_error(1493,3,msgv,"CON");      /* provide specific report */
        return 1; }

    if (pi == NULL) while (1)
      { /* with no primary input we ARE a first stage                 */
        buflen = sizeof(buffer);
        p = fgets(buffer,buflen,stdin);
        if (p == NULL) break;
        buflen = strlen(p);
        if (buflen > 0)
          { i = buflen - 1;
            if (buffer[i] == '\n') { buffer[i] = 0x00; buflen = i; } }
//      if (buflen == 0) break;
        rc = xfl_output(po,buffer,buflen);      /* send it downstream */
        if (rc < 0) break;
      }

    else while (1)
      {
        buflen = sizeof(buffer) - 1;

        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        buffer[buflen] = 0x00;     /* terminate the string for stdout */

        printf("%s\n",buffer);                  /* write it to stdout */

        if (po != NULL) {
        rc = xfl_output(po,buffer,buflen);      /* send it downstream */
        if (rc < 0) break; }

        xfl_readto(pi,NULL,0);    /* consume the record after sending */
      }

    /* dropping out of either loop, if error then exit immediately    */
    if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* console
//MD
//MDThe `console` stage is so named for compatibility with CMS/TSO Pipelines.
//MDIn POSIX Pipelines, `console` serves as a gateway between shell pipes and POSIX Pipelines stages.
//MD
//MDWhen `console` is the first stage of a pipeline, it reads lines of text from file descriptor zero (0)
//MDdelimited by newline characters. In this mode, its output must be connected
//MDto another POSIX Pipelines stage and its output is records with newline characters removed.
//MDOutput records are bounded where newlines occur on input.
//MD
//MDWhen `console` is NOT the first stage of a pipeline,
//MDits input must be connected to another POSIX Pipelines stage. It then reads input records
//MDand writes to file descriptor one (1) with newline characters appended to each line.
//MDIn this mode, `console` may also be connected on output to a following POSIX Pipelines stage
//MDto which it will write the input records unaltered. (No newline inserted.)
//MD
 */


