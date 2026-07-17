#!/bin/bash
# C版墨水屏时钟启动脚本

dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
proj_dir="${dir%/*}"
epd_bin="$proj_dir/clock/build/epd_clock"
weather_py="$dir/weather.py"

# 杀掉旧进程
pkill -f "epd_clock" 2>/dev/null
pkill -f "weather.py" 2>/dev/null

# 启动 C 时钟程序
sudo "$epd_bin" &

# 启动天气更新
/usr/bin/python3 -u "$weather_py" &
