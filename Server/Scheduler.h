#pragma once

#include <map>
#include <stdint.h>
#include <jsoncpp/json/json.h>

#include "Objects.h"
#include "UDPServer.h"

#define CLIENT_TIMEOUT 5000

uint64_t getCurMs();
uint64_t getCurUs();
TimePoint getCurTime();

class Scheduler
{
private:
    SCADA* scada = nullptr;
    UDPServer* udp_server;

    // client_id => arm
    std::map<int, Arm *> arms;
    // client_id => camera
    std::map<int, Camera *> cameras;
    // block_id => block
    std::map<int, Block *> blocks;

    void schedule();
    void checkAlive();
    void sendTasks();
    void handleMsg();

    void handleCamMsg(Message *msg, const Json::Value &json);
    void handleArmMsg(Message *msg, const Json::Value &json);
    void handleScadaMsg(Message *msg, const Json::Value &json);

public:
    Scheduler(UDPServer *s): udp_server(s) {}
    ~Scheduler();

    void run();

    void addArm(double x, double y, double radius, int client_id);
    void addCamera(double x, double y,
                   double w, double h, int client_id);
    void removeArm(int client_id);
    void removeCamera(int client_id);
};
