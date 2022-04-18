#ifndef BLOCK_H
#define BLOCK_H

#include "object.h"
#include "Master.h"

class Block : public Object{
private:

protected:
    bool isBreakable = false, isCrossable = false; //能否被破坏、穿越

public:
    virtual void* getPtr() override{return (Block*)this;}

    bool getCrossable(){return isCrossable;}
    void paint(class QPainter* p)override; //重写Object的显示函数
    void update(int deltaTime) override; //重写Object的更新函数

    Block(class Master* g, float x = 0, float y = 0, int w = 50, int h = 50, float r = 0, QColor color = QColor(120, 120, 120));
    virtual ~Block();
};


#endif // BLOCK_H
