/* pj3_cam.c: Contains CL-Eye Platform interface to the PS3 Eye camera
 *            and wrapping functions to clean up init and deinit of relevant
              structures
 */

#include <stdio.h>
#include <stdlib.h>

#include "include/cam.h"

#define FRAME_RATE 60
#define FRAME_SIZE CLEYE_VGA
#define FRAME_FORMAT CLEYE_COLOR_RAW

#ifndef CAM_EXPOSURE
    #define CAM_EXPOSURE 511
#endif

#ifndef CAM_GAIN
    #define CAM_GAIN 0
#endif

CvCapture *cam_init(void) {
    CvCapture *new = NULL;

    new = cvCaptureFromCAM(0);
    if(!new) {
        fprintf(stderr, "Failed to open video capture\n");
        goto exit;
    }
    
exit:
    
    return new;
}

void cam_destroy(CvCapture ** cam) {
    if(cam) {
        cvReleaseCapture(cam);
        cam = NULL;
    }
}

int cam_capture_frame(CvCapture *cam, CvMat *dest) {
    int err = EXIT_SUCCESS;

    if(!cam || !dest) {
        fprintf(stderr, "Invalid arguments!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    IplImage *framebuf = cvQueryFrame(cam);
    if(!framebuf) {
        fprintf(stderr, "Failed to get frame!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    if(dest->rows != framebuf->height || dest->cols != framebuf->width) {
        fprintf(stderr, "Frame-Matrix size mismatch!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

    cvCopy(framebuf->imageData, dest->data.ptr, NULL);

exit:

    return err;
}
