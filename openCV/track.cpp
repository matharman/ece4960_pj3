#include "include/track.h"

#define HOUGH_PARAM_1 100
#define HOUGH_PARAM_2 60
#define HOUGH_MIN_RADIUS 45
#define HOUGH_MAX_RADIUS 400

using namespace cv;
using namespace std;

/* Given an HSV frame, return the thresholded HSV
 * for hue_lim*, sat_lim*, v_lim* */
Mat Track::hsv_threshold(Mat hsv, Scalar hue_lim, Scalar sat_lim, Scalar val_lim) {
    Mat thres;

    Scalar hsv_lim_lo(hue_lim[0], sat_lim[0], val_lim[0]);
    Scalar hsv_lim_hi(hue_lim[1], sat_lim[1], val_lim[1]);

    inRange(hsv, hsv_lim_lo, hsv_lim_hi, thres);

    return thres;
}

/* Detect circles in the HSV-thresholded image 
 * and return vector containg [x, y, radius] for
 * each circle */
vector<Vec3f> Track::detect_circles(Mat thres) {
    Mat rgb;
    Mat gray;

    cvtColor(thres, rgb, CV_HSV2RGB);
    cvtColor(rgb, gray, CV_RGB2GRAY);

    vector<Vec3f> circles;
    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 
            gray.rows / 16, HOUGH_PARAM_1, HOUGH_PARAM_2,
            HOUGH_MIN_RADIUS, HOUGH_MAX_RADIUS);

    return circles;
}
