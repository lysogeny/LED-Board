#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QAbstractItemModel>
#include <QUdpSocket>

class UdpServer : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit UdpServer(QObject *parent = nullptr);


private:
    void initSocket();
    void readPendingDatagrams();
    QUdpSocket *mUdpSocket;
    void processTheDatagram(QNetworkDatagram datagram);
};

#endif // UDPSERVER_H
