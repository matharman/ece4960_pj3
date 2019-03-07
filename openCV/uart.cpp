#include <iostream>

#include "include/uart.h"

#define UART_DEVICE "/dev/ttyS0"
#define UART_BAUD 9600

using namespace std;

static int uart_port;

int Uart::uart_init(void) {
    if((uart_port = serialOpen(UART_DEVICE, UART_BAUD)) == -1) {
        cerr << "Failed to open " << UART_DEVICE << endl;
        return EXIT_FAILURE;
    }

    if(wiringPiSetup() == -1) {
        cerr << "Failed to init wiring Pi\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void Uart::uart_release(void) {
    serialClose(uart_port);
}

int Uart::uart_write(void *data, size_t len) {
    if(!data) {
        cerr << "Invalid arg to uart_write\n";
        return EXIT_FAILURE;
    }

    unsigned char *bytes = (unsigned char *)data;

    for(size_t i = 0; i < len; i++) {
        serialPutchar(uart_port, bytes[i]);
    }

    return EXIT_SUCCESS;
}
