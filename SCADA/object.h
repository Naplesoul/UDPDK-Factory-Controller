#ifndef OBJECT_H
#define OBJECT_H


#include <QVector>
#include <QPainter>
#include <QtMath>
#include <QObject>

class Object{

protected:
    bool isEnabled, isVisible, isWaitingForClear, isSelected;
    bool dying;
    float xPos, yPos;     //位置
    float tVel, nVel, xVel, yVel;     //速度
    float tAcc, nAcc;
    double rot, rotVel;     //角度和角速度
    int width, height;    //宽度和高度
    int mTime;
    long long mEventTime;
    class Master* mMaster;
    QColor mColor;
    int paintOrder;
    QString mType, mInfo;
public:

    enum Status{IDLE, BUSY, OFFLINE} mStatus = IDLE;
    int id;

    virtual void* getPtr(){return (Object*)this;}
    virtual QString toString();
    virtual void setFromBytes(QByteArray);

    virtual bool contains(float x, float y);

    bool getClearStatus(){return isWaitingForClear;}
    float getX(){return xPos;}
    float getY(){return yPos;}
    int getW(){return width;}
    int getH(){return height;}
    double getRot(){return rot;}
    float getXV(){return xVel;}
    float getYV(){return yVel;}
    int getPaintOrder(){return paintOrder;}
    QString getType(){return mType;}
    QString getInfo(){return mInfo;}
    QColor getColor(){return mColor;}
    void setClearStatus(bool dead = true){isWaitingForClear = dead;}
    void setXY(float x, float y){xPos = x; yPos = y;}
    void setWH(int w, int h){width = w; height = h;}
    void setRot(float r){rot = r;}
    void setVel(float tv, float nv){tVel = tv; nVel = nv;}
    void setRotVel(float rv){rotVel = rv;}
    void setVelXY(float xv, float yv){xVel = xv; yVel = yv;}
    void setColor(int r, int g, int b, float a = 255){mColor = QColor(r, g, b, a);}
    void setColor(QColor c){mColor = c;}
    void setVisible(bool v){isVisible = v;}
    void setSelected(bool s){isSelected = s;}
    void setEventTime(long long t){mEventTime = t;}
    void setStatus(Status s){mStatus = s;}
    virtual void paint(class QPainter* p);
    virtual void update(int deltaTime);
    virtual void processInput(QVector<QString>*){}  //子类需要重写，用于响应输入

    Object(class Master* g, int i = -1){
        mMaster = g;
        isSelected = false;
        xPos = 500.0; yPos = 500.0;
        xVel = 0.0; yVel = 0.0;
        tVel = 0.0; nVel = 0.0;
        tAcc = 0.0; nAcc = 0.0;
        rot = 0.0; rotVel = 0.0;
        width = 0; height = 0;
        mTime = mEventTime = 0;
        id = i;
        mColor = QColor(0, 0, 0); paintOrder = 0;
        isEnabled = true;
        isVisible = true;
        isWaitingForClear = false;
        dying = false;
        mType = "Object";
        mInfo = "";
    }
    virtual ~Object();
};

#endif // OBJECT_H
