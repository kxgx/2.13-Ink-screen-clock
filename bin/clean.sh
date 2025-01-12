#!/bin/bash

f_name=clean.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}/log
service_name="raspi_e-Paper.service"
hold_time_seconds=3  # 设置维持时间，例如300秒（5分钟）

# 创建日志目录，如果它不存在的话
mkdir -p $logdir

# 停止服务
echo "正在停止$service_name服务..."
sudo systemctl stop $service_name

# 使用nohup运行Python脚本，并将输出重定向到日志文件
nohup /usr/bin/python3 -u $dir/$f_name > $logdir/clean-info.log 2>&1 &

# 获取nohup进程的PID
nohup_pid=$!

# 等待维持时间
echo "正在运行脚本，将在 ${hold_time_seconds} 秒后继续..."
sleep $hold_time_seconds

# 结束nohup进程
echo "正在结束nohup进程..."
kill $nohup_pid

echo "脚本运行完成，服务已停止，nohup进程已结束。"
