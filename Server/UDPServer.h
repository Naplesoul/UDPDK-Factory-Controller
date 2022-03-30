#pragma once

#include <map>
#include <list>
#include <mutex>
#include <string.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_PORT 8080

uint64_t getAddr(const struct sockaddr_in &addr);

class UDPClient
{
private:
    static int next_id;

public:
    int client_id;
    struct sockaddr_in addr;
    
    UDPClient(struct sockaddr_in addr):
        client_id(next_id++), addr(addr) {}
    ~UDPClient() {}
};

class Message
{
public:
    int client_id;
    std::string content;

    Message(int client_id, char *buf):
        client_id(client_id), content(buf) {}
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

    void run();
    Message *popMsg();
    void removeClient(int client_id);
    void send2(int client_id, std::string msg);
};
