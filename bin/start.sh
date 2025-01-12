#!/bin/bash

f_name=main.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}/log
screen_name="main_screen"

# 创建日志目录，如果它不存在的话
mkdir -p $logdir

# 结束与指定screen_name相关的所有screen会话（可选，如果您想要保留这个步骤）
pids=$(screen -ls | grep $screen_name | awk -F '.' '{print $1}' | awk '{print $1}')
for pid in $pids; do
    screen -S $pid -X quit
done

# 使用nohup运行Python脚本，并将输出重定向到日志文件
nohup /usr/bin/python3 -u $dir/$f_name > $logdir/info.log 2>&1 &
