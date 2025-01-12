#!/usr/bin/env python
# -*- coding:utf-8 -*-
from PIL import Image, ImageDraw, ImageFont  # 引入图片处理库
import os, sys, re, json, time, datetime  # 引入系统相关库
from borax.calendars.lunardate import LunarDate  # 农历日期以及天干地支纪年法的 Python 库
import logging  # 日志库
import subprocess
import os
from threading import Timer
import requests

white = 255  # 颜色
black = 0
logging.basicConfig(level=logging.DEBUG)

################################ 引入配置文件开始 ################################################
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量
    from waveshare_epd import epd2in13_V4  # 引入墨水屏驱动文件

logging.debug("Loading Fonts")
font01 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 20)  # 字体文件
font02 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 15)  # 字体文件
font03 = ImageFont.truetype(os.path.join(picdir, 'DSEG7Modern-Bold.ttf'), 38)  # 字体文件
font04 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 10)  # 字体文件
font05 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 12)  # 字体文件
font06 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 13)  # 字体文件
################################ 引入配置文件结束 ################################################

def Local_strong_brush():  # 局部强制刷新显示
    i = 0
    while i < 5:
        epd.displayPartial(epd.getbuffer(info_image.rotate(180)))  # 局刷开始
        i = i + 1

def getWeath(city='101060111'):  # 天气函数,下载json天气至本地
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36 Edg/94.0.992.50',
        'Referer': 'http://www.weather.com.cn/'
    }
    response = requests.get('http://d1.weather.com.cn/sk_2d/' + city + '.html', headers=headers)
    response.encoding = 'utf-8'
    Weath = response.text[11:]
    fileHandle = open('weather.json', 'w')
    fileHandle.write(str(Weath))
    fileHandle.close()
    Timer(1800, getWeath).start()  # 定时器函数,间隔三分钟下载文件至本地
    print("天气文件更新")

def gey_yiyan():  # 一言API
    response = requests.get("https://international.v1.hitokoto.cn/?c=k&c=f&c=d&c=a&encode=text&min_length=7&max_length=19")
    response.encoding = 'utf-8'
    yiyan = response.text
    return yiyan

def get_date():  # 返回当前年月日及星期几
    date = datetime.datetime.now()
    today = LunarDate.today()
    week_day_dict = {0: '星期一', 1: '星期二', 2: '星期三', 3: '星期四', 4: '星期五', 5: '星期六', 6: '星期日', }
    day = date.weekday()
    return time.strftime('%Y年%m月%d日') + '  ' + week_day_dict[day] + '  ' + today.strftime('农 %M月%D')

def get_time():  # 返回当前时间,不到秒,大写
    return time.strftime('%H:%M')

def Get_address():  # 获取当前的IP地址
    return (subprocess.check_output(u"hostname -I | cut -d' ' -f1 | head --bytes -1", shell=True).decode('gbk'))

def CPU_temperature():  # CPU温度获取
    temperatura = os.popen('vcgencmd measure_temp').readline()
    temperatura = temperatura.replace('temp=', '').strip()
    return str(temperatura)

def Memory_footprint():  # 显示内存占用百分比
    return (subprocess.check_output(u"free -m | awk -F '[ :]+' 'NR==2{printf \"%d\", ($3)/$2*100}'", shell=True).decode('gbk'))

def CPU_usage():  # 显示CPU占用百分比
    return (str(int(float(os.popen("top -b -n1 | awk '/Cpu\(s\):/ {print $2}'").readline().strip()))))

def power_battery():
    try:
        # 获取电池电量
        battery_percentage = subprocess.check_output(
            "echo \"get battery\" | nc -q 0 127.0.0.1 8423 | awk -F':' '{print $2}'",
            shell=True,
            text=True
        ).strip()

        # 检查电量信息
        if not battery_percentage:
            return "无效"

        # 返回电量百分比
        return f"{int(battery_percentage)}%"
    except Exception as e:
        # 返回错误信息
        return "失败"

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
    '''添加一言API'''
    global Time_keeping
    Time_keeping = 0
    draw.text((4, 84), gey_yiyan(), font=font06, fill=0)
    '''添加一言API end'''
    draw.ellipse((192, 107, 207, 120), 0, 255)  # 时钟图标
    draw.line((199, 109, 199, 114), fill=255, width=1)
    draw.line((200, 114, 204, 114), fill=255, width=1)
    global local_addr  # 获取当前IP地址
    local_addr = Get_address()  # 获取当前IP地址
    draw.text((10, 107), "IP:" + local_addr, font=font05, fill=255)  # 显示当前IP地址

def Weather():  # 在图片中添加天气内容
    Weather_json = open('weather.json', 'r')
    Weather_data = Weather_json.read()
    Weather_json.close()
    Weather_text = json.loads(Weather_data)
    global Weather_position
    global temperature
    global weather
    global wind_direction
    global weather_update
    global weather_date
    global humidity
    Weather_position = Weather_text['cityname']  # 定位位置
    temperature = Weather_text['temp'] + u'°C'  # 温度
    weather = Weather_text['weather']  # 天气情况
    wind_direction = Weather_text['WD']  # 风向
    weather_update = Weather_text['time']  # 天气更新时间
    weather_date = Weather_text['date']  # 日期
    humidity = Weather_text['SD']  # 湿度
    draw.text((150, 25), "天气:", font=font06, fill=0)  # 显示当前天气前缀
    draw.text((150, 45), "温度:", font=font06, fill=0)  # 显示当前温度前缀
    draw.text((150, 65), "湿度:", font=font06, fill=0)  # 显示当前湿度前缀
    draw.text((191, 25), weather, font=font06, fill=0)
    draw.text((191, 45), temperature, font=font06, fill=0)
    draw.text((191, 65), humidity, font=font06, fill=0)
    draw.text((211, 107), weather_update, font=font04, fill=255)  # 显示天气更新时间

def main():
    global font01, font02, font03, font04, font05, font06
    global draw
    global image
    image = Image.new('1', (250, 122), 255)  # 创建图片对象，背景为白色
    draw = ImageDraw.Draw(image)  # 创建画笔
    font01 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 20)  # 设置字体格式
    font02 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 18)
    font03 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 16)
    font04 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 12)
    font05 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 10)
    font06 = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 14)

    while True:
        draw.rectangle((0, 0, 250, 122), 255, 255)  # 清屏
        Top_edge()  # 顶部边缘信息
        Weather()  # 天气信息
        Bottom_edge()  # 底部边缘信息
        epd.display(epd.getbuffer(image))  # 显示图片
        time.sleep(60)  # 每隔60秒刷新一次

if __name__ == '__main__':
    main()