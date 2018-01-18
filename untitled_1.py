# Untitled - By: Deer - 周三 11月 29 2017

import sensor
import time
from pyb import LED
from pyb import UART


sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)#设定采集图像回灰度
sensor.set_framesize(sensor.QVGA)#设定画面大小
sensor.skip_frames(10)

grayscale_thres = (110, 255)#设定二值化阈值
thing_thres = (0,70)#设定查找颜色范围

red_led   = LED(1)#端口控制 LED亮500ms
red_led.on()
time.sleep(500)
red_led.off()

uart = UART(3, 9600, timeout_char = 1000)#串口控制，第一个参数为默认，第二个是波特率，第三个是超时设定



def find_max(blobs):#函数查找最大的色块
    max_size=0
    for blob in blobs:
        if blob.pixels() > max_size:
            max_blob=blob
            max_size = blob.pixels()
    return max_blob


while(True):
    for i in range(20):
        img = sensor.snapshot()
        img.binary([grayscale_thres])#二值化处理
        img.dilate(2)#图像腐蚀处理，消除边缘毛刺

    blobs = img.find_blobs([thing_thres])


    if blobs:
        max_blob = find_max(blobs)
        output_str="[%d]" % (max_blob[4])
        print(output_str)
        uart.write(output_str+'\n')#串口输出

