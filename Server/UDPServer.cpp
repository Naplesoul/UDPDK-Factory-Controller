#include "UDPServer.h"

#include <stdint.h>

int UDPClient::next_id = 0;

uint64_t getAddr(const struct ofp_sockaddr_in &addr)
{
    uint64_t k;
    k = (uint64_t)addr.sin_addr.s_addr << (16);
    k = k | (uint64_t)addr.sin_port;
    return k;
}

UDPServer::UDPServer() {}

UDPServer::~UDPServer()
{
    for (auto c : clients_id_map) {
        delete c.second;
    }
    
    for (auto r : pending_msgs) {
        delete r;
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
    struct ofp_sockaddr_in addr;

    client_mtx.lock();
    auto client = clients_id_map.find(client_id);
    if (client != clients_id_map.end())
        addr = client->second->addr;
    client_mtx.unlock();

    if (client == clients_id_map.end()) return;
    
    send_msg(addr, msg);
}

void UDPServer::recv_callback(const ofp_sockaddr_in &client_addr, void *buf, int nbytes)
{
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
    pending_msgs.push_back(new Message(client->client_id, (char *)buf, nbytes));
    msg_mtx.unlock();
}