#!/bin/bash

f_name=main.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}
screen_name="main_screen"

# 查找并结束所有与指定screen_name相关的screen会话
pids=$(screen -ls | grep $screen_name | awk -F '.' '{print $1}' | awk '{print $1}')
for pid in $pids; do
    screen -S $pid -X quit
done

# 创建一个新的screen会话并运行Python脚本
screen -dmS $screen_name /usr/bin/python3 -u $dir/$f_name > $logdir/log/info.log 2>&1