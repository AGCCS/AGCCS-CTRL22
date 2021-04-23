#!/bin/bash

# copy the most recently compiled binaries to the firmware server
# i.e., we need to extract version and board ids from the configuration,
# pick the binaries from the build and rename it according to our naming
# convention, i.e.
#
# * demesh binaries are named "demesh_<MAJ>_<MIN>_<BRD>.bin", e.g.
#   "demesh_3_4_stick5.bin" for version 3.4 on an M5StickC board
#
# * Ctrl22C binaries are named "ctrl22c_<MAJ>_<MIN>.bin, e.g.
#   "ctrl22c_1_2.bin" for version 1.2,
#

# firmware server location (using scp, i.e. incl login)
DST="pi@192.168.2.108:~/mupgrade"


# try to figure where we are
BASE=.
if [ ! -d $BASE/demesh ]; then
    BASE=..
fi    
if [ ! -d $BASE/demesh ]; then
   echo this script needs to run within demesh or one directory down
   exit;
fi    
    

################################################
# part 1: esp32 firmware

# file to upload
BIN=$BASE/demesh/build/demesh.bin

# check for file
if [ ! -f $BIN ]; then
   echo binary $BIN not found
   exit;
fi

# find version
SRC=$BASE/demesh/main/demesh.c
VERSION_LINE=$(grep "#define DEMESH_VERSION" $SRC)
VERSION_STRING=$(echo $VERSION_LINE | grep -o '".*"')
VERSION_SUFFIX=$(echo $VERSION_STRING  | sed 's/\./_/')
VERSION_SUFFIX=$(echo $VERSION_SUFFIX  | sed 's/\"// g')
if [[ $VERSION_SUFFIX == "" ]] ; then
    echo no version ?
    exit;
fi
    

echo "======" found dmesh version $VERSION_STRING i.e. $VERSION_SUFFIX

# find board
CFG=$BASE/demesh/sdkconfig
if grep -q "BOARD_NOPE=y" $CFG ; then BOARD=nope; fi
if grep -q "BOARD_M5=y" $CFG ; then BOARD=m5stick; fi
if grep -q "BOARD_GPIO2=y" $CFG ; then BOARD=gpio2; fi
if grep -q "BOARD_FGCCS_1_0=y" $CFG ; then BOARD=fgccs10; fi
if grep -q "BOARD_FGCCS_1_2=y" $CFG ; then BOARD=fgccs12; fi
if [[ $BOARD == "" ]] ; then
    echo no/unknown board?
    exit;
fi
echo "======" found target board $BOARD

# copy new binary
FILE=demesh_${BOARD}_${VERSION_SUFFIX}.bin
echo scp $BIN $DST/$FILE
scp -q $BIN $DST/$FILE 


################################################
# part 2: avr firmware

# file to upload
BIN=$BASE/ctrl22c/ctrl22c.bin

# make sure we are up to date
rm -f $BIN > /dev/null
CDIR=$(pwd)
cd $BASE/ctrl22c
make  > /dev/null
cd $CDIR
if [ ! -f $BIN ]; then
   echo binary $BIN not found
   exit;
fi

# find version
SRC=$BASE/ctrl22c/ctrl22c.c
VERSION_LINE=$(grep "#define CTRL22C_VERSION" $SRC)
VERSION_NUMBER=$(echo $VERSION_LINE | grep -o '\<.[0-9]')
VERSION_SUFFIX=$(echo $VERSION_NUMBER  | sed 's/\([1-9]\)$/_\1/')
if [[ $VERSION_SUFFIX == "" ]] ; then
    echo no version ?
    exit;
fi
    
echo "======" found ctrl22c version $VERSION_NUMBER i.e. $VERSION_SUFFIX

# copy new binary
FILE=ctrl22c_${VERSION_SUFFIX}.bin
echo scp $BIN $DST/$FILE
scp -q $BIN $DST/$FILE 


################################################
# part 3: utilities

echo "======" uploading utilities

# copy utils
echo scp $BASE/utils/*.py  $DST/
scp -q $BASE/utils/*.py  $DST/



