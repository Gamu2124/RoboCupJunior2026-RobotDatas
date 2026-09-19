import sensor
import image
from machine import UART
import time
from fpioa_manager import fm
from Maix import GPIO
import math

sensor.reset(dual_buff=True)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)
sensor.run(1)

uart1 = UART(UART.UART1, 115200, 8, 1, 0, timeout=1000, read_buf_len=4096)
fm.register(35, fm.fpioa.UART1_TX, force=True)
fm.register(34, fm.fpioa.UART1_RX, force=True)

clock = time.clock()

ball_thresholds   = [(0, 100, 127, 12, 37, 127)]
blue_threshold    = [(100, 0, 127, -128, -128, -26)]
yellow_threshold  = [(93, 75, 127, -27, 0, 127)]
red_threshold     = [(0, 0, -128, -128, -128, 127)]

shoot_side_blue = None
SWITCH_MARGIN = 10
WIDTH = 160
HEIGHT = 120

sensor.set_vflip(0)
sensor.set_hmirror(0)
sensor.set_contrast(-3)
sensor.set_brightness(-3)
sensor.set_saturation(0)
sensor.skip_frames(time=250)
sensor.set_auto_gain(True)
sensor.set_auto_exposure(True)
sensor.set_auto_whitebal(True)
time.sleep(1.5)

def map_value(value, in_min, in_max, out_min, out_max):
    if value is None: return 255
    if value < 0: return 0
    mapped = int((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
    return min(max(mapped, out_min), out_max)

def clamp_byte(v):
    val = max(0, min(250, int(v)))
    if val == 0: val = 254
    return val

def get_largest_two(blobs):
    largest = None
    second = None
    if not blobs: return None, None
    for b in blobs:
        if largest is None or b.pixels() > largest.pixels():
            second = largest
            largest = b
        elif second is None or b.pixels() > second.pixels():
            second = b
    return largest, second

def avoid_enemy_in_blob(goal_blob, enemy_blobs):
    gx, gy, gw, gh = goal_blob.rect()
    target_x = goal_blob.cx()

    for eb in enemy_blobs:
        if (gx < eb.cx() < gx + gw) and (gy < eb.cy() < gy + gh):
            if eb.cx() < goal_blob.cx():
                target_x = gx + (gw * 0.8)
            else:
                target_x = gx + (gw * 0.2)
            break
    return target_x

def get_largest_blob_info(blobs):
    if not blobs:
        return None, 0, None
    largest = max(blobs, key=lambda b: b.pixels())
    return largest.cx(), largest.w(), largest.cy()

while True:
    clock.tick()
    img = sensor.snapshot()
    img.gamma_corr(gamma=1.0, contrast=1.2, brightness=0)
    img.draw_rectangle((0, 0, 170, 23), color=(0, 0, 0), fill=True)
    blobs_ball   = img.find_blobs(ball_thresholds, pixels_threshold=3, area_threshold=3)
    blobs_blue   = img.find_blobs(blue_threshold, pixels_threshold=100)
    blobs_yellow = img.find_blobs(yellow_threshold, pixels_threshold=100)

    ball_x, ball_w, ball_y = get_largest_blob_info(blobs_ball)
    if ball_x is not None and ball_w < 150 and ball_y >= 60:
        img.draw_circle(int(ball_x), int(ball_y), 5, color=(0, 0, 0))
    else:
        ball_x, ball_y = None, None

    blue_x, blue_w, blue_y = get_largest_blob_info(blobs_blue)
    if blue_x is not None and blue_y < 100:
        img.draw_rectangle((int(blue_x) - 20, int(blue_y) - 20, 40, 40), color=(0, 0, 255))
        img.draw_cross(blue_x, blue_y, color=(255, 255, 255), size=5)
    else:
        blue_x, blue_w, blue_y = None, 0, None

    yellow_x, yellow_w, yellow_y = get_largest_blob_info(blobs_yellow)
    if yellow_x is not None and yellow_y < 100:
        img.draw_rectangle((int(yellow_x) - 20, int(yellow_y) - 20, 40, 40), color=(255, 255, 0))
        img.draw_cross(yellow_x, yellow_y, color=(255, 255, 255), size=5)
    else:
        yellow_x, yellow_w, yellow_y = None, 0, None

    m_ball_x   = clamp_byte(map_value(ball_x, 0, img.width(), 0, 250))
    m_ball_y   = clamp_byte(map_value(ball_y, 0, img.height(), 0, 250))

    m_blue_x   = clamp_byte(map_value(blue_x, 0, img.width(), 0, 250))
    m_blue_w   = clamp_byte(map_value(blue_w, 0, img.width(), 0, 250))

    m_yellow_x = clamp_byte(map_value(yellow_x, 0, img.width(), 0, 250))
    m_yellow_w = clamp_byte(map_value(yellow_w, 0, img.width(), 0, 250))
    data = bytes([255, m_ball_x, m_ball_y, m_blue_x, m_blue_w, m_yellow_x, m_yellow_w, 0])
    uart1.write(data)
