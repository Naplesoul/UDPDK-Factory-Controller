#ifndef GAME_H
#define GAME_H

#include <QVector>
#include <QMap>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <sys/time.h>
#include "label.h"
#include "scada.h"
#include "block.h"
#include "arm.h"
#include "camera.h"
#include "belt.h"


class Master : public QThread{ //Game类继承了QThread，因为游戏的逻辑循环独立于QT组件的事件循环
    Q_OBJECT

private:
    bool isRunning = true, Paused = false; //是否在运行
    int curId = 0;
    double FPS;
    Object* mSelect = nullptr;
    QVector<class Object*> mObjects, mPendingObjects, mDeadObjects; //储存所有的对象
    QVector<class Block*> mBlocks; //储存所有的物块
    QVector<class Arm*> mArms; //储存所有机械臂
    QMap<int, class Object*> mObjMap; //id与对象的对照表

    QVector<QString> mPressedKeys; //被按下的按键列表

    class GameWidget* gameWidget; //游戏组件

    QElapsedTimer* timer; //时钟
    void ProcessInput(); //响应输入
    void Update(int deltaTime); //更新
    void GenerateOutput(); //显示输出

    SCADA* mServer;
protected:
    void run() override;

public:
    QString mInfo;
    int mouseX = 0, mouseY = 0;


    //函数注释写在.cpp中
    void addObject(int id, class Object* obj);
    void removeObject(class Object* obj);
    int getFPS(){return FPS;}
    long long getCurrentTime(){
           struct timeval tv;
           time_t t;
           time(&t);
           gettimeofday(&tv, NULL);
           return t * 1000 + tv.tv_usec / 1000;
        }
    bool getPaused(){return Paused;}
    QVector<Object*>* getObjects(){return &mObjects;}
    QVector<Block*>* getBlocks(){return &mBlocks;}

    void Initialize(); //初始化参数
    void Pause();
    void unPause();

    void setGameWidget(GameWidget* w){gameWidget = w;}
    void setServer(SCADA* s){mServer = s;}
    SCADA* getServer(){return mServer;}
    void removeEverything();

    void keyPress(QString key);
    void keyRelease(QString key);
    void mouseMove(int x, int y);
    void mousePress(int x, int y);
    void mouseRelease(int x, int y);

    Master();
    ~Master();
};




#endif // GAME_H
