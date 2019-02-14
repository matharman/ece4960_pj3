/* pj3_cam.c: Contains CL-Eye Platform interface to the PS3 Eye camera
 *            and wrapping functions to clean up init and deinit of relevant
              structures
 */

#include <stdio.h>
#include <stdlib.h>

#include "include/cam.h"

#define CAM_FRAMERATE 60
#define CAM_EXPOSURE 511
#define CAM_GAIN 0

CvCapture *cam_init(void) {
    int err = 0;

    CvCapture *new = cvCaptureFromCAM(0);
    if(!new) {
        fprintf(stderr, "Failed to open video capture!\n");
        err = -1;
        goto exit;
    }

#if 0
    if(cvSetCaptureProperty(new, CV_CAP_PROP_FPS, (double)CAM_FRAMERATE) == 0) {
        fprintf(stderr, "Failed to set Cam FPS\n");
        err = -1;
        goto exit;
    }

    if(cvSetCaptureProperty(new, CV_CAP_PROP_EXPOSURE, (double)CAM_EXPOSURE) == 0) {
        fprintf(stderr, "Failed to set Cam FPS\n");
        err = -1;
        goto exit;
    }

    if(cvSetCaptureProperty(new, CV_CAP_PROP_GAIN, (double)CAM_GAIN) == 0) {
        fprintf(stderr, "Failed to set Cam FPS\n");
        err = -1;
        goto exit;
    }
#endif
    
exit:

    if(err == -1 && new) {
        cam_destroy(&new);
    }
    
    return new;
}

void cam_destroy(CvCapture ** cam) {
    if(cam && *cam) {
        cvReleaseCapture(cam);
        *cam = NULL;
    }
}

IplImage *cam_init_frame(CvCapture *cam) {
    IplImage *new = NULL;

    if(!cam) {
        fprintf(stderr, "Invalid arguments!\n");
        goto exit;
    }

    IplImage *framebuf = cvQueryFrame(cam);
    if(!framebuf) {
        fprintf(stderr, "Failed to query frame data!\n");
        goto exit;
    }

    new = cvCloneImage(framebuf);
    if(!new) {
        fprintf(stderr, "Failed to clone queried frame!\n");
        goto exit;
    }

exit:

    return new;
}

void cam_destroy_frame(IplImage **frame) {
    if(frame && *frame) {
        cvReleaseImage(frame);
        *frame = NULL;
    }
}

int cam_capture_frame(CvCapture *cam, IplImage *dest) {
    int err = EXIT_SUCCESS;

    if(!cam || !dest) {
        fprintf(stderr, "Invalid arguments!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    IplImage *curr_frame = cvQueryFrame(cam);
    if(!curr_frame) {
        fprintf(stderr, "Failed to get new frame!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    cvCopy(curr_frame, dest, NULL);

exit:

    return err;
}

size_t cam_frame_r(CvCapture *cam) {
    return (size_t)cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT);
}

size_t cam_frame_c(CvCapture *cam) {
    return (size_t)cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH);
}
