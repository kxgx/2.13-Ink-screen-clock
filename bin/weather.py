#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os, sys, re, json, time, datetime
import logging
import subprocess
import os
from threading import Timer
import requests

white = 255  # 颜色
black = 0
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')  # 设置日志级别

picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量

def get_area_id(city_name):
    """从city.js中检索AREAID，无限重试直到成功"""
    url = "https://j.i8tq.com/weather2020/search/city.js"
    while True:
        try:
            response = requests.get(url)
            response.raise_for_status()  # 检查请求是否成功
            # 直接打印原始数据
            logging.debug("Raw data from city.js: %s", response.text)
            # 预处理返回的数据，去除非JSON部分
            city_data_text = response.text.strip()
            city_data_text = city_data_text.split('var city_data = ')[-1].rstrip(';')
            if city_data_text:
                city_data = json.loads(city_data_text)
                # 遍历数据结构，查找城市名称
                for province, cities in city_data.items():
                    for city, districts in cities.items():
                        for district, info in districts.items():
                            if info['NAMECN'] == city_name:
                                return info['AREAID']
                logging.error("城市名称 '%s' 在城市数据中未找到，将使用默认城市信息", city_name)
            else:
                logging.error("从city.js接收到的数据为空")
        except (requests.RequestException, json.JSONDecodeError) as e:
            logging.error("检索或解析城市数据时发生错误: %s", e)
            time.sleep(180)  # 重试前等待

def get_current_city():
    """获取当前城市名称，无限重试直到成功"""
    url = "http://ip-api.com/json/?lang=zh-CN"
    while True:
        try:
            response = requests.get(url)
            response.raise_for_status()  # 检查请求是否成功
            data = response.json()
            if data['status'] == 'success':
                return data['city']
            else:
                logging.error("获取当前城市失败: %s", data['message'])
        except (requests.RequestException, json.JSONDecodeError) as e:
            logging.error("检索或解析当前城市时出现错误: %s", e)
            time.sleep(180)  # 重试前等待
# 注意：无限重试可能会在特定情况下导致程序无法终止，请确保在实际使用中考虑适当的退出条件或限制重试次数。

def schedule_getWeath():
    """调度 getWeath 函数每3分钟执行一次"""
    getWeath()
    Timer(180, schedule_getWeath).start()  # 重新设置定时器

def getWeath(city='101060101'):
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36',
        'Referer': 'https://www.weather.com.cn/'
    }
    current_city = get_current_city()
    if current_city:
        area_id = get_area_id(current_city)
        if area_id:
            city = area_id
    try:
        response = requests.get('https://d1.weather.com.cn/sk_2d/'+city+'.html',headers=headers)
        response.raise_for_status()  # 检查请求是否成功
        response.encoding = 'utf-8'
        Weath = response.text[11:]
        fileHandle = open('/root/2.13-Ink-screen-clock/bin/weather.json', 'w')
        fileHandle.write(str(Weath))
        fileHandle.close()
        print("天气文件更新")
    except requests.RequestException as e:
        logging.error("获取天气数据时出现网络错误: %s", e)
    except Exception as e:
        logging.error("发生了意外错误: %s", e)

try:
    schedule_getWeath()  # 开始调度天气获取函数
except OSError as e:
    logging.info(e)
except KeyboardInterrupt:
    logging.info("检测到键盘中断，正在退出")
except Exception as e:
    logging.error("发生了意外错误: %s", e)
    exit()

# 脚本正常结束后的清理操作
exit()
