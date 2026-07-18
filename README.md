# 墨水屏展示当前时间及天气数据 (C 语言版)

本项目复刻自[Seek-Huang](https://github.com/Seek-Huang)的[代码仓库](https://github.com/Seek-Huang/2.13-Ink-screen-clock)，已将 Python 版移植为纯 C 语言实现。

## C 版本特性

- **零 Python 依赖** — 核心程序为原生 ARM 二进制，无需 PIL/Pillow 等 Python 包
- **内置字体渲染** — 使用 stb_truetype 单头文件库，零外部字体依赖
- **启动更快** — 编译后直接运行，无 Python 解释器开销
- **自动清屏** — 程序退出时通过信号处理自动清除屏幕
- **天气更新** — 保留 weather.py 获取天气数据（需 requests）
- **农历显示** — 通过 Python borax 库计算农历日期

## API 说明

#### 天气 API
使用 [wttr.in](https://wttr.in) 免费天气接口，自动获取天气数据并保存为 JSON 文件。

### 外壳及电池模块
使用 PiSugar3 外壳，获取电源及树莓派 RTC 时间都靠此模块：

     https://github.com/PiSugar/PiSugar/wiki/PiSugar-3-Series#rtc-on-board
     https://www.pisugar.com/

> 如果没有使用此模块则需要更改代码内容，以避免运行出错。

### 墨水屏 2.13inch e-Paper HAT+ 硬件连接
直接插到树莓派 40PIN 排针上，对好引脚。

### 开启 SPI 接口
```bash
sudo raspi-config
# 选择 Interfacing Options -> SPI -> Yes
```

#### 按需开启 RTC
将以下内容写入 `/boot/firmware/config.txt`：
```bash
dtoverlay=i2c-rtc,ds3231
```

重启后检查 SPI：
```bash
ls /dev/spi*
# 应输出 /dev/spidev0.0 和 /dev/spidev0.1
```

## 快速安装（推荐）

使用一键安装脚本，支持 `--c` 标志安装 C 版本：

```bash
# 默认源 + C 版本
curl -sSL https://github.com/kxgx/2.13-Ink-screen-clock/raw/c-bindings/bin/install.sh | sudo bash -s -- --c
```

```bash
# 中国源 + C 版本（推荐国内用户）
curl -sSL https://gitee.com/xingguangk/2.13-Ink-screen-clock/raw/c-bindings/bin/install.sh | sudo bash -s -- --c --zh --cn --gitcn
```

### 参数说明

```
--c                      安装 C 语言版本
--zh                     设置系统语言为 zh_CN.UTF-8
--cn                     替换 apt 镜像源为中国镜像源
--gitcn                  克隆中国仓库 (gitee)
--pisugar-wifi-conf      安装 pisugar-wifi-conf
--pisugar-power-manager  安装 pisugar-power-manager
--version <tag>          指定分支/标签
--debug                  输出详细信息
```

## 手动编译

```bash
# 安装依赖
sudo apt-get update
sudo apt-get install -y git pigpio i2c-tools netcat-openbsd gawk python3 build-essential
sudo pip3 install requests

# 克隆仓库
git clone -b c-bindings https://github.com/kxgx/2.13-Ink-screen-clock.git
cd 2.13-Ink-screen-clock/clock

# 编译
make clean && make

# 运行
sudo ./build/epd_clock
```

## 目录结构

```
2.13-Ink-screen-clock/
├── clock/              # C 时钟程序
│   ├── clock.c         # 主程序源码
│   ├── stb_truetype.h  # 字体渲染库
│   ├── Makefile        # 编译配置
│   └── build/          # 编译输出
├── waveshare_epd/      # C 版 EPD 驱动库
│   ├── include/        # 头文件
│   ├── src/            # 源码 (hal + drivers)
│   └── Makefile
├── bin/
│   ├── install.sh      # 一键安装脚本
│   ├── start.sh        # 启动脚本
│   ├── weather.py      # 天气数据获取
│   ├── weather.json    # 天气数据缓存
│   └── raspi_e-Paper.service  # systemd 服务
└── pic/                # 字体和图片资源
```

## 效果展示

采用局刷方案，程序持续监测数据变化，变化时自动局刷。

![效果](https://github.com/kxgx/2.13-Ink-screen-clock/raw/main/pic/1736749257603.jpg)
