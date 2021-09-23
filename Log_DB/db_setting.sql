/*
 * @Filename : db_setting.sql
 * @Author : Kim Na Yeon
 * @Date : 2021.03.
 * @Update : 2021.05.29 (trigger, event 추가)
 * @Comment : 
 *         -버스 예약 시스템과 관련된 DB를 위한 코드입니다.
 *         -별도의 설정 없이 실행 가능합니다.
 *         -주요기능
 *             · 예약 DB 관리
 *             · 사용자 DB 관리
 *             · 로그 DB 관리
*/


########## DB_lightway
DROP DATABASE IF EXISTS lightway;
CREATE DATABASE lightway;
USE lightway;


########## TABLE_reserve
CREATE TABLE reserve (
    uid varchar(20),
    station_id varchar(10),
    bus_num varchar(10),
    buid varchar(30),
    status int(1)
);


########## TABLE_user
CREATE TABLE user (
    uid varchar(20),
    primary key(uid)
);


########## TABLE_log
CREATE TABLE log (
    date datetime,
    uid varchar(20),
    station_id varchar(10),
    bus_num varchar(10),
    buid varchar(30),
    status varchar(10)
);


########## TRIGGER
DELIMITER //
CREATE TRIGGER tri_reserve_insert
AFTER INSERT
ON reserve
FOR EACH ROW
BEGIN
    INSERT INTO log VALUES (NOW(), NEW.uid, NEW.station_id, NEW.bus_num, NEW.buid, NEW.status);
END //
DELIMITER ;

DELIMITER //
CREATE TRIGGER tri_reserve_update
AFTER UPDATE
ON reserve
FOR EACH ROW
BEGIN
    INSERT INTO log VALUES (NOW(), NEW.uid, NEW.station_id, NEW.bus_num, NEW.buid, NEW.status);
END //
DELIMITER ;


########## EVENT
DELIMITER //
CREATE EVENT evt_delete
ON SCHEDULE EVERY 2 SECOND
DO
    DELETE FROM reserve WHERE uid IN (select * from (select uid from log where (date_add(now(), interval -10 second) < date and date_add(now(), interval -5 second) > date) and (status = 2 or status = 3)) as reseult) //
DELIMITER ;