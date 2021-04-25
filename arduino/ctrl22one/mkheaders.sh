#!/bin/bash

# convert plan text html to C headers to include the respecitive file as
# "PROGMEM const char[]", i.e, we use the tool "xxd" for generating the
# data and add some preface


# figure signature variant (a): convenience for my very usecase
WEBSRC=websrc
WEBINC=headers
AVRBIN=avrfrm
AVRINC=headers
if [ "$#" -eq 0 ]; then
  if [ ! -d ${WEBSRC} ]; then
      echo "mkheaders: please run from ctrl22one base directory (should include \"./${WEBSRC}\")"
      exit
  fi
  if [ ! -d ${WEBINC} ]; then
    echo "mkheaders: please run from ctrl22one base directory (should include \"./${WEBINC}\")"
    exit
  fi
  if [ ! -d ${AVRBIN} ]; then
      echo "mkheaders: please run from base directory (should include \"./${AVRBIN}\")"
      exit
  fi
  if [ ! -d ${AVRINC} ]; then
      echo "mkheaders: please run from base directory (should include \"./${AVRINC}\")"
      exit
  fi
  $0 ./${WEBSRC}/* ./${WEBINC}  
  $0 ./${AVRBIN}/*.bin ./${AVRINC}  
  exit
fi


# figure signature: variant (b): ./mkheaders.sh FILE1 ... FILEn 
if [ "$#" -ge 1 ]; then
  for LSTARG in $@; do true; done
  if [ ! -d ${LSTARG} ]; then
    echo $0 $@ ./
    exit
  fi
fi


# break recursion on ./mkheaders.sh DSTDIR
if [ "$#" -eq 1 ]; then
  if [ -d $1 ]; then
    exit
  fi
  echo NEVER BE HERE ... FIX THIS SCRIPT
  exit
fi

# siganture: we are now nora;ised on "FILE1 .... DESTDIR"

# test file to exist
if [ ! -f $1 ]; then
    echo "cannot open input file \"$1\""
    echo "mkheaders usage: mkheaders.sh FILE1.html [FILE2.css] [FILE3.js] [...] [DESTDIR]"
    exit
fi


# figure suffix
SUFX=""
INP=$1
if [ "${INP: -5}" == ".html" ]; then
    SUFX=html
else
if [ "${INP: -4}" == ".css" ]; then
    SUFX=css
else
if [ "${INP: -3}" == ".js" ]; then
    SUFX=js
else
if [ "${INP: -4}" == ".svg" ]; then
    SUFX=svg
fi
if [ "${INP: -4}" == ".bin" ]; then
    SUFX=bin
fi
fi
fi
fi


# suffix not detected
if [ -z "$SUFX" ]; then
  echo "IGNORING file $1 with extension $SUFX"
  if [ $# -gt 1 ]; then
    shift    
    $0 $@
  fi
  exit
fi

# figure destination
for DST in $@; do true; done
if [ ! -d ${DST} ]; then
  echo "destination directory " $DST " does not exists"
  exit
fi

# disassemble path
SRC=$(dirname $INP)
BASE=$(basename $INP .${SUFX})

#figure destination
OUT=${DST}/${BASE}.${SUFX}.h


# run conversion
VAR=f_$(echo -ne ${BASE} | tr -Cs [:alnum:]  _)_${SUFX}
echo converting ${INP} to ${OUT}
cp ${INP} mkheaders_tmp
echo -ne "\0" >> mkheaders_tmp
echo "PROGMEM const char ${VAR}[] = {" > ${OUT}
xxd -i < mkheaders_tmp >> ${OUT} 
echo "};" >> ${OUT}
echo "unsigned int ${VAR}_len = $(wc -c < ${INP});" >> ${OUT}


# alternative (let xxd doit)
#echo -ne "PROGMEM const " > ${OUT}
#xxd -i ${INP} >> ${OUT} 

# process more files
if [ $# -gt 2 ]; then
    shift    
    ./mkheaders.sh $@
fi
