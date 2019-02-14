CC=g++
CFLAGS=-Wall -Wextra -Wpedantic -g

#SRC=$(wildcard *.c)
SRC=$(wildcard *.cpp)
LIB=`pkg-config --cflags --libs opencv`
BIN=vis

$(BIN) : $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $^ -I. $(LIB)

clean:
	echo "Cleaning binaries..."
	rm pj3_vis
	rm cam
	rm hmog
	echo "Done!"
