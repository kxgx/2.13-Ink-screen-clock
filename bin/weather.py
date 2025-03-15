#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os
import json
import time
import logging
import requests
from threading import Timer

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

def get_ip():
    """从ip.cn获取当前IP地址"""
    url = "https://ip.cn/api/index?ip=&type=0"
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
    }
    try:
        resp = requests.get(url, headers=headers, timeout=10)
        resp.raise_for_status()
        data = resp.json()
        if data.get('code') == 'Success':
            return data.get('ip', '')
        logging.error("获取IP失败: %s", data.get('msg', '未知错误'))
    except Exception as e:
        logging.error("获取IP异常: %s", str(e))
    return None

def get_current_city():
    """通过IP地址获取当前定位城市并去除'市'后缀"""
    ip = get_ip()
    if not ip:
        return None

    url = f"http://ip-api.com/json/{ip}?fields=city&lang=zh-CN"
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
    }
    while True:
        try:
            resp = requests.get(url, headers=headers, timeout=10)
            data = resp.json()
            if data.get('status') == 'success':
                return data.get('city', '').replace('市', '')
            logging.error("定位失败: %s", data.get('message', '未知错误'))
        except Exception as e:
            logging.error("定位异常: %s", str(e))
        time.sleep(180)

def schedule_getWeath():
    """定时任务调度"""
    try:
        getWeath()
    finally:
        Timer(180, schedule_getWeath).start()

def getWeath(default_city='101060101'):
    """获取天气数据核心函数"""
    city_name = get_current_city()
    area_id = default_city
    
    if city_name:
        try:
            area_id = get_area_id(city_name) or default_city
        except Exception as e:
            logging.error("获取区域ID失败: %s", str(e))
    
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
        'Referer': 'https://www.weather.com.cn/'
    }

    try:
        resp = requests.get(
            f'https://d1.weather.com.cn/sk_2d/{area_id}.html',
            headers=headers,
            timeout=15
        )
        resp.encoding = 'utf-8'
        resp.raise_for_status()
        
        weather_data = resp.content[11:].decode('utf-8')
        
        with open('/root/2.13-Ink-screen-clock/bin/weather.json', 'w', encoding='utf-8') as f:
            f.write(weather_data)
            
        logging.info("天气数据更新成功")
    except Exception as e:
        logging.error("天气更新失败: %s", str(e))

def get_area_id(city_name):
    """从city.js中检索AREAID，无限重试直到成功"""
    url = "https://j.i8tq.com/weather2020/search/city.js"
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
    }
    while True:
        try:
            resp = requests.get(url, headers=headers, timeout=10)
            resp.encoding = 'utf-8'
            city_data = json.loads(resp.text.split('=', 1)[1].rstrip(';'))
            for province in city_data.values():
                for city in province.values():
                    for district, info in city.items():
                        if info['NAMECN'] == city_name:
                            return info['AREAID']
            logging.error("未找到城市: %s", city_name)
        except Exception as e:
            logging.error("获取城市ID失败: %s", str(e))
        time.sleep(180)

if __name__ == "__main__":
    try:
        schedule_getWeath()
        while True: 
            time.sleep(1)
    except KeyboardInterrupt:
        logging.info("程序已终止")
