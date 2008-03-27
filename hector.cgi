#! /bin/sh

export LD_LIBRARY_PATH=/home/amok/usr/lib
export HOME=/home/amok/

exec ./build/hector.cgi/1exec "${@}"

