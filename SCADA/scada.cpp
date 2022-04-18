#include "scada.h"

SCADA::SCADA(QObject* parent)
    :QObject(parent)
{
    server_address = QHostAddress("127.0.0.1");
    server_port = 8080;
}

SCADA::~SCADA(){

}

void SCADA::initSocket()
{
    mSocket = new QUdpSocket(nullptr);
    mSocket->bind(12354);
    connect(mSocket, &QUdpSocket::readyRead, this, &SCADA::readPendingDatagrams);
}

void SCADA::readPendingDatagrams()
{
    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(mSocket->bytesAvailable());
    mSocket->readDatagram(array.data(), array.size(), &address, &port);

    mRequestPool.append(array);
}

QByteArray SCADA::getRequest(){
    if (!mRequestPool.empty()){
        QByteArray r = mRequestPool.first();
        mRequestPool.pop_front();
        return r;
    }else return QByteArray();
}


void SCADA::sendToServer(QString msg){
    mSocket->writeDatagram(msg.toLocal8Bit(), server_address, server_port);
}
