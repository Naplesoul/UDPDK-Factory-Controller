#pragma once

#include <map>
#include <list>
#include <string>
#include <limits>
#include <chrono>

using TimePoint = std::chrono::system_clock::time_point;

struct Point
{
    double x, y;
    Point(double x, double y): x(x), y(y) {}
};


class Object
{
private:
    static int next_id;

public:
    int obj_id;
    double x, y;
    double angle;

    virtual void update(int delta_time) {}
    virtual Point getPosition(TimePoint cur_time);
    virtual std::string toJsonString();
    virtual std::string toJsonString(TimePoint cur_time);

    Object(double x, double y, double angle):
        obj_id(next_id++), x(x), y(y), angle(angle) {}
    ~Object(){}
};

bool objCompare(Object *a, Object *b);

class Block: public Object
{
public:
    int block_id;
    int producer_client_id;
    int consumer_client_id = -1;
    double speed;
    TimePoint last_msg_time;

    virtual Point getPosition(TimePoint cur_time) override;
    virtual std::string toJsonString() override;
    virtual std::string toJsonString(TimePoint cur_time) override;

    Block(double x, double y, double angle, int block_id,
          int producer_client_id, double speed, TimePoint msg_time):
        Object(x, y, angle), block_id(block_id),
        producer_client_id(producer_client_id),
        speed(speed), last_msg_time(msg_time) {}
    ~Block() {}
};

class Arm: public Object
{
public:
    double radius;
    int client_id;
    int producer_client_id = -1;
    TimePoint last_heartbeat_time;
    std::map<int, Block *> assigned_blocks;

    void updateHeartbeat();
    bool canCatch(Point pos);
    virtual std::string toJsonString() override;
    virtual std::string toJsonString(TimePoint cur_time) override;

    Arm(double x, double y, double radius, int client_id):
        Object(x, y, 0), radius(radius), client_id(client_id),
        last_heartbeat_time(std::chrono::system_clock::now()) {}
    ~Arm() {}
};

class Camera: public Object
{
public:
    double w, h;
    double end_x = std::numeric_limits<double>::max();
    int client_id;
    TimePoint last_heartbeat_time;
    std::map<int, Arm *> consumers;

    void updateHeartbeat();
    virtual std::string toJsonString() override;
    virtual std::string toJsonString(TimePoint cur_time) override;

    Camera(double x, double y,
           double w, double h, int client_id):
        Object(x, y, 0), w(w), h(h), client_id(client_id),
        last_heartbeat_time(std::chrono::system_clock::now()) {}
    ~Camera() {}
};

class SCADA
{
public:
    int client_id;
    TimePoint last_heartbeat_time;

    SCADA(int client_id): client_id(client_id),
        last_heartbeat_time(std::chrono::system_clock::now()) {}
    ~SCADA() {}

    void updateHeartbeat();
};