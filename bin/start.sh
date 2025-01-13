#!/bin/bash

f_name=main.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}

# 函数：杀死所有运行的main.py进程
kill_process() {
    pid=$(ps -ef | grep "$dir/$f_name" | grep -v grep | awk '{print $2}')
    if [ ! -z "$pid" ]; then
        for id in $pid; do
            kill -9 $id
        done
    fi
}

# 函数：启动main.py
start_process() {
    nohup /usr/bin/python3 -u "$dir/$f_name" > "$logdir/log/info.log" 2>&1 &
}

# 杀死所有已存在的main.py进程
kill_process

# 启动main.py
start_process

# 无限循环，检查main.py是否在运行
while true; do
    sleep 180  # 每180秒检查一次
    pid=$(ps -ef | grep "$dir/$f_name" | grep -v grep | awk '{print $2}')
    if [ -z "$pid" ]; then
        echo "main.py is not running, restarting..."
        start_process
    fi
done