#!/bin/sh

#workdir=`pwd`
workdir=$(cd `dirname $0`; pwd)

while :
do
    stillRunning=$(ps -ef |grep "filecacheproxy/filecacheproxy" |grep -v "grep")
    if [ ! "$stillRunning" ] ; then
        timestamp=`date +"%Y/%m/%d %H:%M:%S"`
        echo "$timestamp   Filecacheproxy is not running, Restart it ....." >> ${workdir}/log/process.log
        cd ${workdir}
        nohup ${workdir}/filecacheproxy &
        cd ..
    fi
    sleep 10
done