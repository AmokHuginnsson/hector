#! /bin/sh

while read line ; do
	true
done

REQUEST_URI=`echo ${QUERY_STRING} | awk -F '=' '{print $NF}'`

echo "Location: https://${SERVER_ADDR}:8443/${REQUEST_URI}\n"

