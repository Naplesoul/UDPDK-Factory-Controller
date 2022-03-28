#include "Scheduler.h"
#include <math.h>

Scheduler::Scheduler(UDPServer* server)
{
    udp_server = server;
}

Scheduler::~Scheduler()
{
    
}

void updateTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec * 1000000 + tv.tv_usec;
}

void Scheduler::run()
{
    updateTime();
    printf("[Sim][%llu] start simulator\n", cur_time);

    while (1) {
        handleMsg();
        sendTasks();
    }
}

void Scheduler::handleMsg()
{
    Message* msg;
    while (msg = udp_server->popMsg()) {
        printf("[SIM][%llu] parse request from client: %d\n\tcontent: %s\n",
               cur_time, msg->client_id, msg->content.c_str());

        Json::Reader reader;
        Json::Value json;

        if (!reader.parse(msg->content, json, false)) {
            printf("\terror: json parse error\n");
            continue;
        }
        
        if (json["source"].empty()) {   //check source
            printf("\terror: missing source!!\n");
        } else if (json["source"].asString() == "camera") {
            handleCamMsg(msg, json);
        } else if (json["source"].asString() == "arm") {
            handleArmMsg(msg, json);
        } else if (json["source"].asString() == "scada") {
            handleScadaMsg(msg, json);    
        }
        
    }
}

void Scheduler::handleCamMsg(Message *msg, const Json::Value &json)
{
    if (!json["blocks"].isArray()
        || json["speed"].empty()
        || json["timestamp"].empty()) {
        
        printf("\terror: missing arguement(s)!!\n");
        return;
    }

    // update camera
    if (cameras.find(msg->client_id) == cameras.end()) {
        Camera *new_cam = new Camera(json["x"].asDouble(),
                                     json["y"].asDouble(),
                                     json["angle"].asDouble(),
                                     json["actual_width"].asDouble(),
                                     json["actual_height"].asDouble(),
                                     msg->client_id);
        cameras[msg->client_id] = new_cam;
    } else {
        Camera *cam = cameras[msg->client_id];
        cam->x = json["x"].asDouble();
        cam->y = json["y"].asDouble();
        cam->w = json["actual_width"].asDouble();
        cam->h = json["actual_height"].asDouble();
        cam->angle = json["angle"].asDouble();
    }

    // add blocks found by camera
    double speed = json["speed"].asDouble();
    uint64_t msg_time = atoll(json["timestamp"].asString().data());

    Json::Value msg_blocks = json["blocks"];
    for (int idx = 0; idx < blocks.size(); idx++) {

        Json::Value msg_block = msg_blocks[idx];
        int block_id = msg_block["id"].asInt();

        if (blocks.find(block_id) == blocks.end()) {
            Block *new_block = new Block(msg_block["x"].asDouble(),
                                         msg_block["y"].asDouble(),
                                         msg_block["angle"].asDouble(),
                                         msg_block["id"].asInt(),
                                         speed, msg_time);
            blocks[block_id] = new_block;
        } else {
            Block *block = blocks[block_id];

            // udp packets may not come in order
            if (msg_time > block->last_msg_time) {
                block->x = msg_block["x"].asDouble();
                block->y = msg_block["y"].asDouble();
                block->angle = msg_block["angle"].asDouble();
                block->speed = speed;
                block->last_msg_time = msg_time;
            }
        }
    }
}

void Scheduler::handleArmMsg(Message *msg, const Json::Value &json)
{
    if (!json["enabled"].isArray()) {
        printf("\terror: missing arguement(s)!!\n");
        return;
    }

    if (arms.find(msg->client_id) == arms.end()) {
        Arm *new_arm = new Arm(json["x"].asDouble(),
                               json["y"].asDouble(),
                               json["angle"].asDouble(),
                               json["r"].asDouble(),
                               msg->client_id,
                               json["enabled"].asBool());
        arms[msg->client_id] = new_arm;
    } else {
        Arm* arm = arms[msg->client_id];
        arm->x = json["x"].asDouble();
        arm->y = json["y"].asDouble();
        arm->radius = json["r"].asDouble();
        arm->angle = json["angle"].asDouble();
        arm->enabled = json["enabled"].asBool();
    }
}

void Scheduler::handleScadaMsg(Message *msg, const Json::Value &json)
{
    if (!mSCADA) mSCADA = new SCADA(msg->client_id);
    if (mSCADA->id == msg->client_id){
        // parse actions from SCADA, like delete divice, relocate divice, or debug
    }
}

void Scheduler::sendTasks()
{
    
}