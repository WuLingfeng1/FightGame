// 房间生命周期管理器实现: 管理联机房间从创建到选人的状态流转
#include "roommanager.h"
#include "networkmanager.h"

#include <QDebug>

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
{
}

void RoomManager::setNetworkManager(NetworkManager *nm)
{
    if (m_nm == nm)
        return;

    if (m_nm) {
        disconnect(m_nm, &NetworkManager::clientConnected, this, &RoomManager::onClientConnected);
        disconnect(m_nm, &NetworkManager::connectedToHost, this, &RoomManager::onConnectedToHost);
        disconnect(m_nm, &NetworkManager::messageReceived, this, &RoomManager::onMessageReceived);
        disconnect(m_nm, &NetworkManager::disconnected, this, &RoomManager::onDisconnected);
        disconnect(m_nm, &NetworkManager::errorOccurred, this, &RoomManager::onErrorOccurred);
    }

    m_nm = nm;

    if (m_nm) {
        connect(m_nm, &NetworkManager::clientConnected, this, &RoomManager::onClientConnected);
        connect(m_nm, &NetworkManager::connectedToHost, this, &RoomManager::onConnectedToHost);
        connect(m_nm, &NetworkManager::messageReceived, this, &RoomManager::onMessageReceived);
        connect(m_nm, &NetworkManager::disconnected, this, &RoomManager::onDisconnected);
        connect(m_nm, &NetworkManager::errorOccurred, this, &RoomManager::onErrorOccurred);
    }

    emit networkManagerChanged();
}

void RoomManager::createRoom(const QString &roomName, quint16 port)
{
    if (!m_nm) {
        qWarning() << "[RoomManager] NetworkManager not set";
        return;
    }

    m_role = QStringLiteral("host");
    emit roleChanged();

    m_nm->startServer(port);
    m_nm->announceRoom(roomName);
    m_hostReady = true;

    qDebug() << "[RoomManager] 房间已创建:" << roomName << "端口:" << port;
}

void RoomManager::joinRoom(const QString &ip, quint16 port)
{
    if (!m_nm) {
        qWarning() << "[RoomManager] NetworkManager not set";
        return;
    }

    m_role = QStringLiteral("client");
    emit roleChanged();

    m_nm->connectToHost(ip, port);
    m_clientReady = true;

    qDebug() << "[RoomManager] 正在加入房间:" << ip << "端口:" << port;
}

void RoomManager::leaveRoom()
{
    if (!m_nm)
        return;

    m_nm->stopServer();
    m_nm->stopDiscovery();
    m_nm->disconnectFromHost();

    m_role.clear();
    emit roleChanged();

    m_hostReady = false;
    m_clientReady = false;

    qDebug() << "[RoomManager] 已离开房间";
}

void RoomManager::onClientConnected()
{
    qDebug() << "[RoomManager] 对手已连接, 跳转到选人界面";

    if (m_nm) {
        QJsonObject msg;
        msg[QStringLiteral("type")] = QStringLiteral("start_game");
        m_nm->sendMessage(msg);
    }

    emit goToSelectScreen(true);
}

void RoomManager::onConnectedToHost()
{
    qDebug() << "[RoomManager] 已连接到主机, 等待主机开始游戏";
}

void RoomManager::onMessageReceived(const QJsonObject &msg)
{
    if (msg[QStringLiteral("type")].toString() == QStringLiteral("start_game")) {
        qDebug() << "[RoomManager] 收到开始游戏消息, 跳转到选人界面";
        emit goToSelectScreen(false);
    }
}

void RoomManager::onDisconnected()
{
    qDebug() << "[RoomManager] 连接断开";

    m_hostReady = false;
    m_clientReady = false;
}

void RoomManager::onErrorOccurred(const QString &error)
{
    qWarning() << "[RoomManager] 错误:" << error;
}
