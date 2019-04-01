#pragma pack(push,1)
struct packet {
	int32_t x;
	int32_t y;
	float vel;
};
#pragma pack(pop)

namespace UDP {
    int udp_init(size_t send_len, size_t recv_len);
    void udp_send(struct packet *data);
    void udp_recv(struct packet *data);
};
