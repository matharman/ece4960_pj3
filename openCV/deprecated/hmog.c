#include <stdio.h>

#include "include/hmog.h"

/* Corner points of image plane from which to generate homography */
#define HMOG_PT_NE 0F
#define HMOG_PT_NW 0F
#define HMOG_PT_SE 0F
#define HMOG_PT_SW 0F

#define HMOG_R_DIM 3
#define HMOG_C_DIM 3
