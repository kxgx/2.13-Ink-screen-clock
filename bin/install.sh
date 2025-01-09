#!/bin/bash

# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

# 默认语言环境
DEFAULT_LANG="en_US.UTF-8"
# 检查是否使用中国镜像源
USE_CN_MIRROR=false
# 控制调试输出
DEBUG=false

# 解析命令行参数
while [ "$#" -gt 0 ]; do
  case "$1" in
    --cn)
    USE_CN_MIRROR=true
    DEFAULT_LANG="zh_CN.UTF-8" # 当使用 --cn 参数时，设置默认语言为中文
    ;;
    --debug)
    DEBUG=true
    ;;
    *)
    echo "未知参数: $1"
    exit 1
    ;;
  esac
  shift
done

# 设置语言环境
export LANG=$DEFAULT_LANG
export LC_ALL=$DEFAULT_LANG

# 如果 locale.gen 中不存在，则添加到 locale.gen
if ! grep -q "^$DEFAULT_LANG UTF-8" /etc/locale.gen; then
  echo "$DEFAULT_LANG UTF-8" >> /etc/locale.gen
fi

# 生成 locale
locale-gen

# 重新配置 locales
dpkg-reconfigure --frontend=noninteractive locales

# 如果需要调试输出
if [ "$DEBUG" = true ]; then
  echo "语言设置为: $DEFAULT_LANG"
  echo "使用中国镜像源: $USE_CN_MIRROR"
fi

# Debian版本相关命令
DEBIAN_VERSION=$(cat /etc/debian_version)

# 检查是否是Raspberry Pi系统
is_raspberry_pi() {
  if grep -q 'Raspberry Pi' /proc/cpuinfo; then
    return 0
  else
    return 1
  fi
}

# 定义链接变量
DEBIAN_MIRROR="http://deb.debian.org/debian/"
DEBIAN_SECURITY_MIRROR="http://security.debian.org/"
PI_SUGAR_POWER_MANAGER_URL="https://cdn.pisugar.com/release/pisugar-power-manager.sh"
INK_SCREEN_CLOCK_REPO_URL="https://gitee.com/xingguangk/2.13-Ink-screen-clock.git"
PIPY_MIRROR="https://pypi.org/simple"

# 如果使用中国镜像源，则更新链接变量
if [ "$USE_CN_MIRROR" = true ]; then
  DEBIAN_MIRROR="https://mirrors.cernet.edu.cn/debian/"
  DEBIAN_SECURITY_MIRROR="https://mirrors.cernet.edu.cn/debian-security"
  PIPY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple"
fi

# 更新源列表函数
update_sources_list() {
  local version=$1
  if [ "$DEBUG" = true ]; then
    echo "正在更新版本 $version 的源列表"
  fi
  sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
  sudo bash -c "cat > /etc/apt/sources.list <<EOF
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
  if [ "$DEBUG" = true ]; then
    echo "正在安装软件包..."
  fi
  sudo apt-get update && sudo apt-get install -y git pigpio raspi-config netcat gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential
}

# 安装pip包函数
install_pip_packages() {
  if [ "$DEBUG" = true ]; then
    echo "正在安装pip软件包..."
  fi
  sudo pip3 install -i $PIPY_MIRROR spidev borax pillow requests
}

# 下载并执行脚本函数
download_and_execute() {
  if [ "$DEBUG" = true ]; then
    echo "正在下载并执行pisugar电源管理脚本..."
  fi
  wget $PI_SUGAR_POWER_MANAGER_URL && bash pisugar-power-manager.sh -c release
}

# 克隆并执行脚本函数
clone_and_execute() {
  if [ "$DEBUG" = true ]; then
    echo "正在克隆并执行墨水屏时钟仓库..."
  fi
  cd ~
  git clone $INK_SCREEN_CLOCK_REPO_URL
  if [ $? -eq 0 ]; then
    cd ~/2.13-Ink-screen-clock/bin/
    sudo chmod +x start.sh
    sudo ./start.sh
  else
    echo "克隆墨水屏时钟仓库失败"
    exit 1
  fi
}

# 主逻辑
# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
  echo "检测到Debian系统"

  # 提取版本号的小数点前的部分
  MAJOR_VERSION=$(echo $DEBIAN_VERSION | cut -d '.' -f 1)

  # 检测是否是Raspberry Pi系统
  if is_raspberry_pi; then
    echo "检测到Raspberry Pi系统"

    # 根据版本号的小数点前的部分执行不同的命令
    case "$MAJOR_VERSION" in
      11)
        echo "执行Debian 11 (Bullseye) 相关操作"
        update_sources_list "bullseye"
        install_packages
        install_pip_packages
        #download_and_execute
        clone_and_execute
        ;;
      12)
        echo "执行Debian 12 (Bookworm) 相关操作"
        update_sources_list "bookworm"
        install_packages
        install_pip_packages
        #download_and_execute
        clone_and_execute
        ;;
      *)
        echo "未知的Debian版本: $MAJOR_VERSION"
        exit 1
        ;;
    esac
  else
    echo "这不是Raspberry Pi系统"
    exit 0
  fi
else
  echo "这不是Debian系统"
  exit 0
fi
