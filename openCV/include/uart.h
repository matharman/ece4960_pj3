#ifndef UART_H
#define UART_H

#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>

namespace Uart {
    int uart_init(void);

    void uart_release(void);

    int uart_write(void *data, size_t len);
}

#endif
