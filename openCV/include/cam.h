#ifndef CAM_H
#define CAM_H

#include "CLEyeMulticam.h"

CLEyeCameraInstance cam_init(void);
void cam_destroy(CLEyeCameraInstance cam);
int cam_frame_width(void);
int cam_frame_height(void);

#endif