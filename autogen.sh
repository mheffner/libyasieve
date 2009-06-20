#! /bin/bash

#export CFLAGS=""

${AUTORECONF:-autoreconf} -i --force -Wno-portability

if [ $? != 0 ]; then
    echo
    echo "ERROR: ${AUTORECONF:-autoreconf} failed to run. Check logs and try again."
    echo
    exit 1;
fi

if test -z "$*"; then
    echo "I am going to run ./configure with no arguments - if you wish "
    echo "to pass any to it, please specify them on the $0 command line."
fi

./configure "$@" --disable-static

echo
echo "Now type 'make' to compile."
echo
