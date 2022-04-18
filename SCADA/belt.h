#ifndef BELT_H
#define BELT_H

#include "object.h"
#include "Master.h"


class Belt : public Object
{
private:
    float mSpeed;  //传送带速度
protected:

public:
    virtual void* getPtr() override{return (Belt*)this;}
    virtual void setFromBytes(QByteArray) override;

    void paint(class QPainter* p)override; //重写Object的显示函数
    void update(int deltaTime) override; //重写Object的更新函数

    void setSpeed(float sp){mSpeed = sp;}

    Belt(Master* g = nullptr, int i = -1);
    virtual ~Belt();
};
#endif // BELT_H
