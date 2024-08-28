/*
 *        Name: cp.c (C program source)
 *              POSIX Pipelines CP stage (z/VM Control Program)
 *        Date: 2024-06-05 (Wednesday) after email from Sir Rob
 *    See also: RXDIAGRC package
 *
 *   Rationale: This stage is intended to be compatible with the 'cp'
 *              counterpart stage in CMS Pipelines. There is an obvious
 *              conflict with the 'cp' command in Unix/Linux/POSIX,
 *              but the latter is never pipelined and so makes
 *              no sense as a "stage". There is no runtime collision.
 *
 *        Note: This stage requires the Boeblingen/Borntraeger module
 *              which is standard on z/Linux. It will not work on other
 *              systems and requires that Linux be running on z/VM.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* see IBM's "s390-tools" package source for these */
#define   VMCP_GETCODE  _IOR(0x10, 1, int)
#define   VMCP_SETBUF   _IOW(0x10, 2, int)
#define   VMCP_GETSIZE  _IOR(0x10, 3, int)

#include <xfl.h>

int main(int argc,char*argv[])
  { static char _eyecatcher[] = "pipeline stage 'cp' main()";
    int rc, fd, buflen;
    char *args, buffer[4096], *msgv[4], em[16], *p, *q;
    struct PIPECONN *pc, *pi, *po, *pn;

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0)
      { /* 0303 E Return code &1 from &2                              */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[1] = "xfl_stagestart()"; /* identify the failing routine */
        xfl_error(303,4,msgv,"VMC");   /* provide XFL specific report */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;
    /* above is (hopefully) a no-op for stages called by the launcher */
    /* because they should get exactly one string and that as argv[1] */

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_F_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_F_INPUT)  { if (pi == NULL) pi = pn; } }

    if (strlen(args) > 0)
      {

    /* try the VMCP device for DIAG 08 for z/VM "console function"    */
    rc = fd = open("/dev/vmcp",O_RDWR);
    if (rc < 0)
      { perror("VMCP: open()");       /* provide standard Unix report */
        /* 0240 E Function &1 not supported                           */
        msgv[1] = "CP";              /* not much better way to say it */
        xfl_error(240,2,msgv,"VMC");       /* provide specific report */
        xfl_stagequit(pc);        /* proper shutdown of the interface */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* set the buffer size to be used by the driver for response      */
    buflen = sizeof(buffer);
    rc = ioctl(fd,VMCP_SETBUF,&buflen);
    /* skipping error checks here and hoping for the best             */

    /* write the command line argstring as a command to the z/VM host */
    rc = write(fd,args,strlen(args));
    if (rc < 0)                   /* aaand... try to deal with errors */
      { rc = errno;   /* hold onto the error number in case it resets */
        perror("VMCP: write()");      /* provide standard Unix report */
        /* 0699 E Return code &1 from &2 (file: &3)                   */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[2] = "write()";      /* the name of the failing function */
        msgv[3] = "/dev/vmcp";      /* the name of the interface file */
        xfl_error(699,4,msgv,"VMC");       /* provide specific report */
        xfl_stagequit(pc);        /* proper shutdown of the interface */
        return 1; }    /* this amounts to an exit - proess terminates */

    /* read the hypervisor command response or result or output       */
    buflen = sizeof(buffer);         /* limit the read to buffer size */
    rc = read(fd,buffer,buflen);      /* read from the VMCP interface */
    if (rc >= 0) buffer[rc] = 0x00;       /* and terminate the string */
    if (rc < 0)                   /* aaand... try to deal with errors */
      { rc = errno;   /* hold onto the error number in case it resets */
        perror("VMCP: read()");       /* provide standard Unix report */
        /* 0699 E Return code &1 from &2 (file: &3)                   */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        msgv[2] = "read()";       /* the name of the failing function */
        msgv[3] = "/dev/vmcp";      /* the name of the interface file */
        xfl_error(699,4,msgv,"VMC");       /* provide specific report */
        xfl_stagequit(pc);        /* proper shutdown of the interface */
        return 1; }    /* this amounts to an exit - proess terminates */
    buflen = rc;            /* retain response length via return code */

    /* recover the return code from the hypervisor command            */
    if (ioctl(fd,VMCP_GETCODE,&rc) == -1) rc = -1;
    /* z/VM CP would also set a condition code which we don't recover */

    /* close the interface device file even though we might re-open   */
    close(fd);

    /* write the response from the command as multi-record output     */
    p = q = buffer;
    while (*p != 0x00)
      {
        while (*p != 0x00 && *p != '\n') p++;       /* find end or NL */
        if (*p == 0x00) break;        /* if at the end then break out */
        *p++ = 0x00;                      /* terminate this substring */
        rc = xfl_output(po,q,strlen(q));    /* send record downstream */
        if (rc < 0) break;    /* if any error then just stop the loop */
        q = p;
      }

      }

    /* at this point we should loop reading records each as a command */
    if (pi != NULL) while (1)
      {

        buflen = sizeof(buffer);        /* start with max record size */
        rc = xfl_peekto(pi,buffer,buflen);        /* sip on the input */
        if (rc < 0) break; /* else */ buflen = rc;
        buffer[rc] = 0x00;                    /* terminate the string */

        /* open the VMCP device for DIAG 08 for z/VM console function */
        rc = fd = open("/dev/vmcp",O_RDWR);
        if (rc < 0)
          { rc = errno;    /* hold the error number in case it resets */
            perror("VMCP: open()");           /* standard Unix report */
            msgv[1] = "CP";          /* not much better way to say it */
            xfl_error(240,2,msgv,"VMC");       /* standard XFL report */
            break; }

        buflen = sizeof(buffer);               /* get the buffer size */
        ioctl(fd,VMCP_SETBUF,&buflen);         /* set the buffer size */

        rc = write(fd,buffer,strlen(buffer));
        if (rc < 0)               /* aaand... try to deal with errors */
          { rc = errno;    /* hold the error number in case it resets */
            perror("VMCP: write()");          /* standard Unix report */
            sprintf(em,"%d",rc); msgv[1] = em;   /* integer to string */
            msgv[2] = "write()";          /* name of failing function */
            msgv[3] = "/dev/vmcp";  /* the name of the interface file */
            xfl_error(699,4,msgv,"VMC");       /* standard XFL report */
            break; }                           /* this is NOT an exit */

        buflen = sizeof(buffer);         /* limit read to buffer size */
        rc = read(fd,buffer,buflen);      /* read from VMCP interface */
        if (rc >= 0) buffer[rc] = 0x00;       /* terminate the string */
        if (rc < 0)               /* aaand... try to deal with errors */
          { rc = errno;   /* hold onto the error number in case it resets */
            perror("VMCP: read()");       /* provide standard Unix report */
            sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
            msgv[2] = "read()";      /* name of failing function */
            msgv[3] = "/dev/vmcp";      /* the name of the interface file */
            xfl_error(699,4,msgv,"VMC");       /* provide specific report */
            break; }       /* this amounts to an exit - proess terminates */
        buflen = rc;        /* retain response length via return code */

        /* recover the return code from this particular command       */
        if (ioctl(fd,VMCP_GETCODE,&rc) == -1) rc = -1;
        close(fd);

        /* write the response from the command as multi-record output */
        p = q = buffer;
        while (*p != 0x00)
          {
            while (*p != 0x00 && *p != '\n') p++;        /* end or NL */
            if (*p == 0x00) break;    /* if at the end then break out */
            *p++ = 0x00;                  /* terminate this substring */
            rc = xfl_output(po,q,strlen(q));    /* send it downstream */
            if (rc < 0) break;   /* if any error then stop inner loop */
            q = p;
          }

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);             /* consume the record */
        if (rc < 0) break;
      }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

    return 0;
  }

/*
//MD
//MD* cp
//MD
//MDUse the `cp` stage to issue VM (CP) commands and recover their output.
//MD
//MDThis stage requires the z/VM hypervisor.
//MDIt really only works on z/Linux when hosted by z/VM.
//MD
//MDThis stage requires privileges. Usually, one must be root to issue CP commands.
//MD
 */


