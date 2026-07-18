#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os
import json
import time
import logging
import requests
import random
import socket
from functools import wraps
from threading import Timer

# 绕过本地代理（系统可能设置了无效的 HTTP_PROXY=127.0.0.1:80）
NO_PROXY = {'http': None, 'https': None}

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

def check_network_connection():
    """检查网络连接状态"""
    try:
        socket.create_connection(("223.5.5.5", 53), timeout=5)
        return True
    except OSError:
        return False

def get_ip():
    """获取当前公网IP，绕过本地代理"""
    urls = [
        "https://api.ipify.org?format=json",
        "https://ifconfig.me/all.json",
        "https://ip.cn/api/index?ip=&type=0",
    ]
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
    }
    NO_PROXY = {'http': None, 'https': None}
    for url in urls:
        try:
            resp = requests.get(url, headers=headers, timeout=10, proxies=NO_PROXY)
            resp.raise_for_status()
            data = resp.json()
            ip = data.get('ip') or data.get('ip_addr')
            if ip:
                return ip
        except Exception:
            continue
    logging.error("获取IP失败")
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
            resp = requests.get(url, headers=headers, timeout=10, proxies=NO_PROXY)
            data = resp.json()
            if data.get('status') == 'success':
                return data.get('city', '').replace('市', '')
            logging.error("定位失败: %s", data.get('message', '未知错误'))
        except Exception as e:
            logging.error("定位异常: %s", str(e))
        time.sleep(180)

def schedule_getWeath():
    """改进的定时任务调度"""
    retry_count = 0
    max_retries = 3
    
    while retry_count < max_retries:
        try:
            getWeath()
            break
        except Exception as e:
            retry_count += 1
            wait_time = min(300, retry_count * 60)  # 指数退避
            logging.error(f"天气更新失败({retry_count}/{max_retries}): {str(e)}")
            time.sleep(wait_time)
    
    # 无论成功与否，都安排下一次执行
    Timer(1800, schedule_getWeath).start()  # 30分钟间隔

def getWeath(default_city='101060101'):
    """改进的天气获取函数"""
    # 1. 检查网络连接
    if not check_network_connection():
        logging.error("网络不可用，跳过天气更新")
        return

    # 2. 获取城市信息
    city_name = None
    try:
        city_name = get_current_city()
    except Exception as e:
        logging.error(f"获取城市失败: {str(e)}")
    
    # 3. 确定区域ID
    area_id = default_city
    if city_name:
        try:
            found_id = get_area_id(city_name)
            if found_id:
                area_id = found_id
                logging.info(f"使用城市区域ID: {area_id} ({city_name})")
        except Exception as e:
            logging.error(f"获取区域ID失败: {str(e)}")

    # 4. 获取天气数据
    weather_apis = [
        f'https://d1.weather.com.cn/sk_2d/{area_id}.html',
        f'https://www.weather.com.cn/weather1d/{area_id}.shtml'
    ]
    
    for api_url in weather_apis:
        try:
            resp = requests.get(
                api_url,
                headers={'User-Agent': 'Mozilla/5.0'},
                timeout=15,
                proxies=NO_PROXY
            )
            resp.raise_for_status()
            
            # 处理不同API的响应格式
            if 'sk_2d' in api_url:
                weather_data = resp.content[11:].decode('utf-8')
            else:
                weather_data = parse_html_weather(resp.text)
            
            # 验证数据有效性
            if validate_weather_data(weather_data):
                script_dir = os.path.dirname(os.path.abspath(__file__))
                json_path = os.path.join(script_dir, 'weather.json')
                with open(json_path, 'w') as f:
                    json.dump(weather_data, f)
                logging.info("天气数据更新成功")
                return
                
        except Exception as e:
            logging.warning(f"天气API {api_url} 失败: {str(e)}")
            continue
    
    logging.error("所有天气API尝试失败")

def parse_html_weather(html):
    """简单解析天气HTML页面，提取天气数据"""
    try:
        from html.parser import HTMLParser
        import re
        
        # 尝试用正则提取JSON数据
        match = re.search(r'var\s+dataSK\s*=\s*(\{.+?\});', html, re.DOTALL)
        if match:
            return json.loads(match.group(1))
    except Exception:
        pass
    return {}

def validate_weather_data(data):
    """验证天气数据是否有效"""
    if not data or not isinstance(data, dict):
        return False
    return 'temp' in data and 'cityname' in data

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
