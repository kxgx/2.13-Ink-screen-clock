#!/usr/bin/env python
# -*- coding:utf-8 -*-
from flask import Flask, render_template_string, request, Response, redirect, url_for
from PIL import Image, ImageDraw, ImageFont  # 引入图片处理库
import os, sys, re, json, time, datetime  # 引入系统相关库
from borax.calendars.lunardate import LunarDate  # 农历日期以及天干地支纪年法的 Python 库
import logging  # 日志库
import subprocess
import io  # 导入 io 模块
from threading import Thread

app = Flask(__name__)

# 设置Flask日志级别为WARNING，减少日志输出
log = logging.getLogger('werkzeug')
log.setLevel(logging.WARNING)

white = 255  # 颜色
black = 0
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

################################引入配置文件开始################################################
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量
    from waveshare_epd import epd2in13_V4  # 引入墨水屏驱动文件
logging.debug("Loading Fonts")
font01 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 20)  # 字体文件
font02 = ImageFont.truetype(os.path.join(picdir, 'GB2312.ttf'), 15)  # 字体文件
font03 = ImageFont.truetype(os.path.join(picdir, 'Fonttt.ttf'), 48)  # 字体文件
font04 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 10)  # 字体文件
font05 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 12)  # 字体文件
font06 = ImageFont.truetype(os.path.join(picdir, '原神cn.ttf'), 13)  # 字体文件
################################引入配置文件结束################################################

# 定义元素位置配置
positions = {
    "date": (2, 2),
    "time": (5, 28),
    "battery_icon_positions": [
        (126, 109), (154, 109),
        (126, 110), (126, 119),
        (127, 119), (154, 119),
        (154, 110), (154, 118),
        (155, 112), (157, 112),
        (155, 116), (157, 116),
        (157, 113), (157, 115)
    ],
    "power_str_position": (129, 108),
    "clock_icon_position": [(192, 107), (207, 120)],
    "clock_hand_positions": [
        (199, 109), (199, 114),
        (200, 114), (204, 114)
    ],
    "ip_address_position": (10, 107),
    "weather_prefix_position": {
        "天气": (150, 25),
        "温度": (150, 45),
        "湿度": (150, 65),
        "城市": (150, 85)
    },
    "weather_value_position": {
        "weather": (191, 25),
        "temperature": (191, 45),
        "humidity": (191, 65),
        "cityname": (191, 85)
    },
    "weather_update_position": (211, 107),
    "bottom_edge_position": [(0, 105), (250, 122)]
}

def get_date():  # 返回当前年月日及星期几
    date = datetime.datetime.now()
    today = LunarDate.today()
    week_day_dict = {0: '星期一', 1: '星期二', 2: '星期三', 3: '星期四', 4: '星期五', 5: '星期六', 6: '星期日'}
    day = date.weekday()
    return time.strftime('%Y年%m月%d日') + '' + week_day_dict[day] + '' + today.strftime('农历%M月%D')

def get_time():  # 返回当前时间,不到秒,大写
    return time.strftime('%H:%M')

def Get_ipv4_address():  # 获取当前的IP地址
    try:
        # 执行命令获取IP地址，并处理输出以仅返回IPv4地址
        ip_output = subprocess.check_output(
            "hostname -I | grep -oE '[0-9]{1,3}(\.[0-9]{1,3}){3}'", shell=True).decode('utf-8').strip()
        # 分割输出以获取单个IP地址列表
        ip_list = ip_output.split()
        # 过滤掉以172开头的IP地址
        filtered_ips = [ip for ip in ip_list if not ip.startswith("172.")]
        # 如果有有效的IP地址，返回第一个，否则返回获取失败
        if filtered_ips:
            return filtered_ips[0]
        else:
            return "地址获取失败"
    except subprocess.CalledProcessError as e:
        return "获取失败"

def power_battery():  # 获取当前电池电量
    return str(int(subprocess.check_output(
        u"echo \"get battery\" | nc -q 0 127.0.0.1 8423|awk -F':' '{print int($2)}'",
        shell=True).decode('gbk'))) + u'%'

def Bottom_edge(draw):  # 在图片中添加底边内容
    draw.rectangle(positions["bottom_edge_position"], 'black', 'black')
    '''电池图标画图'''
    for pos in positions["battery_icon_positions"]:
        if len(pos) == 4:
            draw.line(pos, fill=255, width=1)
        elif len(pos) == 2:
            draw.point(pos, fill=255)

    global power_str
    power_str = power_battery()
    draw.text(positions["power_str_position"], power_str, font=font04, fill=255)  # 显示当前电量百分比

    '''电池图标画图'''
    draw.ellipse(positions["clock_icon_position"], 0, 255)  # 时钟图标
    for hand_pos in positions["clock_hand_positions"]:
        draw.line(hand_pos, fill=255, width=1)

    global local_addr  # 获取当前IP地址
    local_addr = Get_ipv4_address()  # 获取当前IP地址
    draw.text(positions["ip_address_position"], "IP:" + local_addr, font=font05, fill=255)  # 显示当前IP地址

def Weather(draw):  # 在图片中添加天气内容
    with open('/root/2.13-Ink-screen-clock/bin/weather.json', 'r') as f:
        Weather_data = f.read()
    Weather_text = json.loads(Weather_data)
    global weather_position
    global temperature
    global weather
    global wind_direction
    global weather_update
    global weather_date
    global humidity
    weather_position = Weather_text['cityname']  # 定位位置
    temperature = Weather_text['temp'] + u'°C'  # 温度
    weather = Weather_text['weather']  # 天气情况
    wind_direction = Weather_text['WD']  # 风向
    weather_update = Weather_text['time']  # 天气更新时间
    weather_date = Weather_text['date']  # 日期
    humidity = Weather_text['SD']  # 湿度

    draw.text(positions["weather_prefix_position"]["天气"], "天气:", font=font06, fill=0)  # 显示当前天气前缀
    draw.text(positions["weather_prefix_position"]["温度"], "温度:", font=font06, fill=0)  # 显示当前温度前缀
    draw.text(positions["weather_prefix_position"]["湿度"], "湿度:", font=font06, fill=0)  # 显示当前湿度前缀
    draw.text(positions["weather_prefix_position"]["城市"], "城市:", font=font06, fill=0)  # 显示当前城市前缀
    draw.text(positions["weather_value_position"]["weather"], weather, font=font06, fill=0)
    draw.text(positions["weather_value_position"]["temperature"], temperature, font=font06, fill=0)
    draw.text(positions["weather_value_position"]["humidity"], humidity, font=font06, fill=0)
    draw.text(positions["weather_value_position"]["cityname"], weather_position, font=font06, fill=0)
    draw.text(positions["weather_update_position"], weather_update, font=font05, fill=255)  # 显示天气更新时间

def generate_image():
    width, height = 250, 122  # 设置画布大小
    info_image = Image.new('1', (width, height), white)  # 创建白色背景的画布
    draw = ImageDraw.Draw(info_image)

    date_var = get_date()  # 记录开始数据
    draw.text(positions["date"], date_var, font=font02, fill=0)  # 将日期及星期几显示到屏幕

    local_time = get_time()
    draw.text(positions["time"], local_time, font=font03, fill=0)  # 显示当前时间

    Bottom_edge(draw)  # 添加底边内容
    Weather(draw)  # 天气内容

    img_byte_arr = io.BytesIO()
    info_image.save(img_byte_arr, format='PNG')
    img_byte_arr.seek(0)
    return img_byte_arr.getvalue()

@app.route('/')
def index():
    return render_template_string('''
<!DOCTYPE html>
<html>
<head>
    <title>电子墨水屏内容</title>
    <style>
        #screen-container {
            position: relative;
            display: inline-block;
        }
        #settings-link {
            position: absolute;
            top: 300px;
            right: 10px;
            z-index: 1000;
            background-color: rgba(255, 255, 255, 0.8);
            padding: 5px 10px;
            text-decoration: none;
            color: black;
        }
    </style>
    <script>
        function updateImage() {
            var img = document.getElementById('screen');
            img.src = '/image?' + new Date().getTime();
        }

        setInterval(updateImage, 1000);
    </script>
</head>
<body>
    <h1>电子墨水屏内容</h1>
    <div id="screen-container">
        <img id="screen" src="/image" style="transform: scale(2); transform-origin: top left;">
        <a id="settings-link" href="{{ url_for('settings') }}">设置参数</a>
    </div>
</body>
</html>
''')

@app.route('/image')
def image():
    img = generate_image()
    return Response(img, mimetype='image/png')

@app.route('/settings', methods=['GET', 'POST'])
def settings():
    global positions, font01, font02, font03, font04, font05, font06

    if request.method == 'POST':
        # Update positions
        positions["date"] = tuple(map(int, request.form.get('date_position').split(',')))
        positions["time"] = tuple(map(int, request.form.get('time_position').split(',')))
        positions["battery_icon_positions"] = list(map(lambda x: tuple(map(int, x.split(','))), request.form.getlist('battery_icon_positions')))
        positions["power_str_position"] = tuple(map(int, request.form.get('power_str_position').split(',')))
        positions["clock_icon_position"] = list(map(lambda x: tuple(map(int, x.split(','))), request.form.getlist('clock_icon_position')))
        positions["clock_hand_positions"] = list(map(lambda x: tuple(map(int, x.split(','))), request.form.getlist('clock_hand_positions')))
        positions["ip_address_position"] = tuple(map(int, request.form.get('ip_address_position').split(',')))
        positions["weather_prefix_position"]["天气"] = tuple(map(int, request.form.get('weather_prefix_天气').split(',')))
        positions["weather_prefix_position"]["温度"] = tuple(map(int, request.form.get('weather_prefix_温度').split(',')))
        positions["weather_prefix_position"]["湿度"] = tuple(map(int, request.form.get('weather_prefix_湿度').split(',')))
        positions["weather_prefix_position"]["城市"] = tuple(map(int, request.form.get('weather_prefix_城市').split(',')))
        positions["weather_value_position"]["weather"] = tuple(map(int, request.form.get('weather_value_weather').split(',')))
        positions["weather_value_position"]["temperature"] = tuple(map(int, request.form.get('weather_value_temperature').split(',')))
        positions["weather_value_position"]["humidity"] = tuple(map(int, request.form.get('weather_value_humidity').split(',')))
        positions["weather_value_position"]["cityname"] = tuple(map(int, request.form.get('weather_value_cityname').split(',')))
        positions["weather_update_position"] = tuple(map(int, request.form.get('weather_update_position').split(',')))
        positions["bottom_edge_position"] = list(map(lambda x: tuple(map(int, x.split(','))), request.form.getlist('bottom_edge_position')))

        # Update fonts
        font01 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), int(request.form.get('font01_size')))
        font02 = ImageFont.truetype(os.path.join(picdir, 'GB2312.ttf'), int(request.form.get('font02_size')))
        font03 = ImageFont.truetype(os.path.join(picdir, 'Fonttt.ttf'), int(request.form.get('font03_size')))
        font04 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), int(request.form.get('font04_size')))
        font05 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), int(request.form.get('font05_size')))
        font06 = ImageFont.truetype(os.path.join(picdir, '原神cn.ttf'), int(request.form.get('font06_size')))

        return redirect(url_for('index'))

    # Flatten positions dictionary for rendering
    flat_positions = {}
    for key, value in positions.items():
        if isinstance(value, dict):
            for sub_key, sub_value in value.items():
                flat_positions[f"{key}_{sub_key}"] = sub_value
        elif isinstance(value, list):
            flat_positions[key] = ';'.join([','.join(map(str, item)) for item in value])
        else:
            flat_positions[key] = value

    return render_template_string('''
<!DOCTYPE html>
<html>
<head>
    <title>设置参数</title>
</head>
<body>
    <h1>设置参数</h1>
    <form method="post">
        <label for="date_position">日期位置 (x,y):</label>
        <input type="text" id="date_position" name="date_position" value="{{ flat_positions['date'] }}"><br><br>

        <label for="time_position">时间位置 (x,y):</label>
        <input type="text" id="time_position" name="time_position" value="{{ flat_positions['time'] }}"><br><br>

        <label for="battery_icon_positions">电池图标位置 (x1,y1;x2,y2;...):</label>
        <input type="text" id="battery_icon_positions" name="battery_icon_positions" value="{{ flat_positions['battery_icon_positions'] }}"><br><br>

        <label for="power_str_position">电量字符串位置 (x,y):</label>
        <input type="text" id="power_str_position" name="power_str_position" value="{{ flat_positions['power_str_position'] }}"><br><br>

        <label for="clock_icon_position">时钟图标位置 (x1,y1;x2,y2;...):</label>
        <input type="text" id="clock_icon_position" name="clock_icon_position" value="{{ flat_positions['clock_icon_position'] }}"><br><br>

        <label for="clock_hand_positions">时钟指针位置 (x1,y1;x2,y2;...):</label>
        <input type="text" id="clock_hand_positions" name="clock_hand_positions" value="{{ flat_positions['clock_hand_positions'] }}"><br><br>

        <label for="ip_address_position">IP地址位置 (x,y):</label>
        <input type="text" id="ip_address_position" name="ip_address_position" value="{{ flat_positions['ip_address_position'] }}"><br><br>

        <label for="weather_prefix_天气">天气前缀位置 (x,y):</label>
        <input type="text" id="weather_prefix_天气" name="weather_prefix_天气" value="{{ flat_positions['weather_prefix_天气'] }}"><br><br>

        <label for="weather_prefix_温度">温度前缀位置 (x,y):</label>
        <input type="text" id="weather_prefix_温度" name="weather_prefix_温度" value="{{ flat_positions['weather_prefix_温度'] }}"><br><br>

        <label for="weather_prefix_湿度">湿度前缀位置 (x,y):</label>
        <input type="text" id="weather_prefix_湿度" name="weather_prefix_湿度" value="{{ flat_positions['weather_prefix_湿度'] }}"><br><br>

        <label for="weather_prefix_城市">城市前缀位置 (x,y):</label>
        <input type="text" id="weather_prefix_城市" name="weather_prefix_城市" value="{{ flat_positions['weather_prefix_城市'] }}"><br><br>

        <label for="weather_value_weather">天气值位置 (x,y):</label>
        <input type="text" id="weather_value_weather" name="weather_value_weather" value="{{ flat_positions['weather_value_weather'] }}"><br><br>

        <label for="weather_value_temperature">温度值位置 (x,y):</label>
        <input type="text" id="weather_value_temperature" name="weather_value_temperature" value="{{ flat_positions['weather_value_temperature'] }}"><br><br>

        <label for="weather_value_humidity">湿度值位置 (x,y):</label>
        <input type="text" id="weather_value_humidity" name="weather_value_humidity" value="{{ flat_positions['weather_value_humidity'] }}"><br><br>

        <label for="weather_value_cityname">城市名值位置 (x,y):</label>
        <input type="text" id="weather_value_cityname" name="weather_value_cityname" value="{{ flat_positions['weather_value_cityname'] }}"><br><br>

        <label for="weather_update_position">天气更新位置 (x,y):</label>
        <input type="text" id="weather_update_position" name="weather_update_position" value="{{ flat_positions['weather_update_position'] }}"><br><br>

        <label for="bottom_edge_position">底部边缘位置 (x1,y1;x2,y2;...):</label>
        <input type="text" id="bottom_edge_position" name="bottom_edge_position" value="{{ flat_positions['bottom_edge_position'] }}"><br><br>

        <label for="font01_size">Font01 字体大小:</label>
        <input type="number" id="font01_size" name="font01_size" value="20"><br><br>

        <label for="font02_size">Font02 字体大小:</label>
        <input type="number" id="font02_size" name="font02_size" value="15"><br><br>

        <label for="font03_size">Font03 字体大小:</label>
        <input type="number" id="font03_size" name="font03_size" value="48"><br><br>

        <label for="font04_size">Font04 字体大小:</label>
        <input type="number" id="font04_size" name="font04_size" value="10"><br><br>

        <label for="font05_size">Font05 字体大小:</label>
        <input type="number" id="font05_size" name="font05_size" value="12"><br><br>

        <label for="font06_size">Font06 字体大小:</label>
        <input type="number" id="font06_size" name="font06_size" value="13"><br><br>

        <button type="submit">保存设置</button>
    </form>
    <a href="{{ url_for('index') }}">返回主页</a>
</body>
</html>
''', flat_positions=flat_positions)

def screen_updater():
    while True:
        try:
            epd.init()
            Basic_refresh(epd)
            Partial_refresh(epd)
            epd.sleep()
        except Exception as e:
            logging.error(f"Screen update failed: {e}")
        finally:
            time.sleep(60)  # 更新频率为每分钟一次

def Basic_refresh(epd):  # 全刷函数
    logging.info("在启动画布之前，刷新并准备基本内容")  # 开始画布前刷新准备基础内容
    global get_date_var
    get_date_var = get_date()  # 记录开始数据
    draw.text(positions["date"], get_date_var, font=font02, fill=0)  # 将日期及星期几显示到屏幕
    global local_time
    local_time = get_time()
    draw.text(positions["time"], local_time, font=font03, fill=0)  # 显示当前时间
    Bottom_edge(draw)  # 添加底边内容
    Weather(draw)  # 天气内容
    epd.display(epd.getbuffer(info_image.rotate(180)))

def Partial_refresh(epd):  # 局刷函数
    logging.info("部分内容更新，此更新建议与分钟同步，以节省墨水屏的使用寿命")  # 局部内容更新,此更新建议与分钟同步,以节省墨水屏寿命
    epd.displayPartBaseImage(epd.getbuffer(info_image.rotate(180)))
    epd.init()
    while True:
        global local_time
        local_time1 = get_time()
        if local_time1 != local_time:
            draw.rectangle((5, 28, 149, 82), fill=255)  # 时间局刷区域
            draw.text(positions["time"], local_time1, font=font03, fill=0)  # 刷新当前时间
            local_time = local_time1
            Local_strong_brush(epd)  # 局部强刷

        get_date_var1 = get_date()  # 局刷判断,如果时间与前一次不一致说明内容变化,需要刷新显示
        global get_date_var  # 再次声明这个是全局变量
        if get_date_var1 != get_date_var:
            draw.rectangle((2, 2, 250, 16), fill=255)  # 设置头部刷新区域
            draw.text(positions["date"], get_date_var1, font=font02, fill=0)  # 将日期及星期几刷新显示到屏幕
            get_date_var = get_date_var1  # 将更新的值保存到初始变量,直到下一次变化时执行该刷新操作
            logging.debug("头部日期部位发生刷新变化.")
            Local_strong_brush(epd)  # 局部强刷

        global local_addr  # 当前IP地址
        local_addr1 = Get_ipv4_address()
        if local_addr1 != local_addr:
            draw.rectangle((1, 107, 123, 120), fill=0)  # 设置头部刷新区域
            draw.text(positions["ip_address_position"], "IP:" + local_addr1, font=font05, fill=255)  # 显示当前IP地址
            local_addr = local_addr1
            Local_strong_brush(epd)  # 局部强刷

        '''天气局部更新函数'''
        with open('/root/2.13-Ink-screen-clock/bin/weather.json', 'r') as f:
            Weather_data = f.read()
        Weather_text = json.loads(Weather_data)
        global weather_position
        global temperature
        global weather
        global wind_direction
        global weather_update
        global weather_date
        global humidity
        weather_position1 = Weather_text['cityname']  # 定位位置
        temperature1 = Weather_text['temp'] + u'°C'  # 温度
        weather11 = Weather_text['weather']  # 天气情况
        wind_direction1 = Weather_text['WD']  # 风向
        weather_update1 = Weather_text['time']  # 天气更新时间
        weather_date1 = Weather_text['date']  # 日期
        humidity1 = Weather_text['SD']  # 湿度

        if weather11 != weather:
            draw.rectangle((191, 25, 249, 38), fill=255)  # 天气局刷区域
            draw.text(positions["weather_value_position"]["weather"], weather11, font=font06, fill=0)
            weather = weather11
            logging.info("天气局部刷新")
            Local_strong_brush(epd)  # 局部强刷

        if temperature1 != temperature:
            draw.rectangle((191, 45, 249, 57), fill=255)  # 局刷区域
            draw.text(positions["weather_value_position"]["temperature"], temperature1, font=font06, fill=0)
            temperature = temperature1
            logging.info("温度局部刷新")
            Local_strong_brush(epd)  # 局部强刷

        if humidity1 != humidity:
            draw.rectangle((191, 65, 249, 77), fill=255)  # 局刷区域
            draw.text(positions["weather_value_position"]["humidity"], humidity1, font=font06, fill=0)
            humidity = humidity1
            logging.info("湿度局部刷新")
            Local_strong_brush(epd)  # 局部强刷

        if weather_position1 != weather_position:
            draw.rectangle((191, 85, 249, 98), fill=255)  # 局刷区域
            draw.text(positions["weather_value_position"]["cityname"], weather_position1, font=font06, fill=0)
            weather_position = weather_position1
            logging.info("城市局部刷新")
            Local_strong_brush(epd)  # 局部强刷

        if weather_update1 != weather_update:
            draw.rectangle((211, 107, 248, 118), fill=0)  # 设置更新时间刷新区域
            draw.text(positions["weather_update_position"], weather_update1, font=font05, fill=255)  # 显示天气更新时间
            weather_update = weather_update1
            logging.info("天气更新时间局部刷新")
            Local_strong_brush(epd)  # 局部强刷

        '''天气局部更新函数'''
        global power_str
        power_str1 = power_battery()
        if power_str1 != power_str:
            draw.rectangle((128, 110, 153, 117), fill=0)  # 设置更新时间刷新区域
            draw.text(positions["power_str_position"], power_str1, font=font04, fill=255)  # 显示当前电量百分比
            power_str = power_str1
            Local_strong_brush(epd)  # 局部强刷

def Local_strong_brush(epd):  # 局部强制刷新显示
    i = 0
    while i < 5:
        epd.displayPartial(epd.getbuffer(info_image.rotate(180)))  # 局刷开始
        i = i + 1

if __name__ == '__main__':
    try:
        epd = epd2in13_V4.EPD()  # 初始化
        epd.init()  # 设定屏幕刷新模式
        logging.info("Width = %s, Height = %s", format(epd.width), format(epd.height))  # 打印屏幕高度及宽度
        logging.info("初始化并清空显示屏")  # 屏幕开始准备相关展示
        info_image = Image.new('1', (epd.height, epd.width), 255)  # 画布创建准备
        draw = ImageDraw.Draw(info_image)

        # 启动屏幕更新线程
        screen_thread = Thread(target=screen_updater)
        screen_thread.daemon = True
        screen_thread.start()

        # 启动Flask应用
        app.run(host='0.0.0.0', port=5000)
    except OSError as e:
        logging.info(e)
    except KeyboardInterrupt:
        logging.info("检测到键盘中断，正在清理并退出")
        epd.init()
        epd.Clear(0xFF)  # 清除屏幕内容
        epd.sleep()  # 使屏幕进入休眠状态
        epd2in13_V4.epdconfig.module_exit()  # 清理资源
        exit()

    except Exception as e:
        logging.error("发生了意外的错误: %s", e)
        epd.init()
        epd.Clear(0xFF)  # 清除屏幕内容
        epd.sleep()  # 使屏幕进入休眠状态
        epd2in13_V4.epdconfig.module_exit()  # 清理资源
        exit()

    # 脚本正常结束后的清理操作
    epd.init()
    epd.Clear(0xFF)  # 清除屏幕内容
    epd.sleep()  # 使屏幕进入休眠状态
    epd2in13_V4.epdconfig.module_exit()  # 清理资源
    exit()