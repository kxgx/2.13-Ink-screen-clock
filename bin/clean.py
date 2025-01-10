#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys
import os
from waveshare_epd import epd2in13_V4  #引入墨水屏驱动文件
import logging

# 设置 picdir 和 libdir 路径
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)

# 配置日志
logging.basicConfig(level=logging.DEBUG)

def clear_screen():
    try:
        logging.info("Starting e-Paper clear screen script")
        
        # 初始化并创建一个 epd2in13_V4 对象
        epd = epd2in13_V4.EPD()
        
        # 初始化屏幕
        logging.info("Initializing e-Paper display")
        epd.init()
        
        # 清除屏幕
        logging.info("Clearing e-Paper display")
        epd.Clear(0xFF)
        
        # 等待一段时间，确保清除操作完成
        time.sleep(1)
        
        # 将屏幕置于睡眠模式
        logging.info("Putting e-Paper display to sleep")
        epd.sleep()
        
        logging.info("e-Paper display cleared successfully")
        
    except IOError as e:
        logging.info(e)
        
    except KeyboardInterrupt:
        logging.info("Ctrl+C pressed, exiting")
        epd2in13_V4.epdconfig.module_exit(cleanup=True)
        exit()

if __name__ == '__main__':
    clear_screen()
