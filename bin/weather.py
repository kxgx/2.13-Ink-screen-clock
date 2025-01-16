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
logging.basicConfig(level=logging.INFO)  # 设置日志级别

picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量

def get_area_id(city_name):
    """从city.js中检索AREAID"""
    url = "https://j.i8tq.com/weather2020/search/city.js"
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
            logging.error("City name '%s' not found in city data", city_name)
        else:
            logging.error("Empty data received from city.js")
    except requests.RequestException as e:
        logging.error("Network error when retrieving city data: %s", e)
    except json.JSONDecodeError as e:
        logging.error("JSON decode error when parsing city data: %s", e)
    return None
    
def get_current_city():
    """获取当前城市名称"""
    url = "http://ip-api.com/json/?lang=zh-CN"
    try:
        response = requests.get(url)
        response.raise_for_status()  # 检查请求是否成功
        data = response.json()
        if data['status'] == 'success':
            return data['city']
        else:
            logging.error("Failed to get current city: %s", data['message'])
    except requests.RequestException as e:
        logging.error("Network error when retrieving current city: %s", e)
    except json.JSONDecodeError as e:
        logging.error("JSON decode error when parsing current city data: %s", e)
    return None

def getWeath(city='101060111'):
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36',
        'Referer': 'http://www.weather.com.cn/'
    }
    current_city = get_current_city()
    if current_city:
        area_id = get_area_id(current_city)
        if area_id:
            city = area_id
    try:
        response = requests.get('http://d1.weather.com.cn/sk_2d/'+city+'.html',headers=headers)
        response.raise_for_status()  # 检查请求是否成功
        response.encoding = 'utf-8'
        Weath = response.text[11:]
        fileHandle = open('weather.json', 'w')
        fileHandle.write(str(Weath))
        fileHandle.close()
        Timer(180, getWeath).start()  # 定时器函数,间隔三分钟下载文件至本地
        print("天气文件更新")
    except requests.RequestException as e:
        logging.error("Network error when retrieving weather data: %s", e)
    except Exception as e:
        logging.error("An unexpected error occurred: %s", e)

try:
    getWeath()  # 天气获取函数开始运行
except IOError as e:
    logging.info(e)
except KeyboardInterrupt:
    logging.info("Keyboard interrupt detected, exiting gracefully.")
except Exception as e:
    logging.error("An unexpected error occurred: %s", e)
    exit()

# 脚本正常结束后的清理操作
exit()
