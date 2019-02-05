CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g

OPENCV_LIBS=
LIB=-lpthread $(OPENCV_LIBS)

all : CAM TRACK HOMOGRAPHY DETECT

CAM : cam.c main.c
	$(CC) $(CFLAGS) -o camtest $< -I. $(LIB)
