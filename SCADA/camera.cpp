#include "camera.h"
//#include <opencv2/opencv.hpp>
//#include <opencv2/imgproc/types_c.h>
//#include "asmOpenCV.h"

//using namespace cv;

Camera::Camera(Master* g, float x, float y, int w, int h, float r)
    :Object(g)
{
    xPos = x; yPos = y; width = w; height = h; rot = r;
    mColor = QColor(100, 150, 200);
    paintOrder = 20;
    mType = "Camera";
}

Camera::~Camera(){

}

void Camera::paint(class QPainter* p){
    Object::paint(p);
    if (isVisible){
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/3.14159);
        p->setBrush(Qt::NoBrush);
        p->drawRect(int( - width/2), int( - height/2), width, height);   //以pos为中心绘制方块
        p->restore();
    }
}
void Camera::update(int deltaTime)
{
    //Object::update(deltaTime);
    mTime += deltaTime;

}

