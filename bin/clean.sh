#!/bin/bash

f_name=clean.py
dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
logdir=${dir%/*}
screen_name="clean_screen"
service_name="raspi_e-Paper.service"
timeout_seconds=10  # 设置超时时间，例如3600秒（1小时）

# 停止raspi_e-Paper服务
echo "正在停止$service_name服务..."
sudo systemctl stop $service_name

# 查找并结束所有与指定screen_name相关的screen会话
pids=$(screen -ls | grep $screen_name | awk -F '.' '{print $1}' | awk '{print $1}')
for pid in $pids; do
    screen -S $pid -X quit
done

# 创建一个新的screen会话并运行Python脚本，并设置超时时间
timeout $timeout_seconds screen -dmS $screen_name /usr/bin/python3 -u $dir/$f_name > $logdir/log/clean-info.log 2>&1

# 检查是否超时并处理
if [ $? -eq 124 ]; then
    echo "脚本运行超时，已终止"
else
    echo "脚本运行完成"
fi
