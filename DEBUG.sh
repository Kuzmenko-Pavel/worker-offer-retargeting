#!/bin/bash
git log --date=short  --pretty=format:"%h - %ad, %an <%ae>, %ar : %s" > CHANGELOG
git log --format='%aN <%aE>' | sort -f | uniq > AUTHORS
libtoolize --install --copy --force --automake
aclocal -I m4 --install
autoconf
autoheader
automake --add-missing 
./configure --enable-debug
make clean
make
