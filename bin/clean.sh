#!/bin/bash

f_name="clean.py"
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir="${dir}/log"
service_name="raspi_e-Paper.service"
hold_time_seconds=5  # 设置维持时间，例如300秒（5分钟）

# 停止raspi_e-Paper服务
echo "正在停止$service_name服务..."
sudo systemctl stop "$service_name"

# 确保日志目录存在
mkdir -p "${logdir}"

# 获取并终止所有与脚本相关的进程
pids=$(pgrep -f "$dir/$f_name")
if [ -n "$pids" ]; then
    kill -9 $pids
fi

# 使用nohup在后台运行Python脚本，并将输出重定向到日志文件
nohup /usr/bin/python3 -u "$dir/$f_name" > "${logdir}/info-clean.log" 2>&1 &

# 等待维持时间
echo "正在运行脚本，将在 ${hold_time_seconds} 秒后停止服务..."
sleep $hold_time_seconds
