#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include <stdio.h>
#include <stdlib.h>

#include "include/cam.h"

typedef struct _vid_cap_data {
    CLEyeCameraInstance cam;
    PBYTE frame;
} VideoCapData;

typedef struct _img_proc_data {
    IplImage *img;
    int avg_fps;
} ImgProcData;

/* Video capture thread */
static DWORD WINAPI VideoCapThread(LPVOID user_data) {
    VideoCapData *params = (VideoCapData *)user_data;

    while(1) {
        CLEyeCameraGetFrame(params->cam,params->frame);
    }
}

int _tmain(int argc, _TCHAR* argv[]) {

    int err = EXIT_SUCCESS;

    /* Video capture parameters */
    HANDLE cap_thread;
    VideoCapData cap_params = {
        .cam = NULL,
        .frame = NULL
    };

	cap_params.cam = cam_init();
    if(!cap_params.cam) {
        err = EXIT_FAILURE;
        goto exit;
    }

    cap_thread = CreateThread(NULL, 0, &VideoCapThread, &cap_params, 0, 0);
    if(!cap_thread) {
        fprintf(stderr, "Failed to create Capture Thread!\n");
        err = EXIT_FAILURE;
        goto exit;
    }

exit:

    cam_destroy(cap_params.cam);

    return err;
}