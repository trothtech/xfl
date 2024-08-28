/*
 *        Name: buffer.c (C program source)
 *              POSIX Pipelines BUFFER stage
 *        Date: 2009-Mar-05
 *              reads all input records, holds them in memory,
 *              then writes them all out after input is severed
 */

#include <stdio.h>
#include <stdlib.h>

#include <xfl.h>

typedef struct BUFFSTRUCT {
    char *ad;                  /* address of the data for this record */
    int len;                                 /* length of this record */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */
                          } BUFFSTRUCT;

/* ------------------------------------------------------------------ */
int main(int*arg,char*argv[])
  { static char _eyecatcher[] = "pipeline stage 'buffer' main()";
    int rc, i, o, reclen;
    char *bp, *bi;
    struct PIPECONN *pc, *pi, *po, *pn;
    struct BUFFSTRUCT *bs, *bq, bs0;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* start with an array to handle 1000 records */
    bs = malloc(sizeof(bs0)*1000);
    if (bs == NULL) { perror("malloc()"); return 1; }

    /* allocate a chunk of memory to hold the records */
    bi = bp = malloc(1048576);
    if (bp == NULL) { perror("malloc()"); return 1; }

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    /* FIXME: provide an error message "no output stream */
    if (pi == NULL)
      { xfl_error(61,0,NULL,"BUF");        /* provide specific report */
        return 1; }
    if (po == NULL)
      { xfl_error(61,0,NULL,"BUF");        /* provide specific report */
        return 1; }

    /* start with an index offset of zero */
    i = 0;

    reclen = 4096;   /* arbitrary */

    /* "Do Forever" until we break out otherwise */
    while (1) {

        /* perform a PEEKTO and see if there is a record ready        */
        rc = xfl_peekto(pi,bi,reclen);                /* sip on input */
        if (rc < 0) break;

        bq = &bs[i];       /* use BQ as a pointer to struct .. easier */
        bq->ad = bi;               /* member AD points to this record */
        bq->len = rc;     /* member LEN had the length of this record */
        bi = &bi[rc];        /* set index pointer past record content */
        i++;                  /* bump up the index to the next record */

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);   /* consume record after storing */
        if (rc < 0) break;
      }

#ifdef XFL_STAGE_REVERSE
    for (o = i - 1; o > -1; o--)
#else
    for (o = 0; o < i; o++)
#endif
      {
        bq = &bs[o];       /* use BQ as a pointer to struct .. easier */
        /* write this record from memory to the output stream         */
        rc = xfl_output(po,bq->ad,bq->len);
        if (rc < 0) break;
      }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    /* pass a "success" return code to our caller */
    return 0;
  }

/*
//MD
//MD* buffer
//MD
//MDUse the `buffer` stage to hold all input records
//MDuntil the source stage terminates.
//MD
 */


