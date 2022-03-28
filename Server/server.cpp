#include "UDPServer.h"
#include "Scheduler.h"
#include <thread>

UDPServer* udp_server = new UDPServer();
Scheduler* simulator = new Scheduler(udp_server);

void run_udp_server()
{
    udp_server->run();
}

void run_simulator()
{
    simulator->run();
}

int main(int argc, char *argv[])
{
    std::thread udp_thread(run_udp_server);
    std::thread sim_thread(run_simulator);

    udp_thread.join();
    sim_thread.join();

    return 0;
}