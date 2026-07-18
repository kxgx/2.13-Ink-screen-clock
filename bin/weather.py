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

# wttr.in 天气代码 → 中文
WEATHER_CN = {
    '113': '晴', '116': '晴间多云', '119': '多云', '122': '阴',
    '143': '雾', '176': '小雨', '179': '小雪', '182': '雨夹雪',
    '185': '冻雨', '200': '多云', '227': '暴风雪', '230': '暴风雪',
    '248': '雾', '260': '冻雾', '263': '毛毛雨', '266': '小雨',
    '281': '冻雨', '284': '冻雨', '293': '小雨', '296': '小雨',
    '299': '中雨', '302': '中雨', '305': '大雨', '308': '大雨',
    '311': '冻雨', '314': '冻雨', '317': '雨夹雪', '320': '雨夹雪',
    '323': '小雪', '326': '小雪', '329': '中雪', '332': '中雪',
    '335': '大雪', '338': '大雪', '350': '冰雹', '353': '阵雨',
    '356': '中雨', '359': '大雨', '362': '雨夹雪', '365': '雨夹雪',
    '368': '小雪', '371': '大雪', '374': '冰雹', '377': '冰雹',
    '386': '雷阵雨', '389': '雷阵雨', '392': '雷阵雪', '395': '大雪',
}
# 英文兜底映射
WEATHER_EN_CN = {
    'Sunny': '晴', 'Clear': '晴', 'Partly cloudy': '多云', 'Cloudy': '阴',
    'Overcast': '阴', 'Mist': '雾', 'Fog': '雾', 'Freezing fog': '冻雾',
    'Patchy rain possible': '小雨', 'Light drizzle': '毛毛雨',
    'Light rain': '小雨', 'Moderate rain': '中雨', 'Heavy rain': '大雨',
    'Patchy snow possible': '小雪', 'Light snow': '小雪',
    'Moderate snow': '中雪', 'Heavy snow': '大雪', 'Blizzard': '暴风雪',
    'Thunderstorm': '雷阵雨', 'Hail': '冰雹', 'Sleet': '雨夹雪',
    'Thundery outbreaks in nearby': '多云',
    'Freezing drizzle': '冻雨', 'Light rain shower': '阵雨',
    'Moderate or heavy rain shower': '中雨', 'Torrential rain shower': '大雨',
    'Light sleet showers': '雨夹雪', 'Moderate or heavy sleet showers': '雨夹雪',
    'Light snow showers': '小雪', 'Moderate or heavy snow showers': '大雪',
    'Light showers of ice pellets': '冰雹', 'Moderate or heavy showers of ice pellets': '冰雹',
}


def fetch_weather():
    """使用 wttr.in API 获取天气"""
    try:
        url = f'https://wttr.in/{DEFAULT_CITY}?format=j1'
        resp = requests.get(url, headers={'User-Agent': 'curl'}, timeout=15, proxies=NO_PROXY)
        resp.raise_for_status()
        data = resp.json()

        current = data['current_condition'][0]
        # 天气转中文：优先用 weatherCode，其次英文兜底
        code = current['weatherCode']
        weather_en = current['weatherDesc'][0]['value']
        weather_cn = WEATHER_CN.get(code) or WEATHER_EN_CN.get(weather_en, weather_en)
        now = datetime.now()
        weekdays = ['星期一','星期二','星期三','星期四','星期五','星期六','星期日']

        weather_data = {
            'cityname': CITY_CN.get(DEFAULT_CITY, DEFAULT_CITY),
            'temp': current['temp_C'],
            'weather': weather_cn,
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
