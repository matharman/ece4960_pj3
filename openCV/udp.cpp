#include <iostream>

#include "include/udp.h"

using namespace std;

/* Must configure MPCOM server to use these ports in UDP mode */
#define SEND_PORT (12402)
#define SEND_ADDR "127.0.0.1"
#define RECV_PORT (12403)

static struct udp_cfg {
    CUDPSender sender;
    CUDPReceiver receiver;
} cfg;

int UDP::udp_init(size_t send_len, size_t recv_len) {
    if(!InitUDPLib()) {
        cerr << "Failed to initialize UDP lib" << endl;
        return EXIT_FAILURE;
    }

    cfg.sender = CUDPSender(send_len, SEND_PORT, SEND_ADDR);
    cfg.receiver = CUDPReceiver(recv_len, RECV_PORT);

    return EXIT_SUCCESS;
}

void UDP::udp_send(void *data) {
    cfg.sender.SendData(data);
}

void UDP::udp_recv(void *data) {
    cfg.receiver.GetData(data);
}
