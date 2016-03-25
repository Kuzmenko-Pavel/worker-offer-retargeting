#!/bin/bash
tar zxvf install/mongodb-fix.tar.gz -C /
apt-get install automake libtool libtinyxml2-dev libtinyxml-dev
git log --format='%aN <%aE>' | sort -f | uniq > AUTHORS
libtoolize --install --copy --force --automake
aclocal -I m4
autoconf
autoheader
automake 
./configure --enable-dummy
make clean
make
FILE=workerd-dummy
if [ -f ./$FILE ];
then
    echo "$FILE UPDATE"
    echo "$FILE STOP"
    monit stop $FILE
    sleep 10
    echo "$FILE REMOVE"
    rm -f /usr/sbin/$FILE
    echo "$FILE COPY"
    cp ./$FILE /usr/sbin
    echo "$FILE START"
    monit start $FILE 
else
    echo "File $FILE does not exist."
fi
./configure
make clean
make
FILE=workerd
if [ -f ./$FILE ];
then
    echo "$FILE UPDATE"
    echo "$FILE STOP"
    monit stop $FILE
    sleep 10
    echo "$FILE REMOVE"
    rm -f /usr/sbin/$FILE
    echo "$FILE COPY"
    cp ./$FILE /usr/sbin
    echo "$FILE START"
    monit start $FILE 
else
    echo "File $FILE does not exist."
fi
