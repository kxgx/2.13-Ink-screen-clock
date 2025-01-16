#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys
import os
import logging  #日志库
import subprocess
from threading import Timer
import requests

# 设置 picdir 和 libdir 路径
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir): 
    sys.path.append(libdir)#将引入文件添加到环境变量
    from waveshare_epd import epd2in13_V4  #引入墨水屏驱动文件

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

def clear_screen():
    try:
        logging.info("启动电子纸清屏脚本")
        
        # 初始化并创建一个 epd2in13_V4 对象
        epd = epd2in13_V4.EPD()
        
        # 初始化屏幕
        logging.info("初始化电子纸显示屏")
        epd.init()
        
        # 清除屏幕
        logging.info("清除电子纸显示屏内容")
        epd.Clear(0xFF)
        
        # 等待一段时间，确保清除操作完成
        #time.sleep(1)
        
        # 将屏幕置于睡眠模式
        logging.info("将电子纸显示屏置入睡眠状态")
        epd.sleep()
        epd2in13_V4.epdconfig.module_exit(cleanup=True)
        exit()
        
        logging.info("电子纸显示屏清屏成功")
        
    except IOError as e:
        logging.info(e)
        
    except KeyboardInterrupt:
        logging.info("检测到键盘中断，正在退出")
        epd2in13_V4.epdconfig.module_exit(cleanup=True)
        exit()

if __name__ == '__main__':
    clear_screen()
