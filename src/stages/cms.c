/*
 *        Name: cms.c (C program source)
 *              POSIX Pipelines CMS stage
 *        Date: 2024-06-05 (Wednesday) after email from Sir Rob
 *              There is no CMS environment available in this context.
 *              This stage exists primarily so that pipelines taken
 *              from VM/CMS (where they would use CMS Pipelines)
 *              will at least parse correctly.
 *
 *              If an interface to VM/CMS becomes available it would be
 *              utilized here and then this stage would not be a no-op.
 */

#include <stdio.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'cms'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'cms' main()";
    int rc;
    char *msgv[2], em[16];
    struct PIPECONN *pc;

    /* initialize this stage .. just for completeness                 */
    rc = xfl_stagestart(&pc);
    if (rc < 0)
      { /* 0303 E Return code &1 from &2                              */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[1] = "xfl_stagestart()"; /* identify the failing routine */
        xfl_error(303,4,msgv,"CMS");   /* provide XFL specific report */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* 0240 E Function &1 not supported                               */
    msgv[0] = argv[0];            /* this is not actually used by XMM */
    msgv[1] = "CMS";         /* identify the missing function/service */
    xfl_error(240,2,msgv,"CMS");       /* provide XFL specific report */

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 1;
  }

/*
other possibilities ...
0154 E Operating environment not supported by stage
0257 E Subcommand environment &1 not found
0308 E system service &1 not valid
0377 E Subsystem &1 is not defined
0711 E Function not supported: &1
1240 I Unknown system command: &1
 */


