"""
@Filename : station_admin.py
@Author : Kim Na Yeon
@Date : 2021.03.
@Update : 2021.03.
@Comment :
        -정류장 단말기의 관리자용 코드입니다.
        -코드 실행 전, 단말기가 설치된 정류장 및  DB 서버의 정보를 설정해주세요.
        -주요기능
            · 버스 리스트 생성
"""

import RPi.GPIO as GPIO
import pymysql, requests, xmltodict, json, pygame, time, os


# STATION_SET UP
key = ""

city_name = ""
station_num = 
########################################


# ADMIN-CODE_SET UP
admin_exit = "--0"
admin_shutdown = "--1"
admin_reboot = "--2"
########################################


# DB_SET UP
mysql_ip = ""
mysql_user = ""
mysql_passwd = ""

db_name = "lightway"
########################################


# GPIO_SET UP
pin_num_0 = 4
pin_num_1 = 17
pin_num_2 = 18
pin_num_3 = 27
pin_num_4 = 22
pin_num_5 = 23
pin_num_6 = 24
pin_num_7 = 5
pin_num_8 = 6
pin_num_9 = 12
pin_char_hyphen = 13
pin_func_back = 19
pin_func_cancel = 16
pin_func_reply = 26
pin_func_ok = 20

GPIO.setmode(GPIO.BCM)
GPIO.setup(pin_num_0, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_1, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_2, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_3, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_4, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_5, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_6, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_7, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_8, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_num_9, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_char_hyphen, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_func_back, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_func_cancel, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_func_reply, GPIO.IN, GPIO.PUD_UP)
GPIO.setup(pin_func_ok, GPIO.IN, GPIO.PUD_UP)

pygame.mixer.init()
########################################


# STEP1-1. Make a city code
url = "http://openapi.tago.go.kr/openapi/service/BusSttnInfoInqireService/getCtyCodeList?serviceKey={}"\
    .format(key)

content = requests.get(url).content
dic = xmltodict.parse(content)

jsonString = json.dumps(dic['response']['body']['items']['item'], ensure_ascii=False)
jsonObj = json.loads(jsonString)

for i in range(len(jsonObj)):
    if city_name == jsonObj[i]['cityname']:
        city_code = jsonObj[i]['citycode']
        break


# STEP1-2. Make a station ID
url = "http://openapi.tago.go.kr/openapi/service/BusSttnInfoInqireService/getSttnNoList?serviceKey={}&cityCode={}&nodeNm=&nodeNo={}&numOfRows=10&pageNo=1"\
    .format(key, city_code, station_num)

content = requests.get(url).content
dic = xmltodict.parse(content)

jsonString = json.dumps(dic['response']['body']['items']['item'], ensure_ascii=False)
jsonObj = json.loads(jsonString)

station_id = jsonObj['nodeid'][3:]


# STEP1-3. Make a list of buses
url = "http://apis.data.go.kr/6410000/busstationservice/getBusStationViaRouteList?serviceKey={}&stationId={}"\
    .format(key, station_id)

content = requests.get(url).content
dic = xmltodict.parse(content)

jsonString = json.dumps(dic['response']['msgBody']['busRouteList'], ensure_ascii=False)
jsonObj = json.loads(jsonString)

if type(jsonObj) == dict:
    jsonObj = [jsonObj]

bus_list = []
for i in range(len(jsonObj)):
    bus_list.append([jsonObj[i]['routeId'], jsonObj[i]['routeName']])


# Admin Mode
def is_exit(inp):
    if inp == admin_exit:
        print("※ 프로그램 종료 ※")
        pygame.mixer.music.load('/home/pi/bitgil/station/exit.mp3')
        pygame.mixer.music.play()
        time.sleep(3)
        GPIO.cleanup()
        exit()
    elif inp == admin_shutdown:
        print("※ 시스템 종료 ※")
        pygame.mixer.music.load('/home/pi/bitgil/station/shutdown.mp3')
        pygame.mixer.music.play()
        time.sleep(3)
        GPIO.cleanup()
        os.system("sudo shutdown -h now")
    elif inp == admin_reboot:
        print("※ 시스템 재부팅 ※")
        pygame.mixer.music.load('/home/pi/bitgil/station/reboot.mp3')
        pygame.mixer.music.play()
        time.sleep(3)
        GPIO.cleanup()
        os.system("sudo reboot")