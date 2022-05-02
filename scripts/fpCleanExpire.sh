#!/bin/bash

function ReadConf() 
{
  CONF_FILE=$1; ITEM=$2
  value=`awk -F '=' '{a=1}a==1&&$1~/'$ITEM'/{print $2;exit}' $CONF_FILE`
  echo ${value}
}

function ProcessRecordFile()
{
    processfilename=$1
    resultfilename="${resultdir}/${processfilename}_result"
    
    if [ ! -e "${processfilename}" ]; then
        echo "${processfilename} not exist!"
        return
    fi
    
    while read fileid
    do
        fdfs_delete_file /etc/fdfs/client.conf $fileid > /dev/null 2>&1
        echo "$fileid $?" >> ${resultfilename}
    done  < ${processfilename}
    
    mv ${processfilename} ${resultdir}/
}



#workdir, resultdir, configfilename
workdir=$(cd `dirname $0`; pwd)"/records"
resultdir=${workdir}"/cleanresult"
if [ ! -d "${resultdir}" ]; then
    mkdir ${resultdir}
fi
configfilename=$(cd `dirname $0`; pwd)/filecacheproxy.conf

# read expiredays config item
expiredays=( $( ReadConf ${configfilename} expiredays ) )
if [ ${expiredays} \< 1 ]; then
    echo "config item expireday illegal! must bigger than 0"
    exit
fi

echo ${expiredays}

tmpdate=`date +%Y%m%d%H%M --date "${expiredays} days ago"`

tmpfilename="UploadRecord_${tmpdate}.txt"


cd ${workdir}

files=`ls ${workdir}`

for file in ${files}
do
    if [ ${file} \< ${tmpfilename} ] || [ ${file} = ${tmpfilename} ];then
        echo "process file ${file} begin ..."
        ProcessRecordFile ${file}
        echo "process file ${file} done"
    fi
done