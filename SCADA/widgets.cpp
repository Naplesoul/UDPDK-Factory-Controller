#include "widgets.h"

#include "Master.h"
#include "scada.h"
#include "object.h"
#include <QKeyEvent>

GameWidget::GameWidget(QWidget *parent)  //游戏组件的初始化，目前还很粗糙
    : QScrollArea(parent)
{
    resize(1000, 1000);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void GameWidget::addObject(class Object *obj){  //添加对象
    qDebug() << "[GameWidget] Add Object:" << obj->getType() << " At: "<< obj->getX() << "," << obj->getY();
    int i;
    for(i = 0; i < mObjects.length(); i++)
        if(mObjects[i]->getPaintOrder() > obj->getPaintOrder()){
            break;
    }
    mObjects.insert(i, obj);
}

void GameWidget::removeObject(class Object *obj){
    qDebug() << "[GameWidget] Remove Object:" << obj->getType() << " At: "<< obj->getX() << "," << obj->getY();
    mObjects.removeOne(obj);
}

GameWidget::~GameWidget()
{
}

void GameWidget::Initialize(){
    setStyleSheet("QScrollArea{border:0px solid #707070}");

    mMap = new Map(this, this);

    setWidget(mMap);
}

void GameWidget::paint(){ //遍历所有对象，逐个调用对象的paint方法，直接传送QPainter
    mMap->update();
}

void GameWidget::keyPressEvent(QKeyEvent *k){
    if (k->text().length()>0)
        emit(keyPressSignal(k->text()));
    else{
        if (k->key() == Qt::Key_Right)
            emit(keyPressSignal("Right"));
        if (k->key() == Qt::Key_Left)
            emit(keyPressSignal("Left"));
        if (k->key() == Qt::Key_Up)
            emit(keyPressSignal("Up"));
        if (k->key() == Qt::Key_Down)
            emit(keyPressSignal("Down"));
    }
    QWidget::keyPressEvent(k);
}

void GameWidget::keyReleaseEvent(QKeyEvent *k){
    if (k->text().length()>0)
        emit(keyReleaseSignal(k->text()));
    else{
        if (k->key() == Qt::Key_Up)
            emit(keyReleaseSignal("Up"));
        if (k->key() == Qt::Key_Down)
            emit(keyReleaseSignal("Down"));
        if (k->key() == Qt::Key_Left)
            emit(keyReleaseSignal("Left"));
        if (k->key() == Qt::Key_Right)
            emit(keyReleaseSignal("Right"));
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *k){
    emit(mouseMoveSignal(k->x() + horizontalScrollBar()->value(), k->y() + verticalScrollBar()->value()));
    QScrollArea::mouseMoveEvent(k);
}

void GameWidget::mousePressEvent(QMouseEvent* k){
    emit(mousePressSignal(k->x() + horizontalScrollBar()->value(), k->y() + verticalScrollBar()->value()));
    QScrollArea::mousePressEvent(k);
}

void GameWidget::mouseReleaseEvent(QMouseEvent* k){
    emit(mouseReleaseSignal(k->x() + horizontalScrollBar()->value(), k->y() + verticalScrollBar()->value()));
    QScrollArea::mouseReleaseEvent(k);
}


void Map::paintEvent(QPaintEvent* ){
    QPainter p(this);
    p.drawText(1, 16, "Time Now:"+QString::number(qlonglong(mMaster->getCurrentTime())));
    for(auto i : *mGameWidget->getObjects())
        i->paint(&p);
}

Map::Map(QWidget* parent, GameWidget* gameWidget)
    :QWidget(parent){
    mGameWidget = gameWidget;
    mMaster = mGameWidget->getGame();

    resize(1000, 1500);

    setMouseTracking(true);
}

Map::~Map(){

}

MainWindow::MainWindow(QWidget *parent)  //没用
    : QMainWindow(parent)
{
    resize(1350, 1000);

    mMaster = new Master();
    qDebug() << "[Build] Game Core Built";

    mGameWidget = new GameWidget(this);
    mGameWidget->hide();
    mGameWidget->setGame(mMaster);
    mMaster->setGameWidget(mGameWidget);

    qDebug() << "[Build] GameWidget Built";

    if (mGameWidget){
        connect(mGameWidget, &GameWidget::keyPressSignal, mMaster, &Master::keyPress);
        connect(mGameWidget, &GameWidget::keyReleaseSignal, mMaster, &Master::keyRelease);
        connect(mGameWidget, &GameWidget::mouseMoveSignal, mMaster, &Master::mouseMove);
        connect(mGameWidget, &GameWidget::mousePressSignal, mMaster, &Master::mousePress);
        connect(mGameWidget, &GameWidget::mouseReleaseSignal, mMaster, &Master::mouseRelease);
   }else qDebug() << "[Warning] Game: Fail to Connect with GameWidget";

    setWindowTitle("Server - MainWindow");
    show();

    mWelWidget = new WelcomeWidget(this);

}

void MainWindow::StartGame(){
    resize(1000, 1000);
    qDebug() << "[Build] Start Game";
    mWelWidget->hide();
    mGameWidget->show();
    setCentralWidget(mGameWidget);
    mGameWidget->Initialize();
    mMaster->Initialize();
    qDebug() << "[Build] Game Initialized Succeessfully";

    mGameWidget->setFocus();
    mGameWidget->setMouseTracking(true);
    setMouseTracking(true);
    mMaster->start();
    isStarted = true;
}

void MainWindow::saveAndExit(){
    mMaster->removeEverything();
    mWelWidget->show();
    qDebug() << "[System] Game Saved Succeessfully";

    isStarted = false;
}

void MainWindow::ChangePause(){
    if (mGameWidget->isHidden()){
        mGameWidget->show();
        mGameWidget->setFocus();
        mMaster->unPause();
    }
    else{
        mGameWidget->hide();
        mMaster->Pause();
    }
}


MainWindow::~MainWindow()
{
}



WelcomeWidget::WelcomeWidget(QWidget* parent)
    :QWidget(parent){
    resize(1350, 1000);

    playButton = new QPushButton("Start Server", this);
    exitButton = new QPushButton("X", this);

    QFont font;
    font.setFamily("Bulletproof BB");
    font.setPixelSize(230);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0);

    playButton->move(325, 500);
    playButton->resize(700, 150);
    connect(playButton, &QPushButton::clicked, static_cast<MainWindow*>(parent), &MainWindow::StartGame);

    exitButton->move(1190, 0);
    exitButton->resize(160, 120);
    connect(exitButton, &QPushButton::clicked, parent, &QWidget::close);

    setStyleSheet(""
                  "QPushButton:hover{"
                  "background:#FFFFFF;"
                  " border:10px solid #000000;"
                  " padding: 0px; "
                  " color: #000000; "
                  " font-family: 'Bulletproof BB';"
                  " font-size: 80px;"
                  "}"
                  "QPushButton{"
                  "background:#000000;"
                  " border:10px solid #AAAAAA;"
                  " padding: 0px; "
                  " color: #FFFFFF; "
                  " font-family: 'Bulletproof BB';"
                  " font-size: 80px;"
                  "}"
                  );

    show();
}

WelcomeWidget::~WelcomeWidget(){

}



