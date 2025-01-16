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
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(message)s')  # 设置日志级别

picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)  # 将引入文件添加到环境变量

def get_area_id(city_name):
    """从city.js中检索AREAID，使用字典查找优化检索速度"""
    url = "https://j.i8tq.com/weather2020/search/city.js"
    try:
        response = requests.get(url)
        response.raise_for_status()  # 检查请求是否成功
        # 预处理返回的数据，去除非JSON部分
        city_data_text = response.text.strip()
        city_data_text = city_data_text.split('var city_data = ')[-1].rstrip(';')
        if city_data_text:
            city_data = json.loads(city_data_text)
            # 创建城市名称到AREAID的映射字典
            city_mapping = {}
            for province, cities in city_data.items():
                for city, districts in cities.items():
                    for district, info in districts.items():
                        city_mapping[info['NAMECN']] = info['AREAID']
            # 直接从字典中查找城市名称
            area_id = city_mapping.get(city_name)
            if area_id:
                return area_id
            else:
                logging.error("城市名称 '%s' 在城市数据中未找到，将使用默认城市信息", city_name)
        else:
            logging.error("从city.js接收到的数据为空")
    except requests.RequestException as e:
        logging.error("检索城市数据时发生网络错误: %s", e)
    except json.JSONDecodeError as e:
        logging.error("解析城市数据时发生JSON解码错误: %s", e)
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
            logging.error("获取当前城市失败: %s", data['message'])
    except requests.RequestException as e:
        logging.error("检索当前城市时出现网络错误: %s", e)
    except json.JSONDecodeError as e:
        logging.error("解析当前城市数据时出现JSON解码错误: %s", e)
    return None

def getWeath(city='101060101', save_dir='/root/2.13-Ink-screen-clock/bin'):
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
        # 确保保存目录存在
        if not os.path.exists(save_dir):
            os.makedirs(save_dir)
        file_path = os.path.join(save_dir, 'weather.json')
        with open(file_path, 'w') as fileHandle:
            fileHandle.write(str(Weath))
        Timer(180, getWeath, [city, save_dir]).start()  # 定时器函数,间隔三分钟下载文件至本地
        print("天气文件更新")
    except requests.RequestException as e:
        logging.error("获取天气数据时出现网络错误: %s", e)
    except Exception as e:
        logging.error("发生了意外错误: %s", e)

def ensure_directory_exists(save_directory):
    """
    确保指定的目录存在，如果不存在则创建它
    """
    if not os.path.exists(save_directory):
        os.makedirs(save_directory)
        logging.info(f"目录 {save_directory} 不存在，已创建")
    else:
        logging.info(f"目录 {save_directory} 已存在")

def main():
    save_directory = '/root/2.13-Ink-screen-clock/bin'  # 替换为您的指定目录
    ensure_directory_exists(save_directory)  # 确保保存目录存在
    getWeath(save_dir=save_directory)  # 天气获取函数开始运行

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logging.info("检测到键盘中断，正在退出")
        sys.exit()
    except Exception as e:
        logging.error("发生了意外的错误: %s", e)
        logging.info("正在重试")
        main()