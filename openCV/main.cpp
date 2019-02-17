#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <queue>
#include<opencv2/opencv.hpp>

#include "include/track.h"

#define HUE_LIM_DEFAULT Scalar(0, 60)
#define SAT_LIM_DEFAULT Scalar(100, 255)
#define VAL_LIM_DEFAULT Scalar(200, 255)

#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE 8

#define DELTA(curr, prev) \
    (curr - prev)
#define VELOCITY(curr, prev) \
    (DELTA(curr.y, prev.y) / DELTA(curr.x, prev.x))

using namespace std;
using namespace cv;
using namespace Track;

Scalar hsv_hue_lim = HUE_LIM_DEFAULT;
Scalar hsv_sat_lim = SAT_LIM_DEFAULT;
Scalar hsv_val_lim = VAL_LIM_DEFAULT;

queue<Mat> frame_queue;
bool cap_quit = false;

/* Video capture thread */
void video_cap_thread(void) {

    VideoCapture cam(0);
    if(!cam.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return;
    }

    Mat frame;

    while(!cap_quit) {
        cam.read(frame);
        frame_queue.push(frame);
    }

    cam.release();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    ofstream circle_data;

    circle_data.open("circle_data.txt");
    circle_data << "X, Y, Vel\n";

#ifdef GUI_DEMO
    namedWindow("Thresh Params", CV_WINDOW_AUTOSIZE);
    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);

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

#endif

    thread video_capture(video_cap_thread);

    Mat hsv;
    Mat thres;
    vector<vector<Point>> circles;
    vector<Point2f> centroids;
    Point2f prev(0, 0);

#ifdef GUI_DEMO
    cout << "Press any key to exit..." << endl;
#endif

    while(true) {
        if(frame_queue.empty()) {
            continue;
        }

        cvtColor(frame_queue.front(), hsv, CV_BGR2HSV);

        /* Threshold HSV according to compile time parameters */
#ifdef GUI_DEMO
        hsv_hue_lim = Scalar(hsv_hue_lo, hsv_hue_hi);
        hsv_sat_lim = Scalar(hsv_sat_lo, hsv_sat_hi);
        hsv_val_lim = Scalar(hsv_val_lo, hsv_val_hi);
#endif
        hsv_threshold(hsv, thres, hsv_hue_lim, hsv_sat_lim, hsv_val_lim);
        detect_circles(thres, circles);

        if(centroids.size()) {
            prev = centroids[0];
        }
        
        centroids = vector<Point2f>(circles.size());
        calc_centroids(centroids, circles);

        //circle_data << centroids[0].x << "," << centroids[0].y << "," << VELOCITY(centroids[0], prev) << endl;

        for(size_t i = 0; i < circles.size(); i++) {
            drawContours(frame_queue.front(), circles, i, 
                    CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, noArray(), 0, Point());
            circle(frame_queue.front(), centroids[i], 3, CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);
        }

#ifdef GUI_DEMO
        imshow("Tracking", frame_queue.front());
        imshow("Thresholding", thres);
        if(waitKey(1) > 0) {
            break;
        }
#endif
    
        frame_queue.pop();
    }

    cap_quit = true;
    queue<Mat>().swap(frame_queue);
    destroyAllWindows();

    circle_data.close();

    return EXIT_SUCCESS;
}
