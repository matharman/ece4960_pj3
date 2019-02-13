#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <opencv2/core/types_c.h>

#include "include/cam.h"
#include "include/track.h"

struct video_cap_ptrs {
    CvCapture *cam;
    IplImage *frame;
};

/* Video capture thread */
void *video_cap_thread(void *userdata) {
    struct video_cap_ptrs *params = (struct video_cap_ptrs *)userdata;
    CvCapture * cam = params->cam;
    IplImage *frame = params->frame;
    IplImage *grayscale = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);

    CvMemStorage *circles_storage = cvCreateMemStorage(0);
    //CvSeq *circles = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint3D32f), seq_buf);
    CvSeq *circles = NULL;

    printf("Press any key to exit!\n");
    while(1) {
        if(cam_capture_frame(cam, frame) != EXIT_SUCCESS) {
            fprintf(stderr, "Frame capture failed!\n");
            return NULL;
        }
        
        cvCvtColor(frame, grayscale, CV_BGR2GRAY);
        circles = cvHoughCircles(grayscale, circles_storage, CV_HOUGH_GRADIENT, 1, frame->height / 4, 100, 100, 200, 100);

        CvPoint3D32f circle;
        cvSeqPop(circles, &circle);
    
        cvCircle(frame, cvPoint(circle.x, circle.y), circle.z, cvScalar(0, 0, 0, 0), 10, 8, 0);
        cvShowImage("Debug Stream", frame);
        if(cvWaitKey(1) != -1) {
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
    struct video_cap_ptrs cap_params = {
        .cam = NULL,
        .frame = NULL
    };

    cap_params.cam = cam_init();
    if(!cap_params.cam) {
        fprintf(stderr, "Failed to initialize camera!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    cap_params.frame = cam_init_frame(cap_params.cam);
    if(!cap_params.frame) {
        fprintf(stderr, "Failed to initialize frame buffer!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    cvNamedWindow("Debug Stream", CV_WINDOW_AUTOSIZE);

    pthread_t cap_thread;
    pthread_create(&cap_thread, NULL, &video_cap_thread, &cap_params);
    if(!cap_thread) {
        fprintf(stderr, "Failed to create capture thread!\n");
        goto exit;
    }

    pthread_join(cap_thread, NULL);

exit:

    cam_destroy(&cap_params.cam);
    cam_destroy_frame(&cap_params.frame);
    cvDestroyAllWindows();

    return err;
}
