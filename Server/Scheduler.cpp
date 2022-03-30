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
    // remove dead cams, arms and scada
    TimePoint cur_time = getCurTime();
    std::list<int> cam_id_to_remove;
    std::list<int> arm_id_to_remove;
    
    for (auto &cam : cameras) {
        int64_t period = std::chrono::duration_cast
            <std::chrono::milliseconds>
            (cur_time - cam.second->last_heartbeat_time).count();
        if (period > CLIENT_TIMEOUT) {
            cam_id_to_remove.push_back(cam.second->client_id);
        }
    }

    for (auto &arm : cameras) {
        int64_t period = std::chrono::duration_cast
            <std::chrono::milliseconds>
            (cur_time - arm.second->last_heartbeat_time).count();
        if (period > CLIENT_TIMEOUT) {
            arm_id_to_remove.push_back(arm.second->client_id);
        }
    }

    for (int arm_id : arm_id_to_remove) removeArm(arm_id);
    for (int cam_id : cam_id_to_remove) removeCamera(cam_id);

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
    std::list<Block *> blocks_to_remove;

    for (auto &block_entry : blocks) {
        Block *block = block_entry.second;
        Camera *cam = cameras[block->producer_client_id];
        if (!cam) {
            blocks_to_remove.push_back(block);
            continue;
        }

        Point cur_pos = block->getPosition(cur_time);
        double cur_x = cur_pos.x;
        if (cur_x >= cam->end_x) {
            // block is out of camera's domain range
            // delete block
            blocks_to_remove.push_back(block);
            continue;
        }

        if (block->consumer_client_id >= 0) continue;

        // assign the block to an arm
        // choose the arm with the least blocks assigned
        size_t blocks_num = std::numeric_limits<size_t>::max();
        Arm *selected_arm = nullptr;
        for (auto &arm : cam->consumers) {
            size_t cur_blocks_num = arm.second->assigned_blocks.size();
            if (arm.second->canCatch(cur_pos)
                && cur_blocks_num < blocks_num) {
                blocks_num = cur_blocks_num;
                selected_arm = arm.second;
            }
        }

        if (!selected_arm) continue;

        // assign the block
        block->consumer_client_id = selected_arm->client_id;
        selected_arm->assigned_blocks[block->block_id] = block;
    }

    // delete blocks
    for (Block *block : blocks_to_remove) {
        Arm *arm = arms[block->consumer_client_id];
        if (arm) arm->assigned_blocks.erase(block->block_id);
        blocks.erase(block->block_id);
        delete block;
    }
}

void Scheduler::sendTasks()
{
    for (auto &arm_entry : arms) {
        Arm *arm = arm_entry.second;
        Json::Value v;
        Json::Value arr;

        for (auto &b : arm->assigned_blocks)
            arr.append(b.second->toJson());
        v["blocks"] = arr;

        udp_server->send2(arm->client_id, v.toStyledString());
    }

    if (scada) {
        Json::Value v;
        Json::Value arm_arr;
        Json::Value blk_arr;
        Json::Value cam_arr;

        for (auto &a : arms) arm_arr.append(a.second->toJson());
        for (auto &b : blocks) blk_arr.append(b.second->toJson());
        for (auto &c : cameras) cam_arr.append(c.second->toJson());

        v["timestamp"] = Json::Value(std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()));
        v["arms"] = arm_arr;
        v["blocks"] = blk_arr;
        v["cameras"] = cam_arr;
        
        udp_server->send2(scada->client_id, v.toStyledString());
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
    prev_camera->consumers[client_id] = new_arm;
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
        // take all arms in range from prev_camera
        for (auto it = prev_camera->consumers.begin();
             it != prev_camera->consumers.begin();) {
            
            Arm *arm = it->second;
            if (arm->x > x) {
                new_cam->consumers[arm->client_id] = arm;
                it = prev_camera->consumers.erase(it);
            } else ++it;
        }
        new_cam->end_x = prev_camera->end_x;
        prev_camera->end_x = x;

    } else {
        // set new_cam->end_x to the x of the nearest camera behind
        for (auto &cam : cameras) {
            double cam_start_x = cam.second->x;
            if (cam_start_x > x && cam_start_x < new_cam->end_x) {
                new_cam->end_x = cam_start_x;
            }
        }

        for (auto &arm : arms) {
            // take all arms in range
            if (arm.second->x < new_cam->end_x
                && arm.second->x > new_cam->x) {
                
                new_cam->consumers[arm.second->client_id]
                    = arm.second;
            }
        }
    }

    // set arm.producer_client_id
    for (auto consumer : new_cam->consumers) {
        consumer.second->producer_client_id = client_id;
    }
}

void Scheduler::removeArm(int client_id)
{
    Arm *arm = arms[client_id];
    if (!arm) return;

    Camera *cam = cameras[arm->producer_client_id];
    if (cam) cam->consumers.erase(arm->client_id);

    arms.erase(client_id);
    delete arm;
}

void Scheduler::removeCamera(int client_id)
{
    Camera *camera = cameras[client_id];
    if (!camera) return;

    cameras.erase(client_id);

    // find the nearest camera in front
    Camera *prev_camera = nullptr;
    double min_dist = std::numeric_limits<double>::max();
    for (auto &cam : cameras) {
        if (cam.second->x < camera->x
            && (!prev_camera
                || (camera->x - cam.second->x) < min_dist)) {
            min_dist = camera->x - cam.second->x;
            prev_camera = cam.second;
        }
    }

    if (!prev_camera) {
        for (auto &arm : camera->consumers) {
            arm.second->producer_client_id = -1;
        }
        delete camera;
        return;
    }

    // blocks and arms of camera will be inherited by prev_camera
    for (auto &arm : camera->consumers) {
        arm.second->producer_client_id = prev_camera->client_id;
        prev_camera->consumers[arm.second->client_id] = arm.second;
    }

    for (auto &block : blocks) {
        if (block.second->producer_client_id == client_id) {
            block.second->producer_client_id == prev_camera->client_id;
        }
    }
    
    delete camera;
}