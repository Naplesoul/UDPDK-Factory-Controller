#include "Scheduler.h"

#include <chrono>
#include <limits>
#include <unistd.h>

uint64_t getCurMs()
{
    return std::chrono::duration_cast
        <std::chrono::milliseconds>
        (getCurTime().time_since_epoch()).count();
}

uint64_t getCurUs()
{
    return std::chrono::duration_cast
        <std::chrono::microseconds>
        (getCurTime().time_since_epoch()).count();
}

TimePoint getCurTime()
{
    return std::chrono::system_clock::now();
}

Scheduler::~Scheduler()
{
    for (auto arm : arms) delete arm.second;
    for (auto cam : cameras) delete cam.second;
    for (auto block : blocks) delete block.second;
}

void Scheduler::run()
{
    printf("[Sim][%lu] start simulator\n", getCurMs());

    while (1) {
        int64_t start_us = getCurUs();

        handleMsg();
        checkAlive();
        schedule();
        sendTasks();

        int64_t end_us = getCurUs();
        int64_t time_to_sleep = start_us + 15000 - end_us;
        if (time_to_sleep > 0) usleep(time_to_sleep);
    }
}

void Scheduler::checkAlive()
{
    TimePoint cur_time = getCurTime();
    
    for (auto &cam : cameras) {
        int64_t period = std::chrono::duration_cast
            <std::chrono::milliseconds>
            (cur_time - cam.second->last_heartbeat_time).count();
        if (period > CLIENT_TIMEOUT) {
            removeCamera(cam.second->client_id);
        }
    }

    for (auto &arm : cameras) {
        int64_t period = std::chrono::duration_cast
            <std::chrono::milliseconds>
            (cur_time - arm.second->last_heartbeat_time).count();
        if (period > CLIENT_TIMEOUT) {
            removeArm(arm.second->client_id);
        }
    }

    if (scada) {
        int64_t period = std::chrono::duration_cast
            <std::chrono::milliseconds>
            (cur_time - scada->last_heartbeat_time).count();
        if (period > CLIENT_TIMEOUT) {
            delete scada;
            scada = nullptr;
        }
    }
}

void Scheduler::schedule()
{
    TimePoint cur_time = getCurTime();
    std::list<int> block_id_to_remove;

    for (auto &block_entry : blocks) {
        Block *block = block_entry.second;
        Camera *cam = cameras[block->producer_client_id];
        if (!cam) continue;

        double cur_x = block->getPosition(cur_time).x;
        if (cur_x >= cam->end_x) {
            // block is out of camera's domain range
            // delete block
            Arm *arm = arms[block->consumer_client_id];
            if (arm) arm->assigned_blocks.erase(block->block_id);
            block_id_to_remove.push_back(block->block_id);
            continue;
        }

        if (block->consumer_client_id >= 0) continue;

        // assign the block to an arm
        // choose the arm with the least blocks assigned
        size_t blocks_num = std::numeric_limits<size_t>::max();
        Arm *selected_arm = nullptr;
        for (auto arm : cam->consumers) {
            size_t cur_blocks_num = arm->assigned_blocks.size();
            if (cur_blocks_num < blocks_num) {
                blocks_num = cur_blocks_num;
                selected_arm = arm;
            }
        }

        if (!selected_arm) continue;

        // assign the block
        block->consumer_client_id = selected_arm->client_id;
        selected_arm->assigned_blocks[block->block_id] = block;
    }

    // delete blocks
    for (int block_id : block_id_to_remove) {
        delete blocks[block_id];
        blocks.erase(block_id);
    }
}

void Scheduler::sendTasks()
{
    for (auto &arm_entry : arms) {
        Arm *arm = arm_entry.second;
        for (auto &block_entry : arm->assigned_blocks) {
            Block *block = block_entry.second;
            udp_server->send2(arm->client_id, block->toJsonString());
        }
    }
}

void Scheduler::handleMsg()
{
    Message* msg;
    Json::Reader reader;

    while (msg = udp_server->popMsg()) {
        // printf("[SIM][%lu] parse request from client: %d\n\tcontent: %s\n",
        //        getCurMs(), msg->client_id, msg->content.c_str());

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
    Camera *cam = cameras[msg->client_id];
    if (cam) cam->updateHeartbeat();

    if (json["message_type"].empty()) {
        printf("\terror: missing message_type!!\n");
        return;
    }

    if (json["message_type"].asString() == "heartbeat") {
        if (!cam) {
            addCamera(json["x"].asDouble(),
                      json["y"].asDouble(),
                      json["w"].asDouble(),
                      json["h"].asDouble(),
                      msg->client_id);
        }
        return;
    }

    if (!cam) return;

    if (!json["blocks"].isArray()
        || json["speed"].empty()
        || json["timestamp"].empty()) {
        
        printf("\terror: missing arguement(s)!!\n");
        return;
    }

    // add blocks found by camera
    double speed = json["speed"].asDouble();
    uint64_t msg_time_count = atoll(json["timestamp"].asString().data());
    std::chrono::system_clock::duration duration(msg_time_count);
    TimePoint msg_time(duration);

    Json::Value msg_blocks = json["blocks"];
    for (int idx = 0; idx < msg_blocks.size(); idx++) {
        Json::Value msg_block = msg_blocks[idx];
        int block_id = msg_block["id"].asInt();

        Block *block = blocks[block_id];
        if (block) {
            // udp packets may not come in order
            if (msg_time > block->last_msg_time) {
                block->x = msg_block["x"].asDouble();
                block->y = msg_block["y"].asDouble();
                block->angle = msg_block["angle"].asDouble();
                block->speed = speed;
                block->last_msg_time = msg_time;
            }
        } else {
            Block *new_block = new Block(msg_block["x"].asDouble(),
                                         msg_block["y"].asDouble(),
                                         msg_block["angle"].asDouble(),
                                         msg_block["id"].asInt(),
                                         msg->client_id,
                                         speed, msg_time);
            blocks[block_id] = new_block;
        }
    }
}

void Scheduler::handleArmMsg(Message *msg, const Json::Value &json)
{
    Arm *arm = arms[msg->client_id];
    if (arm) arm->updateHeartbeat();

    if (json["message_type"].empty()) {
        printf("\terror: missing message_type!!\n");
        return;
    }

    if (json["message_type"].asString() == "heartbeat") {
        if (!arm) {
            addArm(json["x"].asDouble(),
                   json["y"].asDouble(),
                   json["r"].asDouble(),
                   msg->client_id);
        }
        return;
    }
    
    if (!arm) return;
}

void Scheduler::handleScadaMsg(Message *msg, const Json::Value &json)
{
    if (!scada) scada = new SCADA(msg->client_id);
    else scada->updateHeartbeat();
}

void Scheduler::addArm(double x, double y, double radius, int client_id)
{
    Arm *new_arm = new Arm(x, y, radius, client_id);
    arms[client_id] = new_arm;

    // find the nearest camera in front
    Camera *prev_camera = nullptr;
    for (auto &cam: cameras) {
        double cam_start_x = cam.second->x;
        double cam_end_x = cam.second->end_x;
        if (x > cam_start_x && x < cam_end_x) {
            prev_camera = cam.second;
            break;
        }
    }

    if (!prev_camera) return;

    // set producer and consumer
    new_arm->producer_client_id = prev_camera->client_id;
    prev_camera->consumers.push_back(new_arm);
    prev_camera->consumers.sort(objCompare);
}

void Scheduler::addCamera(double x, double y,
                          double w, double h, int client_id)
{
    Camera *new_cam = new Camera(x, y, w, h, client_id);

    // find the nearest camera in front
    Camera *prev_camera = nullptr;
    for (auto &cam: cameras) {
        double cam_start_x = cam.second->x;
        double cam_end_x = cam.second->end_x;
        if (x > cam_start_x && x < cam_end_x) {
            prev_camera = cam.second;
        }
    }

    cameras[client_id] = new_cam;

    if (prev_camera) {
        for (auto it = prev_camera->consumers.begin();
             it != prev_camera->consumers.begin(); ++it) {
            
            if ((*it)->x > x) {
                // take consumers after it
                new_cam->consumers.insert(new_cam->consumers.begin(),
                                          it, prev_camera->consumers.end());
                prev_camera->consumers.erase(it, prev_camera->consumers.end());
                break;
            }
        }
        new_cam->end_x = prev_camera->end_x;
        prev_camera->end_x = x;

    } else {
        for (auto &cam : cameras) {
            double cam_start_x = cam.second->x;
            if (cam_start_x > x && cam_start_x < new_cam->end_x) {
                new_cam->end_x = cam_start_x;
            }
        }

        for (auto &arm : arms) {
            // take all consumers in range
            if (arm.second->x < new_cam->end_x
                && arm.second->x > new_cam->x) {
                
                new_cam->consumers.push_back(arm.second);
            }
        }
        new_cam->consumers.sort(objCompare);
    }

    for (auto consumer : new_cam->consumers) {
        consumer->producer_client_id = client_id;
    }
}

void Scheduler::removeArm(int client_id)
{

}

void Scheduler::removeCamera(int client_id)
{

}