#!/bin/bash

# get ports
PORTS=$(ls /dev/cu.usbserial-* /dev/cu.SLAB_*)

# convert to array
PORTS=( $PORTS )

# show to user
echo $PORTS

# ask
read -p "choose: " SELECT;

# default
if [ "$SELECT" = "" ] ; then SELECT=1; fi 

# choose
PORT=${PORTS[${SELECT}-1]}

# default
if [ "$PORT" = "" ] ; then echo error; exit -1 ; fi 

# doit 
echo ln -s $PORT ./usb-link 
rm -rf ./usb-link
ln -s $PORT ./usb-link 
