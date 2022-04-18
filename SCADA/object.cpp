#include "object.h"
#include "Master.h"

Object::~Object(){
    if (mType != "Partical" && mType != "DyingLabel" && !mMaster->getPaused()){
        int parX, parY, parXV, parYV, parSize, parTime;
        for (int i = 0; i < (width + height) / 30 + 3; i++){
            parX = xPos + (rand() % width - width / 2); parY = yPos + (rand() % height - height / 2);
            parXV = (rand() % 200 - 100); parYV = (rand() % 200 - 100);
            parSize = rand() % (width + height) / 10 + 8;
            parTime = rand() % 400 + 100;
            mMaster->addObject(-1, new Partical(mMaster, parTime, parX, parY, parSize, parSize, parXV, parYV, mColor));
        }
    }
}

QString Object::toString(){
    if (mType == "Partical") return "";
    QString result = "";
    result += mType + ":";
    result += QString::number(id) + ":";
    result += QString::number(xPos) + ":";
    result += QString::number(yPos) + ":";
    result += QString::number(tVel) + ":";
    result += QString::number(nVel) + ":";
    result += QString::number(width) + ":";
    result += QString::number(height) + ":";
    result += QString::number(rot) + ":";
    result += QString::number(rotVel) + ":";
    result += QString::number(mColor.red()) + ":";
    result += QString::number(mColor.green()) + ":";
    result += QString::number(mColor.blue()) + ":";
    result += QString::number(mColor.alpha()) + ":";
    return result;
}

void Object::setFromBytes(QByteArray){

}

void Object::paint(class QPainter *p){
    p->save();
    p->translate(xPos, yPos);
    p->rotate(rot*180/acos(-1));
    p->setBrush(Qt::NoBrush);
    if (isSelected){
        p->setPen(QColor(200, 50, 80));
        p->drawRect(int( - width/2 - 1), int( - height/2 - 1), width + 2, height + 2);   //以pos为中心绘制方块
        p->drawText(int( - width/2), int( - height/2)-58, "<"+mType+">");
        p->drawText(int( - width/2), int( - height/2)-44, "Id: "+QString::number(id) + "   Status: " + QString::number(mStatus));
        p->drawText(int( - width/2), int( - height/2)-30, "Pos: "+QString::number(xPos)+", "+QString::number(yPos));
        p->drawText(int( - width/2), int( - height/2)-16, "Vel: "+QString::number(xVel)+", "+QString::number(yVel));
        p->drawText(int( - width/2), int( - height/2)-2, "Time: "+QString::number(mEventTime));
    }else{
        p->drawText(int( - width/2), int( - height/2)-2, "<"+mType+">");
    }
    p->restore();
} //子类需要重写，用于显示

void Object::update(int deltaTime){
    mTime += deltaTime;
    xPos += xVel * deltaTime / 1000;
    yPos += yVel * deltaTime / 1000;
    tVel += tAcc * deltaTime / 1000;
    nVel += nAcc * deltaTime / 1000;
    rot += rotVel * deltaTime / 1000;
} //子类需要重写，用于更新


bool Object::contains(float x, float y){
    float x1 = x - xPos;
    float y1 = y - yPos;
    float x2 = x1 * cos(rot) + y1 * sin(rot);
    float y2 = y1 * cos(rot) - x1 * sin(rot);
    return (2*x2 >= -width && 2*x2 <= width && 2*y2 >= -height && 2*y2 <= height);
}
