#include "Objects.h"

#include <string>
#include <jsoncpp/json/json.h>

int Object::next_id = 0;

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

std::string Arm::toJsonString()
{
    Json::Value v;

    v["id"] = Json::Value(obj_id);
    v["class"] = Json::Value("arm");
    v["x"] = Json::Value(x);
    v["y"] = Json::Value(y);
    v["enabled"] = Json::Value(enabled);
    v["angle"] = Json::Value(angle);

    return v.toStyledString();
}

void Block::update(int delta_time)
{
    x += speed * delta_time;
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
    v["arm"] = Json::Value(rsp_arm_id);

    return v.toStyledString();
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