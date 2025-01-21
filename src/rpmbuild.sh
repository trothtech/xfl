#!/bin/sh
#
#         Name: rpmbuild.sh (shell script)
#               build an RPM from the "spec file"
#         Date: 2024-02-23 (Fri), 2024-10-30 (Wed)
#

#
# run from the resident directory
cd `dirname "$0"`

#
# establish certain variables
APPLID="$1"             # argument $1 is the project name (package name)
if [ -z "$APPLID" ] ; then echo "missing APPLID - you're doing it wrong, drive this from 'make'" ; exit 1 ; fi
VERSION="$2"            # argument $2 is the version
if [ -z "$VERSION" ] ; then echo "missing VERSION - you're doing it wrong, drive this from 'make'" ; exit 1 ; fi
PREFIX="/usr"

#
# derive a proper platform identifier
UNAMEM=`uname -m`
UNAMEM=`uname -m | sed 's#^i.86$#i386#' | sed 's#^armv.l$#arm#'`

#
# get a proper release number
if [ ! -s .rpmseq ] ; then echo "1" > .rpmseq ; fi
RELEASE=`cat .rpmseq`

STAGING=`pwd`/rpmbuild.d

#
# clean up from any prior run
make clean 1> /dev/null 2> /dev/null
rm -rf $STAGING

#
# configure the package normally
./configure
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
# build the package normally
make
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# override the PREFIX and run the install step
make PREFIX=$STAGING install
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# create the "sed file"
rm -f rpm.spec.sed
echo "s#%SPEC_PREFIX%#$PREFIX#g" >> rpm.spec.sed
echo "s#%SPEC_APPLID%#$APPLID#g" >> rpm.spec.sed
echo "s#%SPEC_VERSION%#$VERSION#g" >> rpm.spec.sed
echo "s#%SPEC_RELEASE%#$RELEASE#g" >> rpm.spec.sed
echo "s#%SPEC_UNAMEM%#$UNAMEM#g" >> rpm.spec.sed
echo "s#%SPEC_STAGING%#$STAGING#g" >> rpm.spec.sed

#
# process the skeletal spec file into a usable spec file
sed -f rpm.spec.sed < $APPLID.spec.in > $APPLID.spec
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
rm rpm.spec.sed

#
# make it "properly rooted"
mkdir $STAGING/usr
mv $STAGING/bin $STAGING/sbin $STAGING/lib \
   $STAGING/include $STAGING/share $STAGING/libexec \
   $STAGING/usr/.
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# build the RPM file (and keep a log of the process)
rm -f $APPLID.rpm.log
echo "+ rpmbuild -bb --nodeps $APPLID.spec"
        rpmbuild -bb --nodeps $APPLID.spec 2>&1 | tee $APPLID.rpm.log
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
#rm $APPLID.spec

#
# recover the  resulting package file ... yay!
cp -p $HOME/rpmbuild/RPMS/$UNAMEM/$APPLID-$VERSION-$RELEASE.$UNAMEM.rpm .
#                          UNAMEM  APPLID- VERSION- RELEASE. UNAMEM
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
cp -p $APPLID-$VERSION-$RELEASE.$UNAMEM.rpm $APPLID.rpm

#
# remove temporary build directory
rm -rf $STAGING

# increment the sequence number for the next build
expr $RELEASE + 1 > .rpmseq

exit


