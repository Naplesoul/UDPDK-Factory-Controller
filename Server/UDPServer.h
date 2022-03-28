#pragma once

#include <map>
#include <list>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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
    std::mutex req_list_mtx;
    // std::map<int, Client*> clients;
    std::list<UDPClient*> mClients;
    std::list<Message*> mPendingRequests;
public:
    void run();
    Message* popMsg();
    void send2(int client_id, std::string msg);
    void broadcast(std::string msg);

    UDPServer(/* args */);
    ~UDPServer();
};
