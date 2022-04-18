#include "block.h"


Block::Block(Master* g, float x, float y, int w, int h, float r, QColor color)
    :Object(g)
{
    xPos = x; yPos = y; width = w; height = h; rot = r;   //初始化方块的位置、大小参数
    mColor = color;
    mType = "Block";
    mInfo = "Basic";
    paintOrder = 10;
}

Block::~Block(){
}

void Block::paint(class QPainter* p){
    Object::paint(p);
    if (isVisible){
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setBrush(QBrush(mColor));
        p->drawRect(int( - width/2), int( - height/2), width, height);   //以pos为中心绘制方块
        p->restore();
    }
}

void Block::update(int deltaTime){     //用于检测处理碰撞、踩踏、摧毁等事件
//    xVel = rand() % 60 - 30;
//    yVel = rand() % 60 - 30;
    Object::update(deltaTime);
}

