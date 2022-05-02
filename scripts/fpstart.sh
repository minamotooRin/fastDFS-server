#!/bin/sh

pid_fpmoni=`pidof -x fpmoni.sh`
workdir=$(cd `dirname $0`; pwd)

if [ ! "$pid_fpmoni" ] ; then
    if [ ! -e  ${workdir}/log ] ; then
        mkdir -p  ${workdir}/log/bak/
        chmod 777  ${workdir}/log/bak
    fi

    nohup ${workdir}/fpmoni.sh >>  ${workdir}/log/process.log &

    echo "Filecacheproxy start"
fi
