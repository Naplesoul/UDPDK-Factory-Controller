#ifndef SCADA_H
#define SCADA_H

#include <QUdpSocket>
#include <QVector2D>


class SCADA : public QObject
{
    Q_OBJECT
private:
    QHostAddress server_address;
    quint16 server_port;
    QVector<QByteArray> mRequestPool;
    QUdpSocket* mSocket;
protected:

public:
    void initSocket();
    void readPendingDatagrams();
    void sendToServer(QString msg);

    QByteArray getRequest();

    SCADA(QObject* parent);
    ~SCADA();
};

#endif // SCADA_H
