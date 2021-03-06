#! /bin/sh

LOG="/tmp/ssl-redirect.log"

echo "PARAMS: ${@}" > ${LOG}
echo "QUERY_STRING: ${QUERY_STRING}" >> ${LOG}
env >> ${LOG}

if [ "x${REQUEST_METHOD}" = "xPOST" ] ; then
	echo "POST Method" >> ${LOG}
	while read line ; do
		echo "BODY: ${line}" >> ${LOG}
	done
	PARAM=`echo ${HTTP_REFERER} | awk -F '?' '{print "?"$NF}'`
else
	echo "GET Method" >> ${LOG}
fi

REQUEST_URI=`echo ${QUERY_STRING} | sed -s 's/REQUEST_URI\=//'`

echo "Status: 307"
echo "Location: https://${SERVER_ADDR}:8443/${REQUEST_URI}${PARAM}\n"
echo "Location: https://${SERVER_ADDR}:8443/${REQUEST_URI}${PARAM}\n" >> ${LOG}

