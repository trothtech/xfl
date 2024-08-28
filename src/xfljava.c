/*
 *
 *        Name: xfljava.c (C program source)
 *              Language binding layer for XFL for Java (JNI)
 *        Date: 2024-06-11 (Tue) based on other JNI work
 *    See also: PipelineService.java
 *
 */

#include <string.h>
#include <stdlib.h>

#include "xfl.h"

#include <jni.h>








/* ------------------------------------------------------------- version
 * Class:     com_casita_xmitmsgx_MessageService
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_casita_xmitmsgx_MessageService_version
        (JNIEnv *env, jclass  thisObj)     /* jclass replaces jobject */
  {
    char  versionstring[256];
    int vermaj, vermin, verrev, verinc;
    int xmitmsgx_version = XMITMSGX_VERSION;

    vermaj = (xmitmsgx_version & 0xff000000) >> 24;          /* major */
    vermin = (xmitmsgx_version & 0x00ff0000) >> 16;          /* minor */
    verrev = (xmitmsgx_version & 0x0000ff00) >> 8;        /* revision */
    verinc =  xmitmsgx_version & 0x000000ff;             /* increment */

    sprintf(versionstring,"XMITMSGX version %d.%d.%d",vermaj,vermin,verrev,verinc);

    /* convert the C-string (char*) into JNI String (jstring) and return */
    return (*env)->NewStringUTF(env, versionstring);
  }

/* ---------------------------------------------------------------- init
 * Class:     com_casita_xmitmsgx_MessageService
 * Method:    init
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 * Calls:     xminit()
 */
JNIEXPORT jstring JNICALL Java_com_casita_xmitmsgx_MessageService_init
        (JNIEnv *env, jclass  thisObj, jstring lib, jstring opt)
  { int rc, r2;
    const char *fn;
    struct MSGSTRUCT ms0, *ms1;
    jboolean iscopy;
    char buffer[256], *msgv[256], msgt[16];

    /* convert the supplied Java string to a standard C string for fn */
    fn = (*env)->GetStringUTFChars(env, lib, &iscopy);

    /* the static pointer is not null here then we were called multi  */
    if (ms != NULL)
      { if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            rc = xmopen("xmitmsgx",0,ms1);
            if (rc != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        xmprint(302,0,NULL,0,errms);           /* already initialized */
        xmstring(buffer,sizeof(buffer),302,0,NULL,errms);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* allocate memory for a messages struct and point to it          */
    ms = malloc(sizeof(ms0));

    /* if the static pointer is null here then we have a problem      */
    if (ms == NULL)
      { if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            rc = xmopen("xmitmsgx",0,ms1);
            if (rc != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        /* it would be good to get a clue here from error if possible */
        xmprint(301,0,NULL,0,errms);                                    /* FIXME: need the correct error message */
        xmstring(buffer,sizeof(buffer),301,0,NULL,errms);               /* FIXME: need the correct error message */
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* open the messages file and populate the messages struct */
    rc = xmopen((char*)fn,0,ms);
    if (rc != 0)
      { free(ms); ms = NULL;
        if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            r2 = xmopen("xmitmsgx",0,ms1);
            if (r2 != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        sprintf(msgt,"%d",rc);
        msgv[0] = "java";
        msgv[1] = msgt;
        msgv[2] = "xmopen()";
        xmprint(514,3,msgv,0,errms);           /* not yet initialized */
        xmstring(buffer,sizeof(buffer),514,3,msgv,errms);               /* FIXME: need the correct error message */
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* using pfxmaj and pfxmin is definitely outside the XMITMSGX API */
    strncpy(ms->pfxmaj,"XMM",4);
    strncpy(ms->pfxmin,"JNI",4);
    /* this really needs to be changed to allow matching the caller   */

    /* make the address of the struct into a hexadecimal string       */
    sprintf(buffer,"%x",ms);

    /* convert the C-string (char*) into JNI String (jstring) and ret */
    return (*env)->NewStringUTF(env, buffer);
  }

/* --------------------------------------------------------------- print
 * Class:     com_casita_xmitmsgx_MessageService
 * Method:    print
 * Signature: (I[Ljava/lang/String;)Ljava/lang/String;
 * Calls:     xmprint()
 */
JNIEXPORT jstring JNICALL Java_com_casita_xmitmsgx_MessageService_print
        (JNIEnv *env, jclass  thisObj, jint mn, jobjectArray arg)
  { int n, i, j, rc;
    char buffer[256], *msgv[256];
    jobject jojo;
    jboolean iscopy;
    struct MSGSTRUCT ms0, *ms1;

    /* if the static pointer is null then we were called before init  */
    if (ms == NULL)
      { if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            rc = xmopen("xmitmsgx",0,ms1);
            if (rc != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        xmprint(301,0,NULL,0,errms);           /* not yet initialized */
        xmstring(buffer,sizeof(buffer),301,0,NULL,errms);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* number of replacement tokens is indicated by size of the array */
    n = (*env)->GetArrayLength(env,arg);

    msgv[0] = "java";           /* arbitrary - token zero is reserved */
    j = 1;
    for (i = 0; i < n; i++)
      { jojo = (*env)->GetObjectArrayElement(env,arg,i);
        msgv[j] = (*env)->GetStringUTFChars(env, jojo, &iscopy);
        j++; }

    n++;          /* increment token count to handle our quirky logic */
    rc = xmprint(mn,n,msgv,0,ms);        /* print the selectd message */

    /* if we got an error then try to throw it the expected Java way  */
    if (rc < 0)
      /* FIXME: insert proper error handling logic here               */
      { jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"xmprint() failed"); return NULL; }

    /* convert the C-string (char*) into JNI String (jstring) and return */
    buffer[0] = 0x00;
    return (*env)->NewStringUTF(env, buffer);
  }

/* -------------------------------------------------------------- string
 * Class:     com_casita_xmitmsgx_MessageService
 * Method:    string
 * Signature: (I[Ljava/lang/String;)Ljava/lang/String;
 * Calls:     xmstring()
 */
JNIEXPORT jstring JNICALL Java_com_casita_xmitmsgx_MessageService_string
        (JNIEnv *env, jclass  thisObj, jint mn, jobjectArray arg)
  { int n, i, j, rc;
    char buffer[256], *msgv[256];
    jobject jojo;
    jboolean iscopy;
    struct MSGSTRUCT ms0, *ms1;

    /* if the static pointer is null then we were called before init  */
    if (ms == NULL)
      { if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            rc = xmopen("xmitmsgx",0,ms1);
            if (rc != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        xmprint(301,0,NULL,0,errms);           /* not yet initialized */
        xmstring(buffer,sizeof(buffer),301,0,NULL,errms);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* number of replacement tokens is indicated by size of the array */
    n = (*env)->GetArrayLength(env,arg);

    msgv[0] = "java";           /* arbitrary - token zero is reserved */
    j = 1;
    for (i = 0; i < n; i++)
      { jojo = (*env)->GetObjectArrayElement(env,arg,i);
        msgv[j] = (*env)->GetStringUTFChars(env, jojo, &iscopy);
        j++; }

    n++;          /* increment token count to handle our quirky logic */
    rc = xmstring(buffer,sizeof(buffer),mn,n,msgv,ms);

    /* if we got an error then try to throw it the expected Java way  */
    if (rc < 0)
      /* FIXME: insert proper error handling logic here               */
      { jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"xmstring() failed");   }

    /* convert the C-string (char*) into JNI String (jstring) and return */
    return (*env)->NewStringUTF(env, buffer);
  }

/* ---------------------------------------------------------------- quit
 * Class:     com_casita_xmitmsgx_MessageService
 * Method:    quit
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 * Calls:     xmclose()
 */
JNIEXPORT jstring JNICALL Java_com_casita_xmitmsgx_MessageService_quit
        (JNIEnv *env, jclass  thisObj, jstring obj)
  {
    int rc;
    const char *xo;
    char buffer[256];
    jboolean iscopy;
    struct MSGSTRUCT ms0, *ms1;

    /* if the static pointer is null then we were called before init  */
    if (ms == NULL)
      { if (errms == NULL)    /* conditionally initialize error stack */
          { ms1 = malloc(sizeof(ms0));
            if (ms1 == NULL)
              { perror("malloc():");
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            rc = xmopen("xmitmsgx",0,ms1);
            if (rc != 0)
              { perror("xmopen()"); free(ms1);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,"malloc() failed"); return NULL; }
            errms = ms1; }
        xmprint(301,0,NULL,0,errms);           /* not yet initialized */
        xmstring(buffer,sizeof(buffer),301,0,NULL,errms);
        jclass Exception = (*env)->FindClass(env,"java/lang/Exception");
        (*env)->ThrowNew(env,Exception,buffer); return NULL; }

    /* convert the supplied Java string to a standard C string for xo */
    xo = (*env)->GetStringUTFChars(env, obj, &iscopy);

    /* do an orderly shutdown of the library */
    xmclose(ms);
    free(ms); ms = NULL;

    /* if we opened a supplemental handler then close it too          */
    if (errms != NULL) { xmclose(errms); free(errms); errms = NULL; }

    /* convert the C-string (char*) into JNI String (jstring) and return */
    buffer[0] = 0x00;
    return (*env)->NewStringUTF(env, buffer);
  }


