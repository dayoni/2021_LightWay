"""
@Filename : log.py
@Author : Kim Na Yeon
@Date : 2021.05.
@Update : 2021.05.
@Comment : 
        -로그 DB 시각화를 위한 코드입니다.
        -코드 실행 전, DB 서버의 정보를 설정해주세요.
        -주요기능
            · 로그 DB 표시
"""

import pymysql
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import sys, time


# SET UP
mysql_ip = ""
mysql_user = ""
mysql_passwd = ""

db_name = "lightway"
########################################


class Check_db(QThread):
    log_sig = pyqtSignal(tuple)

    def __init__(self):
        super().__init__()

    def run(self):
        db = pymysql.connect(host=mysql_ip, port=3306, user=mysql_user, passwd=mysql_passwd, db=db_name, charset="utf8")
        cursor = db.cursor()

        while True:
            db.commit()

            sql = "select * from log"
            cursor.execute(sql)
            data = cursor.fetchall()
            self.log_sig.emit(data)

            time.sleep(0.5)


class WindowClass(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle(" ")
        self.resize(1100, 710)
        self.centralwidget = QWidget(self)
        self.gridLayout = QGridLayout(self.centralwidget)
        self.gridLayout.setContentsMargins(20, 0, 20, 20)  # left, top, right, bottom

        # check_db
        self.check_db = Check_db()
        self.check_db.log_sig.connect(self.log_table)
        self.check_db.start()

        # icon
        self.icon = QLabel(self.centralwidget)
        self.icon.setPixmap(QPixmap("icon_log.png"))
        self.icon.setFixedSize(300, 300)
        self.icon.setMargin(70)
        self.icon.setScaledContents(True)

        # table Widget
        self.tableWidget = QTableWidget(self.centralwidget)

        self.tableWidget.setShowGrid(False)
        self.tableWidget.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableWidget.setAlternatingRowColors(True)
        self.tableWidget.setStyleSheet('QTableView::item {border-right: 1px solid #d6d9dc;}')
        self.tableWidget.setFocusPolicy(Qt.NoFocus)

        self.tableWidget.setRowCount(1)
        self.tableWidget.verticalHeader().setHighlightSections(False)

        self.tableWidget.setColumnCount(6)
        self.tableWidget.setHorizontalHeaderLabels(['날짜', '사용자 ID', '정류장 ID', '버스 번호', '차량 번호', '상태'])
        self.tableWidget.horizontalHeader().setFixedHeight(30)
        self.tableWidget.horizontalHeader().setStyleSheet("QHeaderView::section { background-color : rgb(200, 195, 188); border-style: none; }")
        self.tableWidget.horizontalHeader().setCascadingSectionResizes(True)
        self.tableWidget.horizontalHeader().setHighlightSections(False)
        self.tableWidget.horizontalHeader().setStretchLastSection(True)

        self.tableWidget.setColumnWidth(0, 230)
        self.tableWidget.setColumnWidth(1, 200)
        self.tableWidget.setColumnWidth(2, 180)
        self.tableWidget.setColumnWidth(3, 120)
        self.tableWidget.setColumnWidth(4, 180)
        self.tableWidget.setColumnWidth(5, 80)

        # layout
        self.gridLayout.addWidget(self.icon, 0, 2, 1, 1)
        self.gridLayout.addWidget(self.tableWidget, 1, 0, 1, 5)

        self.setCentralWidget(self.centralwidget)


    def log_table(self, data):
        self.tableWidget.setRowCount(len(data))

        for i in range(len(data)):
            date = str(data[i][0])
            self.tableWidget.setItem(i, 0, QTableWidgetItem(date))
            for j in range(1, len(data[i])):
                self.tableWidget.setItem(i, j, QTableWidgetItem(data[i][j]))

        self.tableWidget.scrollToBottom()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    myWindow = WindowClass()
    myWindow.show()
    app.exec_()
