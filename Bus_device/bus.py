"""
@Filename : bus.py
@Author : Kim Na Yeon
@Date : 2021.04.
@Update : 2021.05.02 (폰트 및 위젯 위치 조정)
@Comment : 
        -버스 단말기의 코드입니다.
        -코드 실행 전, 단말기가 설치된 버스 및 DB 서버의 정보를 설정해주세요.
        -주요기능
            · 예약 안내
            · 실시간 위치 전송
            · 승차 처리
"""

from mfrc522 import SimpleMFRC522
import RPi.GPIO as GPIO
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.uic import *
import pymysql, requests, xmltodict, json, time, sys, os, serial


# BUS_SET UP
key = ""

license_num = ""
set_bus_list = ['', '']
set_route_list = []
station_list = []
########################################


# DB_SET UP
mysql_ip = ""
mysql_user = ""
mysql_passwd = ""

db_name = "lightway"
########################################


# GPIO_SET UP
pin_buzzer = 26

GPIO.setmode(GPIO.BCM)
GPIO.setup(pin_buzzer, GPIO.OUT)

rfid = SimpleMFRC522()
gps = serial.Serial('/dev/ttyAMA0')
form_class = loadUiType("/home/pi/bitgil/bus/bus.ui")[0]
########################################


def dms_to_xy(dms):
    dms = str(dms)

    d = int(dms[:-8])
    m = float(dms[-8:-3]) / 60
    s = float(dms[-6:]) * 0.0001 * 60 / 3600

    xy = d + m + s
    return xy


class Rfid(QThread):
    def __init__(self):
        super().__init__()

    def run(self):
        db_rfid = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
        cursor_rfid = db_rfid.cursor()

        while True:
            user_id, user_name = rfid.read()

            GPIO.output(pin_buzzer, True)
            time.sleep(0.15)
            GPIO.output(pin_buzzer, False)

            db_rfid.commit()
            sql = "SELECT * FROM reserve WHERE uid = '%s' and status = 1" % (user_id)
            cursor_rfid.execute(sql)
            data_rfid = cursor_rfid.fetchall()

            if len(data_rfid):
                sql = "UPDATE reserve SET status = 3 WHERE uid = '%s'" % (user_id)
                cursor_rfid.execute(sql)
                db_rfid.commit()

                print("User ID : ", user_id)
                print("\n※※※※※탑승 완료 ※※※※※")

                time.sleep(0.5)


class Reserv(QThread):
    reserv_sig = pyqtSignal(list)
    tag_sig = pyqtSignal(str)

    def __init__(self):
        super().__init__()

    def run(self):
        global station_list
        
        db_gps = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
        cursor_gps = db_gps.cursor()

        db_reservation = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
        cursor_reservation = db_reservation.cursor()
        
        reserv_list = []
        
        while True:
            db_reservation.commit()
            print("------------------------------------------------------------")

            # GPS_RX
            while True:
                if gps.readable():
                    data_gps = str(gps.readline())

                    if data_gps[3:8] == 'GPGLL':
                        bus_lat = dms_to_xy(data_gps[9:19])
                        bus_lng = dms_to_xy(data_gps[22:33])
                        break

            # GPS_TX
            sql = "UPDATE busGPS SET w = %lf and h = %lf WHERE buid = '%s'" % (bus_lat, bus_lng, license_num)
            cursor_gps.execute(sql)
            db_gps.commit()

            # status = 1
            sql = "SELECT * FROM reserve WHERE buid = '%s' and status = 1" % (license_num)
            cursor_reservation.execute(sql)
            data_reserv = cursor_reservation.fetchall()

            for i in range(len(data_reserv)):
                station_flag = False

                for j in range(len(reserv_list)):
                    if data_reserv[i][1] in reserv_list[j]:
                        station_flag = True

                        if data_reserv[i][0] in reserv_list[j]:
                            break
                        else:
                            reserv_list[j].append(data_reserv[i][0])
                            break

                if not station_flag:
                    seq_flag = False

                    for k in range(len(station_list)):
                        if data_reserv[i][1] in station_list[k]:
                            new_reserv = [station_list[k][0], station_list[k][1], station_list[k][2], data_reserv[i][0]]
                            print(new_reserv)
                            break

                    if len(reserv_list) == 0:
                        reserv_list.insert(0, new_reserv)
                    else:
                        for j in range(len(reserv_list)):
                            if new_reserv[0] < reserv_list[j][0]:
                                reserv_list.insert(j, new_reserv)
                                seq_flag = True
                                break

                        if not seq_flag:
                            reserv_list.append(new_reserv)

            # status = 2
            sql = "SELECT * FROM reserve WHERE buid = '%s' and status = 2" % (license_num)
            cursor_reservation.execute(sql)
            data_reserv = cursor_reservation.fetchall()

            for i in range(len(data_reserv)):
                for j in range(len(reserv_list)):
                    if data_reserv[i][0] in reserv_list[j]:
                        if len(reserv_list[j]) - 3 == 1:
                            del reserv_list[j]
                        else:
                            reserv_list[j].remove(data_reserv[i][0])
                        break

            # status = 3
            sql = "SELECT * FROM reserve WHERE buid = '%s' and status = 3" % (license_num)
            cursor_reservation.execute(sql)
            data_reserv = cursor_reservation.fetchall()

            for i in range(len(data_reserv)):
                for j in range(len(reserv_list)):
                    if data_reserv[i][0] in reserv_list[j]:
                        if len(reserv_list[j]) - 3 == 1:
                            self.tag_sig.emit(reserv_list[j][2])
                            del reserv_list[j]
                        else:
                            reserv_list[j].remove(data_reserv[i][0])
                        break

            # Print a reserv_list
            for i in range(len(reserv_list)):
                for j in range(len(reserv_list[i])):
                    print(reserv_list[i][j], end=" ")
                print("\n")
            print("\n※ DB 조회 완료 ※")

            self.reserv_sig.emit(reserv_list)

            time.sleep(0.5)


class WindowClass(QMainWindow, form_class):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        self.dialog_setting = QDialog(self)
        self.dialog_tag = QDialog(self)
        self.timer = QTimer(self)

        # Dialog_setting - Set Bus Information
        self.dialog_setting.resize(430, 430)
        self.dialog_setting.setStyleSheet("background-color : rgb(230, 230, 230);")
        self.dialog_setting.setWindowTitle(" ")
        
        label_s1 = QLabel(self.dialog_setting)
        label_s1.resize(160, 110)
        label_s1.move(135, 40)
        label_s1.setPixmap(QPixmap("/home/pi/bitgil/bus/logo_down.png"))
        label_s1.setScaledContents(True)
        
        label_s2 = QLabel("차량", self.dialog_setting)
        label_s2.move(50, 173)
        label_s2.setFont(QFont("GyeonggiTitleL", 16))
        
        label_s3 = QLabel(license_num, self.dialog_setting)
        label_s3.move(115, 175)
        label_s3.setFont(QFont("GyeonggiTitleL", 15))
        
        label_s4 = QLabel("버스", self.dialog_setting)
        label_s4.move(50, 218)
        label_s4.setFont(QFont("GyeonggiTitleL", 16))
        
        self.combo_bnum = QComboBox(self.dialog_setting)
        self.combo_bnum.resize(270, 38)
        self.combo_bnum.move(110, 210)
        self.combo_bnum.setFont(QFont("GyeonggiTitleL", 13))
        for i in range(len(set_bus_list)):
            self.combo_bnum.addItem(set_bus_list[i])
        self.combo_bnum.currentIndexChanged.connect(self.func_set_region)
        
        label_s5 = QLabel("지역", self.dialog_setting)
        label_s5.move(50, 263)
        label_s5.setFont(QFont("GyeonggiTitleL", 16))
        
        self.combo_region = QComboBox(self.dialog_setting)
        self.combo_region.resize(270, 38)
        self.combo_region.move(110, 255)
        self.combo_region.setFont(QFont("GyeonggiTitleL", 13))
        self.combo_region.addItem('')
        self.combo_region.currentIndexChanged.connect(self.func_set_route)
        
        label_s6 = QLabel("경로", self.dialog_setting)
        label_s6.move(50, 308)
        label_s6.setFont(QFont("GyeonggiTitleL", 16))

        self.combo_route = QComboBox(self.dialog_setting)
        self.combo_route.resize(270, 38)
        self.combo_route.move(110, 300)
        self.combo_route.setFont(QFont("GyeonggiTitleL", 13))

        self.set_button = QPushButton("운행 시작", self.dialog_setting)
        self.set_button.resize(130, 45)
        self.set_button.move(145, 360)
        self.set_button.setFont(QFont("GyeonggiTitleM", 15))
        self.set_button.clicked.connect(self.func_set_button)
        
        self.dialog_setting.show()
        self.dialog_setting.exec_()

        # Window - Bus Number
        self.label_bnum.setText(self.combo_bnum.currentText())
        self.label_bnum.setFont(QFont("NanumSquare_ac", 80, QFont.Bold))
        self.label_bnum.setAlignment(Qt.AlignCenter)

        # Window - Logo
        self.label_logo.setPixmap(QPixmap("/home/pi/bitgil/bus/logo_up.png"))
        self.label_logo.setScaledContents(True)
        
        # Window - Exit
        self.button_exit.clicked.connect(self.func_exit)


    def func_set_region(self):
        global set_route_list

        bus_num = self.combo_bnum.currentText()
        url = "http://apis.data.go.kr/6410000/busrouteservice/getBusRouteList?serviceKey={}&keyword={}"\
            .format(key, bus_num)

        content = requests.get(url).content
        dic = xmltodict.parse(content)

        jsonString = json.dumps(dic['response']['msgBody']['busRouteList'], ensure_ascii=False)
        jsonObj = json.loads(jsonString)

        if type(jsonObj) == dict:
            jsonObj = [jsonObj]

        set_region_list = []
        set_route_list = []
        for i in range(len(jsonObj)):
            if str(bus_num) == jsonObj[i]['routeName']:
                set_region_list.append(jsonObj[i]['regionName'])
                set_route_list.append([jsonObj[i]['regionName'], jsonObj[i]['routeId']])

        self.combo_region.clear()
        self.combo_region.addItems(set_region_list)


    def func_set_route(self):
        for i in range(len(set_route_list)):
            if self.combo_region.currentText() == set_route_list[i][0]:
                self.combo_route.clear()
                self.combo_route.addItem(set_route_list[i][1])
                break


    def func_set_button(self):
        global station_list
        route_id = self.combo_route.currentText()

        # Make a station list
        url = "http://apis.data.go.kr/6410000/busrouteservice/getBusRouteStationList?serviceKey={}&routeId={}"\
            .format(key, route_id)

        content = requests.get(url).content
        dic = xmltodict.parse(content)

        jsonString = json.dumps(dic['response']['msgBody']['busRouteStationList'], ensure_ascii=False)
        jsonObj = json.loads(jsonString)

        station_list = []
        for i in range(len(jsonObj)):
            station_list.append([int(jsonObj[i]['stationSeq']), jsonObj[i]['stationId'], jsonObj[i]['stationName']])

        # RFID
        self.rfid = Rfid()
        self.rfid.start()

        # Reserv
        self.reserv = Reserv()
        self.reserv.reserv_sig.connect(self.update_reserv)
        self.reserv.tag_sig.connect(self.update_tag)
        self.reserv.start()

        self.dialog_setting.close()


    def update_reserv(self, data):
        sn_font = "GyeonggiTitleM"
        s_size = 28
        n_size = 30
        
        self.label_s0.setFont(QFont(sn_font, s_size))
        self.label_s1.setFont(QFont(sn_font, s_size))
        self.label_s2.setFont(QFont(sn_font, s_size))
        self.label_s3.setFont(QFont(sn_font, s_size))
        self.label_n0.setFont(QFont(sn_font, n_size))
        self.label_n1.setFont(QFont(sn_font, n_size))
        self.label_n2.setFont(QFont(sn_font, n_size))
        self.label_n3.setFont(QFont(sn_font, n_size))

        self.label_s0.setText("")
        self.label_n0.setText("")
        self.label_s1.setText("")
        self.label_n1.setText("")
        self.label_s2.setText("")
        self.label_n2.setText("")
        self.label_s3.setText("")
        self.label_n3.setText("")

        if len(data) >= 1:
            self.label_s0.setText(data[0][2])
            self.label_n0.setText(str(len(data[0]) - 3))

        if len(data) >= 2:
            self.label_s1.setText(data[1][2])
            self.label_n1.setText(str(len(data[1]) - 3))

        if len(data) >= 3:
            self.label_s2.setText(data[2][2])
            self.label_n2.setText(str(len(data[2]) - 3))

        if len(data) >= 4:
            self.label_s3.setText(data[3][2])
            self.label_n3.setText(str(len(data[3]) - 3))


    def dialog_tag_close(self):
        self.dialog_tag.close()


    def update_tag(self, station_name):
        self.dialog_tag.resize(250, 250)
        self.dialog_tag.move(387, 130)
        self.dialog_tag.setStyleSheet("background-color : rgb(230, 230, 230);")
        self.dialog_tag.setWindowTitle(" ")
        
        label_t1 = QLabel(self.dialog_tag)
        label_t1.resize(65, 65)
        label_t1.move(92.5, 35)
        label_t1.setPixmap(QPixmap("/home/pi/bitgil/bus/icon_success.png"))
        label_t1.setScaledContents(True)
        
        label_t2 = QLabel("탑 승 완 료", self.dialog_tag)
        label_t2.resize(250, 40)
        label_t2.move(0, 120)
        label_t2.setFont(QFont("GyeonggiTitleM", 28))
        label_t2.setAlignment(Qt.AlignCenter)
        
        label_t3 = QLabel("- - - 정류장 - - -", self.dialog_tag)
        label_t3.resize(250, 20)
        label_t3.move(0, 170)
        label_t3.setFont(QFont("GyeonggiTitleL", 13, QFont.Medium))
        label_t3.setAlignment(Qt.AlignCenter)
        
        label_t4 = QLabel("%s" % station_name, self.dialog_tag)
        label_t4.resize(250, 20)
        label_t4.move(0, 195)
        label_t4.setFont(QFont("GyeonggiTitleL", 13))
        label_t4.setAlignment(Qt.AlignCenter)

        self.dialog_tag.show()

        self.timer.start(4000)
        self.timer.timeout.connect(self.dialog_tag_close)
    
    
    def func_exit(self):
        GPIO.cleanup()
        exit()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    myWindow = WindowClass()
    myWindow.show()
    app.exec_()
