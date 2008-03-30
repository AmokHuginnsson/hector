#! /bin/sh

export LD_LIBRARY_PATH=/home/amok/usr/lib
export HOME=/home/amok/

env > /tmp/cgi.log
echo "${@}" >> /tmp/cgi.log

while read LINE ; do
	echo "#${LINE}#" >> /tmp/cgi.log
done

exec ./build/hector.cgi/1exec "${@}"

