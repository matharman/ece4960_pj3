#include <iostream>
#include "include/track.h"

#define HOUGH_PARAM_1 100
#define HOUGH_PARAM_2 60
#define HOUGH_MIN_RADIUS 0
#define HOUGH_MAX_RADIUS 1000

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
void Track::detect_circles(Mat thres, vector<Vec3f> &circles) {
    HoughCircles(thres, circles, CV_HOUGH_GRADIENT, 1, 
            0, HOUGH_PARAM_1, HOUGH_PARAM_2,
            HOUGH_MIN_RADIUS, HOUGH_MAX_RADIUS);
}
