#!/bin/sh
#
#         Name: rxsample.sh (shell script)
#               demonstrate POSIX Pipelines called from Rexx
#         Date: 2024-05-19 (Sun) happy 60th MEN
#

#
# make some detection about this environment
cd `dirname "$0"`
D=`pwd`                         # the directory where these files reside
E=`sh -c ' cd .. ; exec pwd '`  # presumed package root (one level up)

#
# try to extract the installation prefix from the source configuration
F=`grep "^#define" configure.h \
    | grep ' PREFIX ' \
    | awk '{print $3}' \
    | sed 's#"##g'`
if [ ! -z "$F" ] ; then E=$F ; fi

#
# try to be sure that the Rexx sample stage sits alongside other stages
if [ ! -x $E/libexec/xfl/rxstage ] ; then
    echo "+ cp -p rxstage.rx $E/libexec/xfl/rxstage"
            cp -p rxstage.rx $E/libexec/xfl/rxstage
                  chmod a+rx $E/libexec/xfl/rxstage
fi

#
# set or expand the loadable library search
if [ -z "$LD_LIBRARY_PATH" ] ; then LD_LIBRARY_PATH="$E/lib"
                               else LD_LIBRARY_PATH="$E/lib:$LD_LIBRARY_PATH" ; fi
if [ -z "$DYLD_LIBRARY_PATH" ] ; then DYLD_LIBRARY_PATH="$E/lib"
                                 else DYLD_LIBRARY_PATH="$E/lib:$DYLD_LIBRARY_PATH" ; fi
if [ -z "$SHLIB_PATH" ] ; then SHLIB_PATH="$E/lib"
                          else SHLIB_PATH="$E/lib:SHLIB_PATH" ; fi
# the rationale here is that we set all known library search variables
# rather than attemp OS detection - more than one way to skin that cat

#
# conditionally add Regina Rexx lodable libraries to the search
for LD in /usr/opt/regina/lib /usr/opt/regina/lib64 \
          /usr/opt/oorexx/lib /usr/opt/oorexx/lib64 ; do
    if [ -d $LD ] ; then
        LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$LD"
        DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$LD"
        SHLIB_PATH="$SHLIB_PATH:$LD"
    fi
done
export LD_LIBRARY_PATH DYLD_LIBRARY_PATH SHLIB_PATH

#
# conditionally augment the command search path
if [ -d /usr/opt/regina/bin ] ; then PATH="$PATH:/usr/opt/regina/bin" ; fi
if [ -d /usr/opt/oorexx/bin ] ; then PATH="$PATH:/usr/opt/oorexx/bin" ; fi
export PATH

#
# run the sample program
echo "+ pipe ' < testfile.txt | rxstage | console '"
        pipe ' < testfile.txt | rxstage | console '
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

exit


