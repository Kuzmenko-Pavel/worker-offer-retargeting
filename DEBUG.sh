#!/bin/bash
tar zxvf install/mongodb-fix.tar.gz -C /
apt-get install automake libtool libtinyxml2-dev libtinyxml-dev
git log --oneline --decorate --color > ChangeLog
git log --format='%aN <%aE>' | sort -f | uniq > AUTHORS
libtoolize --install --copy --force --automake
aclocal -I m4
autoconf
autoheader
automake 
./configure --enable-debug
make clean
make
