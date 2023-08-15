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

signals:
    void changeLEDstate(uint8_t x, uint8_t y);
    void updatePanelContent(void);
};

#endif // UDPSERVER_H
