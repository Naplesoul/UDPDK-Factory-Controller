#include "belt.h"

Belt::Belt(Master* g, int i)
    :Object(g, i)
{
    mColor = QColor(200, 200, 200);
    xPos = yPos = 500;
    width = 110;
    height = 800;
    mSpeed = 90;
    paintOrder = -10;
    mType = "Belt";
}

Belt::~Belt(){

}

void Belt::paint(class QPainter* p){
    Object::paint(p);
    if (isVisible){
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setBrush(QBrush(mColor));
        p->drawRect(int( - width/2), int( - height/2), width, height);   //以pos为中心绘制方块
        if (isSelected)
            p->drawText(int( - width/2), int( - height/2)-72, "Speed: "+QString::number(mSpeed));
        p->setBrush(QBrush(QColor(120, 120, 120)));
        for (int i = 0; i < height/40;i++)
            for (int j = 0; j < width/40 + 1;j++)
                p->drawRect(5 + int( - width/2) + j*40, int( - height/2) + 40 - (mTime/10)%40 + i*40, 10, 10);
        p->restore();
    }
}

void Belt::update(int deltaTime){
    Object::update(deltaTime);

    //此处完成：强制设置传送带上的物体速度
    for (auto i : *mMaster->getObjects()){
        if (contains(i->getX(), i->getY()) && (i->getType() == "Block" || i -> getType() == "Camera"))
            i->setVelXY(mSpeed * sin(rot), - mSpeed * cos(rot));
        else {
            i->setVelXY(0, 0);
            if (i->getType() == "Block") mMaster->removeObject(i);
        }
    }
}

void Belt::setFromBytes(QByteArray array){
    //qDebug() << "Set Camera Image: " << array.length() << width << height;
    mSpeed = QString(array).toDouble();
}
