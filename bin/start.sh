#!/bin/bash

# 脚本文件名
f_name=main.py
# 获取脚本所在目录的路径
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
# 日志目录，假设是脚本所在目录的上一级目录
logdir=${dir%/*}
# screen会话名称
screen_name="main_screen"

# 函数用于启动Python脚本
start_script() {
    # 创建一个新的screen会话并运行Python脚本
    # 输出重定向到日志文件
    screen -dmS $screen_name /usr/bin/python3 -u $dir/$f_name
}

# 无限循环，直到脚本被手动停止
while true; do
    # 检查是否有与指定screen_name相关的screen会话正在运行
    existing_sessions=$(screen -ls | grep -w $screen_name)

    # 如果找到正在运行的screen会话，则结束它们
    if [ -n "$existing_sessions" ]; then
        pids=$(screen -ls | grep $screen_name | awk -F '.' '{print $1}' | awk '{print $1}')
        for pid in $pids; do
            screen -S $pid -X quit
        done
    fi

    # 启动Python脚本
    start_script

    # 等待一段时间，检查脚本是否还在运行
    sleep 86400
    if ! screen -list | grep -q $screen_name; then
        echo "脚本意外停止。正在重启..."
    fi
done
