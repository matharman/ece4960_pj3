CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g

SRC=$(wildcard *.c)
LIB=`pkg-config --cflags --libs opencv`
BIN=pj3_vis

#$(BIN) : $(SRC)
#	$(CC) $(CFLAGS) -o $(BIN) $^ -I. $(LIB)

all: CAM HMOG

CAM : cam.c main.c
	$(CC) $(CFLAGS) -o cam $^ -I. $(LIB)

HMOG : homography.c main.c
	$(CC) $(CFLAGS) -o hmog $^ -I. $(LIB)

TRACK:

DETECT:

clean:
	echo "Cleaning binaries..."
	rm pj3_vis
	rm cam
	rm hmog
	echo "Done!"
