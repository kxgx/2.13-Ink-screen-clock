#!/bin/bash

f_name=clean.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}
screen_name="clean_screen"
service_name="raspi_e-Paper.service"
hold_time_seconds=10  # 设置维持时间，例如300秒（5分钟）

# 创建一个新的screen会话并运行Python脚本
screen -dmS $screen_name /usr/bin/python3 -u $dir/$f_name > $logdir/log/clean-info.log 2>&1

# 等待维持时间
echo "正在运行脚本，将在 ${hold_time_seconds} 秒后停止服务..."
sleep $hold_time_seconds

# 停止raspi_e-Paper服务
echo "正在停止$service_name服务..."
sudo systemctl stop $service_name

# 查找并结束所有与指定screen_name相关的screen会话
pids=$(screen -ls | grep $screen_name | awk -F '.' '{print $1}' | awk '{print $1}')
for pid in $pids; do
    screen -S $pid -X quit
done

echo "脚本运行完成，服务已停止，screen会话已结束。"
