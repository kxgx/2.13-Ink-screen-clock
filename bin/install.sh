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
    --zh)
    DEFAULT_LANG="zh_CN.UTF-8" # 当使用 --zh 参数时，设置默认语言为中文
    ;;
    --cn)
    USE_CN_MIRROR=true
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
  echo "$DEFAULT_LANG UTF-8" | sudo tee -a /etc/locale.gen
fi

# 生成 locale
if ! sudo locale-gen; then
  echo "生成 locale 失败" >&2
  exit 1
fi

# 重新配置 locales
if ! sudo dpkg-reconfigure --frontend=noninteractive locales; then
  echo "重新配置 locales 失败" >&2
  exit 1
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
INK_SCREEN_CLOCK_REPO_URL="https://github.com/kxgx/2.13-Ink-screen-clock.git"
PIPY_MIRROR="https://pypi.org/simple"
# 修改 Raspberry Pi 特定源链接
RASPBERRY_PI_SOURCE_DEBIAN11="https://archive.raspberrypi.org/debian/"
RASPBERRY_PI_SOURCE_DEBIAN12="https://archive.raspberrypi.com/debian/"

# 如果使用中国镜像源，则更新链接变量
if [ "$USE_CN_MIRROR" = true ]; then
  DEBIAN_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian/"
  DEBIAN_SECURITY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian-security"
  INK_SCREEN_CLOCK_REPO_URL="https://gitee.com/xingguangk/2.13-Ink-screen-clock.git"
  PIPY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple"
  # 使用中国镜像源时，Raspberry Pi 特定源链接保持不变
  RASPBERRY_PI_SOURCE_DEBIAN11="https://mirrors.tuna.tsinghua.edu.cn/raspberrypi/"
  RASPBERRY_PI_SOURCE_DEBIAN12="https://mirrors.tuna.tsinghua.edu.cn/raspberrypi/"
fi

# 更新源列表函数
update_sources_list() {
  local version=$1
  local debian_mirror_in_use=$(grep -oP 'deb\s+\K.+' /etc/apt/sources.list | head -1)
  local raspberry_pi_source_in_use=$(grep -oP 'deb\s+\K.+' /etc/apt/sources.list.d/raspi.list | head -1)

  # 检查并替换 Debian 源
  if [ "$debian_mirror_in_use" != "$DEBIAN_MIRROR" ]; then
    sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
    {
      echo "deb $DEBIAN_MIRROR $version main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_MIRROR $version main contrib non-free non-free-firmware"
      echo "deb $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware"
      echo "deb $DEBIAN_SECURITY_MIRROR $version-security bookworm-security main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_SECURITY_MIRROR $version-security bookworm-security main contrib non-free non-free-firmware"
    } | sudo tee /etc/apt/sources.list > /dev/null
  else
    echo "Debian 源链接已更新，跳过替换" >&2
  fi

  # 检查并替换 Raspberry Pi 特定源
  if [ "$version" == "bullseye" ]; then
    RASPBERRY_PI_SOURCE=$RASPBERRY_PI_SOURCE_DEBIAN11
  elif [ "$version" == "bookworm" ]; then
    RASPBERRY_PI_SOURCE=$RASPBERRY_PI_SOURCE_DEBIAN12
  else
    echo "未知的Debian版本: $version" >&2
    exit 1
  fi

  if [ "$raspberry_pi_source_in_use" != "$RASPBERRY_PI_SOURCE$version" ]; then
    if [ -f "/etc/apt/sources.list.d/raspi.list" ]; then
      sudo cp /etc/apt/sources.list.d/raspi.list /etc/apt/sources.list.d/raspi.list.bak
    fi
    echo "deb $RASPBERRY_PI_SOURCE $version main" | sudo tee /etc/apt/sources.list.d/raspi.list > /dev/null
  else
    echo "Raspberry Pi 源链接已更新，跳过替换" >&2
  fi
}

# 安装包函数
install_packages() {
  if ! sudo apt-get update; then
    echo "更新源列表失败" >&2
    exit 1
  fi
  if ! sudo apt-get upgrade -y; then
    echo "系统更新失败" >&2
    exit 1
  fi
  if ! sudo apt-get install -y git pigpio raspi-config netcat gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential screen; then
    echo "软件包安装失败" >&2
    exit 1
  fi
}

# 安装pip包函数
install_pip_packages() {
  if ! sudo pip3 install -i "$PIPY_MIRROR" spidev borax pillow requests; then
    echo "pip软件包安装包失败" >&2
  fi
}

# 复制服务文件并设置为开机启动
setup_service() {
  local service_path="raspi_e-Paper.service"
  local service1_path="e-Paper_clean.service"
  local service_file_path="$HOME/2.13-Ink-screen-clock/bin/$service_path"
  local service1_file_path="$HOME/2.13-Ink-screen-clock/bin/$service1_path"

  # 检查墨水屏时钟仓库是否存在
  if [ ! -d "$HOME/2.13-Ink-screen-clock" ]; then
    cd ~
    if ! git clone $INK_SCREEN_CLOCK_REPO_URL; then
      echo "克隆墨水屏时钟仓库失败" >&2
      exit 1
    fi
    # 设置start.sh和clean.sh脚本的执行权限
    chmod +x "$HOME/2.13-Ink-screen-clock/bin/start.sh"
    chmod +x "$HOME/2.13-Ink-screen-clock/bin/clean.sh"
  else
    echo "墨水屏时钟仓库文件夹已存在，跳过克隆"
  fi

  # 检查服务文件是否存在
  if [ -f "$service_file_path" ] && [ -f "$service1_file_path" ]; then
    # 检查服务是否已经启用
    if ! systemctl is-enabled $service_path &>/dev/null && ! systemctl is-enabled $service1_path &>/dev/null; then
      # 复制服务文件到 systemd 目录
      if sudo cp "$service_file_path" /etc/systemd/system/ && sudo cp "$service1_file_path" /etc/systemd/system/; then
        # 重载 systemd 管理器配置
        sudo systemctl daemon-reload
        # 启动服务
        sudo systemctl enable $service_path
        sudo systemctl enable $service1_path
        sudo systemctl start $service_path
      else
        echo "复制服务文件失败" >&2
        exit 1
      fi
    else
      echo "服务文件已存在并启用，跳过复制"
    fi
  else
    echo "服务文件不存在于路径: $service_file_path 或 $service1_file_path" >&2
    exit 1
  fi
}

# 主逻辑
# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
  echo "检测到Debian系统"

  # 提取版本号的小数点前的部分
  if ! MAJOR_VERSION=$(echo $DEBIAN_VERSION | cut -d '.' -f 1); then
    echo "无法提取Debian版本号" >&2
    exit 1
  fi

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
        setup_service
        ;;
      12)
        echo "执行Debian 12 (Bookworm) 相关操作"
        update_sources_list "bookworm"
        install_packages
        install_pip_packages
        setup_service
        ;;
      *)
        echo "未知的Debian版本: $MAJOR_VERSION" >&2
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
