 // UDP_FullDuplexS9.cpp : Defines the entry point for the console application.
 //
//#include <winsock2.h>
//#include "xPCUDPSock.h"
#include "stdafx.h"

 // Two neccessary header files you need to include.
 // Place the include for winsock2.h at the beginning of your include statements to avoid a conflict with an older library
 #include <winsock2.h>
 #include "xPCUDPSock.h"

 //#pragma pack(push,1) // Important! Tell the compiler to pack things up tightly 

 ////buffer structure definition
 //struct PACKIN
 //{
 //	float flt1;
 //	float flt2;
 //};


 //struct PACKOUT
 //{
 //	float flt1;
 //	float flt2;
 //};

 //#pragma pack(pop) // Fall back to previous setting

 //int _tmain(int argc, TCHAR* argv[])
 //{
 //	int nRetCode = 0;

 //	// Initialize the UDP lib. If failed, quit running.
 //	if (!InitUDPLib())
 //	{
 //		nRetCode = 2;
 //	}else
 //	{
 //		// Create receiver, with packet size equal to that of PACKIN and port at 12403 or the output port for the Tiva in virtual port 3
 //		CUDPReceiver receiver(sizeof(PACKIN),12403);

 //		// Create sender, with packet size equal to that of PACKOUT and port at port is 12302 or input port for the Tiva in virtual port 2, 
 //		// and remote address 127.0.0.1(localhost)
 //		CUDPSender sender(sizeof(PACKOUT), 12302,"127.0.0.1");
	//	
 //		// Define buffers for input and output
 //		PACKIN pkin;
 //		PACKOUT pkout;
 //		// Routing data endlessly
 //		while(1)
 //		{
 //			// prevent from running to fast
 //			Sleep(1);
 //			// get latest data from receiver
 //			receiver.GetData(&pkin);
	//		
 //			// repack the data
 //			pkout.flt1=pkin.flt1;
 //			pkout.flt2=pkin.flt2;

 //			// send the repacked data through sender
 //			sender.SendData(&pkout);
 //		}		
 //	}


 //	return nRetCode;
 //}

#include <iostream>
#include <csignal>
#include <ctime>
#include <thread>
#include <mutex>
#include <queue>

#include <opencv2/opencv.hpp>
#include "track.h"

#define N_FRAME_COUNT 5
//#define GUI_DEMO
#define CANNY_DEFAULT 60

/* Thresholding parameters */
#define HUE_LIM_DEFAULT Scalar(0, 47)
#define SAT_LIM_DEFAULT Scalar(80, 255)
#define VAL_LIM_DEFAULT Scalar(162, 255)

/* GUI drawing parameters */
#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE 8

using namespace std;
using namespace cv;
using namespace Track;

Scalar hsv_hue_lim = HUE_LIM_DEFAULT;
Scalar hsv_sat_lim = SAT_LIM_DEFAULT;
Scalar hsv_val_lim = VAL_LIM_DEFAULT;

#pragma pack(push,1)
struct packet {
	int32_t x;
	int32_t y;
	float vel;
};
#pragma pack(pop)

mutex udp_mtx;
mutex frame_mtx;
struct packet vis;

bool cap_quit = false;
bool udp_quit = false;

thread udp_thread_handle;
thread video_cap_thread_handle;

Mat thread_frame;
clock_t thread_cap_time;

#if 1
static float reg_param(Point pos[], clock_t time[]) {
    float sum_t = 0;
    float sum_x = 0;
    float sum_tx = 0;
    float sum_t_sqr = 0;

    for(size_t i = 0; i < N_FRAME_COUNT; i++) {
        sum_t += time[i] / (float)CLOCKS_PER_SEC;
        sum_x += pos[i].x;

        sum_tx += (time[i] / (float)CLOCKS_PER_SEC) * pos[i].x;

        sum_t_sqr += (time[i] / (float)CLOCKS_PER_SEC) * (time[i] / (float)CLOCKS_PER_SEC);
    }

    float t_denom = (N_FRAME_COUNT * sum_t_sqr - sum_t * sum_t);
    float vx = 0;

    if(t_denom) {
        vx = (N_FRAME_COUNT * sum_tx - sum_t * sum_x) / t_denom;
    }

    return vx;
}
#endif

static void update_N_params(Point curr_pos, clock_t curr_time, Point N_pos[], clock_t N_time[]) {
    for(size_t i = 0; i < N_FRAME_COUNT - 1; i++) {
        N_pos[i] = N_pos[i + 1];
        N_time[i] = N_time[i + 1];
    }

    N_pos[N_FRAME_COUNT - 1] = Point(320 - curr_pos.x, curr_pos.y);
    N_time[N_FRAME_COUNT - 1] = curr_time;
}

/* Video capture thread */
void video_cap_thread(void) {
    VideoCapture cam(1);
    if(!cam.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return;
    }

    cam.set(CV_CAP_PROP_FPS, 60.0);

    while(!cap_quit) {
		//frame_mtx.lock();
        cam.read(thread_frame);
        thread_cap_time = clock();
		//frame_mtx.unlock();
    }

    cam.release();
}

/* UDP thread */
#define SEND_PORT (12302)
#define SEND_ADDR "127.0.0.1"
#define RECV_PORT (12403)
void udp_thread(void) {

	if (!InitUDPLib()) {
		cerr << "Failed to initialize UDP lib" << endl;
	}

    struct packet pack;
    memset(&pack, 0, sizeof(pack));

	CUDPSender sender(sizeof(struct packet), SEND_PORT, SEND_ADDR);
	//CUDPReceiver receiver(sizeof(struct packet), RECV_PORT);

    while(!udp_quit) {
        udp_mtx.lock();
        memcpy(&pack, &vis, sizeof(pack));
        udp_mtx.unlock();

        sender.SendData(&pack);
		Sleep(1);
    }

    memset(&pack, 0, sizeof(pack));
    sender.SendData(&pack);
}

void handle_sigint(int signum) {
    if(signum == SIGINT) {
        cap_quit = true;
        udp_quit = true;

        udp_thread_handle.join();
        video_cap_thread_handle.join();

        exit(0);
    }
}

int main() {
    signal(SIGINT, handle_sigint);
    int canny_param = CANNY_DEFAULT;

#ifdef GUI_DEMO
    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresh Params", CV_WINDOW_AUTOSIZE);

    int hsv_hue_hi = HUE_LIM_DEFAULT[1];
    int hsv_hue_lo = HUE_LIM_DEFAULT[0];
    int hsv_sat_hi = SAT_LIM_DEFAULT[1];
    int hsv_sat_lo = SAT_LIM_DEFAULT[0];
    int hsv_val_hi = VAL_LIM_DEFAULT[1];
    int hsv_val_lo = VAL_LIM_DEFAULT[0];

    createTrackbar("Hue Hi", "Thresh Params", &hsv_hue_hi, 180, NULL, NULL);
    createTrackbar("Hue Lo", "Thresh Params", &hsv_hue_lo, 180, NULL, NULL);
    createTrackbar("Sat Hi", "Thresh Params", &hsv_sat_hi, 255, NULL, NULL);
    createTrackbar("Sat Lo", "Thresh Params", &hsv_sat_lo, 255, NULL, NULL);
    createTrackbar("Val Hi", "Thresh Params", &hsv_val_hi, 255, NULL, NULL);
    createTrackbar("Val Lo", "Thresh Params", &hsv_val_lo, 255, NULL, NULL);
    createTrackbar("Canny Thres", "Thresh Params", &canny_param, 255, NULL, NULL);
#endif

    //video_cap_thread_handle = thread(video_cap_thread);
    udp_thread_handle = thread(udp_thread);

	Mat curr_frame;
	clock_t curr_time;

    Mat hsv;
    Mat thres;
    vector<vector<Point>> circles;
    vector<Point> centroids;
    float vel_x = 0;

    Point N_centroids[N_FRAME_COUNT] = { Point(0, 0) };
    clock_t N_time[N_FRAME_COUNT] = { 0 };

#ifdef GUI_DEMO
    cout << "Press any key to exit..." << endl;
#endif

	VideoCapture cam(1);
	if (!cam.isOpened()) {
		cerr << "Failed to open camera!" << endl;
		return EXIT_FAILURE;
	}
	cam.set(CV_CAP_PROP_FPS, 60.0);

	int FramerCounter = 0;
	clock_t StartTime, EndTime;
    size_t largest_contour;
    while(true) {
		cam.read(curr_frame);
		curr_time = clock();

        cvtColor(curr_frame, hsv, CV_BGR2HSV);

#ifdef GUI_DEMO
        hsv_hue_lim = Scalar(hsv_hue_lo, hsv_hue_hi);
        hsv_sat_lim = Scalar(hsv_sat_lo, hsv_sat_hi);
        hsv_val_lim = Scalar(hsv_val_lo, hsv_val_hi);
#endif

        hsv_threshold(hsv, thres, hsv_hue_lim, hsv_sat_lim, hsv_val_lim);
        detect_circles(thres, circles, canny_param);

        if(circles.size()) {
            centroids = vector<Point>(circles.size());
            calc_centroids(centroids, circles);

            largest_contour = 0;
            for(size_t i = 0; i < circles.size(); i++) {
                if(arcLength(circles[largest_contour], true) < arcLength(circles[i], true)) { 
                    largest_contour = i;
                }
            }

            vel_x = reg_param(N_centroids, N_time);
            update_N_params(centroids[largest_contour], thread_cap_time, N_centroids, N_time);

#ifdef GUI_DEMO
            drawContours(curr_frame, circles, largest_contour, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, noArray(), 0, Point());
            circle(curr_frame, centroids[largest_contour], 1, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);
#endif
        }
        else {
            vel_x = 0;
            update_N_params(Point(320, 0), curr_time, N_centroids, N_time);
        }

        udp_mtx.lock();
        vis.x = N_centroids[N_FRAME_COUNT - 1].x;
        vis.y = N_centroids[N_FRAME_COUNT - 1].y;
        vis.vel = vel_x;
        udp_mtx.unlock();

		cout << "Time: " << 1000 * (float)N_time[N_FRAME_COUNT - 1] / (float)CLOCKS_PER_SEC << endl;
		cout << "Pos: " << N_centroids[N_FRAME_COUNT - 1] << endl;
		cout << "Vel: " << vel_x << endl << endl << endl;

#ifdef GUI_DEMO
        /* Draw image origin */
        circle(curr_frame, Point(0, 0), 1, CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);

        /* Draw problem domain origin */
        circle(curr_frame, Point(640 / 2 , 480), 1, CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);
        imshow("Tracking", curr_frame);
        imshow("Thresholding", thres);
        if(waitKey(1) > 0) {
            break;
        }
#endif

		if (FramerCounter == 0) {
			StartTime = clock();
		}

		FramerCounter++;

		EndTime = clock();
		if ((EndTime - StartTime) / CLOCKS_PER_SEC >= 1) {
			cout << "Logic FPS: " << FramerCounter << endl;
			FramerCounter = 0;
		}
    }

#ifdef GUI_DEMO
    cap_quit = true;
    udp_quit = true;

    udp_thread_handle.join();
    //video_cap_thread_handle.join();

    destroyAllWindows();
#endif

    return EXIT_SUCCESS;
}
