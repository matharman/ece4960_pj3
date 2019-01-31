/* pj3_cam.c: Contains CL-Eye Platform interface to the PS3 Eye camera
 *            and wrapping functions to clean up init and deinit of relevant
              structures
 */

#include <stdio.h>

#include "CLEyeMulticam.h"

#define FRAME_RATE 60
#define FRAME_SIZE CLEYE_VGA
#define FRAME_FORMAT CLEYE_COLOR_RAW

#ifndef CAM_EXPOSURE
    #define CAM_EXPOSURE 511
#endif

#ifndef CAM_GAIN
    #define CAM_GAIN 0
#endif

struct _cam_params {
    GUID cam_id;
    int frame_w;
    int frame_h;
};

static struct _cam_params EyeParams;

CLEyeCameraInstance cam_init(void) {

    CLEyeCameraInstance new = NULL;

    if(!ClEyeGetCameraCount();) {
        fprintf(stderr, "No Eye Camera detected!\n");
        goto exit;
    }

    EyeParams.cam_id = CLEyeGetCameraUUID(0);
    if(!(new = CLEyeCreateCamera(EyeParams.cam_id))) {
        fprintf(stderr, "Failed to create Eye Camera!\n");
        goto exit;
    }

    CLEyeSetCameraParameter(EyeCamera, CLEYE_EXPOSURE, CAM_EXPOSURE);
    CLEyeSetCameraParameter(EyeCamera, CLEYE_GAIN, CAM_GAIN);
    CLEyeCameraGetFrameDimensions(new, EyeParams.frame_w, EyeParams.frame_h);
    CLEyeCameraStart(new);

exit:
    
    return new;
}

void cam_destroy(CLEyeCameraInstance cam) {
    if(cam) {
        CLEyeCameraStop(cam);
        CLEyeDestroyCamera(cam);
        cam = NULL;
    }
}

int cam_frame_width(void) {
    return EyeParams.frame_w;
}

int cam_frame_height(void) {
    return EyeParams.frame_h;
}