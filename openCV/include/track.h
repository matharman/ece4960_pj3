#ifndef TRACK_H
#define TRACK_H

#include <opencv2/opencv.hpp>

#define TRACK_CIRCLE_RADIUS(vec_elem) (vec_elem[2])
#define TRACK_CIRCLE_CENTER(vec_elem) \
    (Point(vec_elem[0], vec_elem[1]))

namespace Track {

    /* Given an HSV frame, return the thresholded HSV
     * for hue_lim*, sat_lim*, v_lim* */
    void hsv_threshold(cv::Mat hsv, cv::Mat &thres,
            cv::Scalar hue_lim, cv::Scalar sat_lim, cv::Scalar val_lim);

    /* Detect circles in the HSV-thresholded image 
     * and return vector containg [x, y, radius] for
     * each circle */
    void detect_circles(cv::Mat &thres, std::vector<std::vector<cv::Point>> &circles);

    /* Calculate the centroids of circles in a list of contours */
    void calc_centroids(std::vector<cv::Point2f> &centroids, std::vector<std::vector<cv::Point>> circles);
}

#endif
