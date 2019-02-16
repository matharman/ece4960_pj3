#include "include/track.h"

#define CANNY_PARAM_1 60
#define CANNY_PARAM_2 120

using namespace cv;
using namespace std;

/* Given an HSV frame, return the thresholded HSV
 * for hue_lim*, sat_lim*, v_lim* */
void Track::hsv_threshold(Mat hsv, Mat &thres, Scalar hue_lim, Scalar sat_lim, Scalar val_lim) {
    Scalar hsv_lim_lo(hue_lim[0], sat_lim[0], val_lim[0]);
    Scalar hsv_lim_hi(hue_lim[1], sat_lim[1], val_lim[1]);

    inRange(hsv, hsv_lim_lo, hsv_lim_hi, thres);
}

/* Detect circles in the HSV-thresholded image 
 * and return vector containg [x, y, radius] for
 * each circle */
void Track::detect_circles(Mat &thres, vector<vector<Point>> &circles) {
    Mat buf;

    erode(thres, buf, Mat(), Point(-1, -1), 4);
    dilate(buf, thres, Mat(), Point(-1, -1), 5);

    Canny(thres, buf, CANNY_PARAM_1, CANNY_PARAM_2, 3);
    findContours(buf, circles, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
}
