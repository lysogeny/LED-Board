#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QAbstractItemModel>
#include <QUdpSocket>

class UdpLedServer : public QObject
{
    Q_OBJECT

public:
    explicit UdpLedServer (QObject *parent = nullptr);


private:
    void initSocket();
    void readPendingDatagrams();
    QUdpSocket *mUdpSocket;
    void processTheDatagram(QNetworkDatagram datagram);
};

#endif // UDPSERVER_H
