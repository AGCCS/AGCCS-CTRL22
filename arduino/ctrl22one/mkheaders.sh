#!/bin/bash

# convert plan text html to C headers to include the respecitive file as
# "PROGMEM const char[]", i.e, we use the tool "xxd" for generating the
# data and add some preface

# say hello
# echo mkheaders $1

# configure source and destination 
WEBSRC=websrc
WEBINC=webinc

# test directories to exist
if [ ! -d ${WEBSRC} ]; then
    echo "mkheaders: please run from base directory (must include \"./${WEBSRC}\")"
    exit
fi
if [ ! -d ${WEBINC} ]; then
    echo "mkheaders: please run from base directory (must include \"./${WEBINC}\")"
    exit
fi

# default argument
if [ -z "$1" ]; then
  ./mkheaders.sh websrc/*
  exit
fi  

# test file to exist
if [ ! -f $1 ]; then
    echo "cannot open input file \"$1\""
    echo "mkheaders usage: mkheaders.sh FILE1.html [FILE2.css] [FILE3.js] [...]"
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
fi
fi
fi


# suffix not detected
if [ -z "$SUFX" ]; then
  echo "IGNORING file $1 with extension $SUFX"
  if [ $# -gt 1 ]; then
    shift    
   ./mkheaders.sh $@
  fi
  exit
fi

# disassemble path
SRC=$(dirname $INP)
BASE=$(basename $INP .${SUFX})

#figure destination
DST=${SRC}/../${WEBINC}
OUT=${DST}/${BASE}.${SUFX}.h

# test destination directory to exist
if [ ! -d ${DST} ]; then
    echo "destination directory " $DST " does not exists"
    exit
fi

# run conversion
VAR=f_$(echo -ne ${BASE} | tr -Cs [:alnum:]  _)_${SUFX}
echo converting ${INP} to ${OUT}
cp ${INP} mkheaders_tmp
echo -ne "\0" >> mkheaders_tmp
echo "PROGMEM const char ${VAR}[] = {" > ${OUT}
xxd -i < mkheaders_tmp >> ${OUT} 
echo "};" >> ${OUT}

# alternative (let xxd doit)
#echo -ne "PROGMEM const " > ${OUT}
#xxd -i ${INP} >> ${OUT} 

# process more files
if [ $# -gt 1 ]; then
    shift    
    ./mkheaders.sh $@
fi
