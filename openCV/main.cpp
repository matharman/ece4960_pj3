#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <queue>
#include<opencv2/opencv.hpp>

#include "include/track.h"

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

    Scalar hsv_hue_lim(20, 50);
    Scalar hsv_sat_lim(100, 255);
    Scalar hsv_val_lim(100, 255);

#ifdef GUI_DEMO
    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);
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

    //clock_t start;
    //clock_t end;

    while(true) {
        if(frame_queue.empty()) {
            continue;
        }

        //start = clock();
        cvtColor(frame_queue.front(), hsv, CV_BGR2HSV);

        /* Threshold HSV according to compile time parameters */
        hsv_threshold(hsv, thres, hsv_hue_lim, hsv_sat_lim, hsv_val_lim);
        detect_circles(thres, circles);

        if(centroids.size()) {
            prev = centroids[0];
        }
        
        centroids = vector<Point2f>(circles.size());
        calc_centroids(centroids, circles);
        /* X, Y, Vel */
        circle_data << centroids[0].x << "," << centroids[0].y << "," << VELOCITY(centroids[0], prev) << endl;

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

        //end = clock();
        //cout << "\rTime: " << 1000 * (end - start) / CLOCKS_PER_SEC << "ms" << flush;

    }

    cap_quit = true;
    queue<Mat>().swap(frame_queue);
    destroyAllWindows();

    circle_data.close();

    return EXIT_SUCCESS;
}
