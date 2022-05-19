#!/bin/ksh

workdir=$(cd `dirname $0`; pwd)

if [ ! -e log ] ; then
    mkdir -p ${workdir}/log/bak/
    chmod 777 ${workdir}/log/bak
fi


pid_fpmoni=`pidof -x fpmoni.sh`
pid_filecacheproxy=`pidof filecacheproxy`


if [ "$pid_fpmoni" ] ; then
    kill $pid_fpmoni
    timestamp=`date +"%Y/%m/%d %H:%M:%S"`
    echo "fpmoni.sh killed, pid[$pid_fpmoni]"
    echo "$timestamp   fpmoni.sh killed by fpstop.sh"   >> $workdir/log/process.log
fi


if [ "$pid_filecacheproxy" ] ; then
    kill $pid_filecacheproxy
    timestamp=`date +"%Y/%m/%d %H:%M:%S"`
    echo "filecacheproxy killed, pid[$pid_filecacheproxy]"
    echo "$timestamp   filecacheproxy killed by fpstop.sh"  >>$workdir/log/process.log
fi