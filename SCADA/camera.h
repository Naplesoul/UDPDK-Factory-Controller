#ifndef CAMERA_H
#define CAMERA_H
#include "object.h"
#include "Master.h"
#include <QImage>


class Camera : public Object
{
private:
    bool imageNotEmpty = false;
protected:

public:
    virtual void* getPtr() override{return (Camera*)this;}

    void paint(class QPainter* p)override; //重写Object的显示函数
    void update(int deltaTime) override; //重写Object的更新函数

    Camera(Master* g = nullptr, float x = 0, float y = 0, int w = 400, int h = 300, float r = 0);
    virtual ~Camera();
};

#endif // CAMERA_H
