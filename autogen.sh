#!/bin/sh

test -d m4 || mkdir m4
aclocal
autoheader
libtoolize
automake -Wall -Wno-override -a -c
autoconf
