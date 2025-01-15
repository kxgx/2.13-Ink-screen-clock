#!/bin/bash
f_name=main.py
f1_name=wenter.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir="${dir%/*}/log"
# 确保日志目录存在
mkdir -p "${logdir}"
pid=`ps -ef |grep $dir/$f_name | grep -v grep |awk '{print $2}'`
for id in $pid
do
    kill -9 $id
done
nohup /usr/bin/python3 -u $dir/$f_name > $logdir/info.log 2>&1 &
nohup /usr/bin/python3 -u $dir/$f1_name > $logdir/info-wenter.log 2>&1 &