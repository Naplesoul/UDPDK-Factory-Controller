#include "Master.h"
#include "object.h"
#include "widgets.h"
#include <iostream>
#include <QDateTime>
#include <jsoncpp/json/json.h>

Master::Master()     //直接调用初始化函数
    :QThread(0){
    mServer = new SCADA(nullptr);
    mServer->initSocket();
}

Master::~Master(){
}

void Master::run(){
    timer = new QElapsedTimer();
    timer->start();

    mServer->sendToServer("{\"source\": \"scada\"}");
    qDebug() << "[Process] START SCADA";

    //test
//    Arm* arm1 = new Arm(this, 390, 250, M_PI / 2);
//    Arm* arm2 = new Arm(this, 610, 350, -M_PI / 2);
//    addObject(2, arm1);
//    addObject(3, arm2);
//    Block* block1 = new Block(this, 530, 840, 20, 20, 0),
//         * block2 = new Block(this, 480, 740, 20, 20, 0),
//         * block3 = new Block(this, 510, 890, 20, 20, 0),
//         * block4 = new Block(this, 540, 810, 20, 20, 0),
//         * block5 = new Block(this, 496, 690, 20, 20, 0);
//    addObject(4, block1);
//    addObject(5, block2);
//    addObject(6, block3);
//    addObject(7, block4);
//    addObject(8, block5);


    //    arm1->mBlocks.append(block1);
    //    arm1->mBlocks.append(block5);
    //    arm2->mBlocks.append(block2);
    //    arm2->mBlocks.append(block4);
    //    arm2->mBlocks.append(block3);

    //    addObject(10, new Camera(this, 500, 700, 400, 300, M_PI / 2));


//    Belt* belt = new Belt(this);
//    belt->setSpeed(100);
//    belt->setXY(500, 100);
//    belt->setWH(140, 1000);
//    belt->setRot(M_PI/2);
//    addObject(1000, belt);

    //end test

    while(isRunning){
        int deltaTime = timer->elapsed();

        //默认每秒50帧
        while(deltaTime < 100){
            deltaTime = timer->elapsed();
        }
        FPS = 1000 / deltaTime;
        timer->restart();

        //简简单单的循环，响应输入->计算更新->输出
        ProcessInput();
        Update(deltaTime);
        GenerateOutput();
    }
}

void Master::Initialize(){
    isRunning = true;
    Paused = false;
}


void Master::ProcessInput(){
    //这个函数从Server中获取待处理请求，逐个响应
    QByteArray request = mServer->getRequest();
    while(request.size()){
        qDebug() << "Parse Request: \n" << QString(request).toUtf8();
        Json::Reader reader;
        Json::Value obj;
        reader.parse(QString(request).toStdString(), obj, false);

        if (obj.isNull())
            return;
        if (obj["blocks"].isArray()){
            for (auto b : obj["blocks"]){
                Json::Value blockJson = b;
                if (!mObjMap.contains(blockJson["obj_id"].asInt())){
                    addObject(blockJson["obj_id"].asInt(),
                            new Block(this, blockJson["x"].asDouble(), blockJson["y"].asDouble(), 20, 20, blockJson["angle"].asDouble()));
                }else{
                    Block* block = (Block*)mObjMap[blockJson["obj_id"].asInt()];
                    block->setRot(blockJson["angle"].asDouble());
                    //block->setXY(blockJson["x"].asInt(), blockJson["y"].asInt());
                    block->setVelXY(blockJson["speed"].asDouble() * 1000, 0);
                }
            }
        }
        if (obj["arms"].isArray()){
            for (auto a : obj["arms"]){
                Json::Value armJson = a;
                Arm* arm;
                if (!mObjMap.contains(armJson["obj_id"].asInt())){
                    arm = new Arm(this, armJson["x"].asDouble(), armJson["y"].asDouble(), armJson["angle"].asDouble());
                    addObject(armJson["obj_id"].asInt(),
                            arm);
                }else{
                    arm = (Arm*)mObjMap[armJson["obj_id"].asInt()];
                    arm->setRot(armJson["angle"].asDouble());
                    arm->setXY(armJson["x"].asInt(), armJson["y"].asInt());
                }
                if (!armJson["block_obj_ids"].isNull()){
                    arm->mBlocks.clear();
                    for (auto b : armJson["block_obj_ids"]){
                        arm->mBlocks.append((Block*)mObjMap[b.asInt()]);
                    }
                }
            }
        }
        if (obj["cameras"].isArray()){
            for (auto c : obj["cameras"]){
                Json::Value cameraJson = c;
                if (!mObjMap.contains(cameraJson["obj_id"].asInt())){
                    addObject(cameraJson["obj_id"].asInt(),
                            new Camera(this, cameraJson["x"].asDouble(), cameraJson["y"].asDouble(), cameraJson["angle"].asDouble()));
                }else{
                    Camera* camera = (Camera*)mObjMap[cameraJson["obj_id"].asInt()];
                    camera->setRot(cameraJson["angle"].asDouble());
                    camera->setXY(cameraJson["x"].asInt(), cameraJson["y"].asInt());
                    camera->setWH(cameraJson["w"].asInt(), cameraJson["h"].asInt());
                }
            }
        }

        request = mServer->getRequest();
    }
}

void Master::Update(int deltaTime){
    //这个函数遍历更新所有对象，并处理对象增删
    if (Paused) deltaTime = 0; //若暂停则让时间静止

    //遍历更新所有对象，同时检查对象是否待删除
    for(auto i : mObjects){
        i->update(deltaTime);
        i->setEventTime(getCurrentTime());
        if (i->getClearStatus())
            mDeadObjects.push_back(i);
    }

    //将待加入的对象按显示优先级加入对象表，同时加入Widget中等待显示
    for(auto i : mPendingObjects){
        int j;
        for(j = 0; j < mObjects.length(); j++)
            if(mObjects[j]->getPaintOrder() < i->getPaintOrder()){
                break;
        }
        mObjects.insert(j, i);
        if (i->getType() == "Block"){
            mBlocks.push_back(static_cast<Block*>(i));
        }

        qDebug() << "[Game] Add Object:" << i->getType();
        gameWidget->addObject(i);
    }
    mPendingObjects.clear();

    //将死亡的待删除的对象从对象表中移除
    for(auto i : mDeadObjects){
        if (mBlocks.contains(static_cast<Block*>(i)))
            mBlocks.removeOne(static_cast<Block*>(i));
        if (mArms.contains(static_cast<Arm*>(i))){
            mArms.removeOne(static_cast<Arm*>(i));
        }

        mObjects.removeOne(i);
        qDebug() << "[Game] Remove Object:" << i->getType();
        gameWidget->removeObject(i);

        delete i;
    }
    mDeadObjects.clear();

}

void Master::GenerateOutput(){       //调用游戏组件的update，注意这个update默认包含了paintEvent
    //在Widget中显示目前的对象
    gameWidget->paint();

    mServer->sendToServer(
                QString("{\"message_type\": \"heartbeat\", \"source\": \"scada\"}"));
}

void Master::Pause(){
    Paused = true;
    timer->restart();
    qDebug() << "[Process] Pause";
}

void Master::unPause(){
    Paused = false;
    timer->restart();
    qDebug() << "[Process] Continue";
}


//这些函数处理鼠标键盘事件（暂时无用）

void Master::keyPress(QString key){
    mPressedKeys.push_back(key);
    if (key == "p"){
        if (Paused)
            unPause();
        else
            Pause();
    }
    if (key == "d" && mSelect){
        removeObject(mSelect);
    }
    if (key == "s" && mSelect){
    }
    if (key == "Right" && mSelect){
    }
    if (key == "Left" && mSelect){
    }
    if (key == "Up" && mSelect){
    }
    if (key == "Down" && mSelect){
    }
    if (key == "," && mSelect){
    }
    if (key == "." && mSelect){
    }
}

void Master::keyRelease(QString key){
    mPressedKeys.removeOne(key);
}

void Master::mouseMove(int x, int y){
    mouseX = x; mouseY = y;
}

void Master::mousePress(int x, int y){
    mouseX = x; mouseY = y;
    //设置选中的对象
    Object* oldSelect = mSelect;
    if (mSelect){
        mSelect->setSelected(false);
        mSelect = nullptr;
    }
    for (auto i : mObjects){
        //if (x <= i->getX()+i->getW()/2 && x >= i->getX()-i->getW()/2 && y <= i->getY()+i->getH()/2 && y >= i->getY()-i->getH()/2){
        if (i->contains(x, y) && oldSelect != i){
            mSelect = i;
            i->setSelected(true);
            break;
        }
    }
}

void Master::mouseRelease(int x, int y){
    mouseX = x; mouseY = y;
}

void Master::addObject(int id, class Object *obj){
    //添加对象，不能直接添加到对象列表，而是加入缓冲区，是为了避免遍历列表过程中改变列表
    obj->id = id;
    mPendingObjects.push_back(obj);
    if (obj->id >= 0)
        mObjMap[obj->id] = obj;
}

void Master::removeObject(class Object *obj){
    //移除对象，除了从总对象列表中移除，还要从一些特殊列表中移除
    obj->setClearStatus(true);
    for (auto& i : mObjMap.keys()){
        if (mObjMap[i] == static_cast<Arm*>(obj))
            mObjMap.remove(i);
    }
}


void Master::removeEverything(){
    for(auto i: mObjects){
        removeObject(i);
    }
}
