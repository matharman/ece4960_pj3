#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "include/cam.h"

typedef struct _vid_cap_data {
    CvCapture *cam;
    CvMat frame;
} video_cap_data_t;

/* Video capture thread */
void *video_cap_thread(void *userdata) {
    video_cap_data_t *params = (video_cap_data_t *)userdata;

    size_t frame_rows = cam_frame_r(params->cam);
    size_t frame_cols = cam_frame_c(params->cam);

    CvMat *frame = cvCreateMat(frame_rows, frame_cols, CV_8UC1);

    namedWindow("Debug Stream", WINDOW_AUTOSIZE);

    while(1) {
        if(cam_capture_frame(params->cam, frame) != EXIT_SUCCESS) {
            fprintf(stderr, "Frame capture failed!\n");
            return NULL;
        }

        imshow("Debug Stream", frame);
    }
}

int main(int argc, char* argv[]) {

    int err = EXIT_SUCCESS;

    (void)argc;
    (void)argv;

    /* Video capture parameters */
    pthread_t cap_thread;
    video_cap_data_t cap_params = {
        .cam = NULL,
        .frame = NULL
    };

    cap_params.cam = cam_init();
    if(!cap_params.cam) {
        err = EXIT_FAILURE;
        goto exit;
    }

    cap_thread = pthread_create(&cap_thread, NULL, &video_cap_thread, &cap_params);
    if(!cap_thread) {
        fprintf(stderr, "Failed to create Capture Thread!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    pthread_join(&cap_thread, NULL);

exit:

    cam_destroy(cap_params.cam);

    return err;
}
