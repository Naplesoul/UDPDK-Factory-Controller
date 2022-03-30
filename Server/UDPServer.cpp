#include "UDPServer.h"

#include <stdint.h>
#include <arpa/inet.h>

int UDPClient::next_id = 0;

uint64_t getAddr(const struct sockaddr_in &addr)
{
    uint64_t k;
    k = (uint64_t)addr.sin_addr.s_addr << (16);
    k = k | (uint64_t)addr.sin_port;
    return k;
}

UDPServer::UDPServer()
{
    int server_port = DEFAULT_SERVER_PORT;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    int b = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("[UDP] udp sever listening at port %d...\n", server_port);
}

UDPServer::~UDPServer()
{
    for (auto c : clients_id_map) {
        delete c.second;
    }
    
    for (auto r : pending_msgs) {
        delete r;
    }
}

void UDPServer::run()
{
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(struct sockaddr_in);

        int count = recvfrom(server_fd, buf, BUFFER_SIZE,
            0, (struct sockaddr *)&client_addr, &length);
        if (count < 0) {
            printf("[UDP] fail to receive from client\n");
            continue;
        }

        printf("[UDP] received %lu bytes from client ip: %s, port:%d\n",
            strlen(buf), inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        
        UDPClient *client;
        uint64_t addr = getAddr(client_addr);
        auto clt = clients_addr_map.find(addr);

        if (clt == clients_addr_map.end()) {
            client = new UDPClient(client_addr);

            client_mtx.lock();
            clients_addr_map[addr] = client;
            clients_id_map[client->client_id] = client;
            client_mtx.unlock();
        } else {
            client = clt->second;
        }

        msg_mtx.lock();
        pending_msgs.push_back(new Message(client->client_id, buf));
        msg_mtx.unlock();
    }
}

Message *UDPServer::popMsg()
{
    Message* msg = nullptr;
    msg_mtx.lock();

    if (!pending_msgs.empty()) {
        msg = pending_msgs.front();
        pending_msgs.pop_front();
    }

    msg_mtx.unlock();
    return msg;
}


void UDPServer::removeClient(int client_id)
{
    client_mtx.lock();
    auto client = clients_id_map.find(client_id);
    if (client != clients_id_map.end()) {
        uint64_t addr = getAddr(client->second->addr);
        clients_addr_map.erase(addr);
        clients_id_map.erase(client);
    }
    client_mtx.unlock();
}

void UDPServer::send2(int client_id, std::string msg)
{
    struct sockaddr_in addr;
    const socklen_t len = sizeof(struct sockaddr_in);

    client_mtx.lock();
    auto client = clients_id_map.find(client_id);
    if (client != clients_id_map.end())
        addr = client->second->addr;
    client_mtx.unlock();

    if (client == clients_id_map.end()) return;
    sendto(server_fd, msg.data(), msg.size(),
        0, (struct sockaddr *)&addr, len);
}