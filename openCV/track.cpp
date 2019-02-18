#include "include/track.h"

#define EROSIONS 3
#define DILATIONS 5

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
void Track::detect_circles(Mat &thres, vector<vector<Point>> &circles, int canny_param) {
    Mat buf;

    erode(thres, buf, Mat(), Point(-1, -1), EROSIONS);
    dilate(buf, thres, Mat(), Point(-1, -1), DILATIONS);

    Canny(thres, buf, canny_param, canny_param * 2, 3);
    findContours(buf, circles, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
}

/* Calculate the centroids of circles */
void Track::calc_centroids(vector<Point2f> &centroids, vector<vector<Point>> circles) {
    vector<Moments> mu(circles.size());

    for(size_t i = 0; i < mu.size(); i++) {
        mu[i] = moments(circles[i], true);
    }

    for(size_t i = 0; i < mu.size(); i++) {
        centroids[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }
}
