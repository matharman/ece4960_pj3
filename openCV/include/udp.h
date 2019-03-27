#include <winsock2.h>
#include "xPCUDPSock.h"

namespace UDP {
    int udp_init(size_t send_len, size_t recv_len);
    void udp_send(void *data);
    void udp_recv(void *data);
};
