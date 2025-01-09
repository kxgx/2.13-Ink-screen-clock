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
if grep -q 'Raspberry Pi' /proc/cpuinfo; then
  RASPBERRY_PI=true
else
  RASPBERRY_PI=false
fi

# 定义链接变量
DEBIAN_MIRROR="https://mirrors.cernet.edu.cn/debian/"
DEBIAN_SECURITY_MIRROR="https://mirrors.cernet.edu.cn/debian-security"
PI_SUGAR_POWER_MANAGER_URL="https://cdn.pisugar.com/release/pisugar-power-manager.sh"
INK_SCREEN_CLOCK_REPO_URL="https://gitee.com/xingguangk/2.13-Ink-screen-clock.git"
PIPY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple"

# 更新源列表函数
update_sources_list() {
  local version=$1
  sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
  sudo bash -c "cat <<EOF > /etc/apt/sources.list
deb $DEBIAN_MIRROR $version main contrib non-free
# deb-src $DEBIAN_MIRROR $version main contrib non-free

deb $DEBIAN_MIRROR $version-updates main contrib non-free
# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free

deb $DEBIAN_MIRROR $version-backports main contrib non-free
# deb-src $DEBIAN_MIRROR $version-backports main contrib non-free

deb $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free
# deb-src $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free
EOF"
}

# 安装包函数
install_packages() {
  sudo apt-get update && sudo apt-get install -y git pigpio raspi-config netcat gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential
}

# 安装pip包函数
install_pip_packages() {
  sudo pip3 install -i $PIPY_MIRROR spidev borax pillow requests --break-system-packages
}

# 下载并执行脚本函数
download_and_execute() {
  wget $PI_SUGAR_POWER_MANAGER_URL && bash pisugar-power-manager.sh -c release
}

# 克隆并执行脚本函数
clone_and_execute() {
  cd ~ && git clone $INK_SCREEN_CLOCK_REPO_URL && cd ~/2.13-Ink-screen-clock/bin/ && sudo chmod +x start.sh && sudo ./start.sh
}

# 主逻辑
# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
  echo "检测到Debian系统。"

  # 提取版本号的小数点前的部分
  MAJOR_VERSION=$(echo $DEBIAN_VERSION | cut -d '.' -f 1)

  # 检测是否是Raspberry Pi系统
  if [ "$RASPBERRY_PI" = true ]; then
    echo "检测到Raspberry Pi系统。"

    # 根据版本号的小数点前的部分执行不同的命令
    case "$MAJOR_VERSION" in
      11)
        echo "执行Debian 11 (Bullseye) 相关操作"
        update_sources_list "bullseye"
        install_packages
        install_pip_packages
        download_and_execute
        clone_and_execute
        ;;
      12)
        echo "执行Debian 12 (Bookworm) 相关操作"
        update_sources_list "bookworm"
        install_packages
        install_pip_packages
        download_and_execute
        clone_and_execute
        ;;
      *)
        echo "未知的Debian版本: $MAJOR_VERSION"
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