CC=g++
CFLAGS=-Wall -Wextra -Wpedantic -g

#SRC=$(wildcard *.c)
SRC=$(wildcard *.cpp)
LIB=`pkg-config --cflags --libs opencv`

all : bin gui

bin : $(SRC)
	$(CC) $(CFLAGS) -o vis $^ -I. $(LIB)

gui : $(SRC)
	$(CC) $(CFLAGS) -o gui-vis $^ -I. $(LIB) -DGUI_DEMO

clean:
	rm -f gui
	rm -f vis
