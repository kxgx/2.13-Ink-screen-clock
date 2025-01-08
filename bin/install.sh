#!/bin/bash

cat <<'EOF' > /etc/apt/sources.list
# 默认注释了源码镜像以提高 apt update 速度，如有需要可自行取消注释
deb https://mirrors.cernet.edu.cn/debian/ bullseye main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye main contrib non-free

deb https://mirrors.cernet.edu.cn/debian/ bullseye-updates main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye-updates main contrib non-free

deb https://mirrors.cernet.edu.cn/debian/ bullseye-backports main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye-backports main contrib non-free

# 以下安全更新软件源包含了官方源与镜像站配置，如有需要可自行修改注释切换
deb https://mirrors.cernet.edu.cn/debian-security bullseye-security main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian-security bullseye-security main contrib non-free

# deb https://security.debian.org/debian-security bullseye-security main contrib non-free
# # deb-src https://security.debian.org/debian-security bullseye-security main contrib non-free
EOF

sudo apt-get update
sudo apt install -y git
sudo apt-get install -y netcat*
sudo apt-get install -y python3-pip
sudo apt-get install -y python3-pil
sudo apt-get install -y python3-numpy
sudo apt install -y python3-gpiozero
sudo pip3 install spidev borax pillow requests --break-system-packages
