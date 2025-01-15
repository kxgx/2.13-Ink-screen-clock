#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os,sys,re,json,time,datetime  #引入系统相关库
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
    sys.path.append(libdir)#将引入文件添加到环境变量
def getWeath(city='101060111'): #天气函数,下载json天气至本地
     headers = {
         'User-Agent':'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36',
         'Referer':'http://www.weather.com.cn/'
     }
     response = requests.get('http://d1.weather.com.cn/sk_2d/'+city+'.html',headers=headers)
     response.encoding = 'utf-8'
     Weath=response.text[11:]
     fileHandle=open('weather.json','w')
     fileHandle.write(str(Weath))
     fileHandle.close()
     Timer(180, getWeath).start() #定时器函数,间隔三分钟下载文件至本地
     print("天气文件更新")
try:
    getWeath()#天气获取函数开始运行
except IOError as e:
    logging.info(e)
except KeyboardInterrupt:
    logging.info("Keyboard interrupt detected, exiting gracefully.")

except Exception as e:
    logging.error("An unexpected error occurred: %s", e)
    exit()

# 脚本正常结束后的清理操作
exit()
