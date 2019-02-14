#ifndef TRACK_H
#define TRACK_H

#include <opencv2/opencv.hpp>

#define TRACK_CIRCLE_RADIUS(vec_elem) (vec_elem[2])
#define TRACK_CIRCLE_CENTER(vec_elem) \
    (Point(vec_elem[0], vec_elem[1]))

namespace Track {

    /* Given an HSV frame, return the thresholded HSV
     * for hue_lim*, sat_lim*, v_lim* */
    cv::Mat hsv_threshold(cv::Mat hsv, 
            cv::Scalar hue_lim, cv::Scalar sat_lim, cv::Scalar val_lim);

    /* Detect circles in the HSV-thresholded image 
     * and return vector containg [x, y, radius] for
     * each circle */
    std::vector<cv::Vec3f> detect_circles(cv::Mat thres);
}

#endif
