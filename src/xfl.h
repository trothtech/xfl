/*
 *        Name: xfl.h (C program header)
 *        Date: 2023-06-12 (Mon) roughly
 *              2024-01-07 (Sun)
 *              2024-06-21 (Fri) for 1.0.0 at VM Workshop
 *
 * Prefix XFL* has been assigned to the Ductwork/Plenum project by IBM.
 * Please keep IBM informed so that they can update the prefix database.
 */

#ifndef _XFLLIB_H

//static char *_xfl_version = "XFL 1.0.3";
#define  XFL_VERSION  (((1) << 24) + ((0) << 16) + ((3) << 8) + (0))
//static int xfl_version = XFL_VERSION;

/* the following mnemonics represent bits in the flag field           */
#define     XFL_F_INPUT         0x0001
#define     XFL_F_OUTPUT        0x0002
#define     XFL_F_KEEP          0x0010           /* keep during spawn */
#define     XFL_F_SEVERED       0x0020           /* explicit or EPIPE */

/* the following mnemonics are simple numeric but must be unique      */
#define     XFL_E_NONE          0
#define     XFL_E_SEVERED       12         /* to follow CMS Pipelines */
#define     XFL_E_NULLPTR       31                       /* arbitrary */

/* a connection must be input or output but not both */
#define     XFL_E_DIRECTION     100        /* to follow CMS Pipelines */

#define     XFL_E_2756          2756  /* Too many operands. */
#define     XFL_E_2811          2811  /* A stream with the stream identifier specified is already defined. */

/*          XFL_MAX_STREAMS  16       */

#ifdef __cplusplus
extern "C" {
#endif

/* connectors are paired and this struct describes either side, but   */
/* there should always be two: producer (output) and consumer (input) */
typedef struct PIPECONN {

    int fdf;         /* forward flowing file descriptor used for data */
                 /* data flows "downstream" from producer to consumer */
    int fdr;      /* reverse flowing file descriptor used for control */
                 /* control goes "upstream" from consumer to producer */
    int flag;   /* which side of the connection, producer or consumer */

    char name[16];            /* name of connector for a named stream */
    int n;               /* number of connector for a numbered stream */

    int pline, pstep;          /* which pipeline and which step/stage */
    int cpid;             /* PID of child process which inherited FDs */

    int rn;   /* record number - how many records have been processed */
    /* This leads to 1-based indexing because when this is zero       */
    /* nothing has happened yet. So there is no "record zero".        */

    void *buff;        /* optional buffer for shared memory transfers */
    void *glob;                                        /* global area */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */

                        } PIPECONN;

/* This struct describes a stage. All stage structs should be chained */
/* so that the launcher can bring them up and wait for them to exit.  */
typedef struct PIPESTAGE {
    char *text;                       /* string describing this stage */
//  int plinenumb;              /* pipeline where this stage runs N/A */
//  int stagenumb;            /* number of this stage in its line N/A */
    char *label;                          /* pointer to label, if any */
    char *arg0;                          /* executable name or "verb" */
    char *args;                                   /* arguments string */
//  int argc;
//  char **argv;
    int  ipcc;                          /* input pipe connector count */
    void *ipcv[16];              /* input pipe connector vector array */
    int  opcc;                         /* output pipe connector count */
    void *opcv[16];             /* output pipe connector vector array */
    int  xpcc;                         /* COMMON pipe connector count */
    void *xpcv[16];             /* COMMON pipe connector vector array */

    int cpid;             /* PID of child process handling this stage */

    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */
                         } PIPESTAGE;

/* --- function prototypes ------------------------------------------ */

char*xfl_argcat(int,char*[]);     /* gather argc/argv into one string */

int xfl_error(int,int,char**,char*);      /* msgn, msgc, msgv, caller */
int xfl_trace(int,int,char**,char*);      /* msgn, msgc, msgv, caller */
int xfl_pipepair(PIPECONN*[]);             /* allocate an in/out pair */
int xfl_getpipepart(PIPESTAGE**,char*);

int xfl_stagespawn(int,char*[],PIPECONN*[],PIPESTAGE*);

/* --- function prototypes for stages ------------------------------- */

int xfl_stagestart(PIPECONN**);           /* returns a pipeconn array */
int xfl_peekto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_readto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_output(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_sever(PIPECONN*);                   /* disconnect a connector */
int xfl_stagequit(PIPECONN*);          /* releases the pipeconn array */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#define _XFLLIB_H
#endif

/*

consumer sends:
"STAT" ** the only meta data at this point in the development
          producer sends "DATA seq bytes", number of current record and how big

consumer sends:
"PEEK" ** think PIPLOCAT to examine a record
          producer sends data

consumer sends:
"NEXT" ** think PIPINPUT (sort of) consume the record
          producer sends data and advances the sequence count

consumer sends:
"QUIT" ** for SEVER operation
          producer closes file descriptors
          consumer closes file descriptors

consumer sends:
"FAIL" ** if something went wrong

          consumer may also receive a "FAIL" from any of the above
          as long as it is not misunderstood as data (like after STAT)

 */


