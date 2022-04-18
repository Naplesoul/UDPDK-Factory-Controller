#include "arm.h"

Arm::Arm(Master* g, float x, float y, float r)
    :Object(g)
{
    xPos = x; yPos = y; rot = r;
    setWH(40, 40);
    mRadius = 150;
    mColor = QColor(200, 150, 100);
    mType = "Arm";
}

Arm::~Arm(){

}

void Arm::paint(class QPainter* p){
    Object::paint(p);
    if (isVisible){
        p->save();
        p->setPen(QColor(255, 0, 0));
        for (auto b : mBlocks){
            p->drawLine(xPos, yPos, b->getX(), b->getY());
        }
        p->setPen(QColor(100, 100, 120));
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->drawArc(-mRadius, -mRadius, 2*mRadius, 2*mRadius, 0, 16 * 180);
        p->drawLine(0, 0, 0, - mRadius);
        p->drawLine(0, - mRadius, -10, 20 - mRadius);
        p->drawLine(0, - mRadius, 10, 20 - mRadius);
        p->setBrush(QBrush(mColor));
        p->drawRect(int( - width/2), int( - height/2), width, height);   //以pos为中心绘制方块

        p->restore();
    }
}

void Arm::update(int deltaTime){
//    Object::update(deltaTime);
//    //此处完成：发送抓取指令 包括位置、时间戳
//    if (mStatus != IDLE) return;
//    for (auto i : *mMaster->getBlocks()){
//        //机械臂开始运行到抓住的时间间隔约为1.7s
//        float timeInterval = 1.7;
//        //x,y表示机械臂坐标系下的位置
//        float tmpY = i->getX() - xPos + i->getXV() * timeInterval,
//              tmpX = i->getY() - yPos + i->getYV() * timeInterval;
//        float targetY = cos(atan(tmpY / tmpX) + rot),
//              targetX = sin(atan(tmpY / tmpX) + rot);
//        bool inRange = (targetX * targetX + targetY * targetY < mRadius * mRadius);
//        if (!inRange) {
//            continue;
//        }

//        //判断是否要抓，如果快来了就抓，如果在走的路上随机抓
//        bool ifcatch = false;
//        if ((targetX * i->getYV() < 0 && 0 < rot < 1.572) || (targetX * i->getYV() > 0 && 3.142 > rot > 1.57)) {
//            ifcatch = true;
//        } else {
//            ifcatch = (rand() & 1);
//        }
//        if (ifcatch){
//            QString msg = "{\"id\":0, \"time\":0,";
//            msg += "\"x\":"+QString::number(targetX)+",";
//            msg += "\"y\":"+QString::number(targetY)+",";
//            msg += "\"angle\":"+QString::number(i->getRot())+"}";
//            mMaster->getServer()->sendTo(msg, id);
//            mMaster->removeObject(i);
//            this->mStatus = BUSY;
//            break;
//        }
//    }
}
void Arm::setFromBytes(QByteArray array){
    //qDebug() << "Set Camera Image: " << array.length() << width << height;
    mStatus = Status(QString(array).toInt());
}
