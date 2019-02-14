#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

struct video_cap_ptrs {
    VideoCapture cam;
    Mat frame;
};

/* Video capture thread */
void *video_cap_thread(void *userdata) {
    struct video_cap_ptrs *params = (struct video_cap_ptrs *)userdata;
    VideoCapture cam = params->cam;
    Mat frame = params->frame;
    Mat grayscale;
    Mat threshold;
    Mat hsv;
    vector<Vec3f> circles;

    printf("Press any key to exit!\n");
    while(1) {
        cam.read(frame);
        
        cvtColor(frame, hsv, CV_BGR2HSV);
        inRange(hsv, Scalar(24, 255, 194), Scalar(40, 232, 255), threshold);
        cvtColor(hsv, threshold, CV_HSV2RGB);
        cvtColor(threshold, grayscale, CV_RGB2GRAY);
        HoughCircles(grayscale, circles, CV_HOUGH_GRADIENT, 1, frame.rows / 16, 100, 60, 30, 500);

        if(circles.size()) {
            Vec3i c = circles[0];
            Point center = Point(c[0], c[1]);

            circle(frame, center, 1, Scalar(0, 100, 100), 3, 8);
            circle(frame, center, c[2], Scalar(255, 0, 255), 3, 8);
        }

        imshow("Tracking", frame);
        imshow("Thresholding", threshold);
        if(waitKey(1) != -1) {
            break;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {

    int err = EXIT_SUCCESS;

    (void)argc;
    (void)argv;

    /* Video capture parameters */
    struct video_cap_ptrs cap_params;

    cap_params.cam = VideoCapture(0);
    cap_params.cam.read(cap_params.frame);

    //cap_params.cam = cam_init();
    //if(!cap_params.cam) {
    //    fprintf(stderr, "Failed to initialize camera!\n");
    //    err = EXIT_FAILURE;
    //    goto exit;
    //}

    //cap_params.frame = cam_init_frame(cap_params.cam);
    //if(!cap_params.frame) {
    //    fprintf(stderr, "Failed to initialize frame buffer!\n");
    //    err = EXIT_FAILURE;
    //    goto exit;
    //}

    namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholding", CV_WINDOW_AUTOSIZE);

    pthread_t cap_thread;
    pthread_create(&cap_thread, NULL, &video_cap_thread, &cap_params);
    if(!cap_thread) {
        fprintf(stderr, "Failed to create capture thread!\n");
        goto exit;
    }

    pthread_join(cap_thread, NULL);

exit:

    //cam_destroy(&cap_params.cam);
    //cam_destroy_frame(&cap_params.frame);
    destroyAllWindows();

    return err;
}
