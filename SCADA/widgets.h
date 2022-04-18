#ifndef WIDGETS_H
#define WIDGETS_H

#include <QVector>
#include <QMainWindow>
#include <QScrollArea>
#include <QWidget>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>


class GameWidget : public QScrollArea{  //游戏组件，是承载游戏画面的容器
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    void setGame(class Master* game){mMaster = game;} //设置游戏控制引擎
    void addObject(class Object* obj); //添加需要显示的对象
    void removeObject(class Object* obj); //删除对象
    QVector<Object*>* getObjects(){return &mObjects;} //返回所有对象
    Master* getGame(){return mMaster;}

    void Initialize();
    void paint();  //重写paintEvent函数，用于绘制对象
protected:
    void keyPressEvent(QKeyEvent *)override;  //重写keyPressEvent函数，用于获取输入，下同
    void keyReleaseEvent(QKeyEvent *)override;
    void mouseMoveEvent(QMouseEvent *)override;
    void mousePressEvent(QMouseEvent *)override;
    void mouseReleaseEvent(QMouseEvent *)override;
private:
    QVector<class Object*> mObjects; //储存所有待显示的对象
    class Master* mMaster;
    class Map* mMap;

signals:
    void keyPressSignal(QString key); //相关信号，连接Game所在的线程内的槽
    void keyReleaseSignal(QString key);
    void mouseMoveSignal(int x, int y);
    void mousePressSignal(int x, int y);
    void mouseReleaseSignal(int x, int y);
};


class Map : public QWidget{
    Q_OBJECT
private:
protected:
    GameWidget* mGameWidget;
    Master* mMaster;
    void paintEvent(QPaintEvent*)override;
public:
    Map(QWidget* parent = nullptr, GameWidget* gameWidget = nullptr);
    ~Map();
};


class MainWindow : public QMainWindow      //主界面类
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Initialize();
    void ChangePause();
    void OverToWel();
    void StartGame();

    void saveAndExit();

private:
    class WelcomeWidget* mWelWidget;
    GameWidget* mGameWidget;
    QHBoxLayout* mHLayout;
    Master* mMaster;
    bool isStarted = false;

};

class WelcomeWidget : public QWidget
{
    Q_OBJECT
public:
    WelcomeWidget(QWidget* parent = nullptr);
    ~WelcomeWidget();
private:
    QPushButton* playButton,* exitButton;
    QLabel* label, *label2;
};


#endif // WIDGETS_H
