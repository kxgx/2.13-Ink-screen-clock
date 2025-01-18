#!/usr/bin/env python3
import os
import sys

# 设置文件路径
file_path = '/root/2.13-Ink-screen-clock/bin/main.py'

# 读取POST数据
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
content = sys.stdin.read(content_length)

# 保存文件
try:
    with open(file_path, 'w') as file:
        file.write(content)
    response = "<h1>文件保存成功！</h1>"
except Exception as e:
    response = "<h1>文件保存失败：{}</h1>".format(e)

# 返回响应
print("Content-Type: text/html")
print()
print(response)