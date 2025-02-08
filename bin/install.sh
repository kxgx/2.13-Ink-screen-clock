#!/bin/bash

# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

# 默认语言环境
DEFAULT_LANG="en_GB.UTF-8"
# 检查是否使用中国镜像源
USE_CN_MIRROR=false
# 控制调试输出
DEBUG=false
# 检查是否使用中国git仓库
USE_CN_GIT=false
# 检查是否安装pisugar-wifi-conf
USE_PISUGAR_WIFI_CONF=false
# 检查是否安装pisugar-power-manager
USE_PISUGAR_POWER_MANAGER=false

# 解析命令行参数
while [ "$#" -gt 0 ]; do
  case "$1" in
    --zh)
    DEFAULT_LANG="zh_CN.UTF-8" # 当使用 --zh 参数时，设置默认语言为中文
    ;;
    --cn)
    USE_CN_MIRROR=true
    ;;
    --gitcn)
    USE_CN_GIT=true
    ;;
    --pisugar-wifi-conf)
    USE_PISUGAR_WIFI_CONF=true
    ;;
    --pisugar-power-manager)
    USE_PISUGAR_POWER_MANAGER=true
    ;;
    --version)
      if [ -z "$2" ]; then
        echo "错误: --version 参数后需要跟版本号"
        exit 1
      fi
      VERSION="$2"  # 将版本号赋值给变量 VERSION
      shift 2  # 移动参数，跳过版本号参数及其值
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

# 获取当前语言环境变量
CURRENT_LANG=$(locale | grep LANG | sed 's/.*=//')

# 打印当前语言环境变量
echo "当前语言环境: $CURRENT_LANG"

# 设置语言环境
if [ "$CURRENT_LANG" != "$DEFAULT_LANG" ]; then
  echo "当前语言环境不是 $DEFAULT_LANG，将进行设置"

  # 设置新的语言环境变量
  export LANG=$DEFAULT_LANG
  export LC_ALL=$DEFAULT_LANG

  # 检查 locale.gen 中是否已存在 DEFAULT_LANG
  found=0
  while read -r line; do
    if [[ "$line" == "$DEFAULT_LANG UTF-8" ]]; then
      found=1
      break
    fi
  done < /etc/locale.gen

  if [ $found -eq 0 ]; then
    # 如果不存在，则添加到 locale.gen
    echo "$DEFAULT_LANG UTF-8" | sudo tee -a /etc/locale.gen
    # 确保 locale.gen 被正确更新
    if [ $? -ne 0 ]; then
      echo "无法更新 locale.gen 文件" >&2
      exit 1
    fi
  else
    echo "语言环境 $DEFAULT_LANG 已在 locale.gen 中设置，跳过添加"
  fi

  # 生成 locale
  if ! sudo locale-gen; then
    # 如果生成失败，打印错误消息并退出
    echo "生成 locale 失败" >&2
    exit 1
  fi

  # 更新 locale 配置
  if ! sudo update-locale LANG=$DEFAULT_LANG; then
    echo "更新 locale 配置失败" >&2
    exit 1
  fi

  echo "语言环境设置完成"
else
  echo "当前语言环境已经是 $DEFAULT_LANG，跳过设置"
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

# 使用中国镜像源
  DEBIAN_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian"
  DEBIAN_SECURITY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/debian-security"
  PIPY_MIRROR="https://mirrors.tuna.tsinghua.edu.cn/pypi/web/simple"
  # 使用中国镜像源时，Raspberry Pi 特定源链接保持不变
  RASPBERRY_PI_SOURCE_DEBIAN11="https://mirrors.tuna.tsinghua.edu.cn/raspberrypi"
  RASPBERRY_PI_SOURCE_DEBIAN12="https://mirrors.tuna.tsinghua.edu.cn/raspberrypi"


# 定义仓库链接变量
INK_SCREEN_CLOCK_REPO_URL="https://github.com/kxgx/2.13-Ink-screen-clock"
# 如果使用中国仓库，则更新链接变量
if [ "$USE_CN_GIT" = true ]; then
  INK_SCREEN_CLOCK_REPO_URL="https://gitee.com/xingguangk/2.13-Ink-screen-clock"
fi

# 更新源列表函数
update_sources_list() {
  local version=$1
  local debian_mirror_in_use=$(grep -oP 'deb\s+\K.+' /etc/apt/sources.list | head -1)
  local raspberry_pi_source_in_use=$(grep -oP 'deb\s+\K.+' /etc/apt/sources.list.d/raspi.list | head -1)

  # 检查并替换 Debian 源
  if [ "$USE_CN_MIRROR" = true ]; then
  if [ "$debian_mirror_in_use" != "$DEBIAN_MIRROR" ]; then
    sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
    {
    if [ "$version" = "bookworm" ]; then
      echo "deb $DEBIAN_MIRROR $version main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_MIRROR $version main contrib non-free non-free-firmware"
      echo "deb $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free non-free-firmware"
      echo "deb $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free non-free-firmware"
      echo "# deb-src $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free non-free-firmware"
    else
      echo "deb $DEBIAN_MIRROR $version main contrib non-free"
      echo "# deb-src $DEBIAN_MIRROR $version main contrib non-free"
      echo "deb $DEBIAN_MIRROR $version-updates main contrib non-free"
      echo "# deb-src $DEBIAN_MIRROR $version-updates main contrib non-free"
      echo "deb $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free"
      echo "# deb-src $DEBIAN_SECURITY_MIRROR $version-security main contrib non-free"
    fi
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
fi
}

# 安装包函数
install_packages() {
    echo "正在更新源列表"
  if ! sudo apt-get -q update; then
    echo "更新源列表失败" >&2
    exit 1
  fi
    echo "正在安装软件包"
  if ! sudo apt-get -q -y install git pigpio i2c-tools netcat* gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential screen; then
    echo "软件包安装失败" >&2
    exit 1
  fi
}

install_Ink-screen-clock() {
  # 检查墨水屏时钟仓库是否存在
  if [ ! -d "$HOME/2.13-Ink-screen-clock" ]; then
      echo "正在克隆仓库"
    cd ~
    if ! git clone -b $VERSION $INK_SCREEN_CLOCK_REPO_URL; then
      echo "克隆仓库失败" >&2
      exit 1
    fi
  else
    echo "仓库文件夹已存在，跳过克隆"
  fi
}

# 安装pip包函数
install_oline_pip_packages() {
    echo "正在安装pip软件包"
  if ! sudo pip3 install -i "$PIPY_MIRROR" -r "$HOME/2.13-Ink-screen-clock/bin/requirements.txt"; then
    echo "pip软件包安装失败，如果是最新版系统或是非lite系统" >&2
    echo "请手动运行sudo pip3 install -i "$PIPY_MIRROR" -r "$HOME/2.13-Ink-screen-clock/bin/requirements.txt" --break-system-packages" >&2
    exit 1
  fi
}

install_offline_pip_packages() {
    echo "正在安装pip软件包"
  if ! sudo pip3 install --no-index --find-links="$HOME/2.13-Ink-screen-clock/bin/vendor" -r "$HOME/2.13-Ink-screen-clock/bin/requirements.txt"; then
    echo "pip软件包安装失败，如果是最新版系统或是非lite系统" >&2
    echo "请手动运行sudo pip3 install --no-index --find-links="$HOME/2.13-Ink-screen-clock/bin/vendor" -r "$HOME/2.13-Ink-screen-clock/bin/requirements.txt" --break-system-packages" >&2
    exit 1
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
      echo "正在克隆仓库"
    cd ~
    if ! git clone -b $VERSION $INK_SCREEN_CLOCK_REPO_URL; then
      echo "克隆仓库失败" >&2
      exit 1
    fi
    # 设置start.sh和clean.sh脚本的执行权限
    chmod +x "$HOME/2.13-Ink-screen-clock/bin/start.sh"
    chmod +x "$HOME/2.13-Ink-screen-clock/bin/clean.sh"
  else
    echo "仓库文件夹已存在，跳过克隆"
  fi

  # 检查服务文件是否存在
  if [ -f "$service_file_path" ] && [ -f "$service1_file_path" ]; then
    # 检查服务是否已经启用
    if ! systemctl is-enabled $service_path &>/dev/null && ! systemctl is-enabled $service1_path &>/dev/null; then
        echo "正在复制服务文件并启用服务"
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

# 安装pisugar-power-manager函数
install_pisugar-power-manager() {
  if [ "$USE_PISUGAR_POWER_MANAGER" = true ]; then
    echo "正在安装pisugar-power-manager"
    if ! curl -sSL "$PISUGAR_POWER_MANAGER_URL" | sudo bash -s - -c release; then
      echo "pisugar-power-manager安装失败" >&2
      echo "如需要请手动运行curl -sSL $PISUGAR_POWER_MANAGER_URL | sudo bash -s - -c release" >&2
      exit 1
    fi
  fi
}

# 安装pisugar-wifi-conf函数
install_pisugar-wifi-conf() {
  if [ "$USE_PISUGAR_WIFI_CONF" = true ]; then
    echo "正在安装pisugar-wifi-conf"
    if ! curl -sSL "$PISUGAR_WIFI_CONF_URL" | sudo bash; then
      echo "pisugar-wifi-conf安装失败" >&2
      echo "如需要请手动运行curl $PISUGAR_WIFI_CONF_URL | sudo bash" >&2
      exit 1
    fi
  fi
}

# 主逻辑
# 检测是否是Debian系统
if [ -f /etc/debian_version ]; then
    echo "检测到Debian$DEBIAN_VERSION系统"

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
        install_Ink-screen-clock
        install_offline_pip_packages
        #install_oline_pip_packages
        setup_service
        #install_webui
        install_pisugar-wifi-conf
        install_pisugar-power-manager
        ;;
      12)
        echo "执行Debian 12 (Bookworm) 相关操作"
        update_sources_list "bookworm"
        install_packages
        install_Ink-screen-clock
        install_offline_pip_packages
        #install_oline_pip_packages
        setup_service
        #install_webui
        install_pisugar-wifi-conf
        install_pisugar-power-manager
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
