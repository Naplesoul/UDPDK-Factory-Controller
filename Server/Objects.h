#pragma once

#include <list>
#include <string>

class Object
{
private:
    static int next_id;

public:
    int obj_id;
    double x, y;
    double angle;

    virtual void update(int delta_time) {}
    virtual std::string toJsonString();

    Object(double x, double y, double angle):
        obj_id(next_id++), x(x), y(y), angle(angle) {}
    ~Object(){}
};

class Block: public Object
{
public:
    int block_id;
    int producer_client_id;
    int consumer_client_id = -1;
    double speed;
    uint64_t last_msg_time;

    virtual void update(int delta_time) override;
    virtual std::string toJsonString() override;

    Block(double x, double y, double angle, int block_id,
          int producer_client_id, double speed, uint64_t msg_time):
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
    bool enabled;
    int producer_client_id = -1;

    virtual std::string toJsonString() override;

    Arm(double x, double y, double angle,
        double radius, int client_id, bool enabled):
        Object(x, y, angle),
        radius(radius), client_id(client_id), enabled(enabled) {}
    ~Arm() {}
};

class Camera: public Object
{
public:
    double w, h;
    int client_id;
    std::list<Arm *> consumers;

    virtual std::string toJsonString() override;

    Camera(double x, double y, double angle,
           double w, double h, int client_id):
        Object(x, y, angle), w(w), h(h), client_id(client_id) {}
    ~Camera() {}
};

class SCADA
{
public:
    int id;
    int client_id;

    SCADA(int id): id(id) {}
    ~SCADA() {}
};