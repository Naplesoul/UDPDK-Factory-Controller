#include "UDPServer.h"

#include <arpa/inet.h>

#define DEFAULT_SERVER_PORT 8080
#define BUFFER_SIZE 1024

int UDPClient::next_id = 0;

UDPServer::UDPServer(/* args */)
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
    for (auto c : mClients) {
        delete c;
    }
    
    for (auto r : mPendingRequests) {
        delete r;
    }
}

void UDPServer::run()
{
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        int count = recvfrom(server_fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &length);
        if (count < 0) {
            printf("[UDP] fail to receive from client\n");
            continue;
        }

        printf("[UDP] received %lu bytes from client ip: %s, port:%d\n", strlen(buf), inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

        bool new_client = true;
        for (auto c : mClients){
            if (c->addr.sin_addr.s_addr == client_addr.sin_addr.s_addr){

                req_list_mtx.lock();
                mPendingRequests.push_back(new Message(c->client_id, buf));
                req_list_mtx.unlock();

                new_client = false;
                break;
            }
        }
        if (new_client){
            UDPClient* c = new UDPClient(client_addr);
            mClients.push_back(c);

            req_list_mtx.lock();
            mPendingRequests.push_back(new Message(c->client_id, buf));
            req_list_mtx.unlock();
        }
    }
}

void UDPServer::broadcast(std::string msg){
    //printf("[UDP] broadcast\n");
    for (auto c : mClients){
        socklen_t len = sizeof(struct sockaddr_in);
        sendto(server_fd, msg.data(), msg.size(), 0, (struct sockaddr *)&c->addr, len);
    }
}

void UDPServer::send2(int client_id, std::string msg){
    //printf("[UDP] send to client: %d\n", client_id);
    for (auto c : mClients){
        if (c->client_id == client_id){
            socklen_t len = sizeof(struct sockaddr_in);
            sendto(server_fd, msg.data(), msg.size(), 0, (struct sockaddr *)&c->addr, len);
            break;
        }
    }
}

Message* UDPServer::popMsg(){
    Message* req;
    req_list_mtx.lock();
    if (mPendingRequests.empty()) {
        req = nullptr;
    } else {
        req = mPendingRequests.front();
        mPendingRequests.pop_front();
    }
    req_list_mtx.unlock();
    return req;
}