#!/bin/bash
libtoolize --install --copy --force --automake
aclocal -I m4
autoconf
autoheader
automake --add-missing
./configure
make clean
make
