/*
 *        Name: hole.c (C program source)
 *              POSIX Pipelines HOLE stage
 *        Date: 2024-07-09 (Tue)
 */

#include <stdio.h>

#include <xfl.h>

static char _eyecatcher[] = "XFL pipeline stage HOLE";

int main()
  {
    int rc;
    char *msgv[4], em[16];
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0)
      { /* 0303 E Return code &1 from &2                              */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[1] = "xfl_stagestart()"; /* identify the failing routine */
        xfl_error(303,4,msgv,"HOL");   /* provide XFL specific report */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    /* sever output */
    if (po != NULL) xfl_sever(po);

    if (pi != NULL){

    /* just consume all input records and do nothing with them        */
    while (1)
      { /* consume one record from the input stream                   */
        rc = xfl_readto(pi,NULL,0);
        if (rc < 0) break; }

    /* sever input when done */
    xfl_sever(pi);
      }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* hole
//MD
//MDUse the `hole` stage to safely consume all records form preceeding stages.
//MD
 */


