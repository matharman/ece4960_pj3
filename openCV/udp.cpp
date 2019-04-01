#include "stdafx.h"
#include <winsock2.h>
#include "xPCUDPSock.h"

#include <iostream>
#include "udp.h"

/* Must configure MPCOM server to use these ports in UDP mode */
#define SEND_PORT (12402)
#define SEND_ADDR "127.0.0.1"
#define RECV_PORT (12403)

static CUDPSender sender;
static CUDPReceiver receiver;

int UDP::udp_init(size_t send_len, size_t recv_len) {
    if(!InitUDPLib()) {
        std::cerr << "Failed to initialize UDP lib" << std::endl;
        return EXIT_FAILURE;
    }

    sender = CUDPSender(sizeof(struct packet), SEND_PORT, SEND_ADDR);
    receiver = CUDPReceiver(sizeof(struct packet), RECV_PORT);

    return EXIT_SUCCESS;
}

void UDP::udp_send(struct packet *data) {
    sender.SendData(data);
}

void UDP::udp_recv(struct packet *data) {
    receiver.GetData(data);
}
