/*
 *        Name: locate.c (C program source)
 *              POSIX Pipelines LOCATE stage
 *        Date: 2024-05-29 (Wed)
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <xfl.h>

/* DEVELOPMENT */
#include <stdio.h>

static char _eyeball0[] = "XFL pipeline stage 'locate'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'locate' main()";
    int rc, buflen;
    char *args, *p, *q, *needle, buffer[4096];
    struct PIPECONN *pc, *pi, *pop, *pos, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) /* there was an error, then */ return 1;
//printf("locate: argstring '%s'\n",args);

    /* find the needle in the arg string */
    p = args;
    while (*p != 0x00 && *p != '/') p++;
    if (*p == '/') p++;
    q = needle = p;
    while (*q != 0x00 && *q != '/') q++;
    if (*q == '/') *q = 0x00;
//printf("locate: needle '%s'\n",needle);


    /* snag the first input stream and the first two output streams   */
    pi = pop = pos = NULL;
    for (pn = pc; pn != NULL && pi == NULL; pn = pn->next)
      { if (pn->flag & XFL_F_INPUT)  pi = pn; }
    for (pn = pc; pn != NULL && pop == NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) pop = pn; }
    for (       ; pn != NULL && pos == NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) pos = pn; }

//printf("locate: %08X %08X %08X\n",pi,pop,pos);
//system("printenv | grep 'PIPE'");

    while (1)
      {
        /* perform a PEEKTO and see if there is a record ready        */
        buflen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        buffer[buflen] = 0x00;

#ifdef XFL_STAGE_NLOCATE
//printf("nlocate: haystack '%s'\n",buffer);
//printf("nlocate: needle '%s'\n",needle);
        /* is string NOT present? Y: write to primary, N: secondary   */
        if (strstr(buffer,needle) == NULL)
#else
//printf("locate: haystack '%s'\n",buffer);
//printf("locate: needle '%s'\n",needle);
        /* is the string present? Y: write to primary, N: secondary   */
        if (strstr(buffer,needle) != NULL)
#endif
        rc = xfl_output(pop,buffer,buflen);   /* send it down primary */
        else
        rc = xfl_output(pos,buffer,buflen); /* send it down secondary */
        if (rc < 0) break;


        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
        if (rc < 0) break;
      }

    free(args);

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* locate
//MD
//MDUse the `locate` stage to find occurrences of the specified string in the input stream.
//MD
 */


