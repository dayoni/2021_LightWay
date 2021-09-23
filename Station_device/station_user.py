"""
@Filename : station_user.py
@Author : Kim Na Yeon
@Date : 2021.03.
@Update : 2021.03.
@Comment : 
        -정류장 단말기의 사용자용 코드입니다.
        -별도의 설정 없이 실행 가능합니다.
        -주요기능
            · 사용자 식별
            · 예약
"""

from station_admin import *
from mfrc522 import SimpleMFRC522
from gtts import gTTS

db_reservation = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
cursor_reservation = db_reservation.cursor()

db_user = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
cursor_user = db_user.cursor()

rfid = SimpleMFRC522()

pygame.mixer.init()
pygame.mixer.music.load('/home/pi/bitgil/station/start.mp3')
pygame.mixer.music.play()
time.sleep(5)

inp_buf = ''
inp = ''


def button_callback(channel):
    global inp_buf, inp, p
    
    if GPIO.input(pin_num_0) == 0:
        print("callback_0")
        inp_buf = inp_buf + '0'
        pygame.mixer.music.load('/home/pi/bitgil/station/0.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_1) == 0:
        print("callback_1")
        inp_buf = inp_buf + '1'
        pygame.mixer.music.load('/home/pi/bitgil/station/1.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_2) == 0:
        print("callback_2")
        inp_buf = inp_buf + '2'
        pygame.mixer.music.load('/home/pi/bitgil/station/2.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_3) == 0:
        print("callback_3")
        inp_buf = inp_buf + '3'
        pygame.mixer.music.load('/home/pi/bitgil/station/3.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_4) == 0:
        print("callback_4")
        inp_buf = inp_buf + '4'
        pygame.mixer.music.load('/home/pi/bitgil/station/4.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_5) == 0:
        print("callback_5")
        inp_buf = inp_buf + '5'
        pygame.mixer.music.load('/home/pi/bitgil/station/5.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_6) == 0:
        print("callback_6")
        inp_buf = inp_buf + '6'
        pygame.mixer.music.load('/home/pi/bitgil/station/6.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_7) == 0:
        print("callback_7")
        inp_buf = inp_buf + '7'
        pygame.mixer.music.load('/home/pi/bitgil/station/7.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_8) == 0:
        print("callback_8")
        inp_buf = inp_buf + '8'
        pygame.mixer.music.load('/home/pi/bitgil/station/8.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_num_9) == 0:
        print("callback_9")
        inp_buf = inp_buf + '9'
        pygame.mixer.music.load('/home/pi/bitgil/station/9.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_char_hyphen) == 0:
        print("callback_-")
        inp_buf = inp_buf + '-'
        pygame.mixer.music.load('/home/pi/bitgil/station/hyphen.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_func_back) == 0:
        print("callback_back")
        inp_buf = inp_buf[:len(inp_buf)-1]
        pygame.mixer.music.load('/home/pi/bitgil/station/back.mp3')
        pygame.mixer.music.play()
        time.sleep(0.5)
    elif GPIO.input(pin_func_cancel) == 0:
        print("callback_cancel")
        pygame.mixer.music.load('/home/pi/bitgil/station/cancel.mp3')
        pygame.mixer.music.play()
        time.sleep(2)
        inp = 'cancel'
    elif GPIO.input(pin_func_reply) == 0:
        print("callback_replay")
        pygame.mixer.music.play()
        time.sleep(1)
    elif GPIO.input(pin_func_ok) == 0:
        print("callback_ok")
        pygame.mixer.music.load('/home/pi/bitgil/station/ok.mp3')
        pygame.mixer.music.play()
        time.sleep(2)
        inp = inp_buf
        

GPIO.add_event_detect(pin_num_0, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_1, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_2, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_3, GPIO.FALLING, callback=button_callback, bouncetime=200)    
GPIO.add_event_detect(pin_num_4, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_5, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_6, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_7, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_8, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_num_9, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_char_hyphen, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_func_back, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_func_cancel, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_func_reply, GPIO.FALLING, callback=button_callback, bouncetime=200)
GPIO.add_event_detect(pin_func_ok, GPIO.FALLING, callback=button_callback, bouncetime=200)


# STEP1. User Identification
while True:
    db_reservation.commit()
    db_user.commit()
    print("----------------------------------------")
    user_id, user_name = rfid.read()
    user_id = str(user_id)
    print("User ID :", user_id)
    pygame.mixer.music.load('/home/pi/bitgil/station/none.mp3')

    # STEP1-1. Check User DB
    sql = "SELECT * FROM user"
    cursor_user.execute(sql)
    data_user = cursor_user.fetchall()

    if len(data_user) == 0:
        user_id_flag = 0
    else:
        for i in range(len(data_user)):
            if user_id == data_user[i][0]:
                # STEP1-2. Check Reservation DB
                sql = "SELECT * FROM reserve"
                cursor_reservation.execute(sql)
                data_reservation = cursor_reservation.fetchall()

                if len(data_reservation) == 0:
                    user_id_flag = 1
                else:
                    for j in range(len(data_reservation)):
                        if user_id == data_reservation[j][0]:
                            user_id_flag = 2
                            break
                        else:
                            user_id_flag = 1
                break
            else:
                user_id_flag = 0

    # Result of User Identification
    if user_id_flag == 0:
        print("※ 등록된 사용자가 아닙니다 ※")
        pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_0.mp3')
        pygame.mixer.music.play()
        time.sleep(2)
        continue
    elif user_id_flag == 1:
        print("※ 등록된 사용자 입니다 ※")
        pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_1.mp3')
        pygame.mixer.music.play()
    elif user_id_flag == 2:
        print("※ 예약 내역이 존재합니다 (예약취소 : 1) ※")

        sql = "SELECT bus_num FROM reserve WHERE uid = '%s'" % (user_id)
        cursor_reservation.execute(sql)
        bus_num = cursor_reservation.fetchall()[0][0]

        if bus_num.find('-'):
            bus_num = bus_num.replace('-', ' 다시 ')
            bus_num = bus_num + ' 번 버스가 예약되어 있습니다. 예약 내역을 취소하시려면 1번을 누른 후 확인 버튼을 눌러주십시오.'
        tts = gTTS(bus_num, lang='ko')
        tts.save('/home/pi/bitgil/station/user_id_flag_2-1.mp3')
        
        pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_2-1.mp3')
        pygame.mixer.music.play()
        
        print(">>>")
        inp_buf = ''
        inp = ''
        while True:
            if inp == '':
                time.sleep(0.1)
                continue
            elif inp == '1':
                sql = "UPDATE reserve SET status = 2 WHERE uid = '%s'" % (user_id)
                cursor_reservation.execute(sql)
                db_reservation.commit()
        
                print("※ 예약이 취소되었습니다 ※")
                pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_2-2.mp3')
                pygame.mixer.music.play()
                break
            else:
                is_exit(inp)
                print("※ 이용해주셔서 감사합니다. ※")
                pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_2-3.mp3')
                pygame.mixer.music.play()
                break
        continue

    # STEP2. Reservation
    while True:
        print("BUS NUMBER : ")
        inp_buf = ''
        inp = ''
        while True:
            if inp == '':
                time.sleep(0.2)
                continue
            break

        if inp == "cancel":
            print("※ 이용해주셔서 감사합니다. ※")
            pygame.mixer.music.load('/home/pi/bitgil/station/user_id_flag_2-3.mp3')
            pygame.mixer.music.play()
            break
        else:
            is_exit(inp)
            # STEP2-1. Check the Bus List
            for i in range(len(bus_list)):
                if inp == bus_list[i][1]:
                    url = "http://apis.data.go.kr/6410000/busarrivalservice/getBusArrivalItem?serviceKey={}&stationId={}&routeId={}&staOrder=" \
                        .format(key, station_id, bus_list[i][0])

                    content = requests.get(url).content
                    dic = xmltodict.parse(content)

                    jsonString = json.dumps(dic['response']['msgHeader']['resultCode'], ensure_ascii=False)
                    jsonObj = json.loads(jsonString)

                    # STEP2-2. Check Operation Information
                    if jsonObj == "4":
                        reservation_flag = 2
                    else:
                        reservation_flag = 1
                    break
                else:
                    reservation_flag = 0

        # Result of Reservation
        if reservation_flag == 0:
            print("※ 해당 정류장에 정차하는 버스가 아닙니다 ※\n")
            pygame.mixer.music.load('/home/pi/bitgil/station/reservation_flag_0.mp3')
            pygame.mixer.music.play()
        elif reservation_flag == 1:
            jsonString = json.dumps(dic['response']['msgBody']['busArrivalItem'], ensure_ascii=False)
            jsonObj = json.loads(jsonString)

            license_num = jsonObj['plateNo1']
            arrival_time = jsonObj['predictTime1']

            sql = "INSERT INTO reserve(uid, station_id, bus_num, buid, status) VALUES('%s', '%s', '%s', '%s', 1);" % (user_id, station_id, inp, license_num)
            cursor_reservation.execute(sql)
            db_reservation.commit()
            
            if inp.find('-'):
                inp = inp.replace('-', ' 다시 ')
            
            inp = inp + ' 번 버스 예약이 완료되었습니다.' + arrival_time + '분 후 버스 도착 예정입니다. 이용해주셔서 감사합니다.'
            tts = gTTS(inp, lang='ko')
            tts.save('/home/pi/bitgil/station/reservation_flag_1.mp3')

            print("※ 예약이 완료되었습니다 ※")
            pygame.mixer.music.load('/home/pi/bitgil/station/reservation_flag_1.mp3')
            pygame.mixer.music.play()
            break
        elif reservation_flag == 2:
            print("※ 차고지 대기중입니다 ※")
            pygame.mixer.music.load('/home/pi/bitgil/station/reservation_flag_2.mp3')
            pygame.mixer.music.play()
            break
