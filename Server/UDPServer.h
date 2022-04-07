#pragma once

#include <map>
#include <list>
#include <mutex>
#include <string.h>

#include "dpdk.h"

uint64_t getAddr(const struct ofp_sockaddr_in &addr);

class UDPClient
{
private:
    static int next_id;

public:
    int client_id;
    struct ofp_sockaddr_in addr;
    
    UDPClient(struct ofp_sockaddr_in addr):
        client_id(next_id++), addr(addr) {}
    ~UDPClient() {}
};

class Message
{
public:
    int client_id;
    std::string content;

    Message(int client_id, char *buf, int nbytes):
        client_id(client_id), content(buf, nbytes) {}
    ~Message() {}
};

class UDPServer
{
private:
    int server_fd;
    std::mutex msg_mtx;
    std::mutex client_mtx;
    std::list<Message *> pending_msgs;
    std::map<int, UDPClient *> clients_id_map;
    std::map<uint64_t, UDPClient *> clients_addr_map;

public:
    UDPServer();
    ~UDPServer();

    Message *popMsg();
    void removeClient(int client_id);
    void send2(int client_id, std::string msg);
    void recv_callback(const ofp_sockaddr_in &client_addr, void *buf, int nbytes);
};
