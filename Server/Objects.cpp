#include "Objects.h"

#include <string>
#include <math.h>
#include <jsoncpp/json/json.h>

int Object::next_id = 0;

bool objCompare(Object *a, Object *b)
{
    return a->x < b->x;
}

Point Object::getPosition(TimePoint cur_time)
{
    return Point(x, y);
}

Json::Value Object::toJson()
{
    Json::Value v;

    v["obj_id"] = Json::Value(obj_id);
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["angle"] = Json::Value(angle);

    return v;
}

std::string Object::toJsonString()
{
    return Json::FastWriter().write(toJson());
    // return toJson().toStyledString();
}

Point Block::getPosition(TimePoint cur_time)
{
    int64_t delta_time =
        std::chrono::duration_cast<std::chrono::milliseconds>
        (cur_time - last_msg_time).count();
    double cur_x = x + speed * delta_time;
    return Point(cur_x ,y);
}

Json::Value Block::toJson()
{
    Json::Value v;

    v["obj_id"] = Json::Value(obj_id);
    v["id"] = Json::Value(block_id);
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["angle"] = Json::Value(angle);
    v["speed"] = Json::Value(speed);
    v["timestamp"] = Json::Value(std::to_string(
        last_msg_time.time_since_epoch().count()));

    return v;
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

Json::Value Arm::toJson()
{
    Json::Value v;

    v["obj_id"] = Json::Value(obj_id);
    v["id"] = Json::Value(client_id);
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);

    Json::Value arr;
    for (auto &b : assigned_blocks)
        arr.append(b.second->block_id);
    v["block_obj_ids"] = arr;

    return v;
}

void Camera::updateHeartbeat()
{
    last_heartbeat_time = std::chrono::system_clock::now();
}

Json::Value Camera::toJson()
{
    Json::Value v;

    v["obj_id"] = Json::Value(obj_id);
    v["id"] = Json::Value(client_id);
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["w"] = Json::Value(w);
    v["h"] = Json::Value(h);

    return v;
}

void SCADA::updateHeartbeat()
{
    last_heartbeat_time = std::chrono::system_clock::now();
}