#include "Objects.h"

#include <string>
#include <math.h>
#include <jsoncpp/json/json.h>

int Object::next_id = 0;

std::string
toJsonString(const std::map<int, Block *> &m)
{
    Json::Value v;
    Json::Value arr;

    for (auto &b : m) {
        Json::Value block;

        block["id"] = Json::Value(b.second->block_id);
        block["x"] = Json::Value(b.second->x);
        block["y"] = Json::Value(b.second->y);
        block["speed"] = Json::Value(b.second->speed);
        block["angle"] = Json::Value(b.second->angle);
        block["timestamp"] = Json::Value(std::to_string(
            b.second->last_msg_time.time_since_epoch().count()));
        
        arr.append(block);
    }

    v["blocks"] = arr;

    return v.toStyledString();
}

bool objCompare(Object *a, Object *b)
{
    return a->x < b->x;
}

Point Object::getPosition(TimePoint cur_time)
{
    return Point(x, y);
}

std::string Object::toJsonString()
{
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("object");
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["angle"] = Json::Value(angle);

    return v.toStyledString();
}

std::string Object::toJsonString(TimePoint cur_time)
{
    return toJsonString();
}

Point Block::getPosition(TimePoint cur_time)
{
    int64_t delta_time =
        std::chrono::duration_cast<std::chrono::milliseconds>
        (cur_time - last_msg_time).count();
    double cur_x = x + speed * delta_time;
    return Point(cur_x ,y);
}

std::string Block::toJsonString()
{
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("block");
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["speed"] = Json::Value(speed);
    v["angle"] = Json::Value(angle);
    v["arm"] = Json::Value(consumer_client_id);

    return v.toStyledString();
}

std::string Block::toJsonString(TimePoint cur_time)
{
    double cur_x = getPosition(cur_time).x;
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("block");
    v["x"] = Json::Value(cur_x);
    v["y"] = Json::Value(y);
    v["speed"] = Json::Value(speed);
    v["angle"] = Json::Value(angle);
    v["arm"] = Json::Value(consumer_client_id);

    return v.toStyledString();
}

void Arm::updateHeartbeat()
{
    last_heartbeat_time = std::chrono::system_clock::now();
}

bool Arm::canCatch(Point pos)
{
    double d = y - pos.y;
    double l2 = radius * radius - d * d;
    if (l2 <= 0) return false;

    double max_x = x + sqrt(l2);
    return pos.x < max_x;
}

std::string Arm::toJsonString()
{
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("arm");
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["angle"] = Json::Value(angle);

    return v.toStyledString();
}

std::string Arm::toJsonString(TimePoint cur_time)
{
    return toJsonString();
}

void Camera::updateHeartbeat()
{
    last_heartbeat_time = std::chrono::system_clock::now();
}

std::string Camera::toJsonString()
{
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("camera");
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["w"] = Json::Value(w);
    v["h"] = Json::Value(h);
    v["angle"] = Json::Value(angle);

    return v.toStyledString();
}

std::string Camera::toJsonString(TimePoint cur_time)
{
    return toJsonString();
}

void SCADA::updateHeartbeat()
{
    last_heartbeat_time = std::chrono::system_clock::now();
}