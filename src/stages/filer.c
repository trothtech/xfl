/*
 *        Name: filer.c (C program source)
 *              POSIX Pipelines FILER stage (file read)
 *        Date: 2024-05-20 (Mon) taking a break from Rexx development
 *              This stage reads a file and writes records downstream.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

#include <xfl.h>

static char _eyeball0[] = "XFL pipeline stage 'filer'";

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  { static char _eyecatcher[] = "XFL pipeline stage 'filer' main()";
    int rc, fd, buflen, i, j, k, l;
    char *args, *fn, *p, *q, *r, buffer[65536], *msgv[16];
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) {
      printf("filer: xfl_stagestart() returned %d\n",rc); return 1; }

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

    /* take the first blank-delimited word as the name of the file    */
    p = args;
    while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
    q = p;
    while (*q != ' ' && *q != '\t' && *q != 0x00) q++;
    if (*q != 0x00) *q = 0x00;
    fn = p;

    /* open the named file for reading                                */
    rc = fd = open(fn,O_RDONLY);
    if (rc < 0)
      { char em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("filer(): open()");   /* provide standard Unix report */
        /* 0699 E Return code &1 from &2 (file: &3) */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        msgv[2] = "open()";
        msgv[3] = fn;
        xfl_error(699,4,msgv,"FIO");       /* provide specific report */
        return -1;
      }

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    /* 0061 E Output specification missing, "no output"               */
    if (po == NULL) { xfl_error(61,0,NULL,"FIO"); return 1; }

    /* 0087 E This stage must be the first stage of a pipeline        */
    if (pi != NULL) { xfl_error(87,0,NULL,"FIO"); return 1; }

/*
      i == index .. offset into buffer
      j
      k == kap .. end of buffer
      l == length of current segment
 */

    p = q = r = buffer; k = sizeof(buffer) - 1; j = 0; i = 0;
    while (i < k)
      {
        /* get some content from the file */
        if (j == 0)
          { rc = j = read(fd,p,k);
//          if (rc < 0) { }
//printf("RC %d\n",rc);
            k = j;
          }

        /*                                                            */
        l = 0;
        while (*p != '\n' && *p != 0x00 && l < j)
          { q = p; l++; p++; }

        /*                                                            */
        if (*p == '\n')
          { *p++ = 0x00; i = i + l; i++;
            if (*q == '\r') { *q = 0x00; l--; }
//          rc = xfl_output(po,r,l);
//printf("'%s' %d\n",r,l);

        /* write the record to our primary output stream              */
        rc = xfl_output(po,r,l);                /* send it downstream */
        if (rc < 0) break;

        r = p;
//printf("I %d\n",i);

//          break;
          }

        /* now consume the record from the input stream               */
//      xfl_readto(pi,NULL,0);                  /* consume the record */
//      if (rc < 0) break;
      }
    if (rc < 0) return 1;

/*
printf("broke out\n");
    rc = read(fd,p,k);
printf("RC %d\n",rc);
    rc = read(fd,p,k);
printf("RC %d\n",rc);
 */

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* filer (file read), aliased as "&lt;"
//MD
//MDUse `<` to read from a file.
//MD
 */


