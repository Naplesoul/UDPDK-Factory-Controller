#include <thread>
#include "UDPServer.h"
#include "Scheduler.h"
#include "dpdk.h"

#define DEFAULT_SERVER_PORT 8080

UDPServer* udp_server = new UDPServer();
Scheduler* simulator = new Scheduler(udp_server);

void recv_callback(const ofp_sockaddr_in &addr, void *buf, int nbytes)
{
    udp_server->recv_callback(addr, buf, nbytes);
}

void run_simulator()
{
    simulator->run();
}

int main(int argc, char *argv[])
{
    std::thread sim_thread(run_simulator);
    run_dpdk_thread(argc, argv, DEFAULT_SERVER_PORT, recv_callback);

    sim_thread.join();

    return 0;
}