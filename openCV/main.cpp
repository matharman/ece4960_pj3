#include<opencv2/opencv.hpp>
#include <iostream>
#include <pthread.h>

#include "include/track.h"

#define HSV_LIM_H (Scalar(24, 40))
#define HSV_LIM_S (Scalar(255, 232))
#define HSV_LIM_V (Scalar(194, 255))

#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE CV_AA

using namespace std;
using namespace cv;
using namespace Track;

/* Video capture thread */
void *video_cap_thread(void *userdata) {
    (void)userdata;

    VideoCapture cam(0);
    if(!cam.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return NULL;
    }

    Mat frame;
    Mat hsv;
    Mat thres;
    vector<Vec3f> circles;

    cout << "Press any key to exit!" << endl;
    while(true) {
        cam.read(frame);
        cvtColor(frame, hsv, CV_BGR2HSV);

        /* Threshold HSV according to compile time parameters */
        thres = hsv_threshold(hsv, HSV_LIM_H, HSV_LIM_S, HSV_LIM_V);
        circles = detect_circles(thres);

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
        if(waitKey(1) > 0) {
            break;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);

    pthread_t cap_thread;
    pthread_create(&cap_thread, NULL, &video_cap_thread, NULL);
    if(!cap_thread) {
        cerr << "Failed to create capture thread!" << endl;
        return EXIT_FAILURE;
    }

    pthread_join(cap_thread, NULL);

    return EXIT_SUCCESS;
}
