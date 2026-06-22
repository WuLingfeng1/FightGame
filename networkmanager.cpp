// Module
// File: networkmanager.cpp   Version: 0.1.0   License: AGPLv3
// Created:    wangcheng        2026-06-23
// Description:
//     联机网络管理器实现: TCP 建连/断连, UDP 局域网广播发现, JSON 消息协议
#include "networkmanager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    detectLocalIp();
}

NetworkManager::~NetworkManager()
{
    if (m_announceTimer) {
        m_announceTimer->stop();
        delete m_announceTimer;
        m_announceTimer = nullptr;
    }
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    if (m_socket) {
        m_socket->disconnectFromHost();
        if (m_socket->state() == QAbstractSocket::UnconnectedState)
            delete m_socket;
        else
            m_socket->deleteLater();
        m_socket = nullptr;
    }
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
}

void NetworkManager::detectLocalIp()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto &iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp)
            && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            const auto entries = iface.addressEntries();
            for (const auto &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    m_localIp = entry.ip().toString();
                    emit localIpChanged();
                    return;
                }
            }
        }
    }
    m_localIp = QStringLiteral("127.0.0.1");
    emit localIpChanged();
}

void NetworkManager::startServer(quint16 port)
{
    if (m_server) {
        qWarning() << "[NetworkManager] Server already running";
        return;
    }

    m_tcpPort = port;
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection,
            this, &NetworkManager::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred(QStringLiteral("无法启动服务器: ") + m_server->errorString());
        delete m_server;
        m_server = nullptr;
        return;
    }

    m_isServer = true;
    emit isServerChanged();
    qDebug() << "[NetworkManager] 服务器已启动, 端口:" << port;
}

void NetworkManager::stopServer()
{
    if (m_announceTimer) {
        m_announceTimer->stop();
        delete m_announceTimer;
        m_announceTimer = nullptr;
    }
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
    disconnectFromHost();
    m_isServer = false;
    emit isServerChanged();
}

void NetworkManager::connectToHost(const QString &ip, quint16 port)
{
    if (m_socket) {
        qWarning() << "[NetworkManager] 已有连接或正在连接中";
        return;
    }

    m_tcpPort = port;
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        m_isConnected = true;
        emit connectedChanged();
        emit connectedToHost();
        qDebug() << "[NetworkManager] 已连接到主机";
    });
    connect(m_socket, &QTcpSocket::readyRead,
            this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    qDebug() << "[NetworkManager] 正在连接" << ip << ":" << port;
    m_socket->connectToHost(ip, port);
}

void NetworkManager::disconnectFromHost()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        if (m_socket->state() == QAbstractSocket::UnconnectedState) {
            delete m_socket;
        } else {
            m_socket->deleteLater();
        }
        m_socket = nullptr;
    }
    if (m_isConnected) {
        m_isConnected = false;
        m_readBuffer.clear();
        emit connectedChanged();
        emit disconnected();
    }
}

void NetworkManager::startDiscovery(quint16 port)
{
    if (m_isDiscovering)
        return;

    m_discoveryPort = port;
    m_discoveredRooms.clear();
    emit discoveredRoomsChanged();

    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::Any, port,
                           QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit errorOccurred(QStringLiteral("无法绑定发现端口: ") + m_udpSocket->errorString());
        delete m_udpSocket;
        m_udpSocket = nullptr;
        return;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &NetworkManager::onUdpReadyRead);
    m_isDiscovering = true;
    emit isDiscoveringChanged();
    qDebug() << "[NetworkManager] 局域网发现已启动, 端口:" << port;
}

void NetworkManager::stopDiscovery()
{
    if (m_udpSocket && !m_isServer) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
    m_isDiscovering = false;
    emit isDiscoveringChanged();
}

void NetworkManager::announceRoom(const QString &roomName)
{
    m_roomName = roomName;

    if (!m_udpSocket) {
        m_udpSocket = new QUdpSocket(this);
    }

    QJsonObject announce;
    announce[QStringLiteral("type")] = QStringLiteral("room_announce");
    announce[QStringLiteral("name")] = roomName;
    announce[QStringLiteral("ip")] = m_localIp;
    announce[QStringLiteral("port")] = static_cast<int>(m_tcpPort);

    QByteArray data = QJsonDocument(announce).toJson(QJsonDocument::Compact);
    m_udpSocket->writeDatagram(data, QHostAddress::Broadcast, m_discoveryPort);

    if (!m_announceTimer) {
        m_announceTimer = new QTimer(this);
        connect(m_announceTimer, &QTimer::timeout,
                this, &NetworkManager::onAnnounceTimer);
        m_announceTimer->start(2000);
    }
}

void NetworkManager::sendMessage(const QJsonObject &msg)
{
    if (!m_socket || !m_isConnected) {
        qWarning() << "[NetworkManager] 未连接, 无法发送消息";
        return;
    }

    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + '\n';
    m_socket->write(data);
}

void NetworkManager::onNewConnection()
{
    if (m_socket) {
        QTcpSocket *rejected = m_server->nextPendingConnection();
        rejected->disconnectFromHost();
        rejected->deleteLater();
        qWarning() << "[NetworkManager] 拒绝额外连接(仅支持1v1)";
        return;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QTcpSocket::readyRead,
            this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    m_isConnected = true;
    emit connectedChanged();
    emit clientConnected();
    qDebug() << "[NetworkManager] 客户端已连接";
}

void NetworkManager::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    m_readBuffer.append(socket->readAll());

    while (true) {
        int idx = m_readBuffer.indexOf('\n');
        if (idx < 0)
            break;

        QByteArray line = m_readBuffer.left(idx);
        m_readBuffer.remove(0, idx + 1);

        if (line.isEmpty())
            continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "[NetworkManager] JSON 解析错误:" << err.errorString();
            continue;
        }

        if (!doc.isObject()) {
            qWarning() << "[NetworkManager] 收到非对象 JSON";
            continue;
        }

        emit messageReceived(doc.object());
    }
}

void NetworkManager::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket && socket == m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    m_isConnected = false;
    m_readBuffer.clear();
    emit connectedChanged();
    emit disconnected();
    qDebug() << "[NetworkManager] 连接已断开";
}

void NetworkManager::onSocketError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    QString errorMsg = socket ? socket->errorString()
                              : QStringLiteral("未知套接字错误");
    emit errorOccurred(errorMsg);
    qWarning() << "[NetworkManager] 套接字错误:" << errorMsg;
}

void NetworkManager::onUdpReadyRead()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    if (!socket)
        return;

    while (socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        socket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            continue;

        QJsonObject obj = doc.object();
        if (obj[QStringLiteral("type")].toString() != QStringLiteral("room_announce"))
            continue;

        QString roomName = obj[QStringLiteral("name")].toString();
        QString roomIp = obj[QStringLiteral("ip")].toString();

        if (roomIp == m_localIp)
            continue;

        bool found = false;
        for (int i = 0; i < m_discoveredRooms.size(); ++i) {
            QVariantMap room = m_discoveredRooms[i].toMap();
            if (room[QStringLiteral("ip")].toString() == roomIp) {
                room[QStringLiteral("name")] = roomName;
                m_discoveredRooms[i] = room;
                found = true;
                break;
            }
        }

        if (!found) {
            QVariantMap room;
            room[QStringLiteral("name")] = roomName;
            room[QStringLiteral("ip")] = roomIp;
            m_discoveredRooms.append(room);
            qDebug() << "[NetworkManager] 发现房间:" << roomName << "@" << roomIp;
        }

        emit discoveredRoomsChanged();
    }
}

void NetworkManager::onAnnounceTimer()
{
    if (!m_udpSocket || !m_isServer)
        return;

    QJsonObject announce;
    announce[QStringLiteral("type")] = QStringLiteral("room_announce");
    announce[QStringLiteral("name")] = m_roomName;
    announce[QStringLiteral("ip")] = m_localIp;
    announce[QStringLiteral("port")] = static_cast<int>(m_tcpPort);

    QByteArray data = QJsonDocument(announce).toJson(QJsonDocument::Compact);
    m_udpSocket->writeDatagram(data, QHostAddress::Broadcast, m_discoveryPort);
}
