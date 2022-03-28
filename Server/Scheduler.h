#pragma once

#include <map>
#include <list>
#include <vector>
#include <stdint.h>
#include <sys/time.h>
#include <jsoncpp/json/json.h>

#include "Objects.h"
#include "UDPServer.h"

class Scheduler
{
private:
    uint64_t cur_time;
    UDPServer* udp_server;
    SCADA* mSCADA;

    // client_id => arm
    std::map<int, Arm *> arms;
    // client_id => camera
    std::map<int, Camera *> cameras;
    // block_id => block
    std::map<int, Block *> blocks;
    std::list<Object *> clients;

    void handleMsg();
    void sendTasks();

    void handleCamMsg(Message *msg, const Json::Value &json);
    void handleArmMsg(Message *msg, const Json::Value &json);
    void handleScadaMsg(Message *msg, const Json::Value &json);

public:
    Scheduler(UDPServer*);
    ~Scheduler();

    void run();
};
