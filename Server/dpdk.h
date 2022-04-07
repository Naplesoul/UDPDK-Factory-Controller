#include <ofp.h>
#include <string>

int run_dpdk_thread(int argc, char *argv[], int port,
    void (*recv)(const struct ofp_sockaddr_in &addr, void *buf, int nbytes));

void send_msg(const struct ofp_sockaddr_in &addr, const std::string &str);