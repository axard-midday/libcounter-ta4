#!/bin/sh

if [ -e "/usr/local/include/libcounter_ta4.h" ]
then
    rm -Rf "/usr/local/include/libcounter_ta4.h"
fi

cp ../bin/libcounter_ta4.h /usr/local/include/libcounter_ta4.h

if [ -e "/usr/local/lib/libcounter_ta4.so.0.0.3" ]
then
    rm -f "/usr/local/lib/libcounter_ta4.so.0.0.3"
fi

cp ../bin/libcounter_ta4.so.0.0.3 /usr/local/lib/libcounter_ta4.so.0.0.3

if [ -e "/usr/local/lib/libcounter_ta4.so.0.0" ]
then
    rm -f "/usr/local/lib/libcounter_ta4.so.0.0"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/local/lib/libcounter_ta4.so.0.0 

if [ -e "/usr/lib/libcounter_ta4.so.0.0" ]
then
    rm -f "/usr/lib/libcounter_ta4.so.0.0"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/lib/libcounter_ta4.so.0.0 

if [ -e "/usr/local/lib/libcounter_ta4.so.0" ]
then
    rm -f "/usr/local/lib/libcounter_ta4.so.0"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/local/lib/libcounter_ta4.so.0

if [ -e "/usr/lib/libcounter_ta4.so.0" ]
then
    rm -f "/usr/lib/libcounter_ta4.so.0"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/lib/libcounter_ta4.so.0

if [ -e "/usr/local/lib/libcounter_ta4.so" ]
then
    rm -f "/usr/local/lib/libcounter_ta4.so"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/local/lib/libcounter_ta4.so 

if [ -e "/usr/lib/libcounter_ta4.so" ]
then
    rm -f "/usr/lib/libcounter_ta4.so"
fi

ln -s /usr/local/lib/libcounter_ta4.so.0.0.3 /usr/lib/libcounter_ta4.so
