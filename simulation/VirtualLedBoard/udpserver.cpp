#include "udpserver.h"
#include "settings.h"
#include <QUdpSocket>
#include <QNetworkDatagram>

#define UDP_IMAGE_PORT  4242

UdpLedServer ::UdpLedServer (QObject *parent)
    : QObject(parent)
{
    initSocket();
}

void UdpLedServer ::initSocket()
{
    this->mUdpSocket = new QUdpSocket(this);
    this->mUdpSocket->bind(QHostAddress::LocalHost, UDP_IMAGE_PORT);

    connect(this->mUdpSocket, &QUdpSocket::readyRead,
            this, &UdpLedServer ::readPendingDatagrams);
}


void UdpLedServer ::readPendingDatagrams()
{
    while (this->mUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = this->mUdpSocket->receiveDatagram();
        processTheDatagram(datagram);
    }
}

void UdpLedServer::processTheDatagram(QNetworkDatagram datagram) {
    if (datagram.isValid() && datagram.data().length() == PACKET_LENGTH) {
        qDebug() << "Received datagram:" << datagram.data().size();
    }
}
