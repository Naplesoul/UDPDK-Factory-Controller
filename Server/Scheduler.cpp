#include "Scheduler.h"

#include <chrono>

uint64_t getCurTime()
{
    
}

Scheduler::Scheduler(UDPServer* server)
{
    udp_server = server;
}

Scheduler::~Scheduler()
{
    
}

void Scheduler::run()
{
    printf("[Sim][%llu] start simulator\n", getCurTime());

    while (1) {
        handleMsg();
        sendTasks();
    }
}

void Scheduler::sendTasks()
{
    
}

void Scheduler::handleMsg()
{
    Message* msg;
    while (msg = udp_server->popMsg()) {
        printf("[SIM][%llu] parse request from client: %d\n\tcontent: %s\n",
               getCurTime(), msg->client_id, msg->content.c_str());

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
        addCamera(json["x"].asDouble(),
                  json["y"].asDouble(),
                  json["angle"].asDouble(),
                  json["actual_width"].asDouble(),
                  json["actual_height"].asDouble(),
                  msg->client_id);
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
                                         msg->client_id,
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
        addArm(json["x"].asDouble(),
               json["y"].asDouble(),
               json["angle"].asDouble(),
               json["r"].asDouble(),
               msg->client_id,
               json["enabled"].asBool());
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
    if (!scada) scada = new SCADA(msg->client_id);
    if (scada->id == msg->client_id){
        // parse actions from SCADA, like delete divice, relocate divice, or debug
    }
}

void Scheduler::addArm(double x, double y, double angle,
                       double radius, int client_id, bool enabled)
{
    Arm *new_arm = new Arm(x, y, angle, radius, client_id, enabled);
    arms[client_id] = new_arm;

    // find the nearest camera in front
    Camera *front_camera = nullptr;
    for (auto &cam: cameras) {
        double cam_x = cam.second->x;
        if (cam_x < x
            && (front_camera == nullptr
                || front_camera->x < cam_x)) {
            front_camera = cam.second;
        }
    }

    if (!front_camera) return;

    // set producer and consumer
    new_arm->producer_client_id = front_camera->client_id;
    front_camera->consumers.push_back(new_arm);
    front_camera->consumers.sort(objCompare);
}

void Scheduler::addCamera(double x, double y, double angle,
                          double w, double h, int client_id)
{
    Camera *new_cam = new Camera(x, y, angle, w, h, client_id);

    // find the nearest camera in front
    Camera *front_camera = nullptr;
    for (auto &cam: cameras) {
        double cam_x = cam.second->x;
        if (cam_x < x
            && (front_camera == nullptr
                || front_camera->x < cam_x)) {
            front_camera = cam.second;
        }
    }

    cameras[client_id] = new_cam;

    if (front_camera) {
        for (auto it = front_camera->consumers.begin();
             it != front_camera->consumers.begin(); ++it) {
            
            if ((*it)->x > x) {
                // take consumers after it
                new_cam->consumers.insert(new_cam->consumers.begin(),
                                          it, front_camera->consumers.end());
                front_camera->consumers.erase(it, front_camera->consumers.end());
                break;
            }
        }
    } else {
        for (auto &arm : arms) {
            // take all consumers after which does not have producer yet
            if (arm.second->producer_client_id >= 0
                || arm.second->x < x) continue;
            new_cam->consumers.push_back(arm.second);
        }
        new_cam->consumers.sort(objCompare);
    }


    for (auto consumer : new_cam->consumers) {
        consumer->producer_client_id = client_id;
    }
}