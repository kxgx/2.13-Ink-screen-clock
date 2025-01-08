#!/bin/bash

# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
    # 读取Debian版本号
    debian_version=$(cat /etc/debian_version)

    echo "Detected Debian system with version: $debian_version"

    # 检测是否是Raspberry Pi
    if grep -q 'Raspberry Pi' /proc/cpuinfo; then
        echo "This is a Raspberry Pi."
    # 根据版本号执行不同的操作
    case $debian_version in
        11*)
            echo "Debian 11 (Bullseye)"
            # 在这里执行针对Debian 11的操作
sudo cp /etc/apt/sources.list.d/raspi.list /etc/apt/sources.list.d/raspi.list.bak
sudo cat <<'EOF' > /etc/apt/sources.list.d/raspi.list
deb https://mirrors.cernet.edu.cn/raspberrypi/ bullseye main
EOF
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
sudo cat <<'EOF' > /etc/apt/sources.list
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
            ;;
        12*)
            echo "Debian 12 (Bookworm)"
            # 在这里执行针对Debian 12的操作
sudo cp /etc/apt/sources.list.d/raspi.list /etc/apt/sources.list.d/raspi.list.bak
sudo cat <<'EOF' > /etc/apt/sources.list.d/raspi.list
deb https://mirrors.cernet.edu.cn/raspberrypi/ bookworm main
EOF
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
sudo cat <<'EOF' > /etc/apt/sources.list
# 默认注释了源码镜像以提高 apt update 速度，如有需要可自行取消注释
deb https://mirrors.cernet.edu.cn/debian/ bookworm main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm main contrib non-free non-free-firmware

deb https://mirrors.cernet.edu.cn/debian/ bookworm-updates main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm-updates main contrib non-free non-free-firmware

deb https://mirrors.cernet.edu.cn/debian/ bookworm-backports main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm-backports main contrib non-free non-free-firmware

# 以下安全更新软件源包含了官方源与镜像站配置，如有需要可自行修改注释切换
deb https://mirrors.cernet.edu.cn/debian-security bookworm-security main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian-security bookworm-security main contrib non-free non-free-firmware

# deb https://security.debian.org/debian-security bookworm-security main contrib non-free non-free-firmware
# # deb-src https://security.debian.org/debian-security bookworm-security main contrib non-free non-free-firmware
EOF
sudo apt-get update
sudo apt install -y git
sudo apt-get install -y netcat*
sudo apt-get install -y python3-pip
sudo apt-get install -y python3-pil
sudo apt-get install -y python3-numpy
sudo apt install -y python3-gpiozero
sudo pip3 install spidev borax pillow requests --break-system-packages
            ;;
        *)
            echo "Unknown Debian version"
            # 在这里处理未知版本的情况
            exit 0
            ;;
    esac
else
    echo "This is not a Debian system."
    # 在这里处理非Debian系统的情况
    exit 0
fi
