#!/usr/bin/env python
# -*- coding:utf-8 -*-
from PIL import Image,ImageDraw,ImageFont  #引入图片处理库
import os,sys,re,json,time,datetime  #引入系统相关库
from borax.calendars.lunardate import LunarDate #农历日期以及天干地支纪年法的 Python 库
import logging  #日志库
import subprocess
import os
from threading import Timer
import requests
white = 255 #颜色
black = 0
logging.basicConfig(level=logging.INFO)
################################引入配置文件开始################################################
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量
    from waveshare_epd import epd2in13_V4  # 引入墨水屏驱动文件

logging.debug("Loading Fonts")
font01 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 20)  # 字体文件
font02 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 15)  # 字体文件
font03 = ImageFont.truetype(os.path.join(picdir, 'DSEG7Modern-Bold.ttf'), 66)  # 字体文件
font04 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 10)  # 字体文件
font05 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 12)  # 字体文件
font06 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 13)  # 字体文件
################################引入配置文件结束################################################


def Local_strong_brush():  # 局部强制刷新显示
    i = 0
    while i < 5:
        epd.displayPartial(epd.getbuffer(info_image.rotate(180)))  # 局刷开始
        i += 1


def get_date():  # 返回当前年月日及星期几
    date = datetime.datetime.now()
    today = LunarDate.today()
    week_day_dict = {0: '星期一', 1: '星期二', 2: '星期三', 3: '星期四', 4: '星期五', 5: '星期六', 6: '星期日'}
    day = date.weekday()
    return time.strftime('%Y年%m月%d日') + '' + week_day_dict[day] + '' + today.strftime('农历%M月%D')
# 定义一个全局标志变量，用于检查是否已经设置了系统时间或尝试过设置
has_set_system_time = False

def set_system_time_from_hwclock(utc=True):
    """Set the system time from the hardware clock.
    
    Args:
        utc (bool): Whether the RTC is in UTC. Default is True.
    """
    try:
        # 记录调用 hwclock 前的时间
        before_time = datetime.datetime.now()
        logging.debug(f"System time before hwclock call: {before_time}")

        # 构造 hwclock 命令及其参数
        hwclock_args = ['sudo', 'hwclock', '--hctosys']
        if not utc:
            hwclock_args.append('--localtime')
        
        logging.debug(f"Executing hwclock command: {' '.join(hwclock_args)}")
        
        # 使用 subprocess.run 执行 hwclock --hctosys 并捕获输出
        result = subprocess.run(hwclock_args, 
                               check=True,  # 如果命令失败，则抛出 CalledProcessError 异常
                               stdout=subprocess.PIPE, 
                               stderr=subprocess.PIPE, 
                               text=True)
        
        # 等待一小段时间以确保时间更新完成
        time.sleep(0.1)  # 根据需要调整
        
        # 记录调用 hwclock 后的时间
        after_time = datetime.datetime.now()
        logging.debug("System time successfully set from hardware clock.")
        logging.debug(f"System time after hwclock call: {after_time}")
        
        # 检查时间是否发生了倒退
        if after_time < before_time:
            logging.info(f"Time went backwards after hwclock call: before={before_time}, after={after_time}")
        
        return True
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to set system time from hardware clock: {e.stderr}")
        return False
    except Exception as e:
        logging.error(f"An unexpected error occurred: {str(e)}")
        return False

def get_time():
    global has_set_system_time
    
    if not has_set_system_time:
        # 尝试从硬件时钟设置系统时间，默认假设 RTC 是 UTC
        success = set_system_time_from_hwclock(utc=True)
        # 无论成功与否，都更新标志变量以避免重复尝试
        has_set_system_time = True
    
    # 获取并返回当前时间，格式为 HH:MM 大写
    current_time = time.strftime('%H:%M').upper()
    logging.debug(f"Returning current time: {current_time}")
    return current_time

def Get_address():  # 获取当前的IP地址
    return subprocess.check_output(u"hostname -I | cut -d' ' -f1 | head --bytes -1", shell=True).decode('gbk').strip()


def CPU_temperature():  # CPU温度获取
    temperatura = os.popen('vcgencmd measure_temp').readline()
    temperatura = temperatura.replace('temp=', '').strip()
    return str(temperatura)


def Memory_footprint():  # 显示内存占用百分比
    return subprocess.check_output(u"free -m | awk -F '[ :]+' 'NR==2{printf \"%d\", ($3)/$2*100}'", shell=True).decode('gbk')


def CPU_usage():  # 显示CPU占用百分比
    return str(int(float(os.popen("top -b -n1 | awk '/Cpu\(s\):/ {print $2}'").readline().strip())))


def power_battery():  # 获取当前电池电量
    return str(int(subprocess.check_output(u"echo \"get battery\" | nc -q 0 127.0.0.1 8423|awk -F':' '{print int($2)}'", shell=True).decode('gbk'))) + u'%'


# 打印电量信息
print(power_battery())


def Bottom_edge():  # 在图片中添加底边内容
    draw.rectangle((0, 105, 250, 122), 'black', 'black')
    '''电池图标画图'''
    draw.line((126, 109, 154, 109), fill=255, width=1)  # 电池顶边
    draw.line((126, 110, 126, 119), fill=255, width=1)  # 电池左边
    draw.line((127, 119, 154, 119), fill=255, width=1)  # 电池下边
    draw.line((154, 110, 154, 118), fill=255, width=1)
    draw.line((155, 112, 157, 112), fill=255, width=1)
    draw.line((155, 116, 157, 116), fill=255, width=1)
    draw.line((157, 113, 157, 115), fill=255, width=1)
    global power_str
    power_str = power_battery()
    draw.text((128, 108), power_str, font=font04, fill=255)  # 显示当前电量百分比
    '''电池图标画图'''
    draw.ellipse((192, 107, 207, 120), 0, 255)  # 时钟图标
    draw.line((199, 109, 199, 114), fill=255, width=1)
    draw.line((200, 114, 204, 114), fill=255, width=1)
    global local_addr  # 获取当前IP地址
    local_addr = Get_address()  # 获取当前IP地址
    draw.text((10, 107), "IP:" + local_addr, font=font05, fill=255)  # 显示当前IP地址


def Basic_refresh():  # 全刷函数
    logging.info("Refresh and prepare the basic content before starting the canvas")  # 开始画布前刷新准备基础内容
    global get_date_var
    get_date_var = get_date()  # 记录开始数据
    draw.text((2, 2), get_date_var, font=font02, fill=0)  # 将日期及星期几显示到屏幕
    global local_time
    local_time = get_time()
    draw.text((5, 28), local_time, font=font03, fill=0)  # 显示当前时间
    Bottom_edge()  # 添加底边内容
    epd.display(epd.getbuffer(info_image.rotate(180)))


def Partial_full_brush():  # 局部定时全刷函数
    Basic_refresh()  # 全局刷新
    logging.debug("局部定时全局刷新")
    epd.init()


def Partial_refresh():  # 局刷函数
    logging.info(
        "Partial content update, this update is recommended to be synchronized with the minute to save the life of the ink screen")  # 局部内容更新,此更新建议与分钟同步,以节省墨水屏寿命
    epd.displayPartBaseImage(epd.getbuffer(info_image.rotate(180)))
    epd.init()
    while True:
        global local_time
        local_time1 = get_time()
        if local_time1 != local_time:
            draw.rectangle((5, 28, 133, 95), fill=255)  # 时间局刷区域
            draw.text((5, 28), local_time1, font=font03, fill=0)  # 刷新当前时间
            local_time = local_time1
            Local_strong_brush()  # 局部强刷
        get_date_var1 = get_date()  # 局刷判断,如果时间与前一次不一致说明内容变化,需要刷新显示
        global get_date_var  # 再次声明这个是全局变量
        if get_date_var1 != get_date_var:
            draw.rectangle((2, 2, 250, 16), fill=255)  # 设置头部刷新区域
            draw.text((2, 2), get_date_var1, font=font02, fill=0)  # 将日期及星期几刷新显示到屏幕
            get_date_var = get_date_var1  # 将更新的值保存到初始变量,直到下一次变化时执行该刷新操作
            logging.debug("头部日期部位发生刷新变化.")
            Local_strong_brush()  # 局部强刷
        global local_addr  # 当前IP地址
        local_addr1 = Get_address()
        if local_addr1 != local_addr:
            draw.rectangle((1, 107, 94, 120), fill=0)  # 设置头部刷新区域
            draw.text((10, 107), "IP:" + local_addr1, font=font05, fill=255)  # 显示当前IP地址
            local_addr = local_addr1
            Local_strong_brush()  # 局部强刷
        # 显示当前电量百分比
        global power_str
        power_str1 = power_battery()
        power_str = power_str1
        Local_strong_brush()  # 局部强刷

retry_interval = 180  # 设置重试间隔时间（秒）
while True:
    try:
        ##################屏幕初始化#########################
        epd = epd2in13_V4.EPD() #初始化
        epd.init()#设定屏幕刷新模式
        #epd.Clear(0xFF) #清除屏幕内容
        ##################屏幕初始化#########################   
        logging.info("Width = %s, Height = %s", format(epd.width), format(epd.height)) #打印屏幕高度及宽度
        logging.info("初始化并清空显示屏")#屏幕开始准备相关展示
        info_image = Image.new('1', (epd.height, epd.width), 255) #画布创建准备
        draw = ImageDraw.Draw(info_image)
        Basic_refresh() #全局刷新
        Partial_refresh() #局部刷新
        epd.init()
        epd.Clear(0xFF)
        epd.sleep()
        time.sleep(300)
        break  # 如果脚本执行成功，则退出循环
    except (OSError, Exception) as e:  # 捕获你提到的异常
        logging.error("发生了错误: %s", e)
        time.sleep(retry_interval)  # 等待一段时间后重试
    except KeyboardInterrupt:
        logging.info("检测到键盘中断，正在清理并退出")
        epd.init()
        epd.Clear(0xFF)  # 清除屏幕内容
        epd.sleep()       # 使屏幕进入休眠状态
        epd2in13_V4.epdconfig.module_exit()  # 清理资源
        exit()

# 脚本正常结束后的清理操作
epd.init()
epd.Clear(0xFF)  # 清除屏幕内容
epd.sleep()       # 使屏幕进入休眠状态
epd2in13_V4.epdconfig.module_exit()  # 清理资源
exit()