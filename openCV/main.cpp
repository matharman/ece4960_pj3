#include <iostream>
#include <csignal>
#include <ctime>
#include <thread>
#include <mutex>
#include <queue>

#include <opencv2/opencv.hpp>
#include "include/track.h"
#include "include/udp.h"

#define CANNY_DEFAULT 60

/* Thresholding parameters */
#define HUE_LIM_DEFAULT Scalar(0, 60)
#define SAT_LIM_DEFAULT Scalar(150, 255)
#define VAL_LIM_DEFAULT Scalar(200, 255)

/* GUI drawing parameters */
#define CIRCLE_COLOR_RGB (Scalar(255, 0, 255))
#define CIRCLE_THICKNESS 3
#define CIRCLE_LINE_TYPE 8

using namespace std;
using namespace cv;
using namespace Track;
using namespace UDP;

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
struct packet vis;

bool cap_quit = false;
bool udp_quit = false;

thread udp_thread_handle;
thread video_cap_thread_handle;

queue<Mat> frame_queue;
queue<clock_t> cap_time_queue;

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

    N_pos[N_FRAME_COUNT - 1] = Point(curr_pos.x - 320, 480 - curr_pos.y);
    N_time[N_FRAME_COUNT - 1] = curr_time;
}

/* Video capture thread */
void video_cap_thread(void) {
    VideoCapture cam(0);
    if(!cam.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return;
    }

    cam.set(CV_CAP_PROP_FPS, 60.0);

    Mat frame;
    int FramerCounter = 0;
    clock_t StartTime,EndTime;

    while(!cap_quit) {
        cam.read(frame);
        cap_time_queue.push(clock());
        frame_queue.push(frame);

        if(FramerCounter == 0) {
            StartTime = clock();
        }

        FramerCounter++;
        EndTime = clock();
        if((EndTime - StartTime) / CLOCKS_PER_SEC >= 1){
            //cout << "\rFPS: " << FramerCounter << flush;
        }
    }

    cam.release();
}

/* UDP thread */
void udp_thread(void) {
    if(udp_init(sizeof(struct packet), sizeof(struct packet)) != EXIT_SUCCESS) {
        cerr << "Failed to initialize uart\n";
        return;
    }

    struct packet pack;
    memset(&pack, 0, sizeof(pack));

    while(!udp_quit) {
        udp_mtx.lock();
        memcpy(&pack, &vis, sizeof(pack));
        udp_mtx.unlock();

        udp_send(&pack);
        Sleep(1);
    }

    memset(&pack, 0, sizeof(pack));
    udp_send(&pack, sizeof(pack));
}

void handle_sigint(int signum) {
    if(signum == SIGINT) {
        cap_quit = true;
        udp_quit = true;

        udp_thread_handle.join();
        video_cap_thread_handle.join();

        queue<Mat>().swap(frame_queue);
        queue<clock_t>().swap(cap_time_queue);

        exit(0);
    }
}

int main(void) {
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

    video_cap_thread_handle = thread(video_cap_thread);
    udp_thread_handle = thread(udp_thread);

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
            centroids = vector<Point>(circles.size());
            calc_centroids(centroids, circles);

            largest_contour = 0;
            for(size_t i = 0; i < circles.size(); i++) {
                if(arcLength(circles[largest_contour], true) < arcLength(circles[i], true)) { 
                    largest_contour = i;
                }
            }

            vel_x = reg_param(N_centroids, N_time);
            update_N_params(centroids[largest_contour], cap_time_queue.front(), N_centroids, N_time);

#ifdef GUI_DEMO
            drawContours(frame_queue.front(), circles, largest_contour, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, noArray(), 0, Point());
            circle(frame_queue.front(), centroids[largest_contour], 1, CIRCLE_COLOR_RGB, 
                    CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);
#endif
        }
        else {
            vel_x = 0;
            update_N_params(Point(320, 480), cap_time_queue.front(), N_centroids, N_time);
        }

        cout << "Time: " << N_time[N_FRAME_COUNT - 1] / (float)CLOCKS_PER_SEC << endl;
        cout << "Pos: " << N_centroids[N_FRAME_COUNT - 1] << endl;
        cout << "Vel: " << vel_x << endl << endl << endl;

        udp_mtx.lock();
        vis.x = N_centroids[N_FRAME_COUNT - 1].x;
        vis.y = N_centroids[N_FRAME_COUNT - 1].y;
        vis.vel = vel_x;
        udp_mtx.unlock();

#ifdef GUI_DEMO
        /* Draw image origin */
        circle(frame_queue.front(), Point(0, 0), 1, CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);

        /* Draw problem domain origin */
        circle(frame_queue.front(), Point(640 / 2 , 480), 1, CIRCLE_COLOR_RGB, CIRCLE_THICKNESS, CIRCLE_LINE_TYPE, 0);
        imshow("Tracking", frame_queue.front());
        imshow("Thresholding", thres);
        if(waitKey(1) > 0) {
            break;
        }
#endif

        frame_queue.pop();
        cap_time_queue.pop();
    }

#ifdef GUI_DEMO
    cap_quit = true;
    udp_quit = true;

    udp_thread_handle.join();
    video_cap_thread_handle.join();

    queue<Mat>().swap(frame_queue);
    queue<clock_t>().swap(cap_time_queue);
    destroyAllWindows();
#endif

    return EXIT_SUCCESS;
}
