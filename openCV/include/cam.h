#ifndef CAM_H
#define CAM_H

#include <opencv2/highgui/highgui.h>
#include <opencv2/videoio/videoio_c.h>

CvCapture * cam_init(void);
void cam_destroy(CvCapture * cam);
int cam_capture_frame(CvCapture *cam, CvMat *dest);

#endif
