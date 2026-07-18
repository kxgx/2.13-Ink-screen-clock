#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os, json, time, logging, requests
from datetime import datetime
from threading import Timer

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# 绕过本地代理
NO_PROXY = {'http': None, 'https': None}

# 默认城市（长春 wttr.in 格式），可改为你的城市拼音
DEFAULT_CITY = 'Changchun'
# 城市中文名映射
CITY_CN = {'Changchun': '长春', 'Beijing': '北京', 'Shanghai': '上海',
           'Shenzhen': '深圳', 'Guangzhou': '广州', 'Chengdu': '成都',
           'Hangzhou': '杭州', 'Nanjing': '南京', 'Wuhan': '武汉',
           'Chongqing': '重庆', 'Tianjin': '天津', 'Suzhou': '苏州'}

def fetch_weather():
    """使用 wttr.in API 获取天气"""
    try:
        url = f'https://wttr.in/{DEFAULT_CITY}?format=j1'
        resp = requests.get(url, headers={'User-Agent': 'curl'}, timeout=15, proxies=NO_PROXY)
        resp.raise_for_status()
        data = resp.json()

        current = data['current_condition'][0]
        now = datetime.now()
        weekdays = ['星期一','星期二','星期三','星期四','星期五','星期六','星期日']

        weather_data = {
            'cityname': CITY_CN.get(DEFAULT_CITY, DEFAULT_CITY),
            'temp': current['temp_C'],
            'weather': current['weatherDesc'][0]['value'],
            'WD': current['winddir16Point'],
            'WS': current['windspeedKmph'] + 'km/h',
            'SD': current['humidity'] + '%',
            'time': now.strftime('%H:%M'),
            'date': f"{now.month}月{now.day}日({weekdays[now.weekday()]})",
        }

        script_dir = os.path.dirname(os.path.abspath(__file__))
        json_path = os.path.join(script_dir, 'weather.json')
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(weather_data, f, ensure_ascii=False)
        logging.info("天气数据更新成功: %s %s°C %s", weather_data['cityname'],
                     weather_data['temp'], weather_data['weather'])
        return True
    except Exception as e:
        logging.error("天气获取失败: %s", str(e))
        return False

def schedule():
    fetch_weather()
    Timer(1800, schedule).start()

if __name__ == "__main__":
    try:
        schedule()
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logging.info("程序已终止")
