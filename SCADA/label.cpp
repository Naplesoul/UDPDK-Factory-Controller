#include "label.h"
#include "Master.h"
#include <QDebug>

Label::Label(class Master* g, float x, float y, int w, int h, class QString text, QColor color)
    :Object(g){
    xPos = x; yPos = y; width = w; height = h; mString = text; mColor = color;
    mType = "Label";
    mInfo = "Instructions";
    mFont.setFamily("Bulletproof BB");
    mFont.setPixelSize(height * 1.3);
    mFont.setLetterSpacing(QFont::AbsoluteSpacing, 0);
}

void Label::paint(class QPainter *p){
    mFont.setPixelSize(height * 1.3);
    if (isVisible){      //显示文字
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setBrush(mColor);
        p->setPen(mColor);
        p->setFont(mFont);
        p->drawText(int(- width/2) , int( height/2), mString);
        p->restore();
    }
}

void Label::update(int){

}

QString Label::toString(){
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
    result += mString + ":";
    return result;
}

Label::~Label(){
}

DyingLabel::DyingLabel(class Master* g, int time, float x, float y, int w, int h, float xv, float yv, QString text, QColor color)
    :Label(g, x, y, w, h, text, color){
    mType = "DyingLabel";
    mInfo = "Instructions";

    xVel = xv; yVel = yv;
    restOfTime = time;
    dAlpha = 1.0 / restOfTime;
}

void DyingLabel::paint(class QPainter *p){
    if (isVisible){      //显示文字
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setBrush(mColor);
        p->setPen(mColor);
        p->setFont(mFont);
        p->drawText(int(- width/2) , int(+ height/2), mString);
        p->restore();
    }
}

void DyingLabel::update(int deltaTime){
    restOfTime -= deltaTime;
    if (restOfTime <= 0){
        mMaster->removeObject(this);
        return;
    }

    mColor.setAlphaF(mColor.alphaF() - dAlpha * deltaTime);
    Object::update(deltaTime);
}

DyingLabel::~DyingLabel(){
}


void Bar::paint(class QPainter *p){

    if (isVisible){      //显示
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setPen(mColor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(int( - width/2) , int( - height/2), width, height);   //以pos为中心绘制方块

        p->setPen(Qt::NoPen);
        p->setBrush(mColor);
        p->drawRect(int( - width/2) , int( - height/2), int(width* mValue), height);   //以pos为中心绘制方块
        p->restore();
    }
}

void Bar::update(int deltaTime){
    Object::update(deltaTime);
}


Bar::Bar(class Master* g, float v, float x, float y, int w, int h, QColor color)
    :Object(g){
    xPos = x; yPos = y; width = w; height = h;
    mValue = v;
    mColor = color;
    mType = "Bar";
    mInfo = "Show process";
}

Bar::~Bar(){
}

QString Bar::toString(){
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
    result += QString::number(mValue) + ":";
    return result;
}



void Partical::update(int deltaTime){
    restOfTime -= deltaTime;
    if (restOfTime <= 0){
        mMaster->removeObject(this);
        return;
    }

    mColor.setAlphaF(mColor.alphaF() - dAlpha * deltaTime);
    Object::update(deltaTime);
}

void Partical::paint(class QPainter *p){
    if (isVisible){      //随便写的，目前只显示一个小黑块
        p->save();
        p->translate(xPos, yPos);
        p->rotate(rot*180/acos(-1));
        p->setBrush(QBrush(mColor));
        p->setPen(Qt::NoPen);
        p->drawRect(int( - width/2) , int( - height/2), width, height);   //以pos为中心绘制方块
        p->restore();
    }
    Object::paint(p);
}

Partical::Partical(class Master* g, int time, float x, float y, int w, int h, float xv, float yv, QColor color)
    :Object(g){
    xPos = x; yPos = y; width = w; height = h;
    xVel = xv; yVel = yv;
    mColor = color;
    mType = "Partical";
    mInfo = "Basic \n Partical";

    restOfTime = time;
    dAlpha = 1.0 / restOfTime;
}

QString Partical::toString(){
    return "";
}

Partical::~Partical(){

}
