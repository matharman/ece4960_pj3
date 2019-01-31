#include <stdio.h>
#include <stdlib.h>

#include <cv.h>
#include <cxcore.h>
//#include <highgui.h>

#include "include/cam.h"

typedef struct _vid_cap_data {
    CLEyeCameraInstance cam;
    PBYTE frame;
} VideoCapData;

/* Video capture thread */
static DWORD WINAPI VideoCapThread(LPVOID user_data) {
    VideoCapData *params = (VideoCapData *)user_data;

    while(1) {
        /* Capture Frame */
        CLEyeCameraGetFrame(params->cam,params->frame);

        /* Threshold for Ping Pong via HSV */
        printf("Threshold stub\n");

        /* */
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

    if(!(cap_params.cam = cam_init())) {
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