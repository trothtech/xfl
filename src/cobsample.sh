#!/bin/sh
#
#         Name: cobsample.sh (shell script)
#               demonstrate POSIX Pipelines called from COBOL
#         Date: 2024-06-13 (Thu) or probably earlier
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
# try to be sure that the sample COBOL stage sits alongside other stages
if [ ! -x $E/libexec/xfl/cobstage ] ; then

#
# compile the sample COBOL stage
echo "+ cobc -c -x cobstage.cob"
        cobc -c -x cobstage.cob
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# be sure we have the object XFL library
make xfllib.o
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# link the sample COBOL stage with the XFL library
echo "+ cobc -x -o cobstage cobstage.o xfllib.o ..."
cobc -x -o cobstage cobstage.o xfllib.o xmitmsgx.o \
        -lpthread -lrt -lncurses -ldl \
        -L/usr/opt/gmp/lib -lgmp \
        -L/usr/opt/db/lib -ldb
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# copy the compiled COBOL stage into place
    echo "+ cp -p cobstage $E/libexec/xfl/."
            cp -p cobstage $E/libexec/xfl/.
                chmod a+rx $E/libexec/xfl/cobstage
fi

#
# run the sample program
echo "+ pipe ' < testfile.txt | cobstage | console '"
        pipe ' < testfile.txt | cobstage | console '
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

exit


