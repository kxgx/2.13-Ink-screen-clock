#!/bin/bash

# C版墨水屏时钟安装脚本
# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

DEFAULT_LANG="en_GB.UTF-8"
USE_CN_MIRROR=false
USE_CN_GIT=false
USE_PISUGAR_WIFI_CONF=false
USE_PISUGAR_POWER_MANAGER=false
DEBUG=false

while [ "$#" -gt 0 ]; do
  case "$1" in
    --zh) DEFAULT_LANG="zh_CN.UTF-8" ;;
    --cn) USE_CN_MIRROR=true ;;
    --gitcn) USE_CN_GIT=true ;;
    --pisugar-wifi-conf) USE_PISUGAR_WIFI_CONF=true ;;
    --pisugar-power-manager) USE_PISUGAR_POWER_MANAGER=true ;;
    --version)
      if [ -z "$2" ]; then echo "错误: --version 需要跟版本号"; exit 1; fi
      VERSION="$2"; shift 2; continue
      ;;
    --debug) DEBUG=true ;;
    *) echo "未知参数: $1"; exit 1 ;;
  esac
  shift
done

# 语言环境设置
CURRENT_LANG=$(locale | grep LANG | sed 's/.*=//')
echo "当前语言环境: $CURRENT_LANG"

if [ "$CURRENT_LANG" != "$DEFAULT_LANG" ]; then
  echo "设置语言环境为 $DEFAULT_LANG"
  export LANG=$DEFAULT_LANG
  export LC_ALL=$DEFAULT_LANG

  if ! grep -q "$DEFAULT_LANG UTF-8" /etc/locale.gen; then
    echo "$DEFAULT_LANG UTF-8" | sudo tee -a /etc/locale.gen
  fi
  sudo locale-gen
  sudo update-locale LANG=$DEFAULT_LANG
fi

DEBIAN_VERSION=$(cat /etc/debian_version)

is_raspberry_pi() {
  grep -q 'Raspberry Pi' /proc/cpuinfo
}

# 镜像源
DEBIAN_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian"
DEBIAN_SECURITY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian-security"
RASPBERRY_PI_SOURCE="https://mirrors.tuna.tsinghua.edu.cn/raspberrypi"

# 仓库链接
INK_SCREEN_CLOCK_REPO_URL="https://github.com/kxgx/2.13-Ink-screen-clock"
if [ "$USE_CN_GIT" = true ]; then
  INK_SCREEN_CLOCK_REPO_URL="https://gitee.com/xingguangk/2.13-Ink-screen-clock"
fi

update_sources_list() {
  local version=$1
  if [ "$USE_CN_MIRROR" != true ]; then return; fi

  sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak 2>/dev/null
  if [ "$version" = "bookworm" ]; then
    cat <<EOF | sudo tee /etc/apt/sources.list > /dev/null
deb $DEBIAN_MIRROR $version main contrib non-free non-free-firmware
# deb-src $DEBIAN_MIRROR $version main contrib non-free non-free-firmware
deb $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware
# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware
deb $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free non-free-firmware
# deb-src $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free non-free-firmware
EOF
  else
    cat <<EOF | sudo tee /etc/apt/sources.list > /dev/null
deb $DEBIAN_MIRROR $version main contrib non-free
# deb-src $DEBIAN_MIRROR $version main contrib non-free
deb $DEBIAN_MIRROR $version-updates main contrib non-free
# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free
deb $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free
# deb-src $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free
EOF
  fi

  if [ -f /etc/apt/sources.list.d/raspi.list ]; then
    sudo cp /etc/apt/sources.list.d/raspi.list /etc/apt/sources.list.d/raspi.list.bak 2>/dev/null
  fi
  echo "deb $RASPBERRY_PI_SOURCE $version main" | sudo tee /etc/apt/sources.list.d/raspi.list > /dev/null
}

# 安装系统依赖（C版本只需要编译工具和运行时库）
install_packages() {
  echo "正在更新源列表"
  sudo apt-get -q update || { echo "更新源列表失败"; exit 1; }

  echo "正在安装软件包"
  sudo apt-get -q -y install \
    git pigpio i2c-tools netcat-openbsd gawk \
    python3 python3-pip \
    build-essential screen \
    || { echo "软件包安装失败"; exit 1; }
}

# 安装 weather.py 和农历的 Python 依赖
install_python_deps() {
  echo "安装 Python 依赖..."
  local pip_opts="-i https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple"
  sudo pip3 install $pip_opts requests borax 2>/dev/null \
    || sudo pip3 install requests borax --break-system-packages 2>/dev/null \
    || echo "警告: Python 依赖安装失败，天气/农历可能无法工作"
}

# 克隆仓库并编译C程序
install_Ink-screen-clock() {
  if [ ! -d "$HOME/2.13-Ink-screen-clock" ]; then
    echo "正在克隆仓库"
    cd ~
    git clone -b ${VERSION:-c-bindings} $INK_SCREEN_CLOCK_REPO_URL \
      || { echo "克隆仓库失败"; exit 1; }
  else
    echo "仓库已存在，更新代码"
    cd "$HOME/2.13-Ink-screen-clock"
    git pull
  fi

  # 编译 C 时钟程序
  echo "编译 C 时钟程序..."
  cd "$HOME/2.13-Ink-screen-clock/clock"
  make clean && make || { echo "编译失败"; exit 1; }
}

# 设置开机自启服务
setup_service() {
  local service_name="raspi_e-Paper.service"
  local service_file="$HOME/2.13-Ink-screen-clock/bin/$service_name"

  if [ ! -f "$service_file" ]; then
    echo "服务文件不存在: $service_file"
    exit 1
  fi

  if systemctl is-enabled $service_name &>/dev/null; then
    echo "服务已启用，跳过设置"
    return
  fi

  echo "正在设置开机自启服务"
  sudo cp "$service_file" /etc/systemd/system/
  sudo systemctl daemon-reload
  sudo systemctl enable $service_name
  sudo systemctl start $service_name
}

# PiSugar 相关（可选）
PISUGAR_POWER_MANAGER_URL=https://cdn.pisugar.com/release/pisugar-power-manager.sh
install_pisugar_power_manager() {
  if [ "$USE_PISUGAR_POWER_MANAGER" = true ]; then
    echo "正在安装 pisugar-power-manager"
    curl -sSL "$PISUGAR_POWER_MANAGER_URL" | sudo bash -s - -c release \
      || echo "pisugar-power-manager 安装失败，请手动安装"
  fi
}

PISUGAR_WIFI_CONF_URL=https://cdn.pisugar.com/PiSugar-wificonfig/script/install.sh
install_pisugar_wifi_conf() {
  if [ "$USE_PISUGAR_WIFI_CONF" = true ]; then
    echo "正在安装 pisugar-wifi-conf"
    curl -sSL "$PISUGAR_WIFI_CONF_URL" | sudo bash \
      || echo "pisugar-wifi-conf 安装失败，请手动安装"
  fi
}

# ===== 主逻辑 =====
if [ ! -f /etc/debian_version ]; then
  echo "这不是 Debian 系统"; exit 0
fi

echo "检测到 Debian $DEBIAN_VERSION 系统"
MAJOR_VERSION=$(echo $DEBIAN_VERSION | cut -d '.' -f 1)

if ! is_raspberry_pi; then
  echo "这不是 Raspberry Pi 系统"; exit 0
fi

echo "检测到 Raspberry Pi 系统"

case "$MAJOR_VERSION" in
  11)
    echo "Debian 11 (Bullseye)"
    update_sources_list "bullseye"
    ;;
  12)
    echo "Debian 12 (Bookworm)"
    update_sources_list "bookworm"
    ;;
  *)
    echo "未知 Debian 版本: $MAJOR_VERSION"; exit 1
    ;;
esac

install_packages
install_python_deps
install_pisugar_wifi_conf
install_pisugar_power_manager
install_Ink-screen-clock
setup_service

echo ""
echo "============================================"
echo "  C版墨水屏时钟安装完成！"
echo "  服务已启动: systemctl status raspi_e-Paper"
echo "  二进制路径: ~/2.13-Ink-screen-clock/clock/build/epd_clock"
echo "============================================"
