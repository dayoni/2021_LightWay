#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <ao/ao.h>
#include <mpg123.h>

#include <VX/vx.h>
#include <NVX/nvx_timer.hpp>
#include "OVX/FrameSourceOVX.hpp"
#include "NVX/nvx_opencv_interop.hpp"
#include "OVX/RenderOVX.hpp"
#include "NVX/Application.hpp"
#include "OVX/UtilityOVX.hpp"
#include "NVX/ConfigParser.hpp"
#include <NVX/SyncTimer.hpp>

#include "stereo_matching.hpp"
#include "color_disparity_graph.hpp"

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "withrobot_camera.hpp"
#include "darknet.h"
#include <math.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "mysql_connection.h"

#define WIDTH		640
#define HEIGHT		480
#define FPS			30
#define GAIN		150
#define EXPOSURE	150
#define PI 3.14159265358979323846
#define BITS 8
using namespace cv;
using namespace std;

sql::Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet *res;

int UID = 444444;

typedef struct bus_wh {
	long double BW;
	long double BH;
}bus;

typedef struct per_wh {
	long double PW;
	long double PH;
}person;

long double ConvertDecimalDegreesToRadians(long double deg) {
	return (deg *  PI / 180);
}

long double ConvertRadiansToDecimalDegrees(long double rad) {
	return (rad * 180 / PI);
}

double GetDistanceBetweenPoints(long double lat1, long double lon1, long double lat2, long double lon2) {
	long double theta, dist = 0.0;
	if((lat1 == lat2) && (lon1 == lon2))
		return 0;
	else {
		theta = lon1 - lon2;
		dist = sin(ConvertDecimalDegreesToRadians(lat1)) * sin(ConvertDecimalDegreesToRadians(lat2)) \
			+ cos(ConvertDecimalDegreesToRadians(lat1)) * cos(ConvertDecimalDegreesToRadians(lat2)) \
			* cos(ConvertDecimalDegreesToRadians(theta));
	dist = acos(dist);	
	dist = ConvertRadiansToDecimalDegrees(dist);
	dist = dist * 60 * 1.1515;
	dist = dist * 1.609344 * 1000;	

	return dist;
	}
}

/* MYSQL connect */
void db_connect(void) {
	driver = get_driver_instance();
	cout << fixed; cout.precision(15);
	con = driver -> connect("tcp://192.168.44.94", "dayoni", "dayoni");	// dayoni
	//con = driver -> connect("tcp://sangchu99.co.kr:3306", "sangchu", "sangchu");	// sangchu
	stmt = con -> createStatement();
	stmt -> execute("USE TEST_DB");
}

/* 예약 테이블에서 사용자가 예약한 BUID 파싱 */
string tT_pars() {
	string query1, BUID = "";
	int flag1 = 0;
	char buf1[256];

	sprintf(buf1, "SELECT buid, flag from TEST_TABLE WHERE uid=%d", UID);
	query1 = buf1;
	res = stmt -> executeQuery(query1);
	while(res -> next()) {
		BUID = res -> getString("buid");
		flag1 = stoi(res -> getString("flag"));
	}

	if (flag1 == 1) {
		return BUID; }
		
	else if (flag1 == 2) {	
		return (BUID = "Cancle"); }
		
	else if (flag1 == 3) {
		return (BUID = "Complete"); }
		
	else {
		return (BUID = "default"); }


}

int stat_pars(string BUID) {
	int flag2 = 0;
	string query2 = "";
	char buf2[256];

	sprintf(buf2, "SELECT flag from TEST_TABLE WHERE uid=%d and buid='%s'",\
		UID, BUID.c_str());
	query2 = buf2;
	res = stmt -> executeQuery(query2);
	while(res -> next()) {
		flag2 = stoi(res -> getString("flag"));
	}
	return flag2;
}

/* 가져온 BUID를 이용하여 버스위치정보 DB에서 파싱한 버스의 위도, 경도 */
bus tB_pars(string buid) {
	string query3 = "";
	bus b;
	char buf3[256];

	sprintf(buf3, "SELECT w, h from TEST_BG WHERE buid='%s'", buid.c_str());
	query3 = buf3;
	res = stmt -> executeQuery(query3);
	while(res -> next()) {
		b.BW = stold(res -> getString("w"));
		b.BH = stold(res -> getString("h"));
	}
	cout << "TEST_BG -> 버스 위도 : " << b.BW << ", 버스 경도 : "<< b.BH << endl;
	return b;
}

/* 초기설정된 UID를 이용하여 사용자위치정보 DB에서 파싱한 사용자의 위도, 경도 */
person tP_pars(int UID) {
	string query4 = "";
	person p;
	char buf4[256];

	sprintf(buf4, "SELECT w, h from TEST_PG WHERE uid=%d", UID);
	query4 = buf4;
	res = stmt -> executeQuery(query4);
	while(res -> next()) {
		p.PW = stold(res->getString("w"));
		p.PH = stold(res->getString("h"));
	}
	cout << "TEST_PG -> 사용자 위도 : " << p.PW << ", 사용자 경도 : "<< p.PH << endl;
	return p;
}

void *sound_start(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));
	cout << "driver :" << driver << endl;
	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/start.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_newReserve(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/새로운예약내역.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_arrivalBus(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/버스-곧-도착.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_cancleReserve(void *data)
{
	mpg123_handle *mh1;
	char *buffer1;
	size_t buffer_size1;
	size_t done1;
	int err;
	int driver;
	ao_device *dev1;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh1 = mpg123_new(NULL, &err);
	buffer_size1 = mpg123_outblock(mh1);
	buffer1 = (char*)malloc(buffer_size1 * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh1, "/home/lightway/Desktop/Guide_process_test/sound/예약취소.mp3");
	mpg123_getformat(mh1, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev1 = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh1, (unsigned char*)buffer1, buffer_size1, &done1) == MPG123_OK)
			ao_play(dev1, buffer1, done1);

	/* clean up */
	free(buffer1);
	ao_close(dev1);
	mpg123_close(mh1);
	mpg123_delete(mh1);

	return NULL;
}

void *sound_completeReserve(void *data)
{
	mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/승차처리.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}


void *sound_rotation_LS(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/왼쪽돌아직진.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_rotation_RS(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/오른쪽돌아직진.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_arrival_rotation_RS(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/도착_오른쪽돌아직진.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_arrival_rotation_LS(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/도착_왼쪽돌아직진.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *sound_stair_warn(void *data)
{
    mpg123_handle *mh;
	char *buffer;
	size_t buffer_size;
	size_t done;
	int err;
	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	//driver = ao_default_driver_id();
	driver = 1;
	cout << "driver :" << driver << endl;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (char*)malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "/home/lightway/Desktop/Guide_process_test/sound/계단주의.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
			ao_play(dev, buffer, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);

	return NULL;
}

void *start_viv_1(void *data)
{
    FILE *file;
    int pow = 1;
    file = fopen("/dev/ttyTHS0","w");
    fprintf(file, "%d", pow);
    fclose(file);

    return NULL;
}

void *start_viv_2(void *data)
{
    FILE *file;
    int pow = 3;
    file = fopen("/dev/ttyTHS0","w");
    fprintf(file, "%d", pow);
    fclose(file);

    return NULL;
}

void *start_viv_clear(void *data)
{
    FILE *file;
    int pow = 4;
    file = fopen("/dev/ttyTHS0","w");
    fprintf(file, "%d", pow);
    fclose(file);

    return NULL;
}


image ipl_to_image(IplImage* src)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image im = make_image(w, h, c);
    unsigned char *data = (unsigned char *)src->imageData;
    int step = src->widthStep;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return im;
}

image mat_to_image(Mat m)
{
    IplImage ipl = m;
    image im = ipl_to_image(&ipl);
    rgbgr_image(im);
    return im;
}

static bool read(const string &nf, StereoMatching::StereoMatchingParams &config, string &message)
{
    unique_ptr<nvxio::ConfigParser> parser(nvxio::createConfigParser());
    
    parser->addParameter("min_disparity",
                         nvxio::OptionHandler::integer(
                             &config.min_disparity,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(256)));
    parser->addParameter("max_disparity",
                         nvxio::OptionHandler::integer(
                             &config.max_disparity,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(256)));
    parser->addParameter("P1",
                         nvxio::OptionHandler::integer(
                             &config.P1,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(256)));
    parser->addParameter("P2",
                         nvxio::OptionHandler::integer(
                             &config.P2,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(256)));
    parser->addParameter("sad",
                         nvxio::OptionHandler::integer(
                             &config.sad,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(31)));
    parser->addParameter("bt_clip_value",
                         nvxio::OptionHandler::integer(
                             &config.bt_clip_value,
                             nvxio::ranges::atLeast(15) & nvxio::ranges::atMost(95)));
    parser->addParameter("max_diff",
                         nvxio::OptionHandler::integer(
                             &config.max_diff));
    parser->addParameter("uniqueness_ratio",
                         nvxio::OptionHandler::integer(
                             &config.uniqueness_ratio,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(100)));
    parser->addParameter("scanlines_mask",
                         nvxio::OptionHandler::integer(
                             &config.scanlines_mask,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(256)));
    parser->addParameter("flags",
                         nvxio::OptionHandler::integer(
                             &config.flags,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(3)));
    parser->addParameter("ct_win_size",
                         nvxio::OptionHandler::integer(
                             &config.ct_win_size,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(5)));
    parser->addParameter("hc_win_size",
                         nvxio::OptionHandler::integer(
                             &config.hc_win_size,
                             nvxio::ranges::atLeast(0) & nvxio::ranges::atMost(5)));

    message = parser->parse(nf);
    return message.empty();
}



double readCalib(Mat& _map11, Mat& _map12, Mat& _map21, Mat& _map22)
{
    string left_filename = "./calib/left.yaml";
    string right_filename = "./calib/right.yaml";
    FileStorage left_fs(left_filename, FileStorage::READ);
    FileStorage right_fs(right_filename, FileStorage::READ);
    vector<float> vec;

    vec.clear();
    left_fs["camera_matrix"]["data"] >> vec;
    Mat left_C(vec,true);
    left_C = left_C.reshape(0,3);
    vec.clear();
    left_fs["distortion_coefficients"]["data"] >> vec;
    Mat left_D(vec,true);
    left_D = left_D.reshape(0,1);
    vec.clear();
    left_fs["rectification_matrix"]["data"] >> vec;
    Mat left_R(vec,true);
    left_R = left_R.reshape(0,3);
    vec.clear();
    left_fs["projection_matrix"]["data"] >> vec;
    Mat left_P(vec,true);
    left_P = left_P.reshape(0,3);
    left_C.convertTo(left_C,CV_64F);
    left_D.convertTo(left_D,CV_64F);
    left_R.convertTo(left_R,CV_64F);
    left_P.convertTo(left_P,CV_64F);
    initUndistortRectifyMap(left_C, left_D, left_R, left_P, Size(WIDTH, HEIGHT), CV_32FC1, _map11, _map12);

    vec.clear();
    right_fs["camera_matrix"]["data"] >> vec;
    Mat right_C(vec,true);
    right_C = right_C.reshape(0,3);
    vec.clear();
    right_fs["distortion_coefficients"]["data"] >> vec;
    Mat right_D(vec,true);
    right_D = right_D.reshape(0,1);
    vec.clear();
    right_fs["rectification_matrix"]["data"] >> vec;
    Mat right_R(vec,true);
    right_R = right_R.reshape(0,3);
    vec.clear();
    right_fs["projection_matrix"]["data"] >> vec;
    Mat right_P(vec,true);
    right_P = right_P.reshape(0,3);
    right_C.convertTo(right_C,CV_64F);
    right_D.convertTo(right_D,CV_64F);
    right_R.convertTo(right_R,CV_64F);
    right_P.convertTo(right_P,CV_64F);
    initUndistortRectifyMap(right_C, right_D, right_R, right_P, Size(WIDTH, HEIGHT), CV_32FC1, _map21, _map22);
    vec.clear();
    left_fs.release();
    right_fs.release();
    return left_P.at<double>(0,0);
}

void VX2MatC1(vx_context m_vxCtx, vx_image& vxiSrc, Mat& matDst)
{
	vx_imagepatch_addressing_t dst_addr;
	dst_addr.dim_x = matDst.cols;
	dst_addr.dim_y = matDst.rows;
	dst_addr.stride_x = sizeof(vx_uint8);
	dst_addr.stride_y = matDst.step;
	void* dst_ptrs[] = {matDst.data};
	vx_image vxiDst = vxCreateImageFromHandle(m_vxCtx, VX_DF_IMAGE_U8, &dst_addr, dst_ptrs, VX_IMPORT_TYPE_HOST);
	vxReleaseImage(&vxiDst);
}

Mat copyVxImageToCvMat(vx_image ovxImage)
{
    vx_status status;
    vx_df_image df_image = 0;
    vx_uint32 width, height;
    status = vxQueryImage(ovxImage, VX_IMAGE_FORMAT, &df_image, sizeof(vx_df_image));
    if (status != VX_SUCCESS)
        throw runtime_error("Failed to query image");
    status = vxQueryImage(ovxImage, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    if (status != VX_SUCCESS)
        throw runtime_error("Failed to query image");
    status = vxQueryImage(ovxImage, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    if (status != VX_SUCCESS)
        throw runtime_error("Failed to query image");

    if (!(width > 0 && height > 0)) throw runtime_error("Invalid format");

    int depth;
    switch (df_image)
    {
    case VX_DF_IMAGE_U8:
        depth = CV_8U;
        break;
    case VX_DF_IMAGE_U16:
        depth = CV_16U;
        break;
    case VX_DF_IMAGE_S16:
        depth = CV_16S;
        break;
    case VX_DF_IMAGE_S32:
        depth = CV_32S;
        break;
    default:
        throw runtime_error("Invalid format");
        break;
    }

    Mat image(height, width, CV_MAKE_TYPE(depth, 1));

    vx_rectangle_t rect;
    rect.start_x = rect.start_y = 0;
    rect.end_x = width; rect.end_y = height;

    vx_imagepatch_addressing_t addr;
    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = (vx_uint32)image.elemSize();
    addr.stride_y = (vx_uint32)image.step.p[0];
    vx_uint8* matData = image.data;

    status = vxCopyImagePatch(ovxImage, &rect, 0, &addr, matData, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    if (status != VX_SUCCESS)
        throw runtime_error("Failed to copy image patch");
    return image;
}
