# -*- coding:utf-8 -*-
import sys
import io
import argparse
import time
import cv2
from pyzbar import pyzbar
# import tensorflow as tf
import mahjong_common as mjc
import mahjong_loader as mjl
import tensorflow as tf 



import re

def replace_and_find_position(str_1, tsumo, result_str):
    # 使用正则表达式匹配所有项
    items = re.findall(r'\d+[A-Za-z]|[\u4e00-\u9fff]', result_str)
    
    # 检查输入项是否在列表中
    if str_1 in items:
        # 找到第一个匹配项的索引
        position = items.index(str_1) + 1  # 因为索引从0开始，所以加1
        # 从列表中删除第一个匹配项，并插入tsumo
        items[position - 1:position] = [tsumo]  # 替换str_1为tsumo
        # 将更新后的列表重新组合成字符串
        updated_result_str = ''.join(items)
        return updated_result_str, position
    else:
        # 如果项不在列表中，返回原始字符串和位置0
        return result_str, 0

# 假设 result_str 是一个已经定义好的字符串
result_str = "3p3p4s4s北北東東2s2s2m2m9s"

# 输入的单个项和tsumo项
str_1 = "3p"
tsumo = "5p"

# 调用函数
updated_result_str, position = replace_and_find_position(str_1, tsumo, result_str)

# 打印结果
print("更新后的result_str:", updated_result_str)
print("被删除项是第几项:", position)