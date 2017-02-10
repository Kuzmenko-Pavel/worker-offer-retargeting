#!/bin/bash
#apt-get install automake libtool libtinyxml2-dev libtinyxml-dev
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
