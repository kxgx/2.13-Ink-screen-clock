#!/bin/bash

f_name="clean.py"
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir="${dir%/*}/log"
service_name="raspi_e-Paper.service"
 
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

echo "正在清除屏幕内容"
# 运行Python脚本，并将输出重定向到日志文件
/usr/bin/python3 -u "$dir/$f_name" > "${logdir}/info-clean.log" 2>&1 &
