#include<opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

#include "include/track.h"

//#define HSV_LIM_H(min, max) (Scalar(min, max))
//#define HSV_LIM_S(min, max) (Scalar(min, max))
//#define HSV_LIM_V(min, max) (Scalar(min, max))

#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE 8

using namespace std;
using namespace cv;
using namespace Track;

queue<Mat> frame_queue;
mutex q_mtx;

/* Video capture thread */
void video_cap_thread(void) {
    unique_lock<mutex> q_lock(q_mtx, defer_lock);

    VideoCapture cam(0);
    if(!cam.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return;
    }

    Mat frame;

    while(true) {
        cam.read(frame);
        q_lock.lock();
        frame_queue.push(frame);
        q_lock.unlock();
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Scalar hsv_hue_lim(20, 50);
    Scalar hsv_sat_lim(100, 255);
    Scalar hsv_val_lim(100, 255);

#if 0
    namedWindow("Parameters", CV_WINDOW_AUTOSIZE);
    createTrackbar("Hue Hi", "Parameters", (int *)&hsv_hue_lim[0], 255, NULL, NULL);
    createTrackbar("Hue Lo", "Parameters", (int *)&hsv_hue_lim[1], 255, NULL, NULL);
    createTrackbar("Sat Hi", "Parameters", (int *)&hsv_sat_lim[0], 255, NULL, NULL);
    createTrackbar("Sat Lo", "Parameters", (int *)&hsv_sat_lim[1], 255, NULL, NULL);
    createTrackbar("Val Hi", "Parameters", (int *)&hsv_val_lim[0], 255, NULL, NULL);
    createTrackbar("Val Lo", "Parameters", (int *)&hsv_val_lim[1], 255, NULL, NULL);
#endif
    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);
    //namedWindow("RGB", CV_WINDOW_AUTOSIZE);
    //namedWindow("GRAY", CV_WINDOW_AUTOSIZE);

    thread video_capture(video_cap_thread);
    unique_lock<mutex> q_lock(q_mtx, defer_lock);

    Mat frame;
    Mat hsv;
    Mat thres;
    //Mat rgb;
    //Mat gray;
    vector<Vec3f> circles;

    cout << "Press any key to exit" << endl;
    while(true) {
        if(frame_queue.empty()) {
            continue;
        }

        q_lock.lock();
        frame = frame_queue.front();
        frame_queue.pop();
        q_lock.unlock();

        cvtColor(frame, hsv, CV_BGR2HSV);

        /* Threshold HSV according to compile time parameters */
        hsv_threshold(hsv, thres, hsv_hue_lim, hsv_sat_lim, hsv_val_lim);
        detect_circles(thres, circles);

        for(size_t i = 0; i < circles.size(); i++) {
            /* Draw circle center */
            circle(frame, TRACK_CIRCLE_CENTER(circles[i]), 1,
                    CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE);

            /* Draw circle contour */
            circle(frame, TRACK_CIRCLE_CENTER(circles[i]), 
                    TRACK_CIRCLE_RADIUS(circles[i]), CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE);
        }
        
        imshow("Tracking", frame);
        imshow("Thresholding", thres);
        //imshow("RGB", rgb);
        //imshow("GRAY", gray);
        if(waitKey(1) > 0) {
            break;
        }
    }

    return EXIT_SUCCESS;
}
