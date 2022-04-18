#ifndef LABEL_H
#define LABEL_H

#include "object.h"

class Label: public Object{
protected:
    class QString mString;
    QFont mFont;
public:
    virtual void* getPtr() override{return (Label*)this;}

    void paint(class QPainter* p)override;
    void update(int deltaTime)override;
    void setText(class QString text){mString = text;}

    QString toString() override;

    Label(class Master* g, float x = 0, float y = 0, int w = 50, int h = 50, class QString text = "", QColor color = QColor(0, 0, 0));
    ~Label();
};

class DyingLabel: public Label{
private:
    int restOfTime;
    float dAlpha;
public:
    virtual void* getPtr() override{return (DyingLabel*)this;}

    void paint(class QPainter* p)override;
    void update(int deltaTime)override;

    DyingLabel(class Master* g, int time, float x = 0, float y = 0, int w = 50, int h = 50, float xv = 0, float yv = 0, class QString text = "", QColor color = QColor(0, 0, 0));
    ~DyingLabel();
};


class Bar: public Object{
private:
    Object* mOwner;
protected:
    float mValue;
public:
    virtual void* getPtr() override{return (Bar*)this;}

    void paint(class QPainter* p)override;
    void setValue(float value){mValue = value;}
    void update(int deltaTime)override;

    void setOwner(Object* owner){mOwner = owner;}

    QString toString() override;
    Bar(class Master* g, float v = 1.0, float x = 0, float y = 0, int w = 50, int h = 50, QColor color = QColor(0, 0, 0));
    ~Bar();
};

class Partical : public Object{
protected:
    int restOfTime;

    float dAlpha;
public:
    virtual void update(int deltaTime) override;

    void paint(class QPainter* p)override;
    QString toString() override;

    Partical(class Master* g, int time, float x = 0, float y = 0, int w = 10, int h = 10, float xv = 0, float yv = 0, QColor color = QColor(0, 0, 0));
    ~Partical();
};


#endif // LABEL_H
