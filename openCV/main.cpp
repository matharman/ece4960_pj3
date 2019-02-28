#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <mutex>
#include <queue>

#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include "include/track.h"
#include "include/uart.h"

#define CANNY_DEFAULT 60

#define HUE_LIM_DEFAULT Scalar(0, 60)
#define SAT_LIM_DEFAULT Scalar(150, 255)
#define VAL_LIM_DEFAULT Scalar(200, 255)

#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE 8

#define CLOCKS_PER_MSEC ((double)CLOCKS_PER_SEC / 1000)

using namespace std;
using namespace cv;
using namespace Track;
using namespace Uart;

Scalar hsv_hue_lim = HUE_LIM_DEFAULT;
Scalar hsv_sat_lim = SAT_LIM_DEFAULT;
Scalar hsv_val_lim = VAL_LIM_DEFAULT;

mutex uart_mtx;
float uart_state[4];

queue<Mat> frame_queue;
queue<clock_t> cap_time_queue;
bool cap_quit = false;
bool uart_quit = false;

static Point2f velocity(Point2f curr, clock_t curr_time, Point2f prev, clock_t prev_time) {
    double delta_t = (curr_time - prev_time) / CLOCKS_PER_MSEC;
    cout << "Delta T " << delta_t << endl;;
    float v_x = (curr.x - prev.x) / delta_t;
    float v_y = (curr.y - prev.y) / delta_t;

    return Point2f(v_x, v_y);
}

/* Video capture thread */
void video_cap_thread(void) {

    VideoCapture cam(0);
    if(!cam.isOpened()) {
	cerr << "Failed to open camera!" << endl;
	return;
    }

    Mat frame;
    int FramerCounter = 0;
    clock_t StartTime,EndTime;

    while(!cap_quit) {
	cam.read(frame);
	cap_time_queue.push(clock());
	frame_queue.push(frame);

        if(FramerCounter==0) 
            StartTime=clock();

        FramerCounter++;
        EndTime=clock();
        if((EndTime-StartTime)/CLOCKS_PER_SEC>=1){
            //cout << "\rFPS: " << FramerCounter << flush;
            FramerCounter=0;
        }
    }

    cam.release();
}

/* Uart thread */
void uart_thread(void) {
    if(uart_init() != EXIT_SUCCESS) {
	cerr << "Failed to initialize uart\n";
	return;
    }

    float state[4];

    size_t i = 0;
    while(!uart_quit) {
        uart_mtx.lock();
        memcpy(state, uart_state, 4*sizeof(float));
        uart_mtx.unlock();

        for(i = 0; i < 4; i++) {
            uart_write(&state[i], sizeof(float));
            delay(3);
        }
    }

    memset(state, 0, 4*sizeof(float));
    for(i = 0; i < 4; i++) {
        uart_write(&state[i], sizeof(float));
        delay(3);
    }

    uart_release();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    int canny_param = CANNY_DEFAULT;

    cout << "VelX,VelY" << endl;

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

    thread video_capture(video_cap_thread);
    thread uart_send(uart_thread);

    Mat hsv;
    Mat thres;
    vector<vector<Point>> circles;
    vector<Point2f> centroids;
    Point2f vel(0, 0);
    Point2f prev_centroid(0, 0);
    clock_t prev_time = 0;

#ifdef GUI_DEMO
    cout << "Press any key to exit..." << endl;
#endif

    size_t largest_contour;
    while(true) {
	if(frame_queue.empty()) {
	    continue;
	}

	cvtColor(frame_queue.front(), hsv, CV_BGR2HSV);

#ifdef GUI_DEMO
	hsv_hue_lim = Scalar(hsv_hue_lo, hsv_hue_hi);
	hsv_sat_lim = Scalar(hsv_sat_lo, hsv_sat_hi);
	hsv_val_lim = Scalar(hsv_val_lo, hsv_val_hi);
#endif

	hsv_threshold(hsv, thres, hsv_hue_lim, hsv_sat_lim, hsv_val_lim);
	detect_circles(thres, circles, canny_param);

	if(circles.size()) {
	    centroids = vector<Point2f>(circles.size());
	    calc_centroids(centroids, circles);

	    largest_contour = 0;
	    for(size_t i = 0; i < circles.size(); i++) {
		if(arcLength(circles[largest_contour], true) < arcLength(circles[i], true)) {
		    largest_contour = i;
		}
	    }

#ifdef GUI_DEMO
	    drawContours(frame_queue.front(), circles, largest_contour, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, noArray(), 0, Point());
	    circle(frame_queue.front(), centroids[largest_contour], 1, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);

	    imshow("Tracking", frame_queue.front());
	    imshow("Thresholding", thres);
	    if(waitKey(1) > 0) {
	        break;
	    }
#endif

            vel = velocity(Point2f(640, 480) - centroids[largest_contour], cap_time_queue.front(),
                    prev_centroid, prev_time);

            cout << vel.x << "," << vel.y << endl;

            uart_mtx.lock();
            uart_state[0] = 640 - centroids[largest_contour].x;
            uart_state[1] = 480 - centroids[largest_contour].y;
            uart_state[2] = vel.x;
            uart_state[3] = vel.y;
            uart_mtx.unlock();

            prev_centroid = centroids[largest_contour];
            prev_time = cap_time_queue.front();
        }

	frame_queue.pop();
	cap_time_queue.pop();
    }

    cap_quit = true;
    uart_quit = true;

    queue<Mat>().swap(frame_queue);
    queue<clock_t>().swap(cap_time_queue);
#ifdef GUI_DEMO
    destroyAllWindows();
#endif

    return EXIT_SUCCESS;
}
