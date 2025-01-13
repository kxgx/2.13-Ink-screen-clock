# 墨水屏展示当前时间及天气数据

本项目复刻自[Seek-Huang](https://github.com/Seek-Huang/2.13-Ink-screen-clock)的代码仓库
并在此基础上进行改进

## 本仓库已添加[一键安装部署脚本](https://github.com/kxgx/2.13-Ink-screen-clock#%E4%BD%BF%E7%94%A8%E8%84%9A%E6%9C%AC%E7%9B%B4%E6%8E%A5%E5%AE%89%E8%A3%85%E6%8E%A8%E8%8D%90)
注意:请先配置完需要的硬件以及系统配置操作再运行脚本

## API说明:

#### 一言API
此网站可以获取指定长度指定类型的一个句子,可以限定返回长度及返回类型 
    
    https://international.v1.hitokoto.cn/?c=a&encode=text&max_length=20

#### 天气API
获取的天气是通过网页的接口,获取的内容保存在当前目录下的json文件接口的地址是在这里抓取的 
    
    http://www.weather.com.cn/
    
在控制台f12就可以抓取到
手动伪造一个来源即可正常的获取到该数据内容

### 外壳及电池模块
使用的是PiSugar3的外壳,获取电源及树莓派(Raspberry)RTC时间都是靠这一部分模块完成的,附上github地址
    
     https://github.com/PiSugar/PiSugar/wiki/PiSugar-3-Series#rtc-on-board
     https://www.pisugar.com/
ps:如果没有使用此模块则需要更改代码内容,以避免运行出错。

ps:代码里需要nc命令需要安装netcat，安装内容已集成到“[需要安装的软件和依赖](https://github.com/kxgx/2.13-Ink-screen-clock#%E9%9C%80%E8%A6%81%E5%AE%89%E8%A3%85%E7%9A%84%E8%BD%AF%E4%BB%B6%E5%92%8C%E4%BE%9D%E8%B5%96)"
     
### 墨水屏2.13inch e-Paper HAT+硬件连接
连接树莓派的时候，可以直接将板子插到树莓派的 40PIN 排针上去，注意对好引脚。

### 开启SPI接口：
打开树莓派终端，输入以下指令进入配置界面：
sudo raspi-config
选择Interfacing Options -> SPI -> Yes 开启SPI接口
![image](https://www.waveshare.net/w/upload/1/1e/RPI_open_spi.png)

#### 按需开启
PiSugar 3 板载一个 RTC，可以通过 hwclock 轻松使用
将以下内容写入/boot/firmware/config.txt文件：
```Bash
dtoverlay=i2c-rtc,ds3231
```
![image](https://raw.github.com/kxgx/2.13-Ink-screen-clock/main/pic/1.png)

### 重启树莓派：
sudo reboot
检查 /boot/firmware/config.txt，可以看到 'dtparam=spi=on' 已被写入

![image](https://www.waveshare.net/w/upload/4/46/RPI_open_spi_1.jpg)

为了确保 SPI 没有被占用，建议其他的驱动覆盖暂时先关闭。可以使用 ls /dev/spi* 来检查 SPI 占用情况，终端输出 /dev/spidev0.0 和 /dev/spidev0.1 表示 SPI 情况正常

![image](https://www.waveshare.net/w/upload/a/a0/RPI_open_spi_2.jpg)

wifi连接管理可以使用[PiSugar/sugar-wifi-conf:让树莓派提供蓝牙BLE服务，使用小程序可以随时更改树莓派的wifi连接
](https://github.com/PiSugar/PiSugar/wiki/PiSugar-WiFi-config#sugar-wifi-conf)
## 使用脚本直接安装（推荐）
### 参数定义
```Bash
--zh    设置系统语言为zh_CN,UTF-8
--cn    替换apt镜像源为中国镜像源
--gitcn 克隆中国仓库
--debug 输出详细信息
```
### 
```Bash
#中国源默认设置
curl -sSL https://gitee.com/xingguangk/2.13-Ink-screen-clock/raw/main/bin/install.sh | sudo bash
```
```Bash
#中国源全参数设置(不使用--debug参数)
curl -sSL https://gitee.com/xingguangk/2.13-Ink-screen-clock/raw/main/bin/install.sh | sudo bash -s -- --zh --cn --gitcn
```
```Bash
#默认源默认设置
curl -sSL https://github.com/kxgx/2.13-Ink-screen-clock/raw/main/bin/install.sh | sudo bash
```
```Bash
#中国源全参数设置(不使用--debug参数)
curl -sSL https://github.com/kxgx/2.13-Ink-screen-clock/raw/main/bin/install.sh | sudo bash -s -- --zh --cn --gitcn
```

## 需要安装的软件和依赖:
参考
微雪电子 https://www.waveshare.net/wiki/2.13inch_e-Paper_HAT+#Raspberry_Pi
PiSugar 3官方文档 https://github.com/PiSugar/PiSugar/wiki/PiSugar-3-Series#software-installation
```Bash
sudo apt-get update
sudo apt-get install -y git pigpio i2c-tools netcat* gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential
sudo pip3 install spidev borax pillow requests
wget https://cdn.pisugar.com/release/pisugar-power-manager.sh
bash pisugar-power-manager.sh -c release
```
#### 安装命令和执行启动文件组合命令：
```Bash
sudo apt-get update && sudo apt-get install -y git pigpio i2c-tools netcat* gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential && sudo pip3 install spidev borax pillow requests && wget https://cdn.pisugar.com/release/pisugar-power-manager.sh && bash pisugar-power-manager.sh -c release && cd ~/ && git clone https://github.com/kxgx/2.13-Ink-screen-clock.git && cd ~/2.13-Ink-screen-clock/bin/ && sudo chmod +x start.sh && sudo ./start.sh
```
##### 使用国内仓库：
```Bash
sudo apt-get update && sudo apt-get install -y git pigpio i2c-tools netcat* gawk python3-dev python3-pip python3-pil python3-numpy python3-gpiozero python3-pigpio build-essential && sudo pip3 install spidev borax pillow requests && sudo pip3 install spidev borax pillow requests --break-system-packages && wget https://cdn.pisugar.com/release/pisugar-power-manager.sh && bash pisugar-power-manager.sh -c release && cd ~/ && git clone https://gitee.com/xingguangk/2.13-Ink-screen-clock.git && cd ~/2.13-Ink-screen-clock/bin/ && sudo chmod +x start.sh && sudo ./start.sh
```
###### 注意
```Bash
# 如果使用的是非lite系统请在pip3安装部分添加
--break-system-packages
```
在代码文件第三十三行,此次代码需要更改,否则将展示默认城市天气数据
# 效果展示
总体采用局刷方案,程序运行后一直处于程序的获取新数据的过程中,当发现数据变化后即开始自动局刷。
![image](https://github.com/kxgx/2.13-Ink-screen-clock/raw/main/pic/1736749257603.jpg)
