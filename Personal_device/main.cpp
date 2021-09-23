#include "main.hpp"

using namespace cv;
using namespace std;

int flag = 0;
int before_flag = 0;
int flag_path1 = 0; int flag_path2 = 0;
int flag_path3 = 0; int flag_path4 = 0;
int flag_path5 = 0; int flag_path6 = 0;

int flag_dist2 = 0;
string BUID = "";
long double distance_t = 10000.0;
bus bt_f = {123.00000, 123.00000};
bus bt; person pt;

void *get_flag(void *data)
{
    db_connect();

    BUID = tT_pars();
    if(BUID != "Complete" && BUID != "Cancle" && BUID !=  "default") flag = stat_pars(BUID);
    else if(BUID == "Cancle") flag = 2;
    else if(BUID == "Complete") flag = 3;
    else flag = 4;

    return NULL;
}

void *get_gps(void *data)
{
    bt = tB_pars(BUID);
    pt = tP_pars(UID);

    distance_t = GetDistanceBetweenPoints(bt.BW, bt.BH, pt.PW, pt.PH);
    if ((bt.BW == bt_f.BW) && (bt.BH == bt_f.BH)) flag_dist2++;
    else bt_f = bt;

    cout << "거리 : " << distance_t << "(m)" << endl;
    cout << "******************************************************************" << endl;
    cout << endl;

    return NULL;
}

void *get_path(void *data)
{
    pthread_t sound_t1, sound_t2, sound_t3, sound_t4, sound_t5, sound_t6;

    /* 버스보다 사용자가 오른쪽 => 왼쪽으로 돌아 직진하십시오. */
    if((pt.PW - bt_f.BW) > 0)
    {
	flag_path1++;
	flag_path3 = 2;

	if(flag_path1 == 1)
	{
	    pthread_create(&sound_t1, NULL, sound_arrival_rotation_LS, NULL);
	    pthread_detach(sound_t1);
	}

	/* 버스와 사용자의 위도가 같을 때(같은 x선상에 있을 때) => 오른쪽으로 돌아 직진하십시오. */
	if(flag_path3 == 2 && distance_t <= 6.0)
	{
	    flag_path2++;

	    if(flag_path2 == 1)
	    {
		pthread_create(&sound_t3, NULL, sound_rotation_RS, NULL);
		pthread_detach(sound_t3);

	    }

	    if(distance_t < 1.0)
	    {
		flag_path5++;

		if(flag_path5 == 1)
		{
		    pthread_create(&sound_t5, NULL, sound_stair_warn, NULL);
		    pthread_detach(sound_t5);
		    cout << "/* 버스와 사용자 간 거리 차이가 얼마 나지 않을 때 */" << endl;
		}
	    }

	    else
	    {
		cout << "/* 버스와 사용자 간 거리 차이가 있을 때 */" << endl;
	    }
    	}

	/* 버스와 사용자의 위도가 다를 때 => 무시 */
	else
	{
  	    cout << "/* 사용자 길 안내 중 */" << endl;
	}
    }

    /* 버스보다 사용자가 왼쪽 => 오른쪽으로 돌아 직진하십시오. */
    else
    {
	flag_path3++;
	flag_path1 = 2;

	if(flag_path3 == 1)
	{
	    pthread_create(&sound_t2, NULL, sound_arrival_rotation_RS, NULL);
	    pthread_detach(sound_t2);
	}
	
	/* 버스와 사용자의 위도가 같을 때(같은 x선상에 있을 때) => 왼쪽으로 돌아 직진하십시오. */
	if(flag_path1 == 2 && distance_t <= 6.0)
	{
	    flag_path4++;

	    if(flag_path4 == 1)
	    {
		pthread_create(&sound_t4, NULL, sound_rotation_RS, NULL);
		pthread_detach(sound_t4);
	    }

	    if(distance_t < 1.0)
	    {
		flag_path6++;

		if(flag_path6 == 1)
		{
		    pthread_create(&sound_t6, NULL, sound_stair_warn, NULL);
		    pthread_detach(sound_t6);
		    cout << "/* 버스와 사용자 간 거리 차이가 얼마 나지 않을 때 */" << endl;
		}
	    }

	    else
	    {
		cout << "/* 버스와 사용자 간 거리 차이가 있을 때 */" << endl; 
	    }
	}

	else
	{
  	    cout << "/* 사용자 길 안내 중 */" << endl;
	}
    }

    return NULL;
}

int main(int argc, char* argv[]) 
{
    pthread_t con_db1, con_db2, path;
    pthread_t sound1, sound2, sound3, sound4, sound5;
    pthread_t set_viv1, set_viv2, set_viv3, set_viv4, set_viv5, set_viv6, set_viv7, set_viv8, set_viv9;
    int th_return, flag_dist1 = 0;
    
    nvxio::Application &app = nvxio::Application::get();
    ovxio::printVersionInfo();
    string configFile ="/home/lightway/Guide_process/stereo_matching_demo_config.ini";
    StereoMatching::StereoMatchingParams params;
    StereoMatching::ImplementationType implementationType = StereoMatching::HIGH_LEVEL_API;

    char* cfg_file = "/home/lightway/darknet/cfg/yolov3-tiny.cfg";
    char* weight_file = "/home/lightway/darknet/yolov3-tiny.weights";
    char* meta_file = "/home/lightway/darknet/cfg/coco.data";

    float thresh = 0.5;
    float hier_thresh = 0.5;
    image im;
    detection *dets;

    string obj_dist = "";
    Mat left_color, right_color, left_gray, right_gray, map11, map12, map21, map22, l_img, r_img, disp, disp16, disp32, ROI;

    int num = 0;
    int w, h, x, y = 0;
    double min, max = 0.0;
    double _depth = 0.0;

    app.init(argc, argv);
    string error;

    if (!read(configFile, params, error))
    {
        cerr << error;
        return nvxio::Application::APP_EXIT_CODE_INVALID_VALUE;
    }

    // Load DNN and meta file
    metadata meta = get_metadata(meta_file);
    network *net = load_network(cfg_file, weight_file, 0);

    /* camera */
    const char* devPath = "/dev/video0";

    /* Camera Open */
    Withrobot::Camera cap(devPath);
    Withrobot::camera_format camFormat;

    /* Set Camera control */
    cap.set_format(WIDTH, HEIGHT, Withrobot::fourcc_to_pixformat('Y','U','Y','V'), 1, FPS);

    /* get current camera format (image size and frame rate) */
    cap.get_current_format(camFormat);
    cap.set_control("Gain", GAIN);
    cap.set_control("Exposure (Absolute)", EXPOSURE);
    cap.set_control("White Balance Blue Component", 180);
    cap.set_control("White Balance Red Component", 150);

    ovxio::ContextGuard context;
    vxDirective(context, VX_DIRECTIVE_ENABLE_PERFORMANCE);
        
    vx_image vx_left_rect = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_RGB);
    NVXIO_CHECK_REFERENCE(vx_left_rect);
    vx_image vx_right_rect = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_RGB);
    NVXIO_CHECK_REFERENCE(vx_right_rect);
    vx_image disparity_rect = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    NVXIO_CHECK_REFERENCE(disparity_rect);
    vx_image disparity_16 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16);
    NVXIO_CHECK_REFERENCE(disparity_16);

    unique_ptr<StereoMatching> stereo_rect(
    StereoMatching::createStereoMatching(
        context, params,
        implementationType,
        vx_left_rect, vx_right_rect, disparity_rect, disparity_16));
	    

     /* Start streaming */
    if (!cap.start())
    {
	perror("Failed to start(camera).");
	exit(0);
    }
    else
    {
	pthread_create(&sound5, NULL, sound_start, NULL);
	pthread_join(sound5, (void**)&th_return);
	cout << "Success to start(camera)" << endl << endl;
	Mat frame(Size(WIDTH, HEIGHT), CV_8UC2);
	Mat stereo_raw[2];
	double F = readCalib(map11, map12, map21, map22);
	unique_ptr<nvxio::SyncTimer> syncTimer = nvxio::createSyncTimer();
	syncTimer->arm(1. / app.getFPSLimit());
	nvx::Timer totalTimer;
	totalTimer.tic();

	while(1)
	{	
	    pthread_create(&con_db1, NULL, get_flag, NULL);
	    pthread_join(con_db1, NULL);
    
	    cap.get_frame(frame.data, camFormat.image_size, 1);
	    if (frame.empty())
	    {
	        cout << "ERROR! blank frame grabbed" << endl;
	        continue;
	    }

	    split(frame, stereo_raw);

	    // opencv : RGB, visionworks : BGR
	    cvtColor(stereo_raw[1], left_color, CV_BayerGR2RGB);
	    cvtColor(stereo_raw[0], right_color, CV_BayerGR2RGB);

	    remap(left_color, l_img, map11, map12, INTER_LINEAR);
	    remap(right_color, r_img, map21, map22, INTER_LINEAR);

	    vx_image vx_left = nvx_cv::createVXImageFromCVMat(context, l_img);
	    vx_image vx_right = nvx_cv::createVXImageFromCVMat(context, r_img);
	    nvxuCopyImage(context,vx_left,vx_left_rect);
	    nvxuCopyImage(context,vx_right,vx_right_rect);

	    stereo_rect->run();

	    disp16 = copyVxImageToCvMat(disparity_16);
	    disp16.convertTo(disp32, CV_32FC1, 1./16);
	 
	    im = mat_to_image(l_img);
	    network_predict_image(net, im);
	    dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, NULL, 0, &num);
	    do_nms_obj(dets, num, 80, thresh);
    	      

	    if(before_flag == flag)
	    {
		if(before_flag == 1)
		{
		    pthread_create(&set_viv9, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv9);
		    cout << "====== 미처리 예약내역  ======" << endl;
		    pthread_create(&con_db2, NULL, get_gps, NULL);
		    pthread_detach(con_db2);

		    if(distance_t < 100.0)
		    {
			flag_dist1++;

			if(flag_dist1 == 1)
			{
			    pthread_create(&sound4, NULL, sound_arrivalBus, NULL);
			    pthread_detach(sound4);
			}
			
			if(flag_dist2 >= 80)
			{
			    pthread_create(&path, NULL, get_path, NULL);
			    pthread_detach(path);

			    for(int i =0; i <num; i++)
			    {
			    	for(int j =0; j <80; j++)
			    	{
			    	    if(string(meta.names[j]) == "person")
			    	    {

			    	        if (dets[i].prob[j] > 0)
			    	        {
					    w = dets[i].bbox.w;
					    h = dets[i].bbox.h;
					    x = dets[i].bbox.x;
					    y = dets[i].bbox.y;
					    Rect rect(x,y,w*0.1,h*0.1);
					    ROI = disp32(rect);
					    minMaxLoc(ROI, &min, &max);
					    _depth = roundf(0.12 * F / max * 100) / 100;
					    obj_dist = " [ Person ] "+Withrobot::to_string<double>(_depth)+'m';

					    if(_depth < 1)
					    {
				   	        pthread_create(&set_viv1, NULL, start_viv_1, NULL);
						cout << "viv1" << endl;
				   	        pthread_detach(set_viv1);
				   	        rectangle(l_img, Point(x-w/2, y-h/2), Point(x+w/2, y+h/2), Scalar(0,0,255), 2, 8, 0);
				   	        putText(l_img, obj_dist, Point(x-w/2, y-h/2), 1, 1, Scalar(0,0,255), 2);    
				   	    }    

				   	    else if((_depth < 2) && (_depth > 1))
				   	    {
				   	        pthread_create(&set_viv2, NULL, start_viv_2, NULL);
						cout << "viv2" << endl;
				   	        pthread_detach(set_viv2);
				   	        rectangle(l_img, Point(x-w/2, y-h/2), Point(x+w/2, y+h/2), Scalar(0,255,0), 2, 8, 0);
				   	        putText(l_img, obj_dist, Point(x-w/2, y-h/2), 1, 1, Scalar(0,255,0), 2);
				   	    }

				   	    else if (_depth > 2)    
				   	    {    
				   	        pthread_create(&set_viv3, NULL, start_viv_clear, NULL);
						cout << "viv3" << endl;
				   	        pthread_detach(set_viv3);
				   	        rectangle(l_img, Point(x-w/2, y-h/2), Point(x+w/2, y+h/2), Scalar(0,255,0), 2, 8, 0);
				   	        putText(l_img, obj_dist, Point(x-w/2, y-h/2), 1, 1, Scalar(0,255,0), 2);
				   	    }
				        }
				    }
			        }
			    }
		        }
		    }
	        }

		else
		{
	    	    cout << "** before_flag == flag (0, 2, 3, 4일 때) **" << endl;
		    pthread_create(&set_viv4, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv4);
		}
	    }

	    else if(before_flag != flag)
	    {
		before_flag = flag;

		if(before_flag == 1)
		{
		    pthread_create(&set_viv5, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv5);	
		    cout << "====== 새로운 예약내역  ======" << endl;
		    pthread_create(&sound1, NULL, sound_newReserve, NULL);
		    pthread_detach(sound1);
		    cout << "******************************************************************" << endl;
		    cout << endl;
		    flag_dist1 = 0; flag_dist2 = 0;
		    flag_path1 = 0; flag_path2 = 0;
		    flag_path3 = 0; flag_path4 = 0;
		    flag_path5 = 0; flag_path5 = 0;
		    distance_t = 10000.0;
		}

		else if (before_flag == 2)
		{
		    pthread_create(&set_viv6, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv6);
		    cout << "====== 예약취소  ======" << endl;
		    pthread_create(&sound2, NULL, sound_cancleReserve, NULL);
		    pthread_detach(sound2);
		    cout << "******************************************************************" << endl;
		    cout << endl;
		}
			
		else if (before_flag == 3)
		{
		    pthread_create(&set_viv7, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv7);
		    cout << "====== 승차처리  ======" << endl;
		    pthread_create(&sound3, NULL, sound_completeReserve, NULL);
		    pthread_detach(sound3);
		    cout << "******************************************************************" << endl;
		    cout << endl;
		}	
		
		else
		{
		    pthread_create(&set_viv8, NULL, start_viv_clear, NULL);
		    pthread_detach(set_viv8);
		    cout << "====== 예외상황  ======" << endl;
			/* 예외상황 알림 들어가야함 */
		    cout << "******************************************************************" << endl;
		    cout << endl;
		}
	    }

	    syncTimer->synchronize();
	    //imshow("disp", l_img);
	    //waitKey(10);

	    free_image(im);
	    free_detections(dets, 0);
	    vxReleaseImage(&vx_left);
	    vxReleaseImage(&vx_right);
	}
    }
    free_network(net);
    vxReleaseImage(&vx_left_rect);
    vxReleaseImage(&vx_right_rect);
    vxReleaseImage(&disparity_rect);
    //destroyAllWindows();
    cap.stop();
    printf("Done.\n");
    return nvxio::Application::APP_EXIT_CODE_SUCCESS;
}
