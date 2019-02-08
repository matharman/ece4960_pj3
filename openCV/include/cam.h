#ifndef CAM_H
#define CAM_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

CvCapture * cam_init(void);
void cam_destroy(CvCapture ** cam);
int cam_capture_frame(CvCapture *cam, IplImage *dest);
size_t cam_frame_r(CvCapture *cam);
size_t cam_frame_c(CvCapture *cam);

#endif
