#!/bin/bash

# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

# 语言环境设置
DEFAULT_LANG="zh_CN.UTF-8"
export LANG=$DEFAULT_LANG
export LC_ALL=$DEFAULT_LANG
echo "$DEFAULT_LANG UTF-8" >> /etc/locale.gen
dpkg-reconfigure --frontend=noninteractive locales
locale-gen

# Debian版本相关命令
DEBIAN_VERSION=$(cat /etc/debian_version)

# 树莓派Raspberry Pi相关命令
RASPBERRY_PI_CPUINFO=$(grep -q 'Raspberry Pi' /proc/cpuinfo)

# Debian 11 (Bullseye) 相关命令
UPDATE_SOURCES_LIST_BULLSEYE="sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak && sudo cat <<'EOF' > /etc/apt/sources.list
deb https://mirrors.cernet.edu.cn/debian/ bullseye main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye main contrib non-free

deb https://mirrors.cernet.edu.cn/debian/ bullseye-updates main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye-updates main contrib non-free

deb https://mirrors.cernet.edu.cn/debian/ bullseye-backports main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian/ bullseye-backports main contrib non-free

deb https://mirrors.cernet.edu.cn/debian-security bullseye-security main contrib non-free
# deb-src https://mirrors.cernet.edu.cn/debian-security bullseye-security main contrib non-free
EOF"
INSTALL_PACKAGES_BULLSEYE="sudo apt-get update && sudo apt-get install -y git pigpio raspi-config netcat gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential"
INSTALL_PIP_PACKAGES_BULLSEYE="sudo pip3 install -i https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple spidev borax pillow requests --break-system-packages"
DOWNLOAD_AND_EXECUTE_BULLSEYE="wget https://cdn.pisugar.com/release/pisugar-power-manager.sh && bash pisugar-power-manager.sh -c release"
CLONE_AND_EXECUTE_BULLSEYE="cd ~ && git clone https://gitee.com/xingguangk/2.13-Ink-screen-clock.git && cd ~/2.13-Ink-screen-clock/bin/ && sudo chmod +x start.sh && sudo ./start.sh"

# Debian 12 (Bookworm) 相关命令
UPDATE_SOURCES_LIST_BOOKWORM="sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak && sudo cat <<'EOF' > /etc/apt/sources.list
deb https://mirrors.cernet.edu.cn/debian/ bookworm main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm main contrib non-free non-free-firmware

deb https://mirrors.cernet.edu.cn/debian/ bookworm-updates main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm-updates main contrib non-free non-free-firmware

deb https://mirrors.cernet.edu.cn/debian/ bookworm-backports main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian/ bookworm-backports main contrib non-free non-free-firmware

deb https://mirrors.cernet.edu.cn/debian-security bookworm-security main contrib non-free non-free-firmware
# deb-src https://mirrors.cernet.edu.cn/debian-security bookworm-security main contrib non-free non-free-firmware
EOF"
INSTALL_PACKAGES_BOOKWORM="sudo apt-get update && sudo apt-get install -y git pigpio raspi-config netcat gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential"
INSTALL_PIP_PACKAGES_BOOKWORM="sudo pip3 install -i https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple spidev borax pillow requests --break-system-packages"
DOWNLOAD_AND_EXECUTE_BOOKWORM="wget https://cdn.pisugar.com/release/pisugar-power-manager.sh && bash pisugar-power-manager.sh -c release"
CLONE_AND_EXECUTE_BOOKWORM="cd ~ && git clone https://gitee.com/xingguangk/2.13-Ink-screen-clock.git && cd ~/2.13-Ink-screen-clock/bin/ && sudo chmod +x start.sh && sudo ./start.sh"

# 主逻辑
# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
  echo "检测到Debian系统，版本号为: $DEBIAN_VERSION"

  # 检测是否是Raspberry Pi
  if $RASPBERRY_PI_CPUINFO; then
    echo "检测到Raspberry Pi系统。"

    # 根据Debian版本执行不同的命令
    case $DEBIAN_VERSION in
      *bullseye*)
        echo "执行Debian 11 (Bullseye) 相关操作..."
        eval $UPDATE_SOURCES_LIST_BULLSEYE
        eval $INSTALL_PACKAGES_BULLSEYE
        eval $INSTALL_PIP_PACKAGES_BULLSEYE
        eval $DOWNLOAD_AND_EXECUTE_BULLSEYE
        eval $CLONE_AND_EXECUTE_BULLSEYE
        ;;
      *bookworm*)
        echo "执行Debian 12 (Bookworm) 相关操作..."
        eval $UPDATE_SOURCES_LIST_BOOKWORM
        eval $INSTALL_PACKAGES_BOOKWORM
        eval $INSTALL_PIP_PACKAGES_BOOKWORM
        eval $DOWNLOAD_AND_EXECUTE_BOOKWORM
        eval $CLONE_AND_EXECUTE_BOOKWORM
        ;;
      *)
        echo "未知的Debian版本: $DEBIAN_VERSION"
        exit 1
        ;;
    esac
  else
    echo "这不是Raspberry Pi系统。"
    exit 0
  fi
else
  echo "这不是一个Debian系统。"
  exit 0
fi