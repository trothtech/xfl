/*
 *
 *        Name: xflrexx.c (C program source)
 *              Rexx (Regina) interface for POSIX Pipelines
 *        Date: 2024-05-19 (Sun) happy 60th MEN
 *    See also: xflrexx.rx
 *
 * Enable with: Call Rxfuncadd 'xfl', 'xflrexx', 'RxDuctwork'
 *                              funcname,   library,   entryname
 *   Call with: Parse Value xfl(subfunction,args,...) With rc rs
 *
 *  References: IBM publication SC24-6113
 *
 *  Build with: cc -fPIC -o xflrexx.o -c xflrexx.c
 *              cc -shared -o libxflrexx.so xflrexx.o
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "xfl.h"

#define INCL_REXXSAA
#include <rexxsaa.h>

static struct PIPECONN *xfl_rxpc = NULL;

extern int xfl_errno;

/* ------------------------------------------------------------- VERSION
 * Arguments: none
 * Returns: 0, always
 * Retstr: rc, name of package, version
 * The library version is an integer constant globally readable.
 */
int rxversion(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int xfl_version = XFL_VERSION;
    int vv, rr, mm;
    char verstr[16], *msgv[4];

    /* code variation here is historical ... maybe resolved later     */

    /* display the version using the message handler/displayer        */
    vv = (xfl_version >> 24) & 0xff;           /* extract the version */
    rr = (xfl_version >> 16) & 0xff;           /* extract the release */
    mm = (xfl_version >> 8) & 0xff;          /* extract the mod level */
    sprintf(verstr,"%d.%d.%d",vv,rr,mm);
    msgv[0] = "";
    msgv[1] = verstr;
    xfl_error(86,2,msgv,"PIP");        /* provide standardized report */

    /* return the version (numbers only) to the Rexx caller           */
    snprintf(rxrets->strptr,rxrets->strlength,"%d.%d.%d",
       (xfl_version>>24),
      ((xfl_version>>16) & 0xFF),
      ((xfl_version>>8) & 0xFF));
    /* set length of Rexx return string to follow C string length     */
    rxrets->strlength = strlen(rxrets->strptr);

    /* always return without error */
    return 0;
  }

/* ---------------------------------------------------------------- INIT
 * Arguments: none
 * Returns: 0 and sets PIPECONN struct pointer, else rc of stagestart()
 * Retstr: rc, hex address of PIPECONN structs or possible error string
 * Calls: xfl_stagestart()
 */
int rxinit(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int rc;
    struct PIPECONN *pc;

    /* if we have done this before then return no error               */
    if (xfl_rxpc != NULL)
      { snprintf(rxrets->strptr,rxrets->strlength,"%x",xfl_rxpc);
        rxrets->strlength = strlen(rxrets->strptr);
        return 0; }

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc != 0) return rc;

    /* supply a return string */
    snprintf(rxrets->strptr,rxrets->strlength,"%08X",pc);
    rxrets->strlength = strlen(rxrets->strptr);

    xfl_rxpc = pc;
    return 0;
  }

/* -------------------------------------------------------------- PEEKTO
 * Arguments: stream number, variable name (latter not implemented)
 * Returns: status/errno
 * Retstr: rc, record data or possible error string if error
 * Calls: xfl_peekto()
 */
int rxpeekto(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int rc, l, j, buflen;
    char buffer[4096];
    struct PIPECONN *pc, *pi, *pn;

    pc = xfl_rxpc;

    l = rxargv->strlength; j = 0;
    if (l >= sizeof(buffer)) l = sizeof(buffer) - 1;
    strncpy(buffer,rxargv->strptr,l); buffer[l] = 0x00;
    j = atoi(buffer);            /* the number of the selected stream */
    rxargv++;  rxargc--;   /* bump count and pointer to next argument */

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = pc; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

    buflen = sizeof(buffer) - 1;
    rc = xfl_peekto(pi,buffer,buflen);                /* sip on input */
//printf("rxpeekto(): xfl_peekto(,,%d) returned %d\n",buflen,rc);

    if (rc < 0)
      { /* FIXME: return something other than an empty error string   */
        rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
        return rc; }

    buflen = rc;
    if (buflen > rxrets->strlength) buflen = rxrets->strlength;
    sprintf(rxrets->strptr,"%s",buffer);
//  snprintf(rxrets->strptr,buflen,"%s",buffer);
    rxrets->strlength = strlen(rxrets->strptr);

//  rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
    return 0;
  }

/* -------------------------------------------------------------- READTO
 * Arguments: stream number, variable name (latter not implemented)
 * Returns: status/errno
 * Retstr: rc, record data or possible error string if error
 * Calls: xfl_readto()
 */
int rxreadto(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int rc, l, j, buflen;
    char buffer[4096];
    struct PIPECONN *pc, *pi, *pn;

//printf("rxreadto(): %d args\n",rxargc);
    pc = xfl_rxpc;

    l = rxargv->strlength; j = 0;
    if (l >= sizeof(buffer)) l = sizeof(buffer) - 1;
    strncpy(buffer,rxargv->strptr,l); buffer[l] = 0x00;
//printf("rxreadto(): '%s' '%s'\n",rxargv->strptr,buffer);
    j = atoi(buffer);            /* the number of the selected stream */
    rxargv++;  rxargc--;   /* bump count and pointer to next argument */

//printf("rxreadto(): %d\n",j);

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = pc; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_INPUT) pi = pn;

    buflen = sizeof(buffer) - 1;
    rc = xfl_readto(pi,buffer,buflen);          /* consume the record */
//printf("rxreadto(): xfl_readto(,,%d) returned %d\n",buflen,rc);

    if (rc < 0)
      { /* FIXME: return something other than an empty error string   */
        rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
        return rc; }

    buflen = rc;
    if (buflen > rxrets->strlength) buflen = rxrets->strlength;
    snprintf(rxrets->strptr,buflen,"%s",buffer);
    rxrets->strlength = strlen(rxrets->strptr);

//  rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
    return rc;
  }

/* -------------------------------------------------------------- OUTPUT
 * Arguments: stream number, data to write downstream
 * Returns: status/errno
 * Retstr: rc, possible error string
 * Calls: xfl_output()
 */
int rxoutput(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int rc, l, j, buflen;
    char buffer[4096];
    struct PIPECONN *pc, *po, *pn;

//printf("rxoutput(): %d args\n",rxargc);
    pc = xfl_rxpc;

    l = rxargv->strlength; j = 0;
    if (l >= sizeof(buffer)) l = sizeof(buffer) - 1;
    strncpy(buffer,rxargv->strptr,l); buffer[l] = 0x00;
//printf("rxoutput(): '%s' '%s'\n",rxargv->strptr,buffer);
    j = atoi(buffer);            /* the number of the selected stream */
    rxargv++;  rxargc--;   /* bump count and pointer to next argument */

//printf("rxoutput(): %d\n",j);

    /* snag the first output stream from the chain-o-connectors       */
    po = NULL;
    for (pn = pc; po == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_F_OUTPUT) po = pn;

//printf("rxoutput(): '%s'\n",rxargv->strptr);
    rc = xfl_output(po,rxargv->strptr,rxargv->strlength);
//printf("rxoutput(): xfl_output(,,%d) returned %d\n",rxargv->strlength,rc);

    if (rc < 0)
      { /* FIXME: return something other than an empty error string   */
        rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
        return rc; }

//  rc = xmstring(rxrets->strptr,rxrets->strlength,msgn,msgc,msgv,mymsgstruct);
//  rxrets->strlength = strlen(rxrets->strptr);
    rxrets->strptr[0] = 0x00; rxrets->strlength = 0;
    return rc;
  }

/* ---------------------------------------------------------------- QUIT
 * Arguments: hex address of msgstruct (not presently implemented)
 * Returns: 0 and clears PIPECONN struct pointer, else rc of stagequit()
 * Retstr: rc, possible error string
 * Calls: xfl_stagequit()
 */
int rxquit(ULONG rxargc, RXSTRING rxargv[],RXSTRING*rxrets)
  {
    int rc;
    struct PIPECONN *pc;

    /* if we have done this before then return no error               */
    if (xfl_rxpc == NULL)
      { snprintf(rxrets->strptr,rxrets->strlength,"");
        rxrets->strlength = strlen(rxrets->strptr);
        return 0; }

    /* terminate this stage cleanly                                   */
    pc = xfl_rxpc;
    rc = xfl_stagequit(pc);
    if (rc != 0) return rc;

    /* supply an empty return string */
    rxrets->strptr[0] = 0x00; rxrets->strlength = 0;

    xfl_rxpc = NULL;
    return 0;
  }

/* ------------------------------------------------------------------ *
 * Regina Calling Convention:                                         *
 *      name == name by which this function was called (C string)     *
 *    rxargc == number of REXX arguments supplied on the call         *
 *      rxargv == array of arguments (of type RXSTRING)               *
 * queuename == name of the current queue (C string)                  *
 *    retstr == 256 return buffer (of type RXSTRING)                  *
 * Type RXSTRING:                                                     *
 *    ->strlength long int                                            *
 *    ->strptr char pointer                                           *
 * ------------------------------------------------------------------ */
APIRET APIENTRY
RxDuctwork(CONST CHAR *name,
           ULONG rxargc, RXSTRING rxargv[],
           CONST UCHAR *queuename, RXSTRING *retstr)
  {
    int rc, rl;
    char *rs, *sf;
    RXSTRING rxrets;
    char rsdata[4096];

    /* establish a return string for all internal subfunction calls   */
    rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
//  strcpy(rsdata,"Rexx Rocks!"); rxrets.strlength = 12;

    /* all subfunctions require at least one argument                 */
    if (rxargc < 1) return RXFUNC_BADTYPE; /* Incorrect call to routine */

    /* the first argument is the subfunction (sf)                     */
    if (rxargv->strlength < 1) return RXFUNC_BADTYPE;
    sf = rxargv->strptr;         /* we only care about the first byte */
//  rxargv++;  rxargc--;   /* bump count and pointer to next argument */

    /* switch based on subfunction: I, O, P, Q, R, V                  */
    switch (*sf) {
      case 'I': case 'i': /* init */
//      if (rxargc < 1) return RXFUNC_BADTYPE;
//      if (rxargc > 2) return RXFUNC_BADTYPE;
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxinit(rxargc,rxargv,&rxrets);
        if (rc > 0) rc = 0 - rc;       /* force errors to be negative */
        break;
      case 'O': case 'o': /* output */
//      if (rxargc < 1) return RXFUNC_BADTYPE;
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxoutput(rxargc,rxargv,&rxrets);
        break;
      case 'P': case 'p': /* peekto */
//      if (rxargc < 1) return RXFUNC_BADTYPE;
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxpeekto(rxargc,rxargv,&rxrets);
        break;
      case 'R': case 'r': /* readto */
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxreadto(rxargc,rxargv,&rxrets);
        break;
      case 'Q': case 'q': /* quit */
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxquit(rxargc,rxargv,&rxrets);
        if (rc > 0) rc = 0 - rc;       /* force errors to be negative */
        break;
      case 'V': case 'v': /* quit */
        rxrets.strptr = rsdata; rxrets.strlength = sizeof(rsdata) - 1;
        rxargv++;  rxargc--;    /* bump count and pointer to next arg */
        rc = rxversion(rxargc,rxargv,&rxrets);
        if (rc > 0) rc = 0 - rc;       /* force errors to be negative */
        break;
      default: /* uh oh! */
        return RXFUNC_BADTYPE;           /* Incorrect call to routine */
              }

//  retstr->strptr[retstr->strlength] = 0x00;
//  if (rc < 0) if (xfl_errno == XFL_E_SEVERED) rc = XFL_E_SEVERED;
    if (rc < 0) rc = 0 - rc;      /* force negative RC to be positive */
           else rc = 0;            /* but positive RC is not an error */
    sprintf(retstr->strptr,"%d %s",rc,rxrets.strptr);
//  snprintf(retstr->strptr,retstr->strlength,"%d %s",rc,rxrets.strptr);
    retstr->strlength = strlen(retstr->strptr);

    /* return the results */
    return RXFUNC_OK;
  }


