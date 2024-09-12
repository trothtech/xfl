/*
 *        Name: count.c (C program source)
 *              POSIX Pipelines COUNT stage
 *        Date: 2024-06-18 (Tue)
 *
 * (from CMS HELP PIPE COUNT)
 * "count" counts the number of input lines, words, characters,
 *  or any combination thereof. It can also report the length of
 *  the shortest or longest record, or both. At end-of-file on input,
 *  It writes a record with the specified counts.
 *
 * RECORDS is a synonym for LINES.
 * CHARS and BYTES are synonyms for CHARACTERS.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <xfl.h>

#define  XFL_COUNT_CHARS  1
#define  XFL_COUNT_WORDS  2
#define  XFL_COUNT_LINES  3
#define  XFL_COUNT_MINLN  4
#define  XFL_COUNT_MAXLN  5

static char _eyeball0[] = "XFL pipeline stage 'count'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'count' main()";
    int rc, chars, words, lines, minln, maxln, om[8], reclen, i, o;
    char *args, *p, *q, *msgv[2], em[16], buffer[4096];
    struct PIPECONN *pc, *pn, *pi, *po, *po2;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0)
      { /* 0303 E Return code &1 from &2                              */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[1] = "xfl_stagestart()"; /* identify the failing routine */
        xfl_error(303,4,msgv,"CNT");   /* provide XFL specific report */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* string-up the command line arguments .. remember free() below  */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

    /* snag the first input stream and the first output stream        */
    pi = po = po2 = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }
    /* snag the second output stream, if any                          */
    while (pn != NULL && po2 == NULL)
      { if (pn->flag & XFL_F_OUTPUT) po2 = pn;
        pn = pn->next; }

    /* find the first blank-delimited token in the argument string    */
    p = args; while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;

    /* now mark the END of the variable name in the arg string        */
    q = p; while (*q != ' ' && *q != '\t' && *q != 0x00) q++;
    if (*q != 0x00) *q++ = 0x00;

    chars = words = lines = maxln = 0;
    minln = -1;

    while (*p != 0x00 && i < sizeof(om))
      {
printf("count: %s\n",p);
        if (strncasecmp(q,"CHARACTERS",1) == 0) om[i] = XFL_COUNT_CHARS;
        if (strncasecmp(q,"WORDS",1) == 0)      om[i] = XFL_COUNT_WORDS;
        if (strncasecmp(q,"LINES",1) == 0)      om[i] = XFL_COUNT_LINES;
        if (strncasecmp(q,"MINLINE",2) == 0)    om[i] = XFL_COUNT_MINLN;
        if (strncasecmp(q,"MAXLINE",2) == 0)    om[i] = XFL_COUNT_MAXLN;
        if (strncasecmp(q,"RECORDS",1) == 0)    om[i] = XFL_COUNT_LINES;
        if (strncasecmp(q,"CHARS",5) == 0)      om[i] = XFL_COUNT_CHARS;
        if (strncasecmp(q,"BYTES",1) == 0)      om[i] = XFL_COUNT_CHARS;
        i++;
        /* find the next blank-delimited token in the argument string */
        while (*q != 0x00 && (*q == ' ' || *q == '\t')) q++; p = q;
        while (*q != 0x00 && *q != ' ' && *q != '\t') q++; p = q;
        if (*q != 0x00) *q++ = 0x00;
      }

    /* remember to free the strung-up arguments buffer                */
    free(args);

    /* proper pipeline ...                                            *
     *             ... copy all input records, if any, to the output  */
//  if (po != NULL)
    while (1)
      {
        /* perform a PEEKTO and see if there is a record ready        */
        reclen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,reclen);            /* sip on input */
        if (rc < 0) break; /* else */ reclen = rc;

        chars = chars + reclen;
        lines = lines + 1;
        if (reclen < minln) minln = reclen;
        if (reclen > maxln) maxln = reclen;

        /* write the record to our primary output stream              */
        if (po2 != NULL)
        rc = xfl_output(po,buffer,reclen);      /* send it downstream */
        if (rc < 0) break;

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);             /* consume the record */
        if (rc < 0) break;
      }

    /* write the record with all the counted totals                   */
    p = buffer;
    for (o = 0; o < i;o++)
      {
        switch (om[i])
          {
            case XFL_COUNT_CHARS: sprintf(p," %d",chars);
            case XFL_COUNT_WORDS: sprintf(p," %d",words);
            case XFL_COUNT_LINES: sprintf(p," %d",lines);
            case XFL_COUNT_MINLN: sprintf(p," %d",minln);
            case XFL_COUNT_MAXLN: sprintf(p," %d",maxln);
          }
        while (*p != 0x00) p++;
      }
    p = buffer; p++; reclen = strlen(p);
    if (po2 == NULL) po2 = po;
    rc = xfl_output(po,p,reclen);               /* send it downstream */

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }


