CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g

LIB=`pkg-config --cflags --libs opencv`

all : CAM TRACK HOMOGRAPHY DETECT

CAM : cam.c main.c
	$(CC) $(CFLAGS) -o cam $^ -I. $(LIB)

TRACK:

HOMOGRAPHY:

DETECT:

clean:
	rm cam
