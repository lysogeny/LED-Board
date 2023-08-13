#include "udpserver.h"
#include <QUdpSocket>
#include <QNetworkDatagram>

#define UDP_IMAGE_PORT  4242

UdpServer::UdpServer(QObject *parent)
    : QAbstractItemModel(parent)
{
    initSocket();
}

void UdpServer::initSocket()
{
    this->mUdpSocket = new QUdpSocket(this);
    this->mUdpSocket->bind(QHostAddress::LocalHost, UDP_IMAGE_PORT);

    connect(this->mUdpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::readPendingDatagrams);
}


void UdpServer::readPendingDatagrams()
{
    while (this->mUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = this->mUdpSocket->receiveDatagram();
        processTheDatagram(datagram);
    }
}

void UdpServer::processTheDatagram(QNetworkDatagram datagram) {
    if (datagram.isValid()) {
        qDebug() << "Received datagram:" << datagram.data().size();
    }
}
